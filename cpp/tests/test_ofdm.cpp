#include "nr_ofdm/ofdm.hpp"

#include <cmath>
#include <complex>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

void require(bool condition, const char* message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

template <class Function>
void require_rejected(Function&& function, const char* message) {
  try {
    function();
  } catch (const std::invalid_argument&) {
    return;
  }
  throw std::runtime_error(message);
}

void test_qpsk_round_trip() {
  const auto bits = nr_ofdm::deterministic_bits();
  require(bits.size() == 112, "wrong deterministic bit count");
  require(nr_ofdm::qpsk_demodulate(nr_ofdm::qpsk_modulate(bits)) == bits,
          "QPSK round trip failed");
  require_rejected([] { nr_ofdm::qpsk_modulate({0, 2}); }, "invalid bit accepted");
  require_rejected([] { nr_ofdm::qpsk_modulate({0}); }, "odd bit count accepted");
}

void test_fft_round_trip() {
  std::vector<nr_ofdm::Complex> samples(64);
  for (std::size_t index = 0; index < samples.size(); ++index) {
    samples[index] = {static_cast<double>(index % 7), static_cast<double>(index % 5)};
  }
  const auto original = samples;
  nr_ofdm::fft_inplace(samples, false);
  nr_ofdm::fft_inplace(samples, true);
  for (std::size_t index = 0; index < samples.size(); ++index) {
    require(std::abs(samples[index] - original[index]) < 1e-12, "FFT round trip failed");
  }
  require_rejected([] {
    std::vector<nr_ofdm::Complex> bad(63);
    nr_ofdm::fft_inplace(bad, false);
  }, "non-power-of-two FFT accepted");
}

void test_cyclic_prefix() {
  const std::vector<nr_ofdm::Complex> samples = {{1, 0}, {2, 0}, {3, 0}, {4, 0}};
  const auto framed = nr_ofdm::add_cyclic_prefix(samples, 2);
  require(framed.size() == 6 && framed[0] == samples[2] && framed[1] == samples[3],
          "cyclic prefix not copied from symbol end");
  require(nr_ofdm::remove_cyclic_prefix(framed, 2) == samples,
          "cyclic prefix removal failed");
  require_rejected([&] { nr_ofdm::add_cyclic_prefix(samples, 4); },
                   "full-symbol prefix accepted");
}

void test_frame_channel_estimate() {
  const auto metrics = nr_ofdm::run_frame(nr_ofdm::deterministic_bits(), {0.7, -0.2});
  require(std::abs(metrics.channel_estimate - nr_ofdm::Complex(0.7, -0.2)) < 1e-12,
          "pilot channel estimate inaccurate");
  require(metrics.bit_count == 112 && metrics.bit_errors == 0 && metrics.ber == 0,
          "noiseless frame bit recovery failed");
  require(metrics.evm_rms < 1e-12, "noiseless frame EVM too high");
}

void test_frame_rejects_invalid_input() {
  require_rejected([] { nr_ofdm::run_frame({0, 1}, {1.0, 0.0}); },
                   "wrong frame bit count accepted");
  require_rejected([] { nr_ofdm::run_frame(nr_ofdm::deterministic_bits(), {0.0, 0.0}); },
                   "zero channel gain accepted");
}

}  // namespace

int main() {
  try {
    test_qpsk_round_trip();
    test_fft_round_trip();
    test_cyclic_prefix();
    test_frame_channel_estimate();
    test_frame_rejects_invalid_input();
    std::cout << "5 C++ component checks passed\n";
  } catch (const std::exception& error) {
    std::cerr << "C++ component check failed: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
