import numpy as np

from nr_validation.modulation import qpsk_demodulate, qpsk_modulate


def test_qpsk_round_trip() -> None:
    bits = np.array([0, 0, 0, 1, 1, 0, 1, 1], dtype=np.int8)
    assert np.array_equal(qpsk_demodulate(qpsk_modulate(bits)), bits)


def test_qpsk_symbols_have_unit_power() -> None:
    bits = np.tile(np.array([0, 1], dtype=np.int8), 32)
    symbols = qpsk_modulate(bits)
    assert np.allclose(np.abs(symbols) ** 2, 1.0)


def test_qpsk_rejects_invalid_input() -> None:
    with np.testing.assert_raises(ValueError):
        qpsk_modulate(np.array([0, 2], dtype=np.int8))

