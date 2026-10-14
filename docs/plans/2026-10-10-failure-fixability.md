# Which census failures are fixable

Scope: the losses in `docs/RUN_STATISTICS.md` section 2 (second pass, 15 repos, 140 designs). Evidence: the run
records, the Catapult 2026.2 install (`$MGC_HOME/shared/include/ac_math`, the reference in
`shared/pdfdocs/js/ccsDoc.json`), and the papers listed at the end. Ordered by expected yield per effort.

## Translation side

| loss | designs | cause | fix | evidence | effort |
|---|---|---|---|---|---|
| fence violations | 20 | the agent rewrites lines outside the residue list, often cosmetically; attempts move fence to fence | a mechanical **fence repair** after each attempt: restore the pre-pass text on fenced lines, re-run the checks, and only then spend another agent attempt; plus a one-line preservation clause in the prompt | over-editing is the dominant failure of LLM repair; one preservation clause cut excess edit distance by about a third (minimal-edit fidelity study); separating localization from editing reduces stray edits (cascaded code editing); block-level edit formats beat full-file rewrites | small, no API cost |
| dialect leftovers | 6 | `hls_math.h` left in a header; a helper header the agent wrote without `<ac_std_float.h>`; a float LUT | two pre-pass rules (header map entry, required include for any file using the float types) | run records | small |
| translated testbench mismatch | 20 | numeric or control differences after the agent's edits | mismatch localization: diff the two testbench outputs, slice back to the first divergent variable, and give the agent that evidence rather than the whole log | evidence-driven C-to-HLS agent (log normalization, AST backward slicing, dual traces) reports this as the step that makes CSim failures repairable | medium |
| translation does not compile | 4 | agent leaves code that fails clang | already covered by the compile-licensed residue; read case by case | | small |

## Catapult side

| loss | designs | cause | fix | evidence | effort |
|---|---|---|---|---|---|
| `constexpr` function with statements (CRD-2388) | 5 (all of SERI) | Catapult defaults to C++11, which allows only a single `return` in a `constexpr` function | `option set Input/CppStandard c++14` in `run.tcl`; C++14 relaxed `constexpr` is documented as supported | Catapult reference: "By default, Catapult supports C++11. To enable C++14 support, set Input/CppStandard"; allowed values c++98 to c++20 | one line |
| float math (`exp`, `log`, `pow`) | 4 | no synthesizable float math; today only sin/cos/sqrt are shimmed | extend the float shim with `ac_exp_pwl`, `ac_log2_pwl` (log = log2 * ln2), `ac_pow2_pwl`; also `ac_tanh_pwl`, `ac_sigmoid_pwl`, `ac_softmax_pwl` exist | `ac_pow_pwl.h` 3.4.2: "Added floating point support for ac_pow2_pwl() and ac_exp_pwl()"; `ac_log_pwl.h` has an `ac_std_float` overload; all in the install | small to medium; piecewise-linear accuracy must be checked against the testbench tolerance |
| unsized arrays (CIN-84) | 5 | `T x[]` with a runtime length the testbench does not pass as a named array (heap buffer, length variable) | `directive set /top/<arg> -ARRAY_SIZE <n>` from the largest size the testbench allocates, or a stated upper bound; the size rule today only edits the source | Catapult reference, ARRAY_SIZE: "specifies the initial number of elements of each array on the interface"; default 1024 | small, but the bound is a design decision when nothing in the testbench gives it |
| memory ports (SCHD-4) | 3 (Spector) | recovery fired but did not solve it: `histogram` needs 64 ports for data-dependent `bins[data[i]]` in an unrolled loop; `sobel_x/y` read 3 rows of a 2-D image per cycle and the step interleaved the flat array by 2 | histogram: relax the unroll (or II) instead of splitting memory, since no partition helps a data-dependent index; sobel: split by row (`BLOCK_SIZE` = row length, the existing 2-D rule) rather than interleave, which the step chose because it read the dims of the wrong declaration | run logs (`64 are needed, but only 2`; `input_image:rsc(0)(0)`) | small, inside `catapult_degrade_step` |
| pointer casts (CIN-71) | 3 | `int*` reinterpreted as `ac_int*` or similar | rewrite to slice/assign through `ac_int::set_slc`/`slc` on the wider word; mechanical only for the common "cast a word array to a narrower element view" shape | Catapult reports the cast as unsupported; no directive | medium; likely agent work with a worked example in the prompt |
| other | 9 | mostly SERI's constexpr (above) once classified; a few one-offs | add CRD-2388 to the failure classes so the report names it | | trivial |

## Extraction side (upstream, HLSFactory-Agent)

| loss | designs | cause | fix | effort |
|---|---|---|---|---|
| testbench does not link / build | 55 (balor 40) | extraction wrote a testbench whose call signature does not match the kernel, or copied an incomplete folder | build and run the testbench before saving a design (a gate in the extraction agent); our side can only record it | upstream |
| broken Vitis script | 28 | unexpanded `${project_name}` variables (fpgaconvnet 26), a wrong `set_top`, an option 2024.1 does not know | normalize `synth.tcl` the way `repair_top` already does: substitute variables, repair the top, drop unknown options | small, ours |
| testbench reports a mismatch | 20 | fpgaconvnet compares against golden `.dat` files that do not correspond to the extracted parameter header (wrong values from the first output on) | regenerate goldens upstream; otherwise the original has no reference and the translated testbench is compared against nothing | upstream |
| missing include | 11 | include tree not copied, or its path not carried into the script | copy the include tree and add `-I` paths to `add_files -cflags` | upstream, or ours in the script normalizer |
| Vitis timeout | 12 | 600 s budget on large NTT and sparse kernels | raise the source budget to 2400 s like the target | one flag |
| source rejected by Vitis | 11 | genuine (scalar-port address arithmetic, TLS, undeclared identifiers from a trimmed header) | none on our side | |

## What it would buy

On the 140 designs: fence repair is the largest single lever (20 designs past the checks, a share of which then
synthesize at the running ~67% rate); the C++14 option, the exp/log shims, ARRAY_SIZE and the two degrade-step
corrections are each small and together target about 17 Catapult losses. A reasonable expectation is 60 -> 75 to
85 Catapult passes on this half of the census before the remaining 10 repos are counted. The Vitis script
normalizer plus a 2400 s budget would turn about 40 of the 62 Vitis losses into passes, which matters for the
paper's "synthesizes in both" figure, not for the Catapult count.

## Implemented 2026-10-13/14 (fixes 1-7, 9, 10; uncommitted)

Validated on recorded census2 outputs, no agent (credits exhausted); `runs/fixtest*` hold the probes.

| fix | what changed | validation |
|---|---|---|
| 1 C++14 | `run.tcl` sets `Input/CppStandard c++14`; CRD-2388 is its own failure class | SERI k_preparation, k_rr_a: CRD-2388 gone; both now stop at CIN-71 pointer casts (fix 9) |
| 2 fence repair | after a failed attempt, fenced hunks go back to the pre-pass text and the checks rerun; kept if no check gets worse. A line-for-line replace is judged per line; testbench-only headers are not fenced. One preservation sentence in the prompt | replay over the 38 census2 designs whose final attempt had fenced edits: 5 pass outright (AutoNTT_I BARRETT BUs_16, cnn, k_gaussian_quadrature_a/b, pool). Most others are coordinated rewrites that stop compiling when restored. fpgaconvnet's 20 are an extraction defect: no `testbench.cpp`, and the kernel includes its own testbench header |
| 3 recovery step | an already-split array that is still short gets II x ceil(needed/have) on the pipelined loops that index it; the `resource_directive` proc is added when `run.tcl` lacks it | Catapult: histogram (interleave, then II x4), sobel_subdimx (II x2), piMonteCarlo (proc fix) all PASS |
| 4 unsized arrays | sizing extended to pointer ports, `x[/*a * b*/]`, `malloc(sizeof(T) * n)`. A source edit, not the ARRAY_SIZE directive: exact per port, and the same type after decay | all 5 CIN-84 designs size from their testbench; MurmurHash3 PASS, piMonteCarlo PASS after recovery |
| 5 float math | `ac_float_exp.h` shim: exp/log/pow on hyperbolic CORDIC (`ac_hcordic.h`), ~2e-7 worst error. The 4 "float-op" losses were really CIN-290, from native float the agent reintroduced, so `.to_float()`/`.to_double()`/`(float)` in the DUT is now a check failure | shim compiled and checked against libm in the image; CIN-290 check catches fir and cholesky; fft_fixed and twiddles8 (native `2.0 * PI / len`) are still uncaught |
| 6 leftovers | `hls_math.h` -> `<cmath>`; struct/class channel members are not made static or flagged; copied Vitis library headers are not copied; required includes are inserted after each attempt and ignore comments | unit tests; the AutoNTT member channels, AutoNTT_I's `ap_fixed.h` copy, and cnn/k_compress_store's missing include are the cases |
| 7 Vitis script | `normalize_vitis_script`: unset Tcl variables set to the design name, `-I[pwd]/<dir>` only for folders that resolve an otherwise unresolvable include, `-DHLSLIB_SYNTHESIS` with hlslib, removed options dropped; source budget 2400 s | Vitis: fpgaconvnet accum, conv and fork PASS (were 0/26 on `${project_name}`); SERI, spmv and MurmurHash3 pass the script stage and stop at genuine source errors |
| 9 pointer casts | `cat_bytes.h` shim: typed `cat_memcpy`/`cat_memset0`/`cat_store`/`cat_load` over `ac_int`/`ac_fixed`/`ac_ieee_float` slices; the pre-pass maps `memcpy`/`memset`/`reinterpret_cast` stores and loads to them and templates `void*` parameters; any leftover is a check failure | SERI (5): CIN-71 gone, testbenches pass. k_rr_a then needs II 7 plus a 6-way split of `int4d_dmbn`; it schedules but times out at 1200 s in netlisting. k_preparation stops at CIN-84 (testbench never calls the top), k_recurrence_relations/k_rr_b at CRD-416/42 in agent code |
| 10 dataflow macros | a macro whose body is an HLS dataflow pragma marks a region; stages are found across files and must return void; ASM-59 classed with ASM-62 | k_buffer_permutation: HIER-6 -> HIER-11 -> ASM-59/62 (blocks called inside a `for` loop in the top); that loop is agent work, now spelled out in the residue note |

## Sources

- Evidence-Driven LLM Agent for C-to-Synthesizable-C Conversion and Verification, arXiv 2606.28409.
- When Models Edit Too Much: On the Fidelity of Minimal Code Edits, arXiv 2609.04061; minimal-patch summary at co-r-e.com/method/minimal-edit-fidelity.
- To Diff or Not to Diff? Structure-Aware and Adaptive Output Formats for LLM-based Code Editing, arXiv 2604.27296.
- Cascaded Code Editing: Large-Small Model Collaboration, arXiv 2604.19201.
- FPGAgent, arXiv 2608.23630; HLS-Eval, arXiv 2504.12268; HLStrans, arXiv 2507.04315.
- Catapult 2026.2 reference (ARRAY_SIZE directive, Input/CppStandard option) and `ac_math` headers in the install.
