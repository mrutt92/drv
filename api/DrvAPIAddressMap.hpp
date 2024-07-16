// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington
#ifndef DRV_API_ADDRESS_MAP_H
#define DRV_API_ADDRESS_MAP_H
#include <utility>
#include <cstdint>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <sstream>
#include <DrvAPIAddress.hpp>
#include <DrvAPISysConfig.hpp>
#include <DrvAPICoreXY.hpp>
namespace DrvAPI
{

namespace bits {

static inline size_t bitlength(int v) {
    if (v < 0) {
        v = -v;
    }
    size_t l = 0;
    while (v) {
        v >>= 1;
        l++;
    }
    return l;
}

template <typename UINT, unsigned HI, unsigned LO, unsigned TAG=0>
struct bitrange_handle {
public:
    typedef UINT uint_type;
    static constexpr unsigned HI_BIT = HI;
    static constexpr unsigned LO_BIT = LO;

    bitrange_handle(UINT &i) : i(i) {}
    ~bitrange_handle() = default;
    bitrange_handle(bitrange_handle &&o) = default;
    bitrange_handle &operator=(bitrange_handle &&o) = default;
    bitrange_handle(const bitrange_handle &o) = default;
    bitrange_handle &operator=(const bitrange_handle &o) = default;

    static constexpr UINT lo() {
        return LO;
    }
    static constexpr UINT hi() {
        return HI;
    }
    static constexpr UINT bits() {
        return HI - LO + 1;
    }

    static constexpr UINT mask()
    {
        return ((1ull << (HI - LO + 1)) - 1) << LO;
    }

    static UINT getbits(UINT in)
    {
        return (in & mask()) >> LO;
    }

    static void setbits(UINT &in, UINT val)
    {
        in &= ~mask();
        in |=  mask() & (val << LO);
    }

    operator UINT() const { return getbits(i); }

    bitrange_handle &operator=(UINT val) {
        setbits(i, val);
        return *this;
    }

    UINT &i;
};


template <typename UINT>
struct dynamic_bitfield {
public:
    typedef UINT uint_type;

    dynamic_bitfield() : lo_(0), hi_(0) {}

    dynamic_bitfield(UINT hi, UINT lo) : lo_(lo), hi_(hi) {}

    UINT lo() const {
        return lo_;
    }
    UINT hi() const {
        return hi_;
    }
    UINT bits() const {
        return hi() - lo() + 1;
    }
    UINT mask() const {
        return ((1ull << (hi() - lo() + 1)) - 1) << lo();
    }
    UINT getbits(UINT i) const {
        return (i & mask()) >> lo();
    }
    void setbits(UINT &i, UINT val) const{
        i &= ~mask();
        i |=  mask() & (val << lo());
    }

    UINT operator()(UINT i) const {
        return getbits(i);
    }

    UINT lo_;
    UINT hi_;
};

template <typename UINT>
struct dynamic_bitrange_handle {
public:
    typedef UINT uint_type;

    dynamic_bitrange_handle(UINT &i) : i(i) {}
    dynamic_bitrange_handle(UINT &i, const dynamic_bitfield<UINT> bits) : i(i), bits_(bits) {}
    dynamic_bitrange_handle(UINT &i, UINT hi, UINT lo) : i(i), bits_(hi, lo) {}
    ~dynamic_bitrange_handle() = default;
    dynamic_bitrange_handle(dynamic_bitrange_handle &&o) = default;
    dynamic_bitrange_handle &operator=(dynamic_bitrange_handle &&o) = default;
    dynamic_bitrange_handle(const dynamic_bitrange_handle &o) = default;
    dynamic_bitrange_handle &operator=(const dynamic_bitrange_handle &o) = default;

    UINT lo() const {
        return bits_.lo();
    }
    UINT hi() const {
        return bits_.hi();
    }
    UINT bits() const {
        return bits_.bits();
    }

    UINT mask() const {
        return bits_.mask();
    }

    UINT getbits() {
        return bits_.getbits(i);
    }

    void setbits(UINT val) {
        bits_.setbits(i, val);
    }

    operator UINT() const {
        return getbits();
    }

    dynamic_bitrange_handle &operator=(UINT val) {
        setbits(val);
        return *this;
    }

    UINT &i;
    dynamic_bitfield<UINT> bits_;
};

}

struct DrvAPIAbsoluteAddressInfo; //!< forward declaration
struct DrvAPIRelativeAddressInfo; //!< forward declaration
struct DrvAPIAddressInfo; //!< forward declaration


/**
 * decoded information about an address
 */
struct DrvAPIAddressInfo {
public:
    DrvAPIAddressInfo() = default;
    DrvAPIAddressInfo(const DrvAPIAddressInfo &o) = default;
    DrvAPIAddressInfo &operator=(const DrvAPIAddressInfo &o) = default;
    DrvAPIAddressInfo(DrvAPIAddressInfo &&o) = default;
    DrvAPIAddressInfo &operator=(DrvAPIAddressInfo &&o) = default;
    ~DrvAPIAddressInfo() = default;

    /**
     * @return true if this address is in l1 scratchpad
     */
    bool is_l1sp() const {
        return memory_type_ == DrvAPIMemoryL1SP;
    }

    /**
     * set this address to be in l1 scratchpad
     */
    DrvAPIAddressInfo &set_l1sp() {
        memory_type_ = DrvAPIMemoryL1SP;
        return *this;
    }

    /**
     * @return true if this address is in l2 scratchpad
     */
    bool is_l2sp() const {
        return memory_type_ == DrvAPIMemoryL2SP;
    }

    /**
     * set this address to be in l2 scratchpad
     */
    DrvAPIAddressInfo &set_l2sp() {
        memory_type_ = DrvAPIMemoryL2SP;
        return *this;
    }

    /**
     * set this address to be in dram
     */
    bool is_dram() const {
        return memory_type_ == DrvAPIMemoryDRAM;
    }

    /**
     * set this address to be in dram
     */
    DrvAPIAddressInfo& set_dram() {
        memory_type_ = DrvAPIMemoryDRAM;
        return *this;
    }

    /**
     * @return true if this address is in core control
     */
    bool is_core_ctrl() const {
        return !is_l1sp() && !is_l2sp() && !is_dram();
    }

    /**
     * set this address to be in core control
     */
    DrvAPIAddressInfo& set_core_ctrl() {
        memory_type_ = DrvAPIMemoryNTypes;
        return *this;
    }

    /**
     * @return true if this address is absolute
     */
    bool is_absolute() const {
        return is_absolute_;
    }

    /**
     * set this address to be absolute
     */
    DrvAPIAddressInfo& set_absolute(bool v) {
        is_absolute_ = v;
        return *this;
    }

    /**
     * @return true if this address is relative
     */
    bool is_relative() const {
        return !is_absolute_;
    }

    /**
     * set this address to be relative
     */
    DrvAPIAddressInfo& set_relative(bool v) {
        is_absolute_ = !v;
        return *this;
    }

    /**
     * @return the offset of this address
     */
    DrvAPIAddress offset() const { return offset_; }

    /**
     * set the offset of this address
     */
    DrvAPIAddressInfo& set_offset(DrvAPIAddress v) {
        offset_ = v;
        return *this;
    }

    /**
     * @return the pxn of this address
     */
    int64_t pxn() const { return pxn_; }

    /**
     * set the pxn of this address
     */
    DrvAPIAddressInfo& set_pxn(int64_t v) {
        pxn_ = v;
        return *this;
    }

    /**
     * @return the pod of this address
     */
    int64_t pod() const { return pod_; }

    /**
     * set the pod of this address
     */
    DrvAPIAddressInfo& set_pod(int64_t v) {
        pod_ = v;
        return *this;
    }

    /**
     * @return the core of this address
     */
    int64_t core() const { return core_; }

    /**
     * set the core of this address
     */
    DrvAPIAddressInfo& set_core(int64_t v) {
        core_ = v;
        return *this;
    }

    /**
     * to string
     */
    std::string to_string() const {
        std::stringstream ss;
        if (is_absolute()) {
            ss << "{ABSOLUTE,";
            if (is_dram()) {
                ss << "DRAM,";
                ss << "PXN="<<pxn()<<",";
                ss << "0x"<<std::hex<<offset();
            } else if (is_l2sp()) {
                ss << "L2SP,";
                ss << "PXN="<<pxn()<<",";
                ss << "POD="<<pod()<<",";
                ss << "0x"<<std::hex<<offset();
            } else if (is_l1sp()) {
                ss << "L1SP,";
                ss << "PXN="<<pxn()<<",";
                ss << "POD="<<pod()<<",";
                ss << "CORE="<<core()<<",";
                ss << "0x"<<std::hex<<offset();
            } else {
                ss << "CTRL,";
                ss << "PXN="<<pxn()<<",";
                ss << "POD="<<pod()<<",";
                ss << "CORE="<<core()<<",";
                ss << "0x"<<std::hex<<offset();
            }
            ss << "}";
        } else {
            ss << "{RELATIVE,";
            if (is_dram()) {
                ss << "DRAM,";
                ss << "0x"<<std::hex<<offset();
            } else if (is_l2sp()) {
                ss << "L2SP,";
                ss << "0x"<<std::hex<<offset();
            } else if (is_l1sp()) {
                ss << "L1SP,";
                ss << "0x"<<std::hex<<offset();
            } else {
                ss << "CTRL,";
                ss << "0x"<<std::hex<<offset();
            }
            ss << "}";
        }

        return ss.str();
    }

private:
    DrvAPIMemoryType memory_type_ = DrvAPIMemoryNTypes;
    bool is_absolute_ = false;
    DrvAPIAddress offset_ = 0;
    int64_t pxn_ = 0;
    int64_t pod_ = 0;
    int64_t core_ = 0;
};

/**
 * a decoded address
 */
struct DrvAPIAddressDecoder {
public:
    typedef bits::dynamic_bitfield<DrvAPIAddress> bitfield;
    DrvAPIAddressDecoder() = default;
    DrvAPIAddressDecoder(int64_t my_pxn,
                         int64_t my_pod,
                         int64_t my_core,
                         const DrvAPISysConfig& sys = *DrvAPISysConfig::Get())
        : my_pxn_(my_pxn),
          my_pod_(my_pod),
          my_core_(my_core) {
        int pxn_bits = bits::bitlength(sys.numPXN()-1);
        int pod_bits = bits::bitlength(sys.numPXNPods()-1);
        int core_bits = bits::bitlength(sys.numPodCores()-1);
        absolute_pxn_ = bitfield(absolute_is_l2sp_.lo()-1,
                                 absolute_is_l2sp_.lo()-pxn_bits);
        absolute_pod_ = bitfield(absolute_pxn_.lo()-1,
                                absolute_pxn_.lo()-pod_bits);
        absolute_core_ = bitfield(absolute_pod_.lo()-1,
                                  absolute_pod_.lo()-core_bits);
        absolute_dram_offset_ = bitfield(absolute_pxn_.lo()-1, 0);
        absolute_l2sp_offset_ = bitfield(absolute_pod_.lo()-1, 0);
        absolute_l1sp_offset_ = bitfield(absolute_is_ctrl_.lo()-1, 0);
    }

    /**
     * decode an address
     */
    DrvAPIAddressInfo decode(DrvAPIAddress addr) const {
        DrvAPIAddressInfo info;
        if (is_absolute_(addr)) {
            info.set_absolute(true);
            decode_absolute(addr, info);
        } else {
            info.set_absolute(false);
            decode_relative(addr, info);
        }
        return info;
    }

    /**
     * encode an address
     */
    DrvAPIAddress encode(const DrvAPIAddressInfo &info) const {
        if (info.is_absolute()) {
            return encode_absolute(info);
        } else {
            return encode_relative(info);
        }
    }

    /**
     * make an absolute address
     */
    DrvAPIAddress to_absolute(DrvAPIAddress addr) const {
        DrvAPIAddressInfo info = decode(addr);
        if (info.is_absolute()) {
            return addr;
        } else {
            info.set_absolute(true);
            return encode(info);
        }
    }

private:
    void decode_absolute(DrvAPIAddress addr, DrvAPIAddressInfo &info) const {
        if (absolute_is_dram_(addr)) {
            info.set_pxn(absolute_pxn_(addr))
                .set_offset(absolute_dram_offset_(addr))
                .set_dram();
        } else if (absolute_is_l2sp_(addr)) {
            info.set_pxn(absolute_pxn_(addr))
                .set_pod(absolute_pod_(addr))
                .set_offset(absolute_l2sp_offset_(addr))
                .set_l2sp();
        } else if (absolute_is_ctrl_(addr)) {
            info.set_pxn(absolute_pxn_(addr))
                .set_pod(absolute_pod_(addr))
                .set_core(absolute_core_(addr))
                .set_core_ctrl();
        } else {
            info.set_pxn(absolute_pxn_(addr))
                .set_pod(absolute_pod_(addr))
                .set_core(absolute_core_(addr))
                .set_offset(absolute_l1sp_offset_(addr))
                .set_l1sp();
        }
    }

    void decode_relative(DrvAPIAddress addr, DrvAPIAddressInfo &info) const {
        if (relative_is_dram_(addr)) {
            info.set_dram()
                .set_pxn(my_pxn_)
                .set_offset(relative_dram_offset_(addr));
        } else if (relative_is_l2sp_(addr)) {
            info.set_l2sp()
                .set_pxn(my_pxn_)
                .set_pod(my_pod_)
                .set_offset(relative_l2sp_offset_(addr));
        } else {
            info.set_l1sp()
                .set_pxn(my_pxn_)
                .set_pod(my_pod_)
                .set_core(my_core_)
                .set_offset(relative_l1sp_offset_(addr));
        }
    }

    DrvAPIAddress encode_absolute(const DrvAPIAddressInfo &info) const {
        DrvAPIAddress addr = 0;
        is_absolute_.setbits(addr, 1);
        if (info.is_dram()) {
            absolute_is_dram_.setbits(addr, 1);
            absolute_pxn_.setbits(addr, info.pxn());
            absolute_dram_offset_.setbits(addr, info.offset());
        } else if (info.is_l2sp()) {
            absolute_is_l2sp_.setbits(addr, 1);
            absolute_pxn_.setbits(addr, info.pxn());
            absolute_pod_.setbits(addr, info.pod());
            absolute_l2sp_offset_.setbits(addr, info.offset());
        } else if (info.is_l1sp()) {
            absolute_pxn_.setbits(addr, info.pxn());
            absolute_pod_.setbits(addr, info.pod());
            absolute_core_.setbits(addr, info.core());
            absolute_l1sp_offset_.setbits(addr, info.offset());
        } else {
            absolute_is_ctrl_.setbits(addr, 1);
            absolute_pxn_.setbits(addr, info.pxn());
            absolute_pod_.setbits(addr, info.pod());
            absolute_core_.setbits(addr, info.core());
        }
        return addr;
    }

    DrvAPIAddress encode_relative(const DrvAPIAddressInfo &info) const {
        DrvAPIAddress addr = 0;
        if (info.is_dram()) {
            relative_is_dram_.setbits(addr, 1);
            relative_dram_offset_.setbits(addr, info.offset());
        } else if (info.is_l2sp()) {
            relative_is_l2sp_.setbits(addr, 1);
            relative_l2sp_offset_.setbits(addr, info.offset());
        } else {
            relative_l1sp_offset_.setbits(addr, info.offset());
        }
        return addr;
    }

public:
    bitfield is_absolute_ = bitfield(63,63);
    // absolute decoding
    bitfield absolute_is_dram_ = bitfield(62,62);
    bitfield absolute_is_l2sp_ = bitfield(61,61);
    bitfield absolute_is_ctrl_ = bitfield(30,30);
    bitfield absolute_pxn_;
    bitfield absolute_pod_;
    bitfield absolute_core_;
    bitfield absolute_l1sp_offset_;
    bitfield absolute_l2sp_offset_;
    bitfield absolute_dram_offset_;
    // relative decoding
    bitfield relative_is_dram_     = bitfield(31,31);
    bitfield relative_is_l2sp_     = bitfield(30,30);
    bitfield relative_l1sp_offset_ = bitfield(29,00);
    bitfield relative_l2sp_offset_ = bitfield(29,00);
    bitfield relative_dram_offset_ = bitfield(30,00);
    int64_t my_pxn_ = 0;
    int64_t my_pod_ = 0;
    int64_t my_core_ = 0;
};


struct DrvAPIPAddress; //!< forward declaration
struct DrvAPIVAddress; //!< forward declaration


/**
 * This is a decoded software address
 */
struct DrvAPIVAddress
{
    static constexpr unsigned TAG = 0;
    typedef bits::bitrange_handle<DrvAPIAddress, 63, 63> CtrlRegisterHandle; //!< handle for the CtrlRegister bit
    typedef bits::bitrange_handle<DrvAPIAddress, 47, 47> NotScratchpadHandle; //!< handle for the NotScratchpad bit
    typedef bits::bitrange_handle<DrvAPIAddress, 46, 33> PXNHandle; //!< handle for the PXN bits
    typedef bits::bitrange_handle<DrvAPIAddress, 32, 32> GlobalHandle; //!< handle for the global bit
    typedef bits::bitrange_handle<DrvAPIAddress, 31, 26> PodHandle; //!< handle for the Pod bits
    typedef bits::bitrange_handle<DrvAPIAddress, 25, 25> L2NotL1Handle; //!< handle for the L2NotL1 bit
    typedef bits::bitrange_handle<DrvAPIAddress, 22, 20> CoreYHandle; //!< handle for the CoreY bits
    typedef bits::bitrange_handle<DrvAPIAddress, 19, 17> CoreXHandle; //!< handle for the CoreX bits
    typedef bits::bitrange_handle<DrvAPIAddress, 16,  0, TAG>  L1OffsetHandle; //!< handle for the L1Offset bits
    typedef bits::bitrange_handle<DrvAPIAddress, 24,  0, TAG>  L2OffsetHandle; //!< handle for the L2Offset bits
    typedef bits::bitrange_handle<DrvAPIAddress, 57, 48> DRAMOffsetHi10Handle; //!< handle for the DRAMOffsetHi bits
    typedef bits::bitrange_handle<DrvAPIAddress, 32,  0> DRAMOffsetLo33Handle; //!< handle for the DRAMOffsetLo bits

    static DrvAPIPAddress to_physical(DrvAPIAddress addr
                                      ,uint32_t this_pxn
                                      ,uint32_t this_pod
                                      ,uint32_t this_core_y
                                      ,uint32_t this_core_x);

    static  DrvAPIVAddress MyL2Base() {
        DrvAPIVAddress addr = 0;
        addr.ctrl_register() = false;
        addr.l2_not_l1() = true;
        addr.global() = false;
        addr.not_scratchpad() = false;
        return addr;
    }

    static DrvAPIVAddress MyL1Base() {
        DrvAPIVAddress addr = 0;
        addr.ctrl_register() = false;
        addr.l2_not_l1() = false;
        addr.global() = false;
        addr.not_scratchpad() = false;
        return addr;
    }

    static DrvAPIVAddress MainMemBase(uint32_t pxn) {
        DrvAPIVAddress addr = 0;
        addr.ctrl_register() = false;
        addr.not_scratchpad() = true;
        addr.pxn() = pxn;
        addr.global() = false;
        addr.dram_offset_hi10() = 0;
        addr.dram_offset_lo33() = 0;
        return addr;
    }

    static DrvAPIVAddress
    CoreCtrlBase (uint32_t pxn, uint32_t pod, uint32_t core_y, uint32_t core_x) {
        DrvAPIVAddress addr = 0;
        addr.ctrl_register() = true;
        addr.not_scratchpad() = true;
        addr.pxn() = pxn;
        addr.global() = false;
        addr.pod() = pod;
        addr.core_y() = core_y;
        addr.core_x() = core_x;
        return addr;
    }
    DrvAPIVAddress() : addr(0) {}
    DrvAPIVAddress(const DrvAPIVAddress &o) = default;
    DrvAPIVAddress &operator=(const DrvAPIVAddress &o) = default;
    DrvAPIVAddress(DrvAPIVAddress &&o) = default;
    DrvAPIVAddress &operator=(DrvAPIVAddress &&o) = default;
    ~DrvAPIVAddress() = default;

    DrvAPIVAddress(DrvAPIAddress addr) : addr(addr) {}

    CtrlRegisterHandle ctrl_register() {
        return CtrlRegisterHandle(addr);
    }
    
    PXNHandle pxn() {
        return PXNHandle(addr);
    }

    GlobalHandle global() {
        return GlobalHandle(addr);
    }

    PodHandle pod() {
        return PodHandle(addr);
    }

    L2NotL1Handle l2_not_l1() {
        return L2NotL1Handle(addr);
    }

    CoreYHandle core_y() {
        return CoreYHandle(addr);
    }

    CoreXHandle core_x() {
        return CoreXHandle(addr);
    }

    L1OffsetHandle l1_offset() {
        return L1OffsetHandle(addr);
    }

    L1OffsetHandle ctrl_offset() {
        return L1OffsetHandle(addr);
    }

    L2OffsetHandle l2_offset() {
        return L2OffsetHandle(addr);
    }

    NotScratchpadHandle not_scratchpad() {
        return NotScratchpadHandle(addr);
    }

    DRAMOffsetHi10Handle dram_offset_hi10() {
        return DRAMOffsetHi10Handle(addr);
    }

    DRAMOffsetLo33Handle dram_offset_lo33() {
        return DRAMOffsetLo33Handle(addr);
    }

    DrvAPIAddress dram_offset() {
        return (dram_offset_hi10() << 33) | dram_offset_lo33();
    }

    bool is_ctrl_register() {
        return ctrl_register();
    }
    
    bool is_dram() {
        return !is_ctrl_register() && not_scratchpad();
    }

    bool is_l2() {
        return !is_ctrl_register() && !is_dram() && l2_not_l1();
    }

    bool is_l1() {
        return !is_ctrl_register() && !is_dram() && !l2_not_l1();
    }
    
    DrvAPIPAddress to_physical(uint32_t this_pxn
                               ,uint32_t this_pod
                               ,uint32_t core_y
                               ,uint32_t core_x);

    DrvAPIAddress encode() const { return addr; }

    std::string to_string();

    DrvAPIAddress addr;
};

/**
 * make a global address from a local address
 */
static inline DrvAPIAddress toGlobalAddress(DrvAPIAddress local, uint32_t pxn, uint32_t pod, uint32_t core_y, uint32_t core_x) {
    DrvAPIVAddress vaddr(local);
    if (vaddr.not_scratchpad()){
        return local;
    } else if (vaddr.global()) {
        return local;
    } else if (vaddr.is_l2()) {
        vaddr.pxn() = pxn;
        vaddr.pod() = pod;
        vaddr.global() = true;
        return vaddr.encode();
    } else if (vaddr.is_l1()) {
        vaddr.pxn() = pxn;
        vaddr.pod() = pod;
        vaddr.global() = true;
        vaddr.core_y() = core_y;
        vaddr.core_x() = core_x;
        return vaddr.encode();
    }
    throw std::runtime_error("toGlobalAddress: Unknown address type");
}

static inline DrvAPIAddress toGlobalAddress(DrvAPIAddress local, uint32_t pxn, uint32_t pod, uint32_t core) {
    return toGlobalAddress(local, pxn, pod, coreYFromId(core), coreXFromId(core));
}

/**
 * This is a decoded physical address
 */
struct DrvAPIPAddress
{
    static constexpr unsigned TAG = 1;
    typedef bits::bitrange_handle<DrvAPIAddress, 63, 58> TypeHandle; //!< handle for the Type bits
    static constexpr uint32_t TYPE_L1SP = 0b000000; //!< L1 scratchpad
    static constexpr uint32_t TYPE_L2SP = 0b000001; //!< L2 scratchpad
    static constexpr uint32_t TYPE_DRAM = 0b000100; //!< Main memory
    static constexpr uint32_t TYPE_CTRL = 0b001000; //!< Control Register
    typedef bits::bitrange_handle<DrvAPIAddress, 57, 44> PXNHandle; //!< handle for the PXN bits
    typedef bits::bitrange_handle<DrvAPIAddress, 39, 34> PodHandle; //!< handle for the Pod bits
    typedef bits::bitrange_handle<DrvAPIAddress, 30, 28> CoreYHandle; //!< handle for the CoreY bits
    typedef bits::bitrange_handle<DrvAPIAddress, 24, 22> CoreXHandle; //!< handle for the CoreX bits
    typedef bits::bitrange_handle<DrvAPIAddress, 16,  0, TAG> L1OffsetHandle; //!< handle for the L1Offset bits
    typedef bits::bitrange_handle<DrvAPIAddress, 24,  0, TAG> L2OffsetHandle; //!< handle for the L2Offset bits
    typedef bits::bitrange_handle<DrvAPIAddress, 43,  0, TAG> DRAMOffsetHandle; //!< handle for the DRAMOffset bits
    typedef bits::bitrange_handle<DrvAPIAddress, 18, 18> CtrlIsCoreHandle; //!< handle for the CtrlIsCore bits
    typedef bits::bitrange_handle<DrvAPIAddress, 17,  0> CtrlOffsetHandle; //!< handle for the CtrlOffset bits

    /* core control registers */
    static constexpr uint64_t CTRL_CORE_RESET = 0x000; //!< core reset register
    
    DrvAPIPAddress() : addr(0) {}
    DrvAPIPAddress(const DrvAPIPAddress &o) = default;
    DrvAPIPAddress &operator=(const DrvAPIPAddress &o) = default;
    DrvAPIPAddress(DrvAPIPAddress &&o) = default;
    DrvAPIPAddress &operator=(DrvAPIPAddress &&o) = default;
    ~DrvAPIPAddress() = default;

    DrvAPIPAddress(DrvAPIAddress addr) : addr(addr) {}

    TypeHandle type() {
        return TypeHandle(addr);
    }

    PXNHandle pxn() {
        return PXNHandle(addr);
    }

    PodHandle pod() {
        return PodHandle(addr);
    }

    CoreYHandle core_y() {
        return CoreYHandle(addr);
    }

    CoreXHandle core_x() {
        return CoreXHandle(addr);
    }

    L1OffsetHandle l1_offset() {
        return L1OffsetHandle(addr);
    }

    L2OffsetHandle l2_offset() {
        return L2OffsetHandle(addr);
    }

    DRAMOffsetHandle dram_offset() {
        return DRAMOffsetHandle(addr);
    }

    CtrlIsCoreHandle ctrl_is_core() {
        return CtrlIsCoreHandle(addr);
    }

    CtrlOffsetHandle ctrl_offset() {
        return CtrlOffsetHandle(addr);
    }

    DrvAPIAddress encode() const { return addr; }

    std::string to_string();

    DrvAPIAddress addr;
};

}

#endif
