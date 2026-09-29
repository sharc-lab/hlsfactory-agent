# How XLS[cc] Lowers C++ into XLS IR

Paths are relative to `xls/xls/contrib/xlscc/` unless noted. Line numbers were checked on 2026-09-29.

## The core mental model

> **xlscc walks the Clang syntax tree like an interpreter. Instead of computing values, it builds IR nodes. Every C++ variable is just a pointer to "the IR node holding its latest value."**

Most of the lowering code is in `translator.cc` (~7,700 lines). Type classes are in `translator_types.h`.

---

## 1. Types: C++ type → IR type

`TranslateTypeFromClang` (`translator.cc:6932`) does the mapping.

| C++ | IR |
|---|---|
| `short` / `int` / `long`, `long long` | `bits[16]` / `bits[32]` / `bits[64]`. Signedness is recorded on the side and picks the op (`sdiv` vs `udiv`, etc.) |
| `bool` | `bits[1]` |
| `char` | `bits[8]` |
| `XlsInt<W,S>` (via `ac_int`) | `bits[W]` |
| struct / class | tuple of its fields |
| `T arr[N]` | IR array `T[N]` (a value, **not** RAM) |
| `__xls_channel`, `__xls_memory` | not data: they become I/O operations |
| **`float` / `double`** | **Compile-time constants only** |

**Floats are a major gotcha.**
- Float math only works when Clang can evaluate it as a constant (`EvaluateNumericConstExpr`, `translator.cc:5143`).
- Runtime float operations have no IR op mapping, so they fail with "Binary operators unimplemented for type" (`XLSOpcodeFromClang`, `translator.cc:1861`; error at `translator.cc:1971`).
- The only float conversion allowed is to a 64-bit signed int. See tests `FloatConvertToInt` and `ConstexprBinaryOperatorsForFloat` (`unit_tests/translator_logic_test.cc:5040-5085`).
- Vitis code with runtime float math needs another representation, such as `XlsFixed` / `ac_fixed`.

---

## 2. Variables and assignment

- The translator keeps a map from each variable to its current IR node: `context().variables`, part of `TranslationContext` (`translator.h:133`).
- An assignment doesn't write anything. It just points the map entry at a new node.

```c++
int a = x + 1;   // variables[a] → add.1
a = a * 2;       // variables[a] → umul.2   (add.1 still exists, unchanged)
```

That's how C++'s mutable variables become SSA for free.

---

## 3. `if`/`else` → muxes (`sel`)

There are no branches in the IR. Both sides of an `if` are always translated. When a scope closes, any variable that changed inside it gets merged with a select.

```c++
int y = 0;
if (c) { y = a; } else { y = b; }
// → y = sel(c, cases=[b, a])      i.e.  c ? a : b
```

- The if handling (`translator.cc:6527-6558`) pushes a new context carrying the condition (and `!cond` for the else).
- On scope exit, `PropagateVariables` (`translator.cc:295`) calls `PrepareRValueWithSelect` (`translator.cc:2257`) to build the `sel`.
- `return`, `break` and `continue` work the same way. They don't jump anywhere. They set a condition that turns every later assignment into a no-op (`translator.cc:6583-6620`).
- `if constexpr` is evaluated at compile time; only the taken branch is translated (`translator.cc:6528-6533`).

**Hardware cost:** both branches are always built. An `if` chooses a result; it doesn't skip work.

---

## 4. Function calls → separate IR functions + `invoke`

- **Each C++ function becomes its own XLS function,** called with an `invoke` op. `opt_main` inlines them later.
- **Reference parameters and `this` are copy-in/copy-out.** IR values are immutable, so a modified reference argument becomes an extra return value that the caller writes back (`translator.cc:1700`; see the comment at `translator.cc:3655`).
  - This causes an aliasing quirk: passing `&arr[0]` and `&arr[1]` to the same call gives different results than native C++ (test `Aliasing` in `unit_tests/translator_pointer_test.cc`).
- **Recursion is an error** (`translator.cc:4289`). All calls have to be flattened into hardware.
- **Assigning to globals is an error** (`translator.cc:2313`). Use function-local `static` or class members for state.

---

## 5. `read()` / `write()` → predicated I/O ops

- When the translator sees a channel call (`translate_io.cc:786-870`), it records an **`IOOp`** (`translator_types.h:940`).
- The op's **predicate** is the current `if` condition. So a `read()` inside an `if` becomes "receive only when the condition is true." This is the same condition mechanism as in section 3.

### Slices and continuations

Side effects also cut the function into pieces. From `translator_types.h:1176`:

> "When there are side-effecting operations in a C++ function, N+1 XLS IR functions will be generated, where N is the number of side-effecting operations."

- **Slice** = one of those pieces (`GeneratedFunctionSlice`, `translator_types.h:1180`).
- **Continuation** = a value passed from one slice to a later one (`ContinuationValue` / `ContinuationInput`, `translator_types.h:1102`, `:1165`).
- `generate_fsm.cc` assembles the slices into a state machine inside the proc.
- This is how multi-cycle behavior appears: pipelined loops, multiple reads on the same channel, activation barriers.
- This is the most complex part of XLS[cc], and the part that decides throughput. **To study next.**

---

## 6. C++ features that fail (verified in code)

| Construct | Where it's rejected |
|---|---|
| Runtime `float`/`double` math | `translator.cc:1971` |
| Recursion | `translator.cc:4289` |
| Assigning to global variables | `translator.cc:2313` |
| `nullptr` | `translator.cc:6963` |
| C++17 `if (init; cond)` | `translator.cc:6539` |
| Pointer-to-pointer, uninitialized pointers | `unit_tests/translator_pointer_test.cc` |
| No standard library | `-nostdinc` (`cc_parser.cc`); only the stubs in `synth_only/` |

---

## Implications for Vitis → XLS translation

1. **Pre-translation checks:** flag runtime float math, recursion, global writes and pointer tricks before sending code to xlscc. Each one is a hard failure.
2. **Muxes, not branches:** heavy branching costs more area in XLS than you might expect, because every branch is always built.
3. **Throughput:** the slice → FSM machinery (section 5) decides it. Understand that next if performance parity with Vitis matters.

## Related notes

- `pragma_notes.md`: every XLS[cc] pragma and its Vitis mapping.
