# Weekly findings

One section per week: for each design set that went through the pipeline, the stage-by-stage funnel, where
the designs came from, and why each stage lost what it lost. Numbers come from `summary.json` of the runs named.

## Week of 2026-10-05

Pipeline as of 2026-10-07: everything from the previous week plus `#define`-resolved pragma values, testbench-sized
unsized top parameters, `abs`/`sqrt` shims, float literal direct-initialization, the `set_top` repair, and the
unpipeline recovery step; Vitis and Catapult in containers, 2400 s Catapult budget, two recovery rounds.
`run_repo.py --attempts 2 --synth --degrade 2 --synth-timeout 2400 --jobs 3` over the `run_base` repo list.

### Repo census

| source repo | extracted | oracle | Vitis | compiles + matches | checks | Catapult | after recovery | notes |
|---|---|---|---|---|---|---|---|---|
| AlexMontgomerie/fpgaconvnet-hls | 0 | - | - | - | - | - | - | the extraction agent found no standalone design (a generator framework, not a design repo) |
| DARClab-UTD/S2CBench | 22 | 14 | 17 | 18 | 14 | 13 | 13 | rerun with today's rules (first pass was 7 Vitis / 7 Catapult): `set_top` repaired from the testbench call on 10, the 5 Vitis losses are stub testbenches that call nothing (name-prefix fallback added after this run); 3 compile/testbench losses (adpcm, kasumi, md5c), 5 fence violations; the one Catapult loss is cholesky, `1.0 / <ac_ieee_float32>` native arithmetic inside an expression |
| ETHZ-DYNAMO/balor | 40 | 0 | 38 | 35 | 35 | 21 | 21 | oracle 0: every extracted testbench fails to link against its kernel (signature mismatch written by extraction), so the original has no reference and the translated testbench is the agent's; Catapult lost 13 to native float arithmetic surviving the dialect rule (CIN-290, being inspected) and 1 to `cos` without the shim (`twiddles8`, run before the shim include rule caught `.h` files) |
| FedericoSerafini/HLS-CNN | 1 | 1 | 0 | 1 | 0 | - | - | one large CNN; Vitis fails on the extracted script; the agent made 72 edits outside the residue (a rewrite, stopped by the fence) |
| KastnerRG/Spector-HLS | 14 | 13 | 13 | 6 | 1 | 0 | 0 | first pass: 8 testbench failures and 7 fence violations with one cause, the dialect rewrite produced code that does not compile here (non-const `float` tables with literal initializers, int-to-float assignments, a second test program with its own `main`), which the agent then had to edit; three pre-pass rules and the compiler-licensed residue were added from this, rerun queued; template_matching needs `gmp.h` |
| OswaldHe/InTAR | 0 | - | - | - | - | - | - | nothing extracted |
| SFU-HiAccel/AutoNTT | 0 | - | - | - | - | - | - | nothing extracted |
| SFU-HiAccel/BitBlender | 1 | 1 | 0 | 1 | 1 | 0 | 0 | MurmurHash3: unsized `T x[]` parameter the testbench does not pass a named array for, so the size rule has nothing to read (CIN-84) |
| SFU-HiAccel/CHIP-KNN | 2 | 2 | 1 | 2 | 1 | 0 | 0 | knn_fixed_manhattan: pointer cast `int *` to `ac_int *` (CIN-71, unsupported); knn_float_singlepe: 2 fence violations |
| SFU-HiAccel/FORC | 0 | - | - | - | - | - | - | nothing extracted |
| SFU-HiAccel/HiSpMV | 0 | - | - | - | - | - | - | nothing extracted |
| SFU-HiAccel/SyncNN | 3 | 3 | 0 | 2 | 0 | - | - | three CNNs; Vitis fails on an undeclared identifier in the extracted source; fence violations with two causes now ruled: `#pragma SDS` lines (SDSoC) left in place, and `const w_dt v = 1.0;` with a typedef'd float type (explicit constructor) |
| SFU-HiAccel/blaze | 8 | 8 | 7 | 8 | 8 | 0 | 0 | all 8 clear every check; Catapult rejects every one for unsized `double a[/*n*/]` parameters (CIN-84), a comment inside the brackets the size rule did not match (fixed); rerun queued |
| SFU-HiAccel/pasta | 4 | 0 | 0 | 3 | 2 | 1 | 1 | big TAPA/AutoSA kernels; Vitis 2 timeouts and 2 script failures; serpens-32ch left 139 non-static channels because the source declares streams with a name argument `fifo("fifo")` (rule added); spmv-hisparse-mmap 29 fence violations; lu_decompose unsized array parameters whose sizes live in a host program, not the testbench |
| SFU-HiAccel/SERI | 9 | 9 | 0 | 9 | 0 | - | - | all 9 compile and match; Vitis fails on a nested-include path the extraction broke; every design hit the fence on `constexpr double` constants rewritten to a non-literal type (now kept native) and float ternaries, both licensed by the compiler-residue rule since |
| TurakhiaLab/DP-HLS | 0 | - | - | - | - | - | - | nothing extracted |
| UCLA-VAST/CLINK | 2 | 0 | 0 | 2 | 1 | 0 | 0 | two LSTM kernels in C; oracle and Vitis fail on the extracted script; lstm_n: Catapult rejects an array index that is always out of range in the source (a design defect it tolerates in Vitis) |
| UIUC-ChenLab/ScaleHLS-HIDA | 16 | 11 | 11 | 11 | 10 | 0 | 2 | ScaleHLS-generated polybench and CNN kernels with a pragma on every loop; the 5 CNNs (resnet18, vgg16, yolo, zfnet, mobilenet) time out in Vitis and fail the translated testbench; Catapult direct 0 because every kernel's II=1 on a float accumulate fails scheduling, and the recovery loop then carried 3mm (two II relaxations) and bicg (II then interleave) to PASS; the other 8 exhaust two rounds in a chain of II, ports and 40-minute timeouts, so this family wants more rounds or a cheaper first step |
| Xtra-Computing/ThunderGP | 9 | 1 | 0 | 7 | 1 | 0 | 0 | graph-processing kernels; Vitis dies in llvm-link on 6 and the script on 3; casir/spmv: the agent removed the `static` the pre-pass had put on 31 channels each (a regression the check caught); pr: unsized array in a header; fence violations on typedef renames |
| ZongyueQin/ProgSG | 23 | 23 | 2 | 23 | 22 | 1 | 1 | Merlin-style polybench/MachSuite `.c` kernels, the cleanest family yet: every oracle and testbench passes; Vitis times out at 600 s on 18 (heavily pipelined, needs a bigger source budget); Catapult 13 SCHD-20 whose relax-II recovery was a no-op because the step edited only `.cpp` files (fixed), 5 timeouts, 2 ports, 1 unrolled-loop pipeline; atax passes directly |
| bsc-loca/PQC-Crystals-HLS-Accelerators | 2 | 2 | 2 | 2 | 0 | - | - | both fail the static checks on fence violations |
| icl-utk-edu/hpcc | 5 | 5 | 4 | 5 | 5 | 2 | 4 | two direct, two by recovery; the one loss is a library-component shape |

## Week of 2026-09-28

Pipeline as of 2026-09-29: pre-pass (dialect rules, dataflow blocks, dual-port default), Pi agent fenced to the
residue list, static and container checks, Vitis HLS 2024.1 on the source and Catapult 2026.2 on the translation
(both in containers, 2400 s budget), then up to two log-driven recovery rounds. `run.py --synth --degrade 2
--attempts 2 --synth-timeout 2400`.

### HLSFactory built-in sets `vitis_examples` + `pp4fpgas` (run `imp1`, 2026-09-29)

Source: `hlsfactory/hls_dataset_sources/vitis_examples` (AMD's Vitis HLS introductory examples, 23 designs) and
`hls_dataset_sources/pp4fpgas` (Kastner's *Parallel Programming for FPGAs* examples, 5 designs), imported with
`exp/run_translate/import_dataset.py`. First set outside the FFT family.

| stage | gate | passed | lost, and why |
|---|---|---|---|
| 1. Import | sources, one testbench source, a Tcl with `set_top` | 24 / 28 | 3 pp4fpgas folders have no single testbench source (fft, multi-stage fft, merge sort); huffman lists files missing from the folder |
| 2. Vitis synthesis | original synthesizes in Vitis HLS | 23 / 24 | uram_ecc binds storage to URAM, absent on the target part; recorded as metadata, not a filter |
| 3. Translated design compiles | clang on every source in the container | 24 / 24 | none |
| 4. Testbench match | translated testbench output equals the original's | 24 / 24 | none |
| 5. Static checks | required files, no Vitis dialect left, driver complete, fence respected | 20 / 24 | 3 fence violations (agent deleted `<ac_std_float.h>` + the trig shim include, rewrote a coefficient call, edited three lines of the vadd stream loop); 1 genuine residue, `exp()` has no Catapult overload (fixed_point_window_conv) |
| 6. Catapult synthesis | `go extract` completes | 15 / 24 | 2 `%` / `/` on 32-bit integers with no library component (CRAAS-6: ap_arithmetic, uram_ecc); 1 `assert()` (CIN-16, accs_w_asserts; since fixed by `-DNDEBUG`); 2 pointer casts between types (CIN-71: pointer_casting, axi_master; unsupported by Catapult) |
| 7. Recovery | one log-driven retry per round | +0 | none of the 5 is an II or memory-port failure, the two classes the steps cover |

Takeaway: everything that compiles and matches also clears Catapult except for named tool limits. Two of
the nine losses (assert, unrolled-loop pipelining seen on the FFTs) became rules the same day.

### UCLA-VAST/HP-FFT-HLS, 16 designs (runs `r8a`/`r8b`/`r8c`, 2026-09-30)

Source: 16 standardized Vitis designs extracted by HLSFactory-Agent from `UCLA-VAST/HP-FFT-HLS` (1024- and
256-point FFTs at unroll factors 1 to 32, plus no-stage-pipeline and C-style variants). Pipeline as of
2026-09-30: adds `#define`-resolved pragma values, residue lines re-pointed at the output, single-bit `.range`
no longer residue, and the unpipeline recovery step.

| stage | gate | passed | lost, and why |
|---|---|---|---|
| 1. Vitis synthesis | original synthesizes in Vitis HLS | 14 / 16 | n1024_UF16 and n1024_UF32 exceed the 600 s Vitis budget; metadata, not a filter |
| 2. Translated design compiles | clang on every source in the container | 16 / 16 | none |
| 3. Testbench match | translated testbench output equals the original's | 16 / 16 | none |
| 4. Static checks | required files, no Vitis dialect left, driver complete, fence respected | 13 / 16 | 3 fence violations: agent rewrote a loop header and dropped its label (n1024_no_StagePipeline), deleted an unroll + pipeline pragma pair (n256_UF8), deleted three bare loop labels (n256_UF4; harmless, licensed by the fence since after this run) |
| 5. Catapult synthesis | `go extract` completes | 5 / 16 | 4 timeouts at 2400 s (n1024_UF16/UF32, n256_UF16/UF32); 2 `PIPELINE` on a loop Catapult finds fully unrolled after constant propagation (n1024_UF4/UF8, bound is a variable the source rule cannot see); 1 memory ports, 4 needed of 2 (n1024_UF2); 1 float accumulate at II 1, SCHD-20 (n256_no_StagePipeline) |
| 6. Recovery | one log-driven retry per round | 7 / 16 | +2: per-row bank split (`BLOCK_SIZE 256` on `data_rev_stream`) passed n1024_UF2; relax-II to 5 passed n256_no_StagePipeline. The unpipeline step applied on n1024_UF4/UF8 and the retry then timed out |

Comparison and a caveat. The agent-free control with the 09-29 pre-pass (control 5) scored 9 / 16, but in
that run `#pragma hls_unroll UF` reached Catapult unresolved, was rejected as an invalid pragma (CIN-93) and
ignored, so the heavy variants synthesized un-unrolled. r8 resolves `UF` from its `#define`, the unrolls apply,
and exactly those variants now exceed the 40-minute budget. 7 / 16 is the honest number with every pragma
applied; the previous agent run (r5, 09-29) was 5 / 16. What remains is one class, scheduling time on the
highest unroll factors, plus the agent regressions the fence catches.

90-minute budget (2026-10-07, gate driver `synth.py --synth-timeout 5400 --degrade 2` on the r8 outputs): the four heavy
variants reach a verdict inside 90 minutes, the same pipeline-on-a-fully-unrolled-loop error, and the unpipeline recovery
round then runs out of the next 90 minutes on n1024_UF16/UF32 and n256_UF16/UF32, but passes n1024_UF4 (the FFT
count becomes 8 / 16). With the unrolls applied the four largest variants are a cost wall, not a translation wall.
