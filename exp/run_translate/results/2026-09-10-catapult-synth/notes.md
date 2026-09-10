# Catapult synthesis of the translated MAC designs, 2026-09-10

First real Catapult run on the outputs of the Vitis-to-Catapult translation flow. Both committed
designs were synthesized through `go extract` on this machine (chao-srv1.ece.gatech.edu).

## Tool

- Catapult Prime Synthesis **2026.2/1292347** (Production Release, May 14 2026),
  `MGC_HOME=/tools/software/siemens/catapult/latest/Mgc_home` (`latest` -> `2026.2`).
- Run in batch mode: `catapult -shell -file batch.tcl -logfile catapult_run.log`, where
  `batch.tcl` is a three-line wrapper that does `catch {source run.tcl}` and then `exit`
  (without it the shell would sit at the prompt after the script finishes). 20-minute timeout;
  each run actually took under 30 seconds.

## Designs and results

| Design | Source | go extract | RTL written | Latency | Throughput | II | Area score (post-assign) |
|---|---|---|---|---|---|---|---|
| expected_mac | `exp/run_translate/expected/catapult_mac` (hand-translated reference) | completed | `Catapult/mac.v1/concat_rtl.v` + `rtl.v`, VHDL too | 8 | 10 | 1 | 2220.6 (datapath 2204.6) |
| agent_mac | `exp/run_translate/results/2026-09-10-vitis_mac-to-catapult` (live agent output) | completed | same | 8 | 10 | 1 | identical |

The two source sets are byte-identical apart from trailing newlines, and the reports are
identical: same schedule, same bill of materials, same area.

**PASS for both.** Exit status 0, no errors, `go extract` wrote `concat_rtl.v`, `rtl.v`,
`concat_sim_rtl.v`, and VHDL equivalents under `Catapult/mac.v1/`. Reports here: `rtl.rpt`,
`cycle.rpt` per design. Project directories and netlists are not committed.

## Changes made

- **`run.tcl`: none.** The committed script ran unmodified. In particular:
  - `solution library add nangate-45nm_beh -- -rtlsyntool DesignCompiler -vendor Nangate
    -technology 045nm` worked; this install has `$MGC_HOME/pkgs/siflibs/nangate/nangate-45nm_beh.lib`.
  - `solution library add ccs_sample_mem` worked (`$MGC_HOME/pkgs/siflibs/ccs_sample_mem.lib`).
  - `solution options set /Input/CppStandard c++11` was accepted by 2026.2; no c++17 change needed.
  - The headers resolved to Catapult's own copies: the log's input-file tree shows
    `$MGC_HOME/shared/include/ac_fixed.h`, `ac_int.h`, `ac_channel.h`.
- **Environment fix (machine setup, not the design):** the default `MGLS_LICENSE_FILE` in this
  account's `.bashrc` points at `1717@ece-linlic.ece.gatech.edu`, which is down (first run died
  with `mgls_errno = 515`, "License server machine is down or not responding"; port probe
  confirms 1717 closed there). Port 1717 on `ece-winlic.ece.gatech.edu` is open and serves the
  CatapultPrime feature. Both runs used:
  `MGLS_LICENSE_FILE=1717@ece-winlic.ece.gatech.edu SALT_LICENSE_FILE=1717@ece-winlic.ece.gatech.edu`.
- **Added `batch.tcl`** (in each scratch copy, committed here next to the logs) purely to make
  `run.tcl` runnable non-interactively. `run.tcl` itself was not edited.

## Warnings (both runs, benign)

- `LIB-142` Extrapolation detected in the nangate characterization data (sample library, expected).
- `LOOP-26` Cannot unroll loop `/mac/core/main`: the implicit infinite process loop, not the MAC loop.
- `LIB-220` info: `TechLibSearchPath` unset, so the DesignCompiler downstream flow is not wired
  up; irrelevant for HLS-level RTL generation.

## Do the numbers make sense for an 8-tap MAC?

Yes. The `for` loop is left rolled and pipelined at II=1 (`hls_pipeline_init_interval 1`
was detected on `/mac/core/for`), so 8 taps take 8 cycles: latency 8, throughput 10 cycles at
the 5 ns clock (2 extra c-steps in the enclosing `main` loop for the channel I/O and output
write). The bill of materials shows exactly one 16x16->32 multiplier (`mgc_mul(16,1,16,1,32,7)`,
~1239 of the ~2205 datapath area score), one 32-bit accumulator adder (~212), the `coef`
array mapped to a 128-bit direct input port (`ccs_ioport.ccs_in`), and ac_channel in/out mapped
to `ccs_in_wait`/`ccs_out_wait` handshake ports. No memories, small mux for tap selection,
FSM of one 16-area register. That is the canonical serial MAC implementation.

## Not done

- Live agent run (step: `run.py --target catapult`): skipped. This machine's Docker daemon
  refuses the user's socket (`permission denied ... /var/run/docker.sock`), there is no
  `hlsfactory-agent` image visible, and the repo checkout has no `.env` / `OPENROUTER_API_KEY`.
- No downstream DesignCompiler run; `nangate-45nm_beh` is a behavioral sample library, so area
  scores are Catapult estimates, not gate-level numbers.
