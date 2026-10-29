#!/bin/sh
# Regenerates out/ for every probe. Grouped by experiment number in
# docs/notes/xls_pragma_experiment_results.md. Failures are reported and the
# loop keeps going (fifo_depth_internal_0 is expected to fail; see Experiment 2).
cd "$(dirname "$0")"
for f in probes/fifo_depth_*.cc; do         # Experiments 1-3: class style
  echo "== $f"; ./run.sh "$f" --block_from_class Block || true
done
for f in probes/stream_two_blocks*.cc; do   # Experiment 4: free functions + .textproto
  echo "== $f"; ./run.sh "$f" || true
done
