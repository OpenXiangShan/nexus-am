// bit op utils for perf

#ifndef PROBE_BITUTILS_H
#define PROBE_BITUTILS_H

#include <klib.h>

extern uint64_t _perf_get_bit_mask(int low, int high);
extern uint64_t _perf_get_bits(uint64_t raw_data, int low, int high);
extern uint64_t _perf_get_bit(uint64_t raw_data, int position);
extern uint64_t _perf_set_bits(uint64_t raw_data, int low, int high, uint64_t new_value);
extern uint64_t _perf_set_bit(uint64_t raw_data, int position, int new_value);

#endif