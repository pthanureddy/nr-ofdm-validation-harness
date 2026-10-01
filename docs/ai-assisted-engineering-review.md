# AI-assisted engineering review

The C++20 reference path was drafted with AI assistance and reviewed as code,
not accepted as a correct physical-layer implementation on the strength of
generated text. The review boundary was the repository's existing Python/NumPy
reference and its documented 64-subcarrier flat-channel scope.

## Review and verification steps

1. Checked the frame constants against the Python configuration: 64 FFT bins,
   a 16-sample cyclic prefix, pilots every eighth bin, and 112 QPSK data bits.
2. Checked QPSK bit ordering and FFT sign/scaling conventions against the Python
   modulation and `numpy.fft` paths.
3. Replaced a stateful predicate used for counting bit errors with an explicit
   indexed loop so each bit comparison is inspectable and independent of
   algorithm callback behavior.
4. Added invalid-input checks for bit values/count, FFT length, cyclic-prefix
   length, and zero/non-finite channel gain.
5. Ran the existing Python unit tests and added C++ component checks plus three
   cross-language noiseless frame comparisons. GitHub CI compiles with GCC,
   Clang, and MSVC and runs those checks on Linux and Windows.
6. For the later C11 Q15 extension, checked the largest complex-product and
   64-tap FIR sums against the 64-bit accumulator range. Added exact-value,
   rounding, saturation, and invalid-argument checks. Kept these kernels
   separate from the OFDM frame because integration would need an independent
   quantized-frame reference and error-budget tests.

The parity check compares bit count, bit errors, BER, pilot gain estimate, and
EVM. It uses three flat complex channel gains and zero noise. This is a
software consistency check, not proof of numerical equivalence for all inputs,
real-time performance, 5G NR conformance, or radio hardware behavior.

## Follow-up engineering work

- Add multipath and timing-offset models with independent reference vectors.
- Add randomized property tests and boundary cases around equalizer failure.
- Benchmark only with a specified machine, compiler, build mode, and frame set.
- Validate any production use against relevant 3GPP specifications and a real
  radio test environment.
