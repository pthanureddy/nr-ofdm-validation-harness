#include "nr_ofdm/ofdm.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace nr_ofdm {

std::vector<int> deterministic_bits() {
  constexpr int pattern[] = {0, 1, 1, 0, 1, 1, 0, 0};
  std::vector<int> bits(kDataBits);
  for (std::size_t index = 0; index < bits.size(); ++index) {
    bits[index] = pattern[index % 8];
  }
  return bits;
}

std::vector<Complex> qpsk_modulate(const std::vector<int>& bits) {
  if (bits.empty() || bits.size() % 2 != 0) {
    throw std::invalid_argument("QPSK needs a nonempty, even bit count");
  }
  constexpr double scale = 0.70710678118654752440;
  std::vector<Complex> symbols;
  symbols.reserve(bits.size() / 2);
  for (std::size_t index = 0; index < bits.size(); index += 2) {
    if ((bits[index] != 0 && bits[index] != 1) ||
        (bits[index + 1] != 0 && bits[index + 1] != 1)) {
      throw std::invalid_argument("bits must be zero or one");
    }
    symbols.emplace_back((bits[index] == 0 ? 1.0 : -1.0) * scale,
                         (bits[index + 1] == 0 ? 1.0 : -1.0) * scale);
  }
  return symbols;
}

std::vector<int> qpsk_demodulate(const std::vector<Complex>& symbols) {
  std::vector<int> bits;
  bits.reserve(symbols.size() * 2);
  for (const auto symbol : symbols) {
    bits.push_back(symbol.real() < 0 ? 1 : 0);
    bits.push_back(symbol.imag() < 0 ? 1 : 0);
  }
  return bits;
}

void fft_inplace(std::vector<Complex>& samples, bool inverse) {
  const std::size_t count = samples.size();
  if (count == 0 || (count & (count - 1)) != 0) {
    throw std::invalid_argument("FFT size must be a nonzero power of two");
  }
  for (std::size_t index = 1, reversed = 0; index < count; ++index) {
    std::size_t bit = count >> 1;
    while ((reversed & bit) != 0) {
      reversed ^= bit;
      bit >>= 1;
    }
    reversed ^= bit;
    if (index < reversed) {
      std::swap(samples[index], samples[reversed]);
    }
  }
  for (std::size_t length = 2; length <= count; length <<= 1) {
    const double angle = (inverse ? 2.0 : -2.0) * std::numbers::pi /
                         static_cast<double>(length);
    const Complex root(std::cos(angle), std::sin(angle));
    for (std::size_t start = 0; start < count; start += length) {
      Complex twiddle(1.0, 0.0);
      for (std::size_t offset = 0; offset < length / 2; ++offset) {
        const Complex even = samples[start + offset];
        const Complex odd = samples[start + offset + length / 2] * twiddle;
        samples[start + offset] = even + odd;
        samples[start + offset + length / 2] = even - odd;
        twiddle *= root;
      }
    }
  }
  if (inverse) {
    for (auto& sample : samples) {
      sample /= static_cast<double>(count);
    }
  }
}

std::vector<Complex> add_cyclic_prefix(const std::vector<Complex>& samples,
                                        std::size_t prefix_length) {
  if (prefix_length == 0 || prefix_length >= samples.size()) {
    throw std::invalid_argument("invalid cyclic prefix length");
  }
  std::vector<Complex> framed;
  framed.reserve(samples.size() + prefix_length);
  framed.insert(framed.end(), samples.end() - static_cast<std::ptrdiff_t>(prefix_length),
                samples.end());
  framed.insert(framed.end(), samples.begin(), samples.end());
  return framed;
}

std::vector<Complex> remove_cyclic_prefix(const std::vector<Complex>& samples,
                                           std::size_t prefix_length) {
  if (prefix_length == 0 || prefix_length >= samples.size()) {
    throw std::invalid_argument("invalid cyclic prefix length");
  }
  return {samples.begin() + static_cast<std::ptrdiff_t>(prefix_length), samples.end()};
}

FrameMetrics run_frame(const std::vector<int>& bits, Complex channel_gain) {
  if (bits.size() != kDataBits) {
    throw std::invalid_argument("frame requires 112 data bits");
  }
  if (!std::isfinite(channel_gain.real()) || !std::isfinite(channel_gain.imag()) ||
      std::abs(channel_gain) < 1e-12) {
    throw std::invalid_argument("channel gain must be finite and nonzero");
  }
  const auto data_symbols = qpsk_modulate(bits);
  std::vector<Complex> grid(kFftSize);
  for (std::size_t index = 0, data_index = 0; index < kFftSize; ++index) {
    grid[index] = index % kPilotSpacing == 0 ? Complex(1.0, 0.0)
                                              : data_symbols[data_index++];
  }
  auto transmitted_time = grid;
  fft_inplace(transmitted_time, true);
  auto received_time = add_cyclic_prefix(transmitted_time, kCyclicPrefix);
  for (auto& sample : received_time) {
    sample *= channel_gain;
  }
  auto received_grid = remove_cyclic_prefix(received_time, kCyclicPrefix);
  fft_inplace(received_grid, false);

  Complex estimate(0.0, 0.0);
  for (std::size_t index = 0; index < kFftSize; index += kPilotSpacing) {
    estimate += received_grid[index];
  }
  estimate /= static_cast<double>(kFftSize / kPilotSpacing);
  if (std::abs(estimate) < 1e-12) {
    throw std::runtime_error("pilot channel estimate is zero");
  }

  std::vector<Complex> equalized;
  equalized.reserve(kDataBits / 2);
  double error_power = 0.0;
  for (std::size_t index = 0, data_index = 0; index < kFftSize; ++index) {
    if (index % kPilotSpacing == 0) {
      continue;
    }
    const Complex symbol = received_grid[index] / estimate;
    equalized.push_back(symbol);
    error_power += std::norm(symbol - data_symbols[data_index++]);
  }
  const auto recovered = qpsk_demodulate(equalized);
  std::size_t errors = 0;
  for (std::size_t index = 0; index < bits.size(); ++index) {
    if (bits[index] != recovered[index]) {
      ++errors;
    }
  }
  return {estimate, errors, bits.size(), static_cast<double>(errors) / bits.size(),
          std::sqrt(error_power / equalized.size())};
}

}  // namespace nr_ofdm
