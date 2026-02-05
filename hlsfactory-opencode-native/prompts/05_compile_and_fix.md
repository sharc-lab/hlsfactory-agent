# Stage 5: Compile and Fix

## Task

Compile the HLS design at `{{DESIGN_DIR}}` with clang++ and fix any compilation errors.

## Instructions

### Step 1: Initial Compilation

Run the following command to compile all C++ files:

```bash
cd {{DESIGN_DIR}}
clang++ -c -std=c++17 -Wall *.cpp 2>&1
```

Note: We use `-c` to compile to object files without linking (no main function conflict).

### Step 2: Analyze Results

**If compilation succeeds** (no errors):
- Log "Compilation successful" to `{{DESIGN_DIR}}/compile_log.txt`
- Task complete

**If compilation fails**:
- Parse the error messages
- Identify the type of error:
  - Missing headers
  - Syntax errors
  - Type mismatches
  - Undefined references
  - HLS-specific type issues (ap_int, ap_fixed, hls::stream)

### Step 3: Fix Errors

For each error, apply appropriate fixes:

#### Missing Standard Headers
```cpp
// Add missing includes
#include <cstdint>    // for uint32_t, etc.
#include <cstring>    // for memcpy, memset
#include <cstdlib>    // for abs, etc.
```

#### Missing HLS Headers (stub them out for clang)
If HLS-specific headers are missing (`ap_int.h`, `hls_stream.h`), create stub headers:

**Create `{{DESIGN_DIR}}/ap_int.h`:**
```cpp
// Stub for Xilinx ap_int.h - for compilation testing only
#ifndef AP_INT_H
#define AP_INT_H
#include <cstdint>
template<int W> using ap_int = int64_t;
template<int W> using ap_uint = uint64_t;
#endif
```

**Create `{{DESIGN_DIR}}/ap_fixed.h`:**
```cpp
// Stub for Xilinx ap_fixed.h - for compilation testing only
#ifndef AP_FIXED_H
#define AP_FIXED_H
template<int W, int I> using ap_fixed = double;
template<int W, int I> using ap_ufixed = double;
#endif
```

**Create `{{DESIGN_DIR}}/hls_stream.h`:**
```cpp
// Stub for Xilinx hls_stream.h - for compilation testing only
#ifndef HLS_STREAM_H
#define HLS_STREAM_H
#include <queue>
namespace hls {
    template<typename T>
    class stream {
        std::queue<T> data;
    public:
        void write(const T& val) { data.push(val); }
        T read() { T v = data.front(); data.pop(); return v; }
        bool empty() { return data.empty(); }
    };
}
#endif
```

#### Syntax Errors
- Fix typos, missing semicolons, unmatched braces
- Be careful to preserve the original logic

#### Type Mismatches
- Add necessary casts
- Fix function signatures to match calls

#### Undefined Functions
- If a function is declared but not defined, add a stub implementation
- Mark stubs clearly with comments

### Step 4: Retry Compilation

After making fixes:
1. Run `clang++ -c -std=c++17 -Wall *.cpp 2>&1` again
2. If still errors, repeat Step 3
3. **Maximum 3 attempts total**

### Step 5: Log Results

Create `{{DESIGN_DIR}}/compile_log.txt` with:
```
Compilation Log for {{DESIGN_NAME}}
===================================

Attempt 1:
[compiler output]
[fixes applied]

Attempt 2:
[compiler output]
[fixes applied]

...

Final Status: SUCCESS / FAILED
```

### Step 6: Handle Unrecoverable Errors

If after 3 attempts compilation still fails:
1. Log all errors to `compile_log.txt`
2. Add `COMPILATION_FAILED` marker file to the design directory
3. Document what fixes were attempted

## Important Rules

1. **Minimal changes**: Only fix what's necessary for compilation
2. **Preserve functionality**: Don't change algorithm or logic
3. **Document changes**: Comment any code you add or modify
4. **Don't delete code**: Comment out problematic code rather than deleting
5. **HLS pragmas**: Leave all `#pragma HLS` directives unchanged

## Common HLS Compilation Issues

| Error | Typical Fix |
|-------|-------------|
| `ap_int.h not found` | Create stub header |
| `hls/stream.h not found` | Create stub header |
| `undefined reference to main` | Using `-c` flag avoids this |
| `multiple definition` | Check for duplicate includes |
| `incomplete type` | Add forward declarations or include headers |

## Output

- Fixed source files (in place)
- `{{DESIGN_DIR}}/compile_log.txt` - Compilation log
- Optional stub headers if needed
