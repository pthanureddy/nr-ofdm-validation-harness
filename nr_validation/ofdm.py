from __future__ import annotations

from typing import Final

import numpy as np

from .modulation import qpsk_demodulate, qpsk_modulate
from .models import FrameResult, OfdmConfig

PILOT_SYMBOL: Final[complex] = 1.0 + 0.0j


def build_resource_grid(bits: np.ndarray, config: OfdmConfig) -> np.ndarray:
    values = np.asarray(bits, dtype=np.int8)
    if values.size != config.data_bit_count:
        raise ValueError(f"expected {config.data_bit_count} bits, received {values.size}")

    grid = np.empty(config.fft_size, dtype=np.complex128)
    grid[list(config.pilot_indices)] = PILOT_SYMBOL
    grid[list(config.data_indices)] = qpsk_modulate(values)
    return grid


def add_cyclic_prefix(time_domain: np.ndarray, prefix_length: int) -> np.ndarray:
    values = np.asarray(time_domain, dtype=np.complex128)
    if not 0 < prefix_length < values.size:
        raise ValueError("prefix_length must be between zero and the symbol length")
    return np.concatenate((values[-prefix_length:], values))


def remove_cyclic_prefix(received: np.ndarray, prefix_length: int) -> np.ndarray:
    values = np.asarray(received, dtype=np.complex128)
    if values.size <= prefix_length:
        raise ValueError("received symbol is shorter than the cyclic prefix")
    return values[prefix_length:]


def _complex_noise(size: int, noise_std: float, seed: int) -> np.ndarray:
    if noise_std < 0:
        raise ValueError("noise_std cannot be negative")
    generator = np.random.default_rng(seed)
    return noise_std * (
        generator.normal(size=size) + 1j * generator.normal(size=size)
    ) / np.sqrt(2.0)


def run_frame(
    bits: np.ndarray,
    *,
    config: OfdmConfig | None = None,
    channel_gain: complex = 0.92 + 0.03j,
    noise_std: float = 0.01,
    seed: int = 7,
    scenario: str = "nominal",
) -> FrameResult:
    """Transmit and receive one deterministic OFDM frame through a flat channel."""

    cfg = config or OfdmConfig()
    transmitted_bits = np.asarray(bits, dtype=np.int8)
    grid = build_resource_grid(transmitted_bits, cfg)
    time_domain = np.fft.ifft(grid)
    transmitted = add_cyclic_prefix(time_domain, cfg.cyclic_prefix)

    received = channel_gain * transmitted + _complex_noise(transmitted.size, noise_std, seed)
    received_time = remove_cyclic_prefix(received, cfg.cyclic_prefix)
    received_grid = np.fft.fft(received_time)

    pilot_values = received_grid[list(cfg.pilot_indices)]
    channel_estimate = np.mean(pilot_values / PILOT_SYMBOL)
    received_data = received_grid[list(cfg.data_indices)] / channel_estimate
    transmitted_data = grid[list(cfg.data_indices)]
    recovered_bits = qpsk_demodulate(received_data)

    bit_errors = int(np.count_nonzero(recovered_bits != transmitted_bits))
    ber = bit_errors / transmitted_bits.size
    evm_rms = float(
        np.sqrt(np.mean(np.abs(received_data - transmitted_data) ** 2))
        / np.sqrt(np.mean(np.abs(transmitted_data) ** 2))
    )
    passed = ber <= cfg.ber_limit and evm_rms <= cfg.evm_limit

    return FrameResult(
        scenario=scenario,
        noise_std=noise_std,
        channel_gain_estimate_real=float(channel_estimate.real),
        channel_gain_estimate_imag=float(channel_estimate.imag),
        bit_errors=bit_errors,
        bit_count=int(transmitted_bits.size),
        ber=ber,
        evm_rms=evm_rms,
        passed=passed,
    )

