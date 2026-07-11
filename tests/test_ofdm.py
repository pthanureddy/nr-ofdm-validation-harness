import numpy as np

from nr_validation.models import OfdmConfig
from nr_validation.ofdm import add_cyclic_prefix, remove_cyclic_prefix, run_frame
from nr_validation.validation import deterministic_bits


def test_cyclic_prefix_round_trip() -> None:
    symbol = np.arange(16, dtype=float) + 1j * np.arange(16, dtype=float)
    framed = add_cyclic_prefix(symbol, 4)
    assert np.array_equal(remove_cyclic_prefix(framed, 4), symbol)
    assert np.array_equal(framed[:4], symbol[-4:])


def test_pilot_estimate_tracks_channel() -> None:
    config = OfdmConfig()
    result = run_frame(
        deterministic_bits(config),
        config=config,
        channel_gain=0.7 - 0.2j,
        noise_std=0.0,
        scenario="test",
    )
    estimated = complex(result.channel_gain_estimate_real, result.channel_gain_estimate_imag)
    assert abs(estimated - (0.7 - 0.2j)) < 1e-12
    assert result.bit_errors == 0


def test_nominal_frame_is_recovered() -> None:
    result = run_frame(deterministic_bits(OfdmConfig()), noise_std=0.005)
    assert result.bit_errors == 0
    assert result.evm_rms < 0.08
