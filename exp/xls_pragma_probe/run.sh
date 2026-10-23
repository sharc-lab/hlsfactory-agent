#!/bin/sh
# usage: ./run.sh probes/foo.cc [--block_from_class Name] [extra xlscc flags...]
# Runs xlscc -> opt_main and writes everything to out/<name>/ (gitignored).
# Probes with channels need --block_from_class <Class> to come out as a proc;
# without it xlscc emits a plain function with I/O op parameters.
# Needs xlscc and opt_main on PATH.
set -e
src=$1; shift
name=$(basename "$src" .cc)
out=out/$name
mkdir -p "$out"
case " $* " in *" --block_from_class "*) blk="--block_pb $out/$name.block.pb" ;; *) blk= ;; esac
xlscc "$src" $blk "$@" > "$out/$name.ir" 2> "$out/xlscc.log" || { echo "xlscc FAILED"; cat "$out/xlscc.log"; exit 1; }
opt_main "$out/$name.ir" > "$out/$name.opt.ir" 2> "$out/opt.log" || { echo "opt FAILED"; cat "$out/opt.log"; exit 1; }
echo "ok: $out/"
