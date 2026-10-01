#ifndef NR_Q15_Q15_H
#define NR_Q15_Q15_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { NR_Q15_MAX_TAPS = 64 };

typedef struct {
  int16_t i;
  int16_t q;
} nr_q15_iq;

/* Return 0 on success and -1 for an invalid argument. The Q15 result is
 * rounded to nearest with ties away from zero, then saturated to int16_t. */
int nr_q15_complex_multiply(nr_q15_iq left, nr_q15_iq right, nr_q15_iq *result);

/* A zero-history real-tap FIR applied independently to I and Q. Input and
 * output must be separate non-overlapping arrays owned by the caller. No
 * allocation or persistent filter state is used. */
int nr_q15_fir_block(const nr_q15_iq *input, size_t sample_count,
                     const int16_t *taps, size_t tap_count,
                     nr_q15_iq *output);

#ifdef __cplusplus
}
#endif

#endif
