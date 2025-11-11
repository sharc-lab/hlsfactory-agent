import argparse
import os
import shutil
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from pprint import pp
from typing import TypeVar

from dotenv import load_dotenv
from joblib import Parallel, delayed
from llm import Model
from llm_openrouter import OpenRouterChat
from pydantic import BaseModel, ConfigDict

"""
High-level Synthesis (HLS) repository analyzer.

Builds an OpenRouter-backed LLM client, sends selected repository files to the
LLM to identify top-level designs, and extracts sub-components per design.

Prompt size is constrained by file-type allowlist, skip-dirs, and size limits.
These limits can be overridden via CLI flags.
"""

T_unwrap = TypeVar("T_unwrap")

# Prompt-size controls
ALLOWED_SUFFIXES: set[str] = {
    ".cpp",
    ".c",
    ".cc",
    ".cxx",
    ".h",
    ".hpp",
    ".hxx",
}
SKIP_DIR_NAMES: set[str] = {
    ".git",
    ".svn",
    ".hg",
    "build",
    "cmake-build",
    "out",
    "dist",
    "venv",
    ".venv",
    "node_modules",
    "third_party",
    "external",
    ".cache",
}
# Roughly ~40k tokens if chars/4; adjust as needed
MAX_PROMPT_CHARS: int = 150_000
# Prevent any single giant file from blowing the budget
MAX_FILE_BYTES: int = 100_000


def unwrap(x: T_unwrap | None, err_msg: str | None = None) -> T_unwrap:
    """
    Return x if not None, otherwise raise ValueError with an optional message.
    """
    if x is None:
        # raise ValueError("Unwrapped a None Value")
        if err_msg is None:
            raise ValueError("Unwrapped a None Value")
        else:
            raise ValueError(f"Unwrapped a None Value: {err_msg}")

    return x


def build_model(model_id: str, openrouter_key: str) -> Model:
    """
    Construct an OpenRouterChat model bound to the given model_id and key.
    """
    llm: OpenRouterChat = OpenRouterChat(
        model_id=model_id,
        key=openrouter_key,
        api_base="https://openrouter.ai/api/v1",
        headers={"HTTP-Referer": "https://llm.datasette.io/", "X-Title": "LLM"},
        supports_schema=True,
    )
    return llm  # type: ignore


class HLSDesign(BaseModel):
    """
    A single top-level HLS kernel and its dependent source files.
    """
    model_config = ConfigDict(extra="forbid")

    kernel_name: str
    source_files: list[str]


class HLSDesigns(BaseModel):
    """
    Collection of discovered HLS designs.
    """
    model_config = ConfigDict(extra="forbid")

    designs: list[HLSDesign]


def extract_top_level_designs(src_dir: Path, llm: Model) -> HLSDesigns:
    """
    Build a prompt from filtered repository files and ask the LLM to enumerate
    top-level HLS kernels and their dependent source files.
    """
    prompt_system = """
    You are an AI assistant and high-level synthesis (HLS) design expert that analyzes a collection of unstructured High-Level Synthesis (HLS) source code files.
    
    Your task is to:
    1. Identify kernels: Find all top-level HLS designs or kernels
    2. Determine dependencies: For each identified kernel, list all source files it depends on, including files where it includes functions, classes, or constructs.
    3. Output format: Provide the results in a JSON array with the structure defined below.

    You will output a JSON array with the following structure:
    ```json
    [
        {
            "kernel_name": "<kernel_name>",
            "source_files": ["<file1.cpp>", "<file2.h>", "<file3.cpp>"]
        },
        ...
    ]
    ```

    Additional Guidelines:
    - Multiple kernels may share the same source files; ensure each kernel lists all its dependencies.
    - Only include C/C++ source and header files in `source_files` (.c, .cpp, .cc, .cxx, .h, .hpp, .hxx).
    - If uncertain whether a file is required, include it (prefer false positives over false negatives).
    - Do not include files that are clearly irrelevant (e.g., documentation, configs unrelated to kernels).
    - Ignore files that are not used by any kernel.
    - `source_files` should be a list of relative paths to the files from the provided source directory; do not include files that do not exist.
    """

    prompt_user = ""
    used_chars = 0
    for file_path in src_dir.rglob("*"):
        # Skip directories by name anywhere in the path
        parts = set(file_path.parts)
        if parts & SKIP_DIR_NAMES:
            continue
        if not file_path.is_file():
            continue
        if file_path.suffix.lower() not in ALLOWED_SUFFIXES:
            continue
        try:
            if file_path.stat().st_size > MAX_FILE_BYTES:
                continue
        except OSError:
            continue
        relative_path = file_path.relative_to(src_dir)
        try:
            file_txt = file_path.read_text()
        except Exception:
            continue

        block = f"```{str(relative_path)}\n{file_txt}\n```\n\n"
        # Stop if adding this block would exceed the budget
        if used_chars + len(block) > MAX_PROMPT_CHARS:
            break
        prompt_user += block
        used_chars += len(block)

    r = llm.prompt(
        prompt_user, system=prompt_system, schema=HLSDesigns.model_json_schema()
    )
    r._force()
    r_text = r.text()
    designs = HLSDesigns.model_validate_json(r_text)
    return designs


class SubComponents(BaseModel):
    model_config = ConfigDict(extra="ignore")

    sub_components: list[str]


def break_down_hls_design(design: HLSDesign, src_dir: Path, llm: Model) -> list[str]:
    """
    Given a single design's selected files, ask the LLM to identify synthesizable
    sub-components (non-top-level functions that are part of the kernel).
    """
    prompt_system = """
    You are an AI assistant and high-level synthesis (HLS) design expert.

    Given the HLS source code files, identify all the sub-components in the design.

    A sub-component is a separate C++ function in the code that is not the top-level function and is part of the HLS design.
    This sub-component must be under the hierarchy of the top-level HLS function.
    This can also include sub-functions that are called by other sub-functions.
    A sub-component should be identified by the function name.
    If a sub-component is not listed as a separate C++ function, it should not be listed at all.

    These sub-components must be HLS synthesizable code part of the HLS design.
    We are not interested in testbench functions or runtime functions that are not part of the synthesizable code.
    Therefore, do not include functions that are clearly part of the testbench or runtime support code.

    Things that are NOT sub-components and should NOT be included in the output:
    - OpenCL code for kernel invocation, runtime setup, and teardown functions
    - Testbench functions that call the top-level kernel function
    - Code use to generate data to feed into the kernel
    - Any code that uses `malloc`

    Output the list of sub-components in the design as a json list of strings as shown below.
    ```json
    [
        "subcomponent_1",
        "subcomponent_2",
        ...
    ]
    ```
    """

    prompt_user = ""
    for file_path in design.source_files:
        full_path = (src_dir.resolve() / file_path)
        try:
            if full_path.stat().st_size > MAX_FILE_BYTES:
                continue
            file_txt = full_path.read_text()
        except Exception:
            continue
        prompt_user += f"```{str(file_path)}\n"
        prompt_user += file_txt
        prompt_user += "\n```\n"
        prompt_user += "\n"

    r = llm.prompt(
        prompt_user,
        system=prompt_system,
        schema=SubComponents.model_json_schema(),
    )
    r._force()
    r_text = r.text()
    print("LLM Response Text:")
    print(r_text)
    sub_components = SubComponents.model_validate_json(r_text)
    sub_components_list = sub_components.sub_components
    return sub_components_list


def main(args: argparse.Namespace) -> None:
    src_dir: Path = args.src_dir
    dst_dir: Path = args.dst_dir
    # assert isinstance(src_dir, Path)
    # assert isinstance(dst_dir, Path)
    if not isinstance(src_dir, Path):
        raise ValueError("src_dir is not a valid Path")
    if not isinstance(dst_dir, Path):
        raise ValueError("dst_dir is not a valid Path")

    if not src_dir.exists():
        raise FileNotFoundError(f"Source directory {src_dir} does not exist.")

    if dst_dir.exists():
        shutil.rmtree(dst_dir)
    dst_dir.mkdir(parents=True, exist_ok=False)

    load_dotenv()
    openrouter_key = unwrap(
        os.getenv("OPENROUTER_API_KEY"), "OPENROUTER_API_KEY not found"
    )
    if not isinstance(openrouter_key, str):
        raise ValueError("OPENROUTER_API_KEY is not a valid string")
    if openrouter_key is None:
        raise ValueError("OPENROUTER_API_KEY is None or empty")

    model_id__extract_top_level_designs: str = args.model_id__extract_top_level_designs
    model_id__break_down_hls_design: str = args.model_id__break_down_hls_design

    model__extract_top_level_designs = build_model(
        model_id__extract_top_level_designs, openrouter_key
    )
    model__break_down_hls_design = build_model(
        model_id__break_down_hls_design, openrouter_key
    )

    print(f"Processing source directory to extract top-level designs: {src_dir}")
    designs = extract_top_level_designs(src_dir, model__extract_top_level_designs)
    filtered_design_list: list[HLSDesign] = []
    for design in designs.designs:
        existing_files = [p for p in design.source_files if (src_dir / p).exists()]
        if len(existing_files) != len(design.source_files):
            missing_count = len(design.source_files) - len(existing_files)
            print(
                f"Warning: Dropping {missing_count} non-existent file(s) from '" \
                f"{design.kernel_name}'"
            )
        filtered_design_list.append(
            HLSDesign(kernel_name=design.kernel_name, source_files=existing_files)
        )
    designs = HLSDesigns(designs=filtered_design_list)
    pp(designs)
    (dst_dir / "designs.json").write_text(designs.model_dump_json(indent=4))

    dir_subcomponents = dst_dir / "subcomponents"
    dir_subcomponents.mkdir(parents=True, exist_ok=False)

    def _sanitize_filename(name: str) -> str:
        return name.replace(" ", "_").replace("/", "_")

    def _process_design(idx: int, design: HLSDesign) -> None:
        try:
            print(f"Processing design to extract sub-components: {design.kernel_name}")
            model = model__break_down_hls_design
            sub_components = break_down_hls_design(design, src_dir, model)
            pp(sub_components)
            safe_name = _sanitize_filename(design.kernel_name)
            (dir_subcomponents / f"{idx}__{safe_name}.json").write_text(
                SubComponents(sub_components=sub_components).model_dump_json(indent=4)
            )
        except Exception as e:
            print(f"Error processing design {design.kernel_name}: {e}")

    Parallel(n_jobs=8, backend="threading")(
        delayed(_process_design)(idx, design)
        for idx, design in enumerate(designs.designs)
    )


def parse_args() -> argparse.Namespace:
    """
    CLI for configuring source/destination paths, model IDs, and prompt budgets.
    """
    parser = argparse.ArgumentParser(description="HLSFactory Agent")
    parser.add_argument(
        "src_dir",
        type=Path,
        help="Path to the source directory containing HLS files.",
    )
    parser.add_argument(
        "--dst_dir",
        type=Path,
        default=Path("./HLSDesigns"),
        help="Path to the destination directory for extracted designs.",
    )
    parser.add_argument(
        "--model_id__extract_top_level_designs",
        type=str,
        default="deepseek/deepseek-v3.2-exp",
        # default="qwen/qwen3-coder-30b-a3b-instruct",
        # default="deepseek/deepseek-coder",
        help="Model ID for LLM to use.",
    )
    parser.add_argument(
        "--model_id__break_down_hls_design",
        type=str,
        default="deepseek/deepseek-v3.2-exp",
        help="Model ID for LLM to use.",
    )
    parser.add_argument(
        "--max_prompt_chars",
        type=int,
        default=150_000,
        help="Maximum total characters to include across all files in the discovery prompt.",
    )
    parser.add_argument(
        "--max_file_bytes",
        type=int,
        default=100_000,
        help="Maximum file size (bytes) to include in prompts.",
    )

    args = parser.parse_args()
    return args


if __name__ == "__main__":
    args = parse_args()
    # Allow overriding prompt budgets from CLI
    global MAX_PROMPT_CHARS, MAX_FILE_BYTES
    try:
        MAX_PROMPT_CHARS = int(args.max_prompt_chars)
        MAX_FILE_BYTES = int(args.max_file_bytes)
    except Exception:
        # If invalid values are provided, keep defaults
        pass
    main(args)
