// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington
#include <elf.h>
#include <DrvAPI.hpp>
#include <unordered_map>
#include <inttypes.h>
#include "common.hpp"

using namespace DrvAPI;

enum {
    ARG_THIS_EXE = 0,
    ARG_PH_EXE = 1,
};

struct Place {
    int64_t pxn;
    int64_t pod;
    int64_t core_y;
    int64_t core_x;
    Place(): pxn(0), pod(0), core_y(0), core_x(0) {}
    Place(int64_t pxn, int64_t pod, int64_t core_y, int64_t core_x):
        pxn(pxn), pod(pod), core_y(core_y), core_x(core_x) {}
};

class PANDOHammerExe {
public:
    PANDOHammerExe() : fp_(nullptr), ehdr_(nullptr) {}
    PANDOHammerExe(const char *fname): fp_(nullptr), ehdr_(nullptr) {
        fp_ = fopen(fname, "rb");
        if (fp_ == nullptr) {
            throw std::runtime_error("Could not open file");
        }

        struct stat st;
        if (stat(fname, &st) != 0) {
            throw std::runtime_error("Could not stat file");
        }

        ehdr_ = (Elf64_Ehdr*)mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fileno(fp_), 0);
        if (ehdr_ == MAP_FAILED) {
            throw std::runtime_error("Could not mmap file");
        }
        symtab_init();
        bump_allocator_init();
    }

    ~PANDOHammerExe() {
        if (ehdr_ != nullptr) {
            munmap(ehdr_, sizeof(Elf64_Ehdr));
        }
        if (fp_ != nullptr) {
            fclose(fp_);
        }
        symtab_.clear();
    }

    PANDOHammerExe(const PANDOHammerExe&) = delete;
    PANDOHammerExe(PANDOHammerExe&&) = delete;
    PANDOHammerExe& operator=(const PANDOHammerExe&) = delete;
    PANDOHammerExe& operator=(PANDOHammerExe&&) = delete;    
    
    static std::shared_ptr<PANDOHammerExe> Open(const char *fname) {
        return std::make_shared<PANDOHammerExe>(fname);
    }

    DrvAPIVAddress symbol(const std::string& symname) const {
        if (symtab_.count(symname) == 0) {
            throw std::runtime_error("Symbol not found");
        }
        return DrvAPIVAddress{symtab_.at(symname)};
    }

    
    
    template <typename T>
    DrvAPI::DrvAPIPointer<T> symbol(const std::string& symname, const Place &place) const {
        DrvAPIVAddress addr = symbol(symname);
        addr.global() = true;
        if (addr.is_l1()) {
            addr.pxn() = place.pxn;
            addr.pod() = place.pod;
            addr.core_y() = place.core_y;
            addr.core_x() = place.core_x;
        } else if (addr.is_l2()) {
            addr.pxn() = place.pxn;
            addr.pod() = place.pod;         
        }
        return DrvAPI::DrvAPIPointer<T>(addr.encode());
    }

    DrvAPIPointer<void> allocate(size_t size) {
        // minimum allocation size is 16B
        size = std::max(size, (size_t)16);
        // align to 16B
        size = (size + 15) & ~15;
        // allocate
        auto ret = bump_allocator_;
        bump_allocator_ += size;
        return ret;
    }
private:
    Elf64_Shdr *sections_begin() const {
        return (Elf64_Shdr*)((char*)ehdr_ + ehdr_->e_shoff);
    }

    Elf64_Shdr *sections_end() const {
        return (Elf64_Shdr*)((char*)sections_begin() + ehdr_->e_shnum * ehdr_->e_shentsize);
    }    

    void symtab_init() {
        // scan for symbol tables
        for (Elf64_Shdr *shdr = sections_begin(); shdr != sections_end(); shdr++) {
            if (shdr->sh_type == SHT_SYMTAB)
                symtab_add(shdr);
        }
    }

    Elf64_Sym *symtab_begin(Elf64_Shdr *symtab_shdr) const {
        return (Elf64_Sym*)((char*)ehdr_ + symtab_shdr->sh_offset);
    }
    
    Elf64_Sym *symtab_end(Elf64_Shdr *symtab_shdr) const {
        return (Elf64_Sym*)((char*)symtab_begin(symtab_shdr) + symtab_shdr->sh_size);
    }

    const char *sym_name(Elf64_Shdr *strtab_shdr, Elf64_Sym *sym) const {
        return (const char*)ehdr_ + strtab_shdr->sh_offset + sym->st_name;
    }
    
    void symtab_add(Elf64_Shdr *symtab_shdr) {
        // get the string table
        Elf64_Shdr *strtab = sections_begin() + symtab_shdr->sh_link;
        for (Elf64_Sym *sym = symtab_begin(symtab_shdr); sym != symtab_end(symtab_shdr); sym++) {
            if (sym->st_name != 0) {
                symtab_[sym_name(strtab, sym)] = sym->st_value;
            }
        }
    }

    void bump_allocator_init() {
        // add 1MB to the end of data section
        bump_allocator_ = symbol<char>("end", Place{0,0,0,0}) + 1024*1024;
        // align to 4KB
        bump_allocator_ = (bump_allocator_ + 4095) & ~4095;
    }

    FILE *fp_;
    Elf64_Ehdr *ehdr_;
    std::unordered_map<std::string, DrvAPIAddress> symtab_;
    DrvAPI::DrvAPIPointer<char> bump_allocator_;
};

int CommandProcessor(int argc, char *argv[])
{
    auto ph_exe = PANDOHammerExe::Open(argv[ARG_PH_EXE]);
    Place place{0,0,0,0};

    auto cp_ready = ph_exe->symbol<int64_t>("cp_ready", place);
    auto ph_ready = ph_exe->symbol<int64_t>("ph_ready", place);

    // wait for PH to be ready
    // it's important to wait for the PH first
    // since it needs to complete loading
    int64_t num_ready = 0;
    printf("CP: waiting for PH threads to be ready: Cores: %d, Threads/Core: %d\n"
           ,numPodCores(), THREADS_PER_CORE);
    
    while ((num_ready = *ph_ready) < THREADS_PER_CORE*numPodCores()) {
        DrvAPI::wait(100);
    }    
    
    auto frontiers = ph_exe->symbol<frontier_data>("frontier", place);

    frontier_ref curr = &frontiers[0];
    frontier_ref next = &frontiers[1];
    frontier_ref resv = &frontiers[2];

    for (int i = 0; i < 3; i++) {
        frontier_ref f = &frontiers[i];
        f.size() = 0;
        f.vertices() = ph_exe->allocate(sizeof(vertex_t)*1024);
        f.is_dense() = false;
    }
    // FENCE here
    *cp_ready = 1; // signal to ph core's that we are ready
    return 0;
}

declare_drv_api_main(CommandProcessor);
