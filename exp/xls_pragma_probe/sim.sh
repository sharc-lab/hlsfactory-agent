#!/bin/sh
# usage: ./sim.sh probes/foo.cc        (run ./run.sh on the probe first)
# Cycle-accurate block simulation (eval_proc_main --backend=block_interpreter)
# of out/<name>/<name>.block.ir, using the test vectors in probes/<name>.sim/:
# <channel>.in for each input channel, <channel>.out for each output channel,
# one XLS value per line (e.g. bits[32]:5). Outputs are checked against the
# .out files. Writes out/<name>/sim.log and out/<name>/sim_cycles.txt (the
# cycle of the last output) and prints that cycle count.
# Needs eval_proc_main on PATH.
set -e
src=$1
name=$(basename "$src" .cc)
vec=${src%.cc}.sim
out=out/$name
ins=; outs=
for f in "$vec"/*.in;  do ins="$ins${ins:+,}$(basename "$f" .in)=$f"; done
for f in "$vec"/*.out; do outs="$outs${outs:+,}$(basename "$f" .out)=$f"; done
eval_proc_main "$out/$name.block.ir" --backend=block_interpreter \
  --block_signature_proto="$out/$name.sig.textproto" \
  --inputs_for_channels="$ins" --expected_outputs_for_channels="$outs" \
  --ticks=-1 --output_stats_path="$out/sim_cycles.txt" > "$out/sim.log" 2>&1 \
  || { echo "sim FAILED"; tail -20 "$out/sim.log"; exit 1; }
echo "ok: $name last output at cycle $(cat "$out/sim_cycles.txt")"
