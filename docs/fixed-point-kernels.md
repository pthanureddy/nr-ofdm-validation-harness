# C11 fixed-point IQ kernels

The `c/` module provides two small signal-processing building blocks that use
caller-owned buffers and have no heap allocation or persistent state. They are
separate from the floating-point OFDM frame path. Their purpose is to expose
integer arithmetic choices and test boundaries typical of lower-level signal
processing, not to claim a real-time or DSP implementation.

## Format and arithmetic

- An `int16_t` Q15 value represents its signed integer divided by 32768.
  The representable interval is [-1.0, 32767/32768].
- Each product is accumulated in signed 64-bit Q30. Conversion back to Q15
  rounds to nearest with ties away from zero and then saturates to
  [-32768, 32767]. No wraparound is permitted.
- `nr_q15_complex_multiply` computes `(a.i + j*a.q) * (b.i + j*b.q)`.
  For example, `(16384, 0) * (16384, 0)` returns `(8192, 0)`.
- `nr_q15_fir_block` applies real-valued taps independently to I and Q.
  `output[n] = sum(input[n-k] * taps[k])` over available prior samples; earlier
  samples are treated as zero. Each output component is rounded and saturated
  once, after the full sum.

The maximum absolute single Q15 product is 2^30. A complex multiplication
adds or subtracts two products, and a permitted 64-tap FIR sums at most 64
products. Both fit in the signed 64-bit accumulator used by the implementation.

## API contract

Both functions return 0 on success and -1 for invalid arguments. The FIR
requires non-null input, taps, and output, at least one sample, and 1-64 taps.
Input and output arrays must not overlap. The caller owns all storage. The
function computes a zero-history block, so separate calls do not preserve
samples from a previous block.

## Build and checks

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

`c_q15_unit` covers exact-value complex products, positive and negative
rounding ties, saturation, FIR boundary behavior, and rejected arguments.
CI builds and tests the module with GCC and Clang on Linux and MSVC on Windows.

## Limits

The kernels are scalar C11. They do not use SIMD, interrupts, DMA, DSP
intrinsics, or multi-core scheduling. No deadline, throughput, or worst-case
execution-time claim is made. Quantized OFDM-frame integration would require
an independent numerical reference and an explicit error budget.
