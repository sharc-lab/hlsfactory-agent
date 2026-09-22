from __future__ import annotations

from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Callable


@dataclass
class SynthResult:
    status: str  # PASS | FAILED | TIMEOUT
    latency: int | None = None
    throughput: int | None = None
    area: float | None = None
    first_error: str = ""
    failure_class: str = ""


@dataclass(frozen=True)
class PragmaRule:
    """How one source-tool pragma maps onto the target tool."""

    source: str
    target: str | None
    placement: str
    note: str = ""
    category: str = "translate"


@dataclass(frozen=True)
class TargetSpec:
    name: str
    display_name: str
    include_dir: Path
    include_dir_name: str
    cxx_std_check: str
    header_map: dict[str, str]
    type_map: dict[str, str]
    pragma_rules: tuple[PragmaRule, ...]
    forbidden_patterns: tuple[str, ...]
    required_files: tuple[str, ...]
    driver_file: str
    driver_required_tokens: tuple[str, ...]
    top_marker: str
    extra_rules: tuple[str, ...] = field(default_factory=tuple)
    directive_rules: dict[str, str] = field(default_factory=dict)
    blocking_types: tuple[str, ...] = field(default_factory=tuple)
    # additional header directories copied into the run area as (source_dir, name_in_run_area)
    extra_include_dirs: tuple[tuple[Path, str], ...] = field(default_factory=tuple)
    # synthesis gate: `$NAME` in synth_command expands from synth_env, then the environment
    synth_command: tuple[str, ...] = field(default_factory=tuple)
    synth_timeout_s: int = 1200
    synth_env: dict[str, str] = field(default_factory=dict)
    # files written into the run area before synth_command runs
    synth_files: dict[str, str] = field(default_factory=dict)
    # glob, relative to the run area, that must match for a PASS to count
    synth_success_marker: str = ""
    parse_report: Callable[[Path], SynthResult] | None = None

    @property
    def include_dir_names(self) -> list[str]:
        return [self.include_dir_name] + [name for _, name in self.extra_include_dirs]

    def to_dict(self) -> dict:
        d = asdict(self)
        d["include_dir"] = str(self.include_dir)
        d["extra_include_dirs"] = [[str(p), n] for p, n in self.extra_include_dirs]
        d["parse_report"] = self.parse_report.__name__ if self.parse_report else None
        return d
