#!/bin/sh
# Regenerates out/ for every probe, then simulates every probe that has test
# vectors (probes/<name>.sim/). Experiment numbers refer to
# docs/notes/xls_pragma_experiment_results.md. Failures are reported and the
# loop keeps going. Expected failures: fifo_depth_internal_0 (xlscc error,
# Exp 2) and the simulation of stream_two_blocks_reorder_noflops (deadlock, Exp 6).
cd "$(dirname "$0")"
for f in probes/fifo_depth_*.cc probes/array_*.cc probes/pipeline_*.cc; do   # class style
  echo "== $f"; ./run.sh "$f" --block_from_class Block || true
done
for f in probes/stream_two_blocks*.cc; do   # free functions + .textproto
  echo "== $f"; ./run.sh "$f" || true
done
for d in probes/*.sim; do
  echo "== sim ${d%.sim}"; ./sim.sh "${d%.sim}.cc" || true
done
