"""Compare the native noiseless OFDM path with the NumPy reference."""

from __future__ import annotations

import json
import subprocess
import sys

from nr_validation.models import OfdmConfig
from nr_validation.ofdm import run_frame
from nr_validation.validation import deterministic_bits


def main() -> int:
    binary = sys.argv[1]
    config = OfdmConfig()
    bits = deterministic_bits(config)
    for real, imag in ((0.92, 0.03), (0.7, -0.2), (-0.4, 0.6)):
        process = subprocess.run(
            [binary, str(real), str(imag)],
            check=True, capture_output=True, text=True,
        )
        native = json.loads(process.stdout)
        reference = run_frame(
            bits, config=config, channel_gain=complex(real, imag), noise_std=0.0
        )
        assert native["bit_count"] == reference.bit_count == 112
        assert native["bit_errors"] == reference.bit_errors == 0
        assert abs(native["channel_estimate_real"] - reference.channel_gain_estimate_real) < 1e-12
        assert abs(native["channel_estimate_imag"] - reference.channel_gain_estimate_imag) < 1e-12
        assert abs(native["evm_rms"] - reference.evm_rms) < 1e-12
        assert abs(native["ber"] - reference.ber) < 1e-12
    print("3 C++/Python noiseless frame parity checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
