from nr_validation.models import OfdmConfig
from nr_validation.validation import run_scenario, run_validation_suite


def test_nominal_scenario_passes() -> None:
    result = run_scenario("nominal")
    assert result.passed
    assert result.ber == 0.0


def test_noisy_scenario_is_rejected() -> None:
    result = run_scenario("noisy")
    assert not result.passed
    assert result.evm_rms > OfdmConfig().evm_limit


def test_validation_suite_summary() -> None:
    summary = run_validation_suite()
    assert summary.total_requirements == 4
    assert summary.passed_requirements == 4
    assert summary.failed_requirements == 0
    assert summary.passed

