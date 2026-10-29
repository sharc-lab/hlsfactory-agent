# Daily notes

One entry per working session, newest last. Two sentences: what worked, what did not.
Concrete (numbers, design names, error codes); written for someone who was not in the session.

## 2026-09-21 (Monday)
Worked: Ran all 16 hpfft designs through Catapult 2026.2 in both arms (with and without the 221 resource directives) plus Vitis HLS 2024.1 on the originals, after a set of arm-identical source fixes made the translated C++ synthesizable at all.
Didn't: All 221 directives errored with `Unknown path '/FFT_TOP/<var>:rsc'` (before/after byte-identical), and only 3 of 16 designs reach `go extract` in either arm, so the pipeline's pass rate was measuring compile-and-correctness, not synthesizability.

## 2026-09-22 (Tuesday)
Worked: Found that `directive get "/TOP/*/…:rsc" -match glob` after `go assembly` lists the real resource paths, and applying INTERLEAVE 8 to the discovered `data_0` paths on n1024_UF2 was accepted 8/8 and cut latency 16% (2x1024-deep RAMs -> 16x128-deep banks) — directives can be derived from the tool instead of predicted.
Didn't: The 13 non-synthesizing designs fail for structural translation reasons (arrays shared across sibling blocks, logic mixed with interconnect, II=1 float accumulators needing II>=4, too few RAM ports), none of which directives can fix.

## 2026-09-23 (Wednesday)
Worked: Replaced predicted Catapult directive paths with a `resource_directive` Tcl proc that enumerates real `:rsc` paths after `go assembly` (plus dimension-aware partition rules and full array dims in scan); on n1024_UF2 all 17 rendered directives hit real resources, `go extract` passed, latency fell 16% (59132 -> 49404) with the RAM banking changed as directed, 13 tests pass, nothing committed.
Didn't: Docker is still unusable here (not in the `docker` group; rootless needs /etc/subuid) so the LLM stage can't be exercised, and the first validation exposed that `complete dim=1` on a 2-D array used to mean 1024 registers (MEM-8) — a table bug the old zero-hit paths had been hiding.

## 2026-09-24 (Thursday)
Worked: Docker access arrived, the `hlsfactory-agent` image was built and the full pipeline (pre-pass, Pi agent, checks, synthesis gate) ran end to end on this machine for the first time: `vitis_mac` passes the gate with its derived directive hitting 1 resource, and all 16 hpfft FFTs pass the translation checks after a pre-pass fix for multi-line top signatures.
Didn't: 0 of 16 FFTs synthesize — every one is rejected by Catapult's front end within seconds (9x `uint16_t` undefined CRD-20, 4x `std::complex` CIN-15, 3x the same after re-run), so the dialect rules for `<stdint.h>`, `std::complex` -> `ac_complex` and float -> `ac_ieee_float32` are the blocking work before derived directives or degrade steps can matter.

## 2026-09-25 (Friday)
Worked: Set up a persistent tmux session and detached job launching after an SSH drop killed run 2; the dialect rules moved the pipeline's synthesis result from 1/17 to 3/17 (MAC + both original_C_style) with no front-end rejections left, and a new DUT-only check layer (forbidden float math incl. cosf, std::complex, non-static channels, missing <stdint.h>) plus `--attempts` now catches the agent undoing the pre-pass instead of letting it reach Catapult.
Didn't: Run 3 with two attempts stayed at 3/17 because the agent regresses the pre-pass in every retry it gets (deletes `#include <stdint.h>`, reintroduces std::complex and hls_vector, drops `static`), so 3 designs failed checks and 2 reached Catapult with removed includes; with pipelining now binding, 6 of 17 hit the 20-minute gate timeout in Catapult's loops/memories transforms.

## 2026-09-26 (Saturday)
Worked: Fenced the agent to the residue list (an edit elsewhere fails the check and the retry prompt quotes the pre-pass text to restore), replaced runtime `cos`/`sin` with a CORDIC `ac_float_trig.h` shim built on vendored `ac_math` plus a 1e-4 testbench tolerance, added a synth-only Vitis `TargetSpec` with `run.py --synth {both,source,target,none}`, `--synth-timeout` and a content-hash synthesis cache (MAC: 44 s in Vitis, then instant), rendered `BIND_STORAGE ram_1p/s2p/2p/t2p` to `ccs_ram_sync_{singleport,1R1W,dualport}` mappings and `STREAM depth` to `FIFO_DEPTH` on `:cns` channel paths (both accepted by Catapult in probes, MEM-4), imported 24 HLSFactory vitis_examples/pp4fpgas designs of which 23 synthesize in Vitis (uram_ecc needs a URAM part), and turned control 2's two compile errors into pre-pass rules (`x.range(i, i)` -> `x[i]`, 12 designs; native double arithmetic feeding an `ac_ieee_float64` wrapped per operand, CIN-291, 4 designs); 17 tests pass, nothing committed.
Didn't: Run 4 (fenced agent, 2 attempts, old fence) passed the checks on 5/17 and the Catapult gate on 3 (MAC, both original_C_style); re-scored with the current fence 10/17 pass static, but the 5 extra designs gated give 1 SCHD-4 and 4 timeouts. Control 2 (pre-pass + shims, no agent) was 0/16 on the two errors above; control 3 with the new rules is 2/16 (both original_C_style, the CORDIC twiddles synthesize at 302224 / 101898 cycles), 2 SCHD-20 (no_StagePipeline, float accumulate at II=1), 2 SCHD-4 (n256_UF1/UF2, 26 directives applied) and 10 timeouts. Probes on n1024_UF2 showed the timeouts are the 20-min budget, not the CORDIC loop: with a literal twiddle table or with a 60-min budget the flow ends after 24-30 min at the same SCHD-4 (512-word array needs 2 ports, Catapult's default `ccs_ram_sync_1R1W` has 1), so the next lever is a dual-port-by-default memory policy in the directive channel: two probes of it were inconclusive (mapping every `:rsc` path also hit the channel ports, IFSYN-7; filtering on `directive get -FIFO_DEPTH` skipped everything since it succeeds on memories too), so the policy should be emitted per array from the scan's inventory via `resource_directive <var> -MAP_TO_MODULE {ccs_sample_mem.ccs_ram_sync_dualport}`, which is untested.

## 2026-09-27 (Sunday)
Worked: Wired the dual-port default into the directive channel: the scan now inventories every DUT array (`arrays`: name, dims, size, scope, const) and every array above Catapult's `MEM_MAP_THRESHOLD` (32 words) with no `BIND_STORAGE`, not const and not register-mapped gets `-MAP_TO_MODULE {ccs_sample_mem.ccs_ram_sync_dualport}`, the memory Vitis gives arrays implicitly; `run.tcl` also sets `FIFO_DEPTH 8` globally (HIER-47 on unbuffered channels); 17 tests pass, nothing committed. Control 4 (pre-pass + directives, no agent, 2400 s budget) is 2/16 like control 3 but every design moved past its old error into a policy class: SCHD-4 cleared on n256_UF1/UF2 and n1024_UF2 (the 28-30 dual-port directives all hit).
Didn't: The four remaining walls are architecture, not dialect. (a) channel-fifo, 5 designs (n1024_UF1, n256_UF1/UF2/UF4/UF8): a static `ac_channel` between two inlined functions has no legal module even with FIFO_DEPTH 8 applied (HIER-47 plus an internal assertion ASSERT-1), so `#pragma HLS dataflow` regions need the DATAFLOW -> `hls_design block` policy the pre-pass does not apply; (b) SCHD-4 again on n1024_UF2 but now "4 needed, 2 available" of the dual-port RAM for `data_rev_stream`, which is an INTERLEAVE-from-the-error degrade step; (c) SCHD-20 float accumulate at II=1 on both no_StagePipeline designs and n256_UF16, the relax-II step; (d) 40-minute timeouts on n1024 UF4/UF8/UF16/UF32 and n256_UF32, the highest unroll factors, inside the memories transform.

## 2026-09-29 (Tuesday)
Worked: Removed the vendored `ac_math` (tool libraries stay with the tool) and moved synthesis into containers: `hlsfactory_agent/tools.py` resolves the host installs into read-only mounts (Catapult from `MGC_HOME`, Vitis HLS 2024.1 with its matching Vivado, after hls-eval's resolver), `run_synth` runs the tool in a fresh container as the calling user, the image gained `libc6-dev-i386`, `locales`/`en_US.UTF-8` and `libtinfo5` (the three things that broke in throwaway-container tests), and `run.py` now stops at compile by default with `--synth` (both sides), `--no-translate` and `--agent-tools` (off: the agent never sees a tool); MAC synthesizes through the container path in Vitis (16 s) and Catapult (latency 8). Also added the DATAFLOW -> `hls_design block` pre-pass rule (callees of a dataflow region whose parameters are channels or scalars) and the degrade loop (`run_synth_degraded`, `--degrade 2`): SCHD-87's minimum II raises pipeline pragmas, SCHD-4's port shortfall interleaves the array SCHD-39 names; relax-II rescued n1024_no_StagePipeline (Catapult PASS at 394572 cycles) and the block rule passed n1024_UF1 (12032 cycles), the first stream-structured FFT with no hand edits; 20 tests pass, nothing committed.
Later the same day: control 5 (blocks + dual-port + FIFO, 2400 s, no agent) finished 9/16 (from 2/16): the block rule cleared every channel failure, leaving 5 memory-port shortfalls and 2 SCHD-20. The full pipeline with the agent, `--synth --degrade 2 --synth-timeout 2400`, ran on the 24 imported designs: 20/24 pass the checks (3 fence violations, 1 genuine `exp` residue) and 15/24 synthesize in Catapult; the failures name new classes, `%`/`/` with no library component (CRAAS-6, ap_arithmetic and uram_ecc), `assert` (CIN-16, accs_w_asserts, fixed by `-DNDEBUG` in `run.tcl`), pointer casts (CIN-71, pointer_casting and axi_master, an architecture class). On the FFT 16 the same pipeline has, with 3 designs still in Catapult, 10/13 through the checks (the fence stopped the agent deleting pipeline pragmas, an include, and one 26-line type rewrite) and 5 Catapult PASS including the degrade loop's relax-II rescue of n1024_no_StagePipeline inside the run; n256_UF4/UF8 failed on `PIPELINE` plus complete `UNROLL` on one loop, which Catapult refuses, now a pre-pass rule. The port step was revised from what the run used: a 2-D array accessed row-parallel gets one bank per row (`BLOCK_SIZE` = inner size) and a repeat round compounds the split; validated on control 5's n1024_UF2, Catapult PASS at 11008 cycles. Scratch drivers retired to `hpfft_synth/retired/`.
Didn't: The FFT 16 through the full pipeline ended at 12/16 past the checks (4 fence violations) and 5/16 Catapult PASS (n1024_UF1, n1024_original_C_style, n256_UF2, n256_original_C_style, and n1024_no_StagePipeline by relax-II), below control 5's 9/16 because the run predates two rules it needed: 4 designs (n1024_UF4/UF8, n256_UF4/UF8) died on the pipeline-plus-complete-unroll pair now dropped by the pre-pass, and n1024_UF2 on the old interleave step, with n256_UF32 (ports, then II) and n1024_UF32 (40-min timeout) the genuine leftovers; a rerun with tonight's rules is the next measurement.

## 2026-09-30 (Wednesday)
Worked: Created `docs/WEEKLY_FINDINGS.md` (one funnel table per design set and week: source repo, per-stage passed/total, why each stage lost what it lost) with the imported vitis_examples/pp4fpgas set (24 imported of 28, 23 Vitis, 24 compile and match, 20 checks, 15 Catapult) and reran the FFT 16 through the full pipeline three times as the first hour of each rerun exposed a gap: macro pragma values (`UF`) are now resolved from their `#define` (Catapult does not macro-expand pragmas, so the agent had been deleting the whole pragma block), residue entries carry their text and are re-pointed at the output file after insertions (the fence was licensing the wrong lines), a rewritten single-bit `.range(i, i)` is no longer residue (a phantom residue on a blank line sent the agent into a 30-minute read loop), a "cannot pipeline a fully unrolled loop" verdict now names the loop and its pipeline pragma is removed by a recovery step, and bare loop-label deletions are fence-free; 20 tests pass, nothing committed.
Didn't: The final FFT run (r8) is 13/16 past the checks and 7/16 Catapult PASS, 5 direct plus the per-row bank split rescuing n1024_UF2 and relax-II rescuing n256_no_StagePipeline inside the pipeline; the remaining 6 Catapult failures are 2400 s timeouts on the highest unroll factors (n1024_UF16/UF32, n256_UF16/UF32, and n1024_UF4/UF8 after their pipeline pragma was removed). Control 5's 9/16 is not comparable: its `hls_unroll UF` was rejected as an invalid pragma (CIN-93) and ignored, so those variants synthesized un-unrolled; 7/16 is the honest number with every pragma applied.

## 2026-10-07 (Wednesday)
Worked: Ran the two things the week closed on. (1) The six FFT timeouts with a 90-minute budget: the four heavy variants (n1024_UF16/UF32, n256_UF16/UF32) now reach a real verdict inside 90 minutes, the same pipeline-on-a-fully-unrolled-loop error, and their unpipeline recovery rounds are running. (2) The repo census: `run_repo.py` over the 25 `run_base` repos with `--attempts 2 --synth --degrade 2 --synth-timeout 2400`, one repo after another (driver retries a repo once on an upstream model error). Three repos in: fpgaconvnet-hls extracts nothing; S2CBench 22 designs, 7 Catapult on the first pass; balor 40 designs, 21 Catapult. Each census repo produced rules the same day: `set_top <folder name>` written by extraction for a differently named function (15 of 22 S2CBench Vitis failures) is repaired from the testbench's call or a name prefix (`translate.repair_top`, both drivers); unsized `T x[]` top parameters get the size the testbench passes (CIN-84, 9 of 22); `abs(` -> `cat_abs(` shim (CIN-16); `sqrt` joins the trig shim; `ac_ieee_floatNN v = <literal>` becomes direct initialization (CRD-415); a `#define`'d `float`/`double` is now rewritten like any type (13 of 40 balor designs kept native double through a macro, CIN-290). Also: the synthesis cache key carries the budget, the gate driver gained `--synth-timeout/--degrade/--only`, `run_repo.py` gained the synthesis flags and now honours `--attempts`; `docs/WEEKLY_FINDINGS.md` has the census table. Later repos added: the fence now licenses every pre-pass line the container compiler rejects (Spector-HLS: the dialect rewrite itself produced illegal code and the agent was blocked from fixing it), float literal tables stay native whether const or not, a second test program with its own `main` is not copied, `#pragma SDS` lines are dropped, typedef'd float types get the literal rule, unsized parameters are also sized from `malloc`/`new` with sizes resolved through local `int` assignments (blaze: all 8 designs). 23 tests pass, nothing committed.
Didn't: The S2CBench rerun with the new rules and the rest of the census (22 repos) are still running; balor's extracted testbenches all fail to link against their kernels (oracle 0/40), an extraction defect outside this pipeline; cholesky still fails the checks on an agent-written support header.

## 2026-10-08 (Thursday)
Worked: Committed the pre-Bambu tree as c985a8e (container-backed synthesis, dual-port default, recovery rounds, imported set, census findings; 23 tests). Added Bambu (PandA) 2024.10 as a target: `tools.py` resolver over the extracted AppImage, `targets/bambu.py` (driver `run_bambu.sh`, log parser, failure classes, INFER-to-MINIMAL recovery step), per-target `PRAGMA_FORMS`/`DIALECT_TYPES` in `rewrite.py` (Bambu keeps native float, drops PIPELINE, label above the pragma, a top returning an ac type by value becomes void + `ret_out`), 10 ns clock and `-I.`; validated end to end on the MAC. Imported set 15 -> 21 of 24 after the four rules; FFT set 0 -> 1 of 16 (7 clang-plugin crashes on `llvm.bitreverse`, 5 aggregate-channel ports, 1 segfault; tool walls). Census first pass complete: 181 designs over 25 repos, oracle 95, Vitis 100, compile+match 156, checks 102, Catapult 38 + 4 recovery = 42; report republished.
Didn't: The census second pass over the 12 repos that predate their rules is prepared and deliberately not launched; Bambu's aggregate-channel and bitreverse limits have no rule; the Bambu tree (7 files) is uncommitted.

## 2026-10-09 (Friday)
Worked: Instrumented the repo driver (`hlsfactory_agent/stats.py`: per-stage timers, 30 s host sampler, retry accounting; `exp/run_translate/stats_report.py` -> `docs/RUN_STATISTICS.md`) and started the census second pass over all 25 repos with 3 agent attempts, first-pass extractions reused; 15 repos recorded by 17:04 (140 designs, 60 Catapult passes so far; balor 21 -> 36 of 40, Spector 0 -> 4, blaze 0 -> 3, SERI 0 -> 5 past the checks but all on CRD-2388 constexpr-with-statements). Retry curve over 125 designs: 22% at attempt 1, attempt 2 rescues 44% of the rest, attempt 3 another 22%; 3 designs thrash identically. Wrote the Dynamatic assessment (`docs/plans/2026-10-09-dynamatic-target.md`, nothing implemented).
Didn't: The pass stopped at 19:50 with OpenRouter credits exhausted (402 from 18:15 on); DP-HLS, CLINK, ScaleHLS-HIDA, ThunderGP and the partial ProgSG ran with a dead agent and were discarded, so 10 repos remain plus the S2CBench jpeg pair whose containers vanished at 06:28; resume with `census2.sh repos.txt` once credits are added. Two code fixes on the way: the sampler's stop event (crashed the first S2CBench run) and a corrupt rewrite_log.json in the README step (balor). Nothing committed.

## 2026-10-13 (Tuesday)
Worked: Implemented fixes 1-7 from `docs/plans/2026-10-10-failure-fixability.md` and validated each on recorded census2 outputs, without the agent:
- C++14 in `run.tcl` clears SERI's CRD-2388.
- Fence repair (mechanical restore, line-granular, kept only if no check gets worse) passes 5 of the 38 fenced failures on replay.
- The recovery step relaxes the II when a split array still collides, and adds the missing `resource_directive` proc. histogram, sobel_subdimx and piMonteCarlo now synthesize.
- Pointer and commented-size ports are sized from the testbench, which clears CIN-84 on all 5 designs (MurmurHash3 PASS).
- `ac_float_exp.h` (CORDIC exp/log/pow) plus a native-float check in the DUT.
- `hls_math.h` mapping, struct-member channels, copied Vitis library headers, required includes inserted after each attempt.
- `normalize_vitis_script` (fpgaconvnet accum/conv/fork now pass Vitis) and a 2400 s source budget.
- Testbench-only headers are exempt from the fence and the DUT rules.
- 27 tests pass, nothing committed.

Didn't:
- No end-to-end rerun: OpenRouter credits are still exhausted, so the prompt clause and fence repair inside the loop are untested with the agent.
- The 4 "float-op" losses turned out to be native float the agent reintroduced (CIN-290), not exp/log; 2 of them (`2.0 * PI / len` shapes) are still uncaught.
- SERI now stops at CIN-71 pointer casts.
- fpgaconvnet's fence failures are an extraction defect (no `testbench.cpp`, and the kernel includes its testbench header).

## 2026-10-14 (Wednesday)
Worked:
- `cat_bytes.h` shim and pre-pass rule for CIN-71 (SERI's packed_array): all 5 SERI designs pass their testbench without pointer casts.
- k_rr_a with 5 recovery rounds: II 7, then a 6-way split of `int4d_dmbn`; it schedules but times out at 1200 s.
- Dataflow regions behind macros, with stages in other headers, now get `hls_design block`. k_buffer_permutation moves from HIER-11 to ASM-59/62, a loop around block calls that the agent must restructure.
- OpenRouter credits are back ($250).
- 28 tests pass, nothing committed.

Didn't:
- k_rr_a is not rerun with a longer timeout.
- The census second pass has not resumed yet.


Later:
- Smoke test of 5 designs on deepseek-v4.1-flash, registered with Pi through `models.json`: all 5 pass checks, pool and histogram synthesize. Cost $0.99.
- Smoke fixes: the fence restores deleted folders, chmod after each agent run, channel aliases become static, `#else` members stay non-static, and a model guard catches Pi's silent fallback.
- The pre-pass wraps double literals in float-typed ternaries (`: fp_t(0.0)`, k_rr_a CRD-42). The check flags `float(...)` casts (cnn CIN-290).
- census3 started on v4.1-flash over 25 repos.
- The 9 fpgaconvnet `*_layer` designs failed Catapult with CRD-1066 in hlslib `Flatten.h`. Helper types sit in an anonymous namespace, so the class using them differs between the two translation units. The pre-pass now names such namespaces (`inline namespace anon_<file>`) when they hold only types, templates and inline code. relu_layer then synthesizes (latency 2).

## 2026-10-23 (Friday)

- The NFS home went stale on 10-15 23:09 and stayed down a week: `uv` lives there, so census3's last 7 repos failed in 2 s each. Resumed with the venv's own python (local disk) and a retry on exit 126/127; 183 of 242 designs were already done. A `uv run` from the tmux shell then removed and recreated the venv under the running census: it survived, but never again (memory updated).
- Census3 analysis (183 designs): 172 pass checks, 89 synthesize. Fence failures are the main retry cause (98 designs on attempt 1); 122 whole-restore repairs were rejected 103 times. The rejected single-hunk cases were our own pre-pass gaps: `<ap_axi_sdata.h>` with no shim (42 designs; the licensing regex missed clang's `fatal error`), `<gmp.h>`, DP-HLS relative includes to flattened files, and a sqrt shim that overflowed (NaN in backprop's testbench).
- Changes: `ap_axi_sdata.h` shim; sqrt through ac_math's float overload (backprop testbench passes); flattened-include resolution; host-only includes dropped; `fatal error` licensed; fence repair restores per file then per hunk and licenses what cannot be restored; the agent container no longer dies after `sleep 2h` (resnet18, 7353 s); Vitis budget 900 s (saves ~6 h a census); SCHD-39 array name from scoped messages (DP-HLS rounds targeted the class `Align`); the `BLOCK_SIZE` recovery step dropped (0 of 6).
- Experiments: a 10 ns clock does not clear DP-HLS's SCHD-20 (a loop recurrence in cycles, not a path delay), so no clock-relax step. FIFO depth against Catapult's `cap>=0` assertion and the corrected DP-HLS recovery chain are running in tmux `hlsfactory`.
- 30 tests pass; a fence rerun of 8 census3 designs on the new code is queued.

## 2026-10-24 (Saturday)

- Census4 finished overnight: all 242 designs, 25 repos, three lanes of 11/11/10 designs in parallel (32 containers, load ~26, 68 GB), 7.6 h wall from 13:44 to 21:22 (census3 at 3 in flight took 27 h for 183). No license-checkout failures at 32 Catapult and Vitis seats. Cost $14.58.
- Results: 236 pass checks (98%), 122 synthesize in Catapult (50%; 98 direct, 24 after recovery). On the 192 designs both passes ran, census4 vs census3: checks 187 vs 181, Catapult 99 vs 89, first-attempt passes 127 vs 87, cost $12.27 vs $15.34. On the 140 designs census2 also ran: Catapult 91 vs 60.
- Fence repair: 137 repairs, 76 of them turned the attempt into a pass; 2678 fenced hunks, 623 restored, 300 licensed, 843 check rounds. Retries fell from 96 to 80 designs; the remaining retries are mostly leftover Vitis dialect, not the fence.
- Catapult failures left: SCHD-20 25 (DP-HLS loop recurrences), timeouts 21, CIN-84 12 (arrays inside `ac_std_float`), ASSERT-1 8 (Catapult crash), CIN-15 5, SCHD-4 5. Recovery: relax_ii 16/96, relax_ii_for 5/5, interleave 3/48.
- census3 (183 designs) and the smoke runs are archived under `/export/hdd/scratch/tshukla31/hlsfactory-census-archive/`. `RUN_STATISTICS.md`, the dashboard and WEEKLY_FINDINGS now describe census4 only.

## 2026-10-25 (Sunday)

- Dynamatic is the fourth target, as an initial exploration of translating into a smaller language (plain C, one file, `main()` with `CALL_KERNEL`). Image `hlsfactory/dynamatic:latest` built from upstream `main` in 15 minutes; `fir` from the integration tests compiles, writes VHDL and matches in GHDL co-simulation in 17 s.
- Harness: `TargetSpec.language="c"` (clang C17 syntax check, clang++ testbench build, no `testbench.cpp`), `fence=False`, `ToolInstallation.image` so `run_synth` can run in a tool's own image, `targets/dynamatic.py`, pre-pass entries for C types and clang loop pragmas, prompt wording for the C target. 31 tests pass.
- First run, 16 designs for $0.20: all 15 translatable ones pass the checks on attempt 1; 10 synthesize and match in co-simulation (all 8 ProgSG kernels, ave8, fir). The same 8 ProgSG kernels mostly timed out in Vitis and none passed Catapult directly in census4.
- Fixed from the run: unsized array parameters crash the translator (pre-pass sizes them, the check forbids them); the classifier read an INFO line (`Placed simple buffers`) as a failure class (fatal lines only now); timeouts keep the tool log. Found, not fixed: int<->float units are 32-bit only (idct), a second `CALL_KERNEL` breaks the verifier (lstm_n), ScaleHLS gesummv's GHDL co-simulation exceeds 1200 s.
- Design names collide across repos in one runs dir: the ScaleHLS pair was rerun in `runs/dyn1_scalehls`. Nothing committed.

Evening: image rebuilt with CBC (`--enable-cbc`), driver on `fpga20` MILP buffering, recovery falls back to
`on-merges`. First fpga20 run failed every design: the profiler reads `main()`'s stdout as its input vectors, so
any `printf` breaks `compile`. Pre-pass redirects prints to stderr, check forbids stdout, rule in the prompt.
`TargetSpec.extra_checks` added; Dynamatic's requires one `CALL_KERNEL` (lstm_n now fails the check, not the
synthesis). Rerun on the same translations (`runs/dyn2`, no agent calls): all 10 still pass co-sim, 3 to 5x
fewer cycles on the independent-iteration kernels (stencil2d 421980 -> 78503, fir 84499 -> 17901), bicg and
jacobi-1d unchanged (loop-carried dependence). adpcm's mismatch is an out-of-bounds load the profiler caught.
Synthesis is 1 to 8 minutes per design now (seconds before). 31 tests pass; nothing committed.

## 2026-10-28 (Tuesday)

Extraction repair so the original testbenches run (step 1 of the plan). Of census4's 144 oracle losses, 52 were
link failures (the oracle compiled every source while the testbench already included the kernel, or a second
`main`), 52 missing headers (escaped `../../include` paths, headers never copied, TAPA, `ap_cint.h`), the rest
data or runtime. New `hlsfactory_agent/extraction_repair.py`: `oracle_sources` takes the `synth.tcl` source set
and `-cflags`, drops a source another unit includes and extra `main`s; `repair_includes` points escaped includes
at the local copy or recovers the file from a repository clone; `repair_data_files` likewise for data files.
`run_oracle_check` builds that set, falls back to one translation unit on duplicate symbols, takes `dir_scratch`
and a timeout; `ap_cint.h` stand-in; `run_repo.py --oracle-only --repair-extraction --repo-clone --oracle-timeout
--reoracle`. Rerun on copies of the census4 extractions (`runs/oracle5/<repo>`, 900 s budget, no agent):
oracle 98 -> 170 of 242 (balor 0 -> 40, DP-HLS 0 -> 16, ThunderGP 1 -> 9, ScaleHLS 11 -> 14, CLINK 0 -> 2,
pasta 0 -> 2). Two SERI "passes" were hollow (a stub testbench that never called the kernel; now the kernel
compiles and clang rejects an implicit `ap_uint` conversion). Left: AutoNTT (TAPA, 10), NN2FPGA (agent-written
`test_config.hpp` against a generated matrix, 14), fpgaconvnet (goldens made up, 21), S2CBench (agent-written
testbenches, 8), DP-HLS 9 (clang strictness on array references, `N_BLOCKS`, 2 crashes). Mishap: cloning the
25 repos into the scratchpad filled the shared 70 GB root volume; clones now live under /export/hdd (memory note).
34 tests pass; nothing committed.
