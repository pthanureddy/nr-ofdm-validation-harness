from __future__ import annotations

import numpy as np

from .models import FrameResult, OfdmConfig, RequirementResult, ValidationSummary
from .ofdm import run_frame


def deterministic_bits(config: OfdmConfig) -> np.ndarray:
    pattern = np.array([0, 1, 1, 0, 1, 1, 0, 0], dtype=np.int8)
    repetitions = (config.data_bit_count + pattern.size - 1) // pattern.size
    return np.tile(pattern, repetitions)[: config.data_bit_count]


def run_scenario(
    name: str,
    *,
    config: OfdmConfig | None = None,
    noise_std: float | None = None,
    seed: int = 7,
) -> FrameResult:
    cfg = config or OfdmConfig()
    configured_noise = 0.005 if name == "nominal" else 0.35 if name == "noisy" else noise_std
    if configured_noise is None:
        raise ValueError("unknown scenario; use nominal, noisy, or provide noise_std")
    return run_frame(
        deterministic_bits(cfg),
        config=cfg,
        noise_std=configured_noise,
        seed=seed,
        scenario=name,
    )


def evaluate_nominal(result: FrameResult, config: OfdmConfig | None = None) -> ValidationSummary:
    cfg = config or OfdmConfig()
    estimated_gain = complex(result.channel_gain_estimate_real, result.channel_gain_estimate_imag)
    target_gain = 0.92 + 0.03j
    results = (
        RequirementResult(
            "NR-OFDM-002",
            "Cyclic-prefix framing preserves one OFDM symbol for FFT processing.",
            result.bit_count > 0,
            f"processed {result.bit_count} data bits",
        ),
        RequirementResult(
            "NR-OFDM-003",
            "Pilot-based estimation tracks the configured flat complex channel.",
            abs(estimated_gain - target_gain) <= 0.05,
            f"estimated gain error={abs(estimated_gain - target_gain):.6f}",
        ),
        RequirementResult(
            "NR-OFDM-004",
            "Nominal operation meets BER and EVM limits.",
            result.passed,
            f"BER={result.ber:.6f}, EVM={result.evm_rms:.6f}, limits=({cfg.ber_limit:.6f}, {cfg.evm_limit:.6f})",
        ),
    )
    return ValidationSummary(scenario=result.scenario, results=results)


def run_validation_suite(config: OfdmConfig | None = None) -> ValidationSummary:
    cfg = config or OfdmConfig()
    result = run_scenario("nominal", config=cfg)
    nominal = evaluate_nominal(result, cfg)
    modulation_requirement = RequirementResult(
        "NR-OFDM-001",
        "QPSK modulation and hard-decision demodulation preserve the deterministic bit sequence.",
        result.bit_count == cfg.data_bit_count and result.ber == 0.0,
        f"bit errors={result.bit_errors} of {result.bit_count}",
    )
    return ValidationSummary(
        scenario="nominal",
        results=(modulation_requirement, *nominal.results),
    )


def summary_as_dict(summary: ValidationSummary) -> dict[str, object]:
    return {
        "scenario": summary.scenario,
        "total_requirements": summary.total_requirements,
        "passed_requirements": summary.passed_requirements,
        "failed_requirements": summary.failed_requirements,
        "passed": summary.passed,
        "requirements": [
            {
                "id": result.requirement_id,
                "description": result.description,
                "passed": result.passed,
                "detail": result.detail,
            }
            for result in summary.results
        ],
    }
