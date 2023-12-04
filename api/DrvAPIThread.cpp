// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#include "DrvAPIThread.hpp"
#include "DrvAPIMain.hpp"
#include "DrvAPIGlobal.hpp"
#include "DrvAPIAllocator.hpp"
#include "DrvAPIAddressMap.hpp"
#include "DrvAPIAddressToNative.hpp"
#include <iostream>

using namespace DrvAPI;


/**
 * class that allocates a stack buffer for the coroutine
 * this allocates memory from the modeled memHierarcy memories
 */
struct modeled_memory_stack_allocator {
public:
    modeled_memory_stack_allocator() = default;
    modeled_memory_stack_allocator
    (uint64_t pxn, uint64_t pod, uint64_t core, uint64_t thread, uint64_t threads)
        : pxn_(pxn)
        , pod_(pod)
        , core_(core)
        , thread_(thread)
        , threads_(threads) {
    }

    boost::context::stack_context allocate() {
        boost::context::stack_context sctx;
        // 1. determine end of l1sp statics
        auto &l1sp_statics = DrvAPI::DrvAPISection::GetSection(DrvAPIMemoryL1SP);

        DrvAPI::DrvAPIAddress l1sp_static_base =
            l1sp_statics.getBase(pxn_, pod_, core_);

        DrvAPI::DrvAPIAddress l1sp_static_end
            = l1sp_static_base
            + l1sp_statics.getSize();

        l1sp_static_end
            = DrvAPI::toGlobalAddress
            (l1sp_static_end, pxn_, pxn_,coreYFromId(core_), coreXFromId(core_));


        // 2. determine the total available stack size and divide amongst theads
        // this is just the rest of l1sp
        uint64_t stack_bytes = coreL1SPSize() - l1sp_statics.getSize();
        uint64_t stack_words = stack_bytes / sizeof(uint64_t);
        uint64_t thread_stack_words = stack_words / threads_;
        uint64_t thread_stack_bytes = thread_stack_words * sizeof(uint64_t);

        // 3. calculate the top of the stack for this thread
        DrvAPI::DrvAPIAddress stack_top
            = l1sp_static_end
            + (thread_+1)*thread_stack_bytes
            - sizeof(uint64_t);

        // 4. get the native stack pointer using toNative()
        size_t _;
        DrvAPIAddressToNative(stack_top, &sctx.sp, &_);
        sctx.size = thread_stack_bytes;
        return sctx;
    }
    void deallocate(boost::context::stack_context &sctx) {
        sctx.sp = nullptr;
        sctx.size = 0;
    }

    uint64_t pxn_  = 0;
    uint64_t pod_  = 0;
    uint64_t core_ = 0;
    uint64_t thread_ = 0;
    uint64_t threads_ = 0;
};

DrvAPIThread::DrvAPIThread()
    : thread_context_(nullptr)
    , state_(new DrvAPIThreadIdle)
    , main_(nullptr)
    , argc_(0)
    , argv_(nullptr) {
}

void DrvAPIThread::start() {
    auto coro_function = [this](coro_t::push_type &sink) {
        this->main_context_ = &sink;
        this->yield();
        while (true) {
            if (this->main_) {
                this->main_(argc_, argv_);
                this->main_ = nullptr;
                this->state_ = std::make_shared<DrvAPITerminate>();
            }
            this->yield();
        }
    };
    if (stack_in_modeled_memory_) {
        modeled_memory_stack_allocator allocator
            (pxn_id_, pod_id_, core_id_, id_, core_threads_);
        thread_context_ = std::make_unique<coro_t::pull_type>(allocator, coro_function);
    } else {
        thread_context_ = std::make_unique<coro_t::pull_type>(coro_function);
    }
}

/* should only be called from the thread context */
void DrvAPIThread::yield(DrvAPIThreadState *state) {
    state_ = std::make_shared<DrvAPIThreadState>(*state);
    yield();
}

/* should only be called from the thread context */
void DrvAPIThread::yield() {
    (*main_context_)();
}

/* should only be called from the main context */
void DrvAPIThread::resume() {
    (*thread_context_)();
}

thread_local DrvAPIThread *DrvAPIThread::g_current_thread = nullptr;


DrvAPIThread *DrvAPIGetCurrentContext() {
    return DrvAPIThread::g_current_thread;
}

void DrvAPISetCurrentContext(DrvAPI::DrvAPIThread *thread) {
    DrvAPIThread::g_current_thread = thread;
}
