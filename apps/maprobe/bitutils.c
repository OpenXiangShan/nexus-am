#include "bitutils.h" 

inline uint64_t _perf_get_bit_mask(int low, int high) {
    assert(low < high);
    assert(low >= 0);
    assert(high < 63);
    return ((1 << high) - 1) >> low << low;
}

inline uint64_t _perf_get_bits(uint64_t raw_data, int low, int high) {
    assert(low < high);
    assert(low >= 0);
    assert(high < 63);
    uint64_t mask = (1 << high) - 1;
    return (raw_data & mask) >> low;
}

inline uint64_t _perf_get_bit(uint64_t raw_data, int position) {
    assert(position >= 0);
    assert(position <= 63);
    return (raw_data >> position) & 1;
}

inline uint64_t _perf_set_bits(uint64_t raw_data, int low, int high, uint64_t new_value) {
    assert(low < high);
    assert(low >= 0);
    assert(high < 63);
    int mask = _perf_get_bit_mask(low, high);
    return (raw_data & mask) | ((new_value << low) & mask);
}

inline uint64_t _perf_set_bit(uint64_t raw_data, int position, int new_value) {
    assert(position >= 0);
    assert(position <= 63);
    return raw_data & ((new_value & 1) << position);
}
