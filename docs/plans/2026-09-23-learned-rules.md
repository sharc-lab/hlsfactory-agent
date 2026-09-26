# Learned rewrite rules (2026-09-23)

The hpfft run needed 11 hand fixes before Catapult would synthesize anything. Each was found the same
way: synthesize, read the tool's error, change one mapping, synthesize again. A hand-written rule table
is always one design set behind. This makes that loop part of the pipeline, with the rule table as data
the agent may extend, a budget, and promotion only after evidence.

## Two kinds of synthesis failure

| kind | examples (hpfft) | handled by |
|---|---|---|
| dialect: the target names or places a construct differently | `std::complex` -> `ac_complex`, float -> `ac_ieee_float32`, pragma before the loop not inside it, `static ac_channel`, `<stdint.h>` | a rule; learnable |
| architecture: the target cannot express the design as written | arrays shared between blocks (ASM-35), logic in an interconnect block, float accumulator at II=1 (SCHD-20), dataflow over arrays | a policy; not learnable |

`FAILURE_CLASSES` in `targets/catapult.py` already classifies every error. Each class gets a route:
`rule` (enter the loop), `policy` (stop, report), `env` (license, timeout: retry once, no model spend).
Memory-port shortfalls route to the directive channel, not to rules.

## Rules as data

`targets/<tool>_rules.json`, applied in order by a fixed interpreter in `rewrite.py`. The agent edits JSON,
never Python. One rule:

```json
{"id": "complex-to-ac_complex", "kind": "type", "match": "\\bstd::complex<", "replace": "ac_complex<",
 "fixes": ["CIN-15"], "status": "promoted",
 "learned_from": {"design": "hpfft/n1024_UF2", "error": "CIN-15", "date": "2026-09-21"},
 "validated_on": ["n1024_UF2", "n1024_original_C_style"]}
```

`kind` is one of a closed set with fixed semantics: `header`, `type`, `include` (insert once at top),
`qualifier` (prefix a declaration), `pragma` (rename + `placement`: before_function | before_loop).
The validator rejects a rule whose `replace` introduces a numeric literal absent from `match`
(the existing never-guess-at-numerics line), or whose `kind` is not in the set. Today's regex type
rewrites and header/mode maps move into this file as promoted rules with `learned_from: docs`.

## The loop (per design, after static and container checks pass)

1. `run_synth`. PASS: done. FAILED with route `policy`/`env`: record class and first error, stop.
2. Route `rule` and budget left: the retry prompt carries the error lines with file:line, the rule schema,
   and the current table, and asks for one candidate rule into `run_area/candidate_rules.json`.
3. Validate the candidate. Re-run the pre-pass on the original input with promoted + candidate rules,
   agent residue pass as today, checks, synth.
4. Budget: 3 candidates per design; a candidate that leaves the error class unchanged ends the loop.
   Worst case is 4 synthesis runs per design (1-13 min each on chao-srv1).
5. `check_data.json` gains `synth` (the `SynthResult`) and `rules: {candidates, budget_used}`.

## Promotion (offline, `hlsfactory_agent/rules.py promote`)

A candidate is promoted when it (a) validates, (b) fixed a `rule`-routed failure on two designs from
different families, and (c) applied to every design with a recorded synth PASS, matches nothing or
still passes compile, testbench, and synth. (c) is mechanical, no model. Promotion writes the rule
into the table with `validated_on`, as a commit for review. Rules are never edited in place; a bad
promoted rule is demoted with its provenance kept.

Results report the pass rate against a rule-table version (git rev of the json), since the rate is a
function of what the table has seen.

## Steps

1. Rules-as-data: move header/type/mode/pragma maps out of `rewrite.py` into `catapult_rules.json`
   (and `xlscc_rules.json`), add the validator, seed the six hpfft dialect rules. Existing tests keep
   their expectations. Tanmay.
2. Routes on `FAILURE_CLASSES`; call `run_synth` from `translate.run` after checks; policy failures
   reported, nothing else changes. Tanmay + Justin (touches `translate.py`).
3. Candidate-rule prompt, loop, budget. Tanmay.
4. `rules.py promote` replay. Tanmay.
5. Re-run hpfft (16) and PQC (2) through the loop; report pass@table-version. Needs Docker on chao-srv1.

Out of scope here, tracked separately: the dataflow policy (flat vs blocks-with-channels), raising II
from the SCHD-87 minimum, `cos`/`sin` (residue with an explicit instruction), `BIND_STORAGE RAM_2P`
and stream depth in the directive channel.
