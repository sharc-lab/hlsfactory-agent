#!/bin/sh
# usage: ./run.sh probes/foo.cc [--block_from_class Name] [extra xlscc flags...]
# Runs xlscc -> opt_main -> codegen_main and writes everything to out/<name>/
# (gitignored): <name>.ir, <name>.opt.ir, <name>.v (FIFOs as xls_fifo_wrapper
# instances, module not defined) and <name>.mat.v (FIFOs materialized inline).
# Probes with channels need either --block_from_class <Class> (class style) or
# a block spec probes/<name>.textproto next to the .cc (free-function style,
# needed for multi-block designs); run.sh picks up the textproto by itself.
# Without either, xlscc emits a plain function with I/O op parameters.
# Needs xlscc, opt_main and codegen_main on PATH.
set -e
src=$1; shift
name=$(basename "$src" .cc)
spec=${src%.cc}.textproto
out=out/$name
mkdir -p "$out"
case " $* " in
  *" --block_from_class "*) blk="--block_pb $out/$name.block.pb" ;;
  *) if [ -f "$spec" ]; then blk="--block_pb $spec --block_pb_text"; else blk=; fi ;;
esac
xlscc "$src" $blk "$@" > "$out/$name.ir" 2> "$out/xlscc.log" || { echo "xlscc FAILED"; cat "$out/xlscc.log"; exit 1; }
opt_main "$out/$name.ir" > "$out/$name.opt.ir" 2> "$out/opt.log" || { echo "opt FAILED"; cat "$out/opt.log"; exit 1; }
cg="--generator=pipeline --pipeline_stages=2 --delay_model=unit --reset=rst"
codegen_main "$out/$name.opt.ir" $cg > "$out/$name.v" 2> "$out/codegen.log" || { echo "codegen FAILED"; cat "$out/codegen.log"; exit 1; }
codegen_main "$out/$name.opt.ir" $cg --materialize_internal_fifos > "$out/$name.mat.v" 2>> "$out/codegen.log" || { echo "codegen (materialized) FAILED"; cat "$out/codegen.log"; exit 1; }
warn=$(grep -vE "^(Parsing file|Generating IR|Saving )" "$out/xlscc.log" || true)
[ -n "$warn" ] && { echo "xlscc warnings:"; echo "$warn"; }
echo "ok: $out/"
