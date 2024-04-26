#ifndef FOREACH_HPP
#define FOREACH_HPP
#include <utility>
#ifdef RISCV
#include <cello.hpp>
#endif

namespace common {
/**
 * @brief A serial for-each loop.
 */
struct serial_foreach {
    template <typename Idx, typename Body>
    void operator()(Idx start, Idx stop, Idx step, Body && body) const {
        for (Idx i = start; i < stop; i += step) {
            body(i);
        }
    }
};

/**
 * @brief A serial for-each loop with block size.
 */
struct serial_foreach_block {
    template <typename Idx, typename Body>
    void operator()(Idx start, Idx stop, Idx block_size, Body &&body) const {
        for (Idx i = start; i < stop; i += block_size) {
            Idx end = i + block_size;
            if (end > stop) {
                end = stop;
            }
            body(i, end);
        }
    }
};

#ifdef RISCV
/**
 * @brief A parallel for-each loop.
 */
struct parallel_foreach {
    template <typename Idx, typename Body>
    void operator()(Idx start, Idx stop, Idx step, Body && body) const {
        cello::parallel_for(start, stop, step, std::forward<Body>(body));
    }
};

/**
 * @brief A parallel for-each loop with block size.
 */
struct parallel_foreach_block {
    template <typename Idx, typename Body>
    void operator()(Idx start, Idx stop, Idx block_size, Body && body) const {
        cello::parallel_for(start, stop, block_size, [body, stop, block_size](Idx i) mutable {
            Idx end = i + block_size;
            if (end > stop) {
                end = stop;
            }
            body(i, end);
        });
    }
};

#else
using parallel_foreach = serial_foreach;
using parallel_foreach_block = serial_foreach_block;
#endif
}

#endif // FOREACH_HPP
