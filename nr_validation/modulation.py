from __future__ import annotations

import numpy as np

QPSK_SCALE = 1.0 / np.sqrt(2.0)


def qpsk_modulate(bits: np.ndarray) -> np.ndarray:
    """Map pairs of bits to unit-power QPSK symbols."""

    values = np.asarray(bits, dtype=np.int8)
    if values.ndim != 1 or values.size % 2:
        raise ValueError("QPSK modulation requires a one-dimensional even-length bit array")
    if np.any((values != 0) & (values != 1)):
        raise ValueError("bits must contain only zero or one")

    real = np.where(values[0::2] == 0, 1.0, -1.0)
    imag = np.where(values[1::2] == 0, 1.0, -1.0)
    return QPSK_SCALE * (real + 1j * imag)


def qpsk_demodulate(symbols: np.ndarray) -> np.ndarray:
    """Recover bits using a hard decision on the real and imaginary axes."""

    values = np.asarray(symbols, dtype=np.complex128)
    if values.ndim != 1:
        raise ValueError("QPSK demodulation requires a one-dimensional symbol array")

    bits = np.empty(values.size * 2, dtype=np.int8)
    bits[0::2] = np.real(values) < 0
    bits[1::2] = np.imag(values) < 0
    return bits

