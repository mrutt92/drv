#include <cello_core_drvx_allocator.hpp>
#include <cello_drvx_internal.hpp>
#include <array>

//#define DEBUG
#ifdef DEBUG
#define pr_dbg(fmt, ...)                                                \
    do {                                                                \
        printf("DEBUG: %s: " fmt , __PRETTY_FUNCTION__, ##__VA_ARGS__); \
        fflush(stdout);                                                 \
    } while (0)
#else
#define pr_dbg(fmt, ...)                                                \
    do {                                                                \
    } while (0)
#endif

#define FIELD(type, name, field)                \
    type field;                                 \
    const type& name() const { return field; }  \
    type& name() { return field; }              \


namespace
{
typedef uint8_t byte;
typedef std::array<byte, 8> byte_array;
}

namespace
{
/**
 * @brief a convenience class for handling bitfields
 */
template <typename UINT, uint64_t HI, uint64_t LO, unsigned TAG=0>
struct bitfield_handle {
public:
    typedef UINT uint_type;
    static constexpr uint64_t HI_BIT = HI;
    static constexpr uint64_t LO_BIT = LO;

    bitfield_handle(DrvAPI::value_handle<UINT> i) : i(i) {}
    ~bitfield_handle() = default;
    bitfield_handle(bitfield_handle &&o) = default;
    bitfield_handle &operator=(bitfield_handle &&o) = default;
    bitfield_handle(const bitfield_handle &o) = default;
    bitfield_handle &operator=(const bitfield_handle &o) = default;

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
        return ((1ull << (HI - LO + 1ull)) - 1ull) << LO;
    }

    static UINT getbits(UINT in)
    {
        return (in & mask()) >> LO;
    }

    static void setbits(DrvAPI::value_handle<UINT> in, UINT val)
    {
        in = in & ~mask();
        in = in | ( mask() & (val << LO));
    }

    operator UINT() const { return getbits(i); }

    bitfield_handle &operator=(UINT val) {
        setbits(i, val);
        return *this;
    }

    DrvAPI::value_handle<UINT> i;
};

} // namespace

namespace cello
{

/**
 * @brief block of memory
 */
struct block {
    FIELD(uint64_t,               info, info_);
    FIELD(DrvAPI::pointer<block>, next, next_);
    FIELD(DrvAPI::pointer<block>, prev, prev_);
    FIELD(byte_array,             data, data_);

    template <typename Src, typename Dst>
    static void copy(Dst &dst, const Src &src) {
        dst.info() = src.info();
        dst.next() = src.next();
        dst.prev() = src.prev();
    }
};

/**
 * @brief allocator
 */
struct allocator {
    FIELD(DrvAPI::pointer<block>, free_list, free_list_);
    FIELD(int64_t,                lock, lock_);
};

}

namespace DrvAPI {

/**
 * specialization of byte_array
 */
template <>
class value_handle<byte_array> {
    DRV_API_VALUE_HANDLE_DEFAULTS_TRIVIAL(byte_array)
    value_handle<byte> operator[](size_t index) {
        return value_handle<byte>(address() + index);
    }
    const value_handle<byte> operator[](size_t index) const {
        return value_handle<byte>(address() + index);
    }
};

/**
 * specialization of the value_handle for cello::block
 */
template <>
class value_handle<cello::block> {
    DRV_API_VALUE_HANDLE_CONSTRUCTORS(cello::block)
    DRV_API_VALUE_HANDLE_ADDRESSOF_OPERATORS(cello::block)
    DRV_API_VALUE_HANDLE_INTERNAL(cello::block)
    DRV_API_VALUE_HANDLE_FIELD(cello::block, info, uint64_t, info_)
    DRV_API_VALUE_HANDLE_FIELD(cello::block, next, pointer<cello::block>, next_)
    DRV_API_VALUE_HANDLE_FIELD(cello::block, prev, pointer<cello::block>, prev_)
    DRV_API_VALUE_HANDLE_FIELD(cello::block, data, byte_array, data_)
    bitfield_handle<uint64_t, 0, 0> is_free() {
        return bitfield_handle<uint64_t, 0, 0>(info());
    }
    const bitfield_handle<uint64_t, 0, 0> is_free() const {
        return bitfield_handle<uint64_t, 0, 0>(info());
    }
    bitfield_handle<uint64_t, 1, 1> is_predecessor_free() {
        return bitfield_handle<uint64_t, 1, 1>(info());
    }
    const bitfield_handle<uint64_t, 1, 1> is_predecessor_free() const {
        return bitfield_handle<uint64_t, 1, 1>(info());
    }
    bitfield_handle<uint64_t, 63, 2> size() {
        return bitfield_handle<uint64_t, 63, 2>(info());
    }
    const bitfield_handle<uint64_t, 63, 2> size() const {
        return bitfield_handle<uint64_t, 63, 2>(info());
    }
    value_handle<uint64_t> footer() {
        return value_handle<uint64_t>(address() + size() - sizeof(uint64_t));
    }
    const value_handle<uint64_t> footer() const {
        return value_handle<uint64_t>(address() + size() - sizeof(uint64_t));
    }
    value_handle<cello::block> successor() {
        return value_handle<cello::block>(address() + size());
    }
    const value_handle<cello::block> successor() const {
        return value_handle<cello::block>(address() + size());
    }
    const value_handle<uint64_t> predecessor_size() const {
        return value_handle<uint64_t>(address() - sizeof(uint64_t));
    }
    value_handle<cello::block> predecessor() {
        return value_handle<cello::block>(address() - (uint64_t)predecessor_size());
    }
    const value_handle<cello::block> predecessor() const {
        return value_handle<cello::block>(address() - (uint64_t)predecessor_size());
    }
};

/**
 * specialization of the value_handle for cello::allocator
 */
template <>
class value_handle<cello::allocator> {
    DRV_API_VALUE_HANDLE_CONSTRUCTORS(cello::allocator)
    DRV_API_VALUE_HANDLE_ADDRESSOF_OPERATORS(cello::allocator)
    DRV_API_VALUE_HANDLE_INTERNAL(cello::allocator)
    DRV_API_VALUE_HANDLE_FIELD(cello::allocator, free_list, pointer<cello::block>, free_list_)
    DRV_API_VALUE_HANDLE_FIELD(cello::allocator, lock, int64_t, lock_)
    bool empty() const {
        DrvAPI::pointer<cello::block> free_list_ptr = free_list();
        DrvAPI::value_handle<cello::block> free_list = *free_list_ptr;
        if (!free_list.is_free()) {
            return true;
        }
        return false;
    }
};

}

namespace cello
{


DrvAPI::dram_static<allocator> allocator; //!< allocator
constexpr uint64_t SLAB_SIZE = sizeof(block) + 4*1024; //!< size of a slab

/**
 * print the free list
 */
void print_free_list(const char *prefix)
{
    DrvAPI::pointer<block> curr_block_ptr = allocator.free_list();
    printf("%s: free list:\n", prefix);
    do {
        DrvAPI::value_handle<block> curr_block = *curr_block_ptr;
        printf("%s: block: %lx is_free = %d, is_predecessor_free = %d, size = %lu\n",
               prefix,
                (uint64_t)curr_block.address(),
               (bool)curr_block.is_free(),
               (bool)curr_block.is_predecessor_free(),
               (uint64_t)curr_block.size()
               );
        curr_block_ptr = curr_block.next();
    } while (curr_block_ptr != allocator.free_list());
}

DrvAPI::pointer<block> new_slab(uint64_t size)
{
    DrvAPI::pointer<block> slab_ptr =  DrvAPI::DrvAPIMemoryAlloc
        (DrvAPI::DrvAPIMemoryDRAM
         , size + sizeof(uint64_t)
         );
    auto slab = *slab_ptr;
    // set the size
    slab.size() = size;
    slab.is_free() = true;
    slab.is_predecessor_free() = false;
    pr_dbg("setting slab.size() = %lu (= %lu)\n", size, (uint64_t)slab.size());
    slab.footer() = slab.size();
    // have it point to itself
    slab.next() = slab.address();
    slab.prev() = slab.address();
    return slab_ptr;
}

/**
 * @brief initialize the allocator
 */
void allocator_init()
{
    DrvAPI::DrvAPIMemoryAllocatorInit();
    // initialize the allocator with a starting slab
    DrvAPI::pointer<block> slab_ptr =  new_slab(SLAB_SIZE);
    // set the free list to this slab
    allocator.free_list() = slab_ptr;
    allocator.lock() = 0;
    DrvAPI::value_handle<block> slab = *slab_ptr;
    //print_free_list("allocator_init");
}


/**
 * @brief allocate memory
 */
DrvAPI::pointer<void> allocate(uint64_t size)
{
    // align the size to 8 bytes
    size = (size + 7) & ~7;
    // make sure at least min size
    size = std::max(size, sizeof(block));
    // add one word for the info field
    size += sizeof(uint64_t);

    cello::lock_guard lock(allocator.lock().address());
    //print_free_list("allocate: start");
 scan_free_list:
    DrvAPI::pointer<block> curr_block_ptr = allocator.free_list();
    // cycle through the free list until we find a block that fits
    do {
        if ((*curr_block_ptr).is_free() &&
            (*curr_block_ptr).size() >= size) {
            if ((*curr_block_ptr).size() > size + sizeof(block) + sizeof(uint64_t)) {
                // split the block
                DrvAPI::value_handle<block> curr_block = *curr_block_ptr;
                DrvAPI::pointer<block> new_block_ptr = curr_block.address() + size;
                DrvAPI::value_handle<block> new_block = *new_block_ptr;
                // set the size of the new block
                new_block.size() = curr_block.size() - size;
                new_block.is_free() = true;
                new_block.is_predecessor_free() = false;
                new_block.footer() = new_block.size();
                // update the current block size
                curr_block.size() = size;
                curr_block.footer() = size;
                // update the free list
                if (curr_block.next() == curr_block_ptr) {
                    // if the block is the only one in the list
                    new_block.next() = new_block_ptr;
                    new_block.prev() = new_block_ptr;
                } else {
                    // remove the block from the free list
                    DrvAPI::value_handle<block> next_block = *(curr_block.next());
                    DrvAPI::value_handle<block> prev_block = *(curr_block.prev());
                    new_block.next() = next_block.address();
                    new_block.prev() = prev_block.address();
                    next_block.prev() = new_block.address();
                    prev_block.next() = new_block.address();
                }
                allocator.free_list() = new_block_ptr;
                curr_block.is_free() = false;
                pr_dbg("returning curr_block: is_free = %d, is_predecessor_free = %d, size = %lu\n",
                       (bool)curr_block.is_free(),
                       (bool)curr_block.is_predecessor_free(),
                       (uint64_t)curr_block.size()
                       );
                // return the address of the data
                //print_free_list("allocate: split");
                return curr_block.data().address();
            } else {
                // remove the block from the free list
                DrvAPI::value_handle<block> curr_block = *curr_block_ptr;
                DrvAPI::value_handle<block> next_block = *(curr_block.next());
                DrvAPI::value_handle<block> prev_block = *(curr_block.prev());
                // if the block is not the only one in the list
                next_block.prev() = prev_block.address();
                prev_block.next() = next_block.address();
                // update the successor
                allocator.free_list() = next_block.address();
                curr_block.successor().is_predecessor_free() = false;
                curr_block.is_free() = false;
                // return the address of the data
                //print_free_list("allocate as-is");
                return curr_block.data().address();
            }
        }
        curr_block_ptr = (*curr_block_ptr).next();
    } while (curr_block_ptr != allocator.free_list());

    // allocate a new slab
    DrvAPI::pointer<block> slab_ptr = new_slab(size*2);
    // insert into free list
    if (allocator.empty()) {
        allocator.free_list() = slab_ptr;
    } else {
        DrvAPI::value_handle<block> slab = *slab_ptr;
        DrvAPI::value_handle<block> next_block = *allocator.free_list();
        DrvAPI::value_handle<block> prev_block = *(next_block.prev());
        slab.next() = next_block.address();
        slab.prev() = prev_block.address();
        next_block.prev() = slab.address();
        prev_block.next() = slab.address();
    }
    goto scan_free_list;
    return {-1ul};
}

/**
 * @brief deallocate memory
 */
void deallocate(DrvAPI::pointer<void> ptr, uint64_t size)
{
    cello::lock_guard lock(allocator.lock().address());
    pr_dbg("deallocate(%lx, %lu)\n", (uint64_t)ptr, (uint64_t)size);
    DrvAPI::pointer<block> free_block_ptr = ptr - (sizeof(block) - sizeof(uint64_t));
    DrvAPI::value_handle<block> free_block = *free_block_ptr;
    // four cases
    pr_dbg("block.size() = %lu\n", (uint64_t)free_block.size());
    pr_dbg("block.successor().address() = %lx\n", (uint64_t)free_block.successor().address());
    if (!free_block.is_predecessor_free() && !free_block.successor().is_free()) {
        // 1. neither precessor nor successor are free
        //printf("case 1\n");
        // insert into the free list
        free_block.is_free() = true;
        free_block.successor().is_predecessor_free() = true;
        if (allocator.empty()) {
            free_block.next() = free_block_ptr;
            free_block.prev() = free_block_ptr;
            allocator.free_list() = free_block_ptr;
        } else {
            DrvAPI::value_handle<block> next_block = *allocator.free_list();
            DrvAPI::value_handle<block> prev_block = *next_block.prev();
            free_block.next() = next_block.address();
            free_block.prev() = prev_block.address();
            next_block.prev() = free_block.address();
            prev_block.next() = free_block.address();
        }
    } else if (free_block.is_predecessor_free() && !free_block.successor().is_free()) {
        // 2. predecessor is free, but successor is not
        //printf("case 2\n");
        // coalesce with the predecessor
        DrvAPI::value_handle<block> predecessor = free_block.predecessor();
        predecessor.size() = free_block.size() + predecessor.size();
        predecessor.footer() = predecessor.size();
        predecessor.successor().is_predecessor_free() = true;
    } else if (!free_block.is_predecessor_free() && free_block.successor().is_free()) {
        // 3. predecessor is not free, but successor is
        //printf("case 3\n");
        DrvAPI::value_handle<block> successor = free_block.successor();
        free_block.size() = free_block.size() + successor.size();
        free_block.is_free() = true;
        free_block.footer() = free_block.size();
        DrvAPI::value_handle<block> next_block = *successor.next();
        DrvAPI::value_handle<block> prev_block = *successor.prev();
        if (next_block.address() == successor.address()) {
            free_block.next() = free_block.address();
            free_block.prev() = free_block.address();
        } else {
            free_block.next() = next_block.address();
            free_block.prev() = prev_block.address();
            next_block.prev() = free_block.address();
            prev_block.next() = free_block.address();
        }
        allocator.free_list() = free_block.address();
    } else {
        // 4. both predecessor and successor are free
        //printf("case 4\n");
        DrvAPI::value_handle<block> predecessor = free_block.predecessor();
        DrvAPI::value_handle<block> successor = free_block.successor();
        predecessor.size() = predecessor.size() + free_block.size() + successor.size();
        predecessor.footer() = predecessor.size();
        // remove the successor from the free list
        DrvAPI::value_handle<block> next_block = *successor.next();
        DrvAPI::value_handle<block> prev_block = *successor.prev();
        next_block.prev() = prev_block.address();
        prev_block.next() = next_block.address();
        allocator.free_list() = next_block.address();
    }
    //print_free_list("deallocate");
}

}
