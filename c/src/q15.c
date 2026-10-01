#include "nr_q15/q15.h"

#include <limits.h>

static int16_t round_and_saturate(int64_t q30) {
  int64_t rounded = q30 >= 0 ? (q30 + INT64_C(16384)) / INT64_C(32768)
                             : -((-q30 + INT64_C(16384)) / INT64_C(32768));
  if (rounded > INT16_MAX) {
    return INT16_MAX;
  }
  if (rounded < INT16_MIN) {
    return INT16_MIN;
  }
  return (int16_t)rounded;
}

int nr_q15_complex_multiply(nr_q15_iq left, nr_q15_iq right,
                            nr_q15_iq *result) {
  if (result == NULL) {
    return -1;
  }
  const int64_t real = (int64_t)left.i * right.i - (int64_t)left.q * right.q;
  const int64_t imag = (int64_t)left.i * right.q + (int64_t)left.q * right.i;
  result->i = round_and_saturate(real);
  result->q = round_and_saturate(imag);
  return 0;
}

int nr_q15_fir_block(const nr_q15_iq *input, size_t sample_count,
                     const int16_t *taps, size_t tap_count,
                     nr_q15_iq *output) {
  if (input == NULL || output == NULL || taps == NULL || sample_count == 0 ||
      tap_count == 0 || tap_count > NR_Q15_MAX_TAPS) {
    return -1;
  }
  for (size_t sample = 0; sample < sample_count; ++sample) {
    int64_t real = 0;
    int64_t imag = 0;
    const size_t active_taps = sample + 1 < tap_count ? sample + 1 : tap_count;
    for (size_t tap = 0; tap < active_taps; ++tap) {
      real += (int64_t)input[sample - tap].i * taps[tap];
      imag += (int64_t)input[sample - tap].q * taps[tap];
    }
    output[sample].i = round_and_saturate(real);
    output[sample].q = round_and_saturate(imag);
  }
  return 0;
}
