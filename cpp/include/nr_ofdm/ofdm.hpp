#pragma once

#include <complex>
#include <cstddef>
#include <vector>

namespace nr_ofdm {

using Complex = std::complex<double>;
constexpr std::size_t kFftSize = 64;
constexpr std::size_t kCyclicPrefix = 16;
constexpr std::size_t kPilotSpacing = 8;
constexpr std::size_t kDataBits = 2 * (kFftSize - kFftSize / kPilotSpacing);

struct FrameMetrics {
  Complex channel_estimate;
  std::size_t bit_errors;
  std::size_t bit_count;
  double ber;
  double evm_rms;
};

std::vector<int> deterministic_bits();
std::vector<Complex> qpsk_modulate(const std::vector<int>& bits);
std::vector<int> qpsk_demodulate(const std::vector<Complex>& symbols);
void fft_inplace(std::vector<Complex>& samples, bool inverse);
std::vector<Complex> add_cyclic_prefix(const std::vector<Complex>& samples,
                                        std::size_t prefix_length);
std::vector<Complex> remove_cyclic_prefix(const std::vector<Complex>& samples,
                                           std::size_t prefix_length);
FrameMetrics run_frame(const std::vector<int>& bits, Complex channel_gain);

}  // namespace nr_ofdm
