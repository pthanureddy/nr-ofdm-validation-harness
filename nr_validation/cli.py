from __future__ import annotations

import argparse
import json
import sys

from .validation import run_scenario, run_validation_suite, summary_as_dict


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Run the NR-inspired OFDM validation harness")
    parser.add_argument(
        "--scenario",
        choices=("nominal", "noisy", "suite"),
        default="nominal",
        help="validation scenario to execute",
    )
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    if args.scenario == "suite":
        payload = summary_as_dict(run_validation_suite())
        passed = bool(payload["passed"])
    else:
        result = run_scenario(args.scenario)
        payload = result.as_dict()
        passed = result.passed if args.scenario == "nominal" else True

    print(json.dumps(payload, indent=2, sort_keys=True))
    return 0 if passed else 1


if __name__ == "__main__":
    sys.exit(main())

