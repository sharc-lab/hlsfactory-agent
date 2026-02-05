# Stage 2: Extract Design

## Task

Extract the HLS design "{{DESIGN_NAME}}" from the source repository into a clean, standalone folder.

## Input

- Source repository: `{{SOURCE_REPO}}`
- Design information from `{{OUTPUT_DIR}}/designs.json`
- Target design: `{{DESIGN_NAME}}`

## Instructions

1. **Read the design info**: Load `{{OUTPUT_DIR}}/designs.json` and find the entry for "{{DESIGN_NAME}}".

2. **Create output directory**: Create `{{OUTPUT_DIR}}/{{DESIGN_NAME}}/`

3. **Copy source files**: Copy all source files listed for this design:
   - Maintain a flat structure (all files in the design folder)
   - If there are naming conflicts, rename with a prefix
   - Update `#include` paths if necessary to reflect the new flat structure

4. **Rename for clarity** (if needed):
   - Main kernel file should be named `<kernel_name>.cpp`
   - Main header should be named `<kernel_name>.h`
   - Keep other helper files with their original names

5. **Fix include paths**: Update any `#include` statements to work with the new flat structure:
   - Change `#include "../common/header.h"` to `#include "header.h"`
   - Change `#include "subdir/file.h"` to `#include "file.h"`

6. **Do NOT copy testbench files yet** - that's handled in Stage 3.

## Output Structure

```
{{OUTPUT_DIR}}/{{DESIGN_NAME}}/
├── <kernel_name>.cpp     # Main kernel implementation
├── <kernel_name>.h       # Header file (if exists)
└── <other_helpers>.cpp   # Any additional helper files
```

## Verification

After extraction, verify:
- All necessary source files are present
- Include paths are updated correctly
- No broken includes remain

## Notes

- Preserve all HLS pragmas exactly as they are
- Preserve all comments
- Only modify include paths, nothing else at this stage
