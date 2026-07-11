from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class OfdmConfig:
    """Configuration for the small reference resource grid."""

    fft_size: int = 64
    cyclic_prefix: int = 16
    pilot_spacing: int = 8
    evm_limit: float = 0.08
    ber_limit: float = 0.0

    def __post_init__(self) -> None:
        if self.fft_size <= 0 or self.fft_size % 2 != 0:
            raise ValueError("fft_size must be a positive even number")
        if not 0 < self.cyclic_prefix < self.fft_size:
            raise ValueError("cyclic_prefix must be between zero and fft_size")
        if self.pilot_spacing <= 0:
            raise ValueError("pilot_spacing must be positive")

    @property
    def pilot_indices(self) -> tuple[int, ...]:
        return tuple(range(0, self.fft_size, self.pilot_spacing))

    @property
    def data_indices(self) -> tuple[int, ...]:
        pilots = set(self.pilot_indices)
        return tuple(index for index in range(self.fft_size) if index not in pilots)

    @property
    def data_bit_count(self) -> int:
        return 2 * len(self.data_indices)


@dataclass(frozen=True)
class FrameResult:
    scenario: str
    noise_std: float
    channel_gain_estimate_real: float
    channel_gain_estimate_imag: float
    bit_errors: int
    bit_count: int
    ber: float
    evm_rms: float
    passed: bool

    def as_dict(self) -> dict[str, object]:
        return {
            "scenario": self.scenario,
            "noise_std": self.noise_std,
            "channel_gain_estimate": {
                "real": self.channel_gain_estimate_real,
                "imag": self.channel_gain_estimate_imag,
            },
            "bit_errors": self.bit_errors,
            "bit_count": self.bit_count,
            "ber": self.ber,
            "evm_rms": self.evm_rms,
            "passed": self.passed,
        }


@dataclass(frozen=True)
class RequirementResult:
    requirement_id: str
    description: str
    passed: bool
    detail: str


@dataclass(frozen=True)
class ValidationSummary:
    scenario: str
    results: tuple[RequirementResult, ...]

    @property
    def total_requirements(self) -> int:
        return len(self.results)

    @property
    def passed_requirements(self) -> int:
        return sum(result.passed for result in self.results)

    @property
    def failed_requirements(self) -> int:
        return self.total_requirements - self.passed_requirements

    @property
    def passed(self) -> bool:
        return self.failed_requirements == 0

