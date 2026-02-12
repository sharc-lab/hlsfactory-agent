# Stage 1: Analyze Repository

## Task

Analyze the HLS source repository at `{{SOURCE_REPO}}` and identify all distinct HLS designs/kernels.
You MUST create/overwrite `{{OUTPUT_DIR}}/designs.json` with valid JSON. Do not ask questions. Do not provide a narrative response.
If you cannot write the file, output a single line: `FAILED: could not write designs.json` and nothing else.

## Instructions

1. **Scan the repository**: Look at all `.c`, `.cpp`, `.h`, `.hpp` files in the directory tree.

2. **Identify HLS kernels**: Find functions that appear to be HLS top-level kernels. Look for:
   - Functions with HLS pragmas (`#pragma HLS`)
   - Functions that process arrays/streams in a synthesizable manner
   - Functions documented as "top" or "kernel" functions
   - Functions referenced in any TCL files as top-level

3. **Map dependencies**: For each kernel, identify which source files it depends on:
   - The file containing the kernel function
   - Header files it includes
   - Files containing helper functions it calls
   - DO NOT include testbench files in the kernel dependencies

4. **Identify testbenches**: Note any testbench files (containing `main()`, file I/O, printf) and which kernel they test.

5. **Output**: Create a file `{{OUTPUT_DIR}}/designs.json` with this structure:

```json
{
  "source_repo": "{{SOURCE_REPO}}",
  "designs": [
    {
      "kernel_name": "name_of_kernel_function",
      "design_name": "human_readable_design_name",
      "source_files": [
        "relative/path/to/kernel.cpp",
        "relative/path/to/header.h"
      ],
      "testbench_files": [
        "relative/path/to/tb.cpp"
      ],
      "top_function": "kernel_function_name"
    }
  ]
}
```

**Strict requirements for designs.json**
- Must be valid JSON (no comments, no trailing commas, no extra text)
- Keys must be exactly: `source_repo`, `designs`, `kernel_name`, `design_name`, `source_files`, `testbench_files`, `top_function`
- Overwrite any existing file at that path
- Before finishing, validate the JSON using:
  - `python -m json.tool "{{OUTPUT_DIR}}/designs.json"`

## Important Notes

- Be thorough - scan ALL directories, not just top-level
- A repository may contain multiple independent designs
- Some files may be shared between designs (common headers)
- Exclude build artifacts, object files, and generated files
- If unsure whether something is a kernel, include it (better to over-identify)

## Commands to Use

Use file searching and reading commands to explore the repository:
- List directories and files
- Read source files to understand their purpose
- Look for TCL files that might specify top-level functions
- Check for README or documentation files
- Use file-writing tools to save `{{OUTPUT_DIR}}/designs.json`

## Output Location

Save the analysis to: `{{OUTPUT_DIR}}/designs.json`
