#ifndef _BITSET_H
#define _BITSET_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vector>

#if __cplusplus >= 201103L
#include <atomic>
#endif 

class BitSet {
public:
#if __cplusplus >= 201103L
    explicit BitSet(std::size_t n_bits)
        : n_bits_(n_bits),
          cells_(std::vector<std::atomic<uint64_t>>((n_bits + 63) / 64))
    {
    }
#else
    explicit BitSet(std::size_t n_bits)
        : n_bits_(n_bits),
          cells_(std::vector<uint64_t>((n_bits + 63) / 64, 0))
    {
    }
#endif

    /*  If bit at idx is 0  ->  set it to 1 and return false
        If bit at idx is 1  ->  leave it unchanged and return true
        Thread-safe, wait-free on x86/ARM/PPC. */
    bool testAndSet(std::size_t idx)
    {
        std::size_t cell = idx >> 6;          // idx / 64
        unsigned    bit  = idx & 63;          // idx % 64
        uint64_t mask = 1ULL << bit;

        /*  We need an atomic “read-modify-write” that gives us the *old* value
            of the bit.  fetch_or returns the previous full 64-bit word. */
#if __cplusplus >= 201103L
        uint64_t old = cells_[cell].fetch_or(mask, std::memory_order_acq_rel);
#else
        uint64_t *data = cells_.data();
        data += cell;
        uint64_t old = __atomic_fetch_or(data, mask, __ATOMIC_ACQ_REL);
#endif

        return (old & mask) != 0;             // true  →  bit was already 1
                                              // false →  we flipped 0→1
    }

    /*  Set bit idx to 0.  Pure store, no return value. */
    void clear(std::size_t idx)
    {
        std::size_t cell = idx >> 6;
        unsigned    bit  = idx & 63;
        uint64_t mask = 1ULL << bit;

        /*  We simply turn the bit off.  No need for a full RMW; a blind
            atomic AND is enough. */
#if __cplusplus >= 201103L
        cells_[cell].fetch_and(~mask, std::memory_order_release);
#else
        uint64_t *data = cells_.data();
        data += cell;
        __atomic_fetch_and(data, ~mask, __ATOMIC_RELEASE);
#endif
    }

#if __cplusplus >= 201103L
    // non-copyable (but you can add move if you want)
    BitSet(const BitSet&) = delete;
    BitSet& operator=(const BitSet&) = delete;
#endif 

    size_t size() const { return this->n_bits_; }

private:
    std::size_t n_bits_;
#if __cplusplus >= 201103L
    std::vector<std::atomic<uint64_t>> cells_;
#else
    std::vector<uint64_t> cells_;
#endif
};

#endif // _BITSET_H
