# Requirements and Test Traceability

| ID | Requirement | Evidence |
| --- | --- | --- |
| NR-OFDM-001 | The modem shall map an even-length bit sequence to QPSK symbols and recover the original bits through hard-decision demodulation. | `tests/test_modulation.py::test_qpsk_round_trip` |
| NR-OFDM-002 | The OFDM framer shall prepend a cyclic prefix and the receiver shall remove it before FFT processing. | `tests/test_ofdm.py::test_cyclic_prefix_round_trip` |
| NR-OFDM-003 | The receiver shall estimate the configured flat complex channel from known pilot subcarriers and use the estimate for data equalization. | `tests/test_ofdm.py::test_pilot_estimate_tracks_channel` |
| NR-OFDM-004 | The nominal scenario shall meet the configured BER and EVM limits. | `tests/test_validation.py::test_nominal_scenario_passes` |
| NR-OFDM-005 | A deliberately noisy scenario shall be reported as a validation failure rather than being silently accepted. | `tests/test_validation.py::test_noisy_scenario_is_rejected` |
| NR-OFDM-006 | The complete validation suite shall return a structured summary with all requirements passing. | `tests/test_validation.py::test_validation_suite_summary` |

The requirements use a simplified flat-channel model and are not claims about 3GPP conformance. They define the behavior that this repository actually implements.

