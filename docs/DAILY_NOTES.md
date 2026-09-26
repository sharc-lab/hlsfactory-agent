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
