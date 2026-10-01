#include "nr_q15/q15.h"

#include <stdint.h>
#include <stdio.h>

static int failures = 0;

static void check(int condition, const char *name) {
  if (!condition) {
    fprintf(stderr, "FAILED: %s\n", name);
    ++failures;
  }
}

static void test_complex_multiply(void) {
  nr_q15_iq output = {0, 0};
  check(nr_q15_complex_multiply((nr_q15_iq){16384, 0},
                                (nr_q15_iq){16384, 0}, &output) == 0 &&
            output.i == 8192 && output.q == 0,
        "complex multiplication real quarter");
  check(nr_q15_complex_multiply((nr_q15_iq){0, 16384},
                                (nr_q15_iq){0, 16384}, &output) == 0 &&
            output.i == -8192 && output.q == 0,
        "complex multiplication imaginary quarter");
  check(nr_q15_complex_multiply((nr_q15_iq){-32768, -32768},
                                (nr_q15_iq){-32768, -32768}, &output) == 0 &&
            output.i == 0 && output.q == 32767,
        "complex multiplication saturates");
}

static void test_rounding_and_invalid_input(void) {
  nr_q15_iq output = {0, 0};
  check(nr_q15_complex_multiply((nr_q15_iq){1, -1},
                                (nr_q15_iq){16384, 0}, &output) == 0 &&
            output.i == 1 && output.q == -1,
        "Q15 ties round away from zero");
  check(nr_q15_complex_multiply((nr_q15_iq){0, 0},
                                (nr_q15_iq){0, 0}, NULL) == -1,
        "complex multiply rejects null result");
}

static void test_fir(void) {
  const nr_q15_iq input[] = {{16384, 0}, {0, 16384}, {-16384, 0}};
  const int16_t taps[] = {16384, 16384};
  nr_q15_iq output[3] = {{0, 0}, {0, 0}, {0, 0}};
  check(nr_q15_fir_block(input, 3, taps, 2, output) == 0 &&
            output[0].i == 8192 && output[0].q == 0 &&
            output[1].i == 8192 && output[1].q == 8192 &&
            output[2].i == -8192 && output[2].q == 8192,
        "zero-history complex IQ FIR");
}

static void test_fir_saturation_and_validation(void) {
  const nr_q15_iq input[] = {{32767, -32768}, {32767, -32768}};
  const int16_t taps[] = {32767, 32767};
  nr_q15_iq output[2] = {{0, 0}, {0, 0}};
  check(nr_q15_fir_block(input, 2, taps, 2, output) == 0 &&
            output[1].i == 32767 && output[1].q == -32768,
        "FIR saturation in both directions");
  check(nr_q15_fir_block(NULL, 2, taps, 2, output) == -1,
        "FIR rejects null input");
  check(nr_q15_fir_block(input, 0, taps, 2, output) == -1,
        "FIR rejects zero samples");
  check(nr_q15_fir_block(input, 2, taps, NR_Q15_MAX_TAPS + 1, output) == -1,
        "FIR rejects too many taps before reading them");
}

int main(void) {
  test_complex_multiply();
  test_rounding_and_invalid_input();
  test_fir();
  test_fir_saturation_and_validation();
  if (failures != 0) {
    return 1;
  }
  puts("4 C11 Q15 kernel groups passed");
  return 0;
}
