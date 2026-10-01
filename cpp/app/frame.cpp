#include "nr_ofdm/ofdm.hpp"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char** argv) {
  try {
    if (argc != 3) {
      throw std::invalid_argument("usage: nr_ofdm_frame CHANNEL_REAL CHANNEL_IMAG");
    }
    std::size_t real_end = 0;
    std::size_t imag_end = 0;
    const std::string real_arg(argv[1]);
    const std::string imag_arg(argv[2]);
    const double real = std::stod(real_arg, &real_end);
    const double imag = std::stod(imag_arg, &imag_end);
    if (real_end != real_arg.size() || imag_end != imag_arg.size()) {
      throw std::invalid_argument("channel gain must be numeric");
    }
    const auto result = nr_ofdm::run_frame(nr_ofdm::deterministic_bits(), {real, imag});
    std::cout << std::setprecision(17)
              << "{\"channel_estimate_real\":" << result.channel_estimate.real()
              << ",\"channel_estimate_imag\":" << result.channel_estimate.imag()
              << ",\"bit_errors\":" << result.bit_errors
              << ",\"bit_count\":" << result.bit_count
              << ",\"ber\":" << result.ber
              << ",\"evm_rms\":" << result.evm_rms << "}\n";
  } catch (const std::exception& error) {
    std::cerr << "nr_ofdm_frame: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
