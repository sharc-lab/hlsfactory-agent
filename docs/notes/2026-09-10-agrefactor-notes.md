# AgRefactor reading notes

Paper: "AgRefactor: Self-Evolving Agentic Workflow for HLS Compatibility and Performance", Yang Zou, Zijian Ding,
Yizhou Sun, Jason Cong (UCLA). arXiv 2606.30949, June 2026, revised August 2026.

Why it matters for us: it is the closest published system to our translation agent. It rewrites software C/C++
into Vitis-compatible HLS C and then optimizes it, with an LLM workflow that gets better across tasks. Our job is
the same shape one level over: rewrite HLS C for one tool into HLS C for another.

## The workflow, in order (Section III-A)

1. Test generator: writes the testbench and interface assumptions before any refactoring.
2. Category-specific identifiers, run in parallel: each looks for one family of non-synthesizable constructs.
3. Item organizer: merges and deduplicates what the identifiers found.
4. Planner: produces the full refactoring plan, informed by memory (below).
5. Refactor worker: applies the plan.
6. Synthesis and simulation validation.
7. Analyzer-fixer pair: loops on errors, at most 3 retries.
8. Performance optimization agent, after synthesis: iterative, with a tree-shaped working memory.

## Self-evolving memory (Section III-B)

- Entry: (initial program, identified constructs, refactoring strategy, generalized critique from the analyzer with
  program-specific names removed).
- Retrieval: distance mixes an embedding of the raw program and an embedding of the identified constructs,
  alpha = 0.6 on the program side. It pulls one plan from the most similar successful trial and critiques from the
  three most similar programs.
- Global and persistent across tasks. Appendix A-A shows it accumulates over epochs and transfers to unseen kernels,
  worth 12.4 percent relative improvement over a memoryless baseline.
- Failure mode they admit: when the Vitis manual already covers the construct, retrieved critiques from dissimilar
  programs add noise. Two benchmarks got worse for this reason.

## Deterministic tools (Section III-C)

- HeteroRefactor is integrated as the automated refactoring tool. It fails on nested pointer arithmetic and on STL
  containers.
- A tool specialist agent does lightweight rewrites to make code HeteroRefactor-compatible, for example removing
  problematic headers. Worth 13.2 percent more successes (129 to 146). Tool-enabled runs average 4 minutes versus 10
  without tools, and tool calls cost no tokens.

## Verification (Sections III-D, III-E, Appendix A-C)

- Vitis HLS, U200 and U55C parts.
- Correctness by LLM-generated testbenches plus C simulation. Single-shot testbenches inflated pass rates by about
  60 percent, so they added an engineer-rater loop to improve coverage.
- Performance metric: latency speedup, geometric mean, against AutoDSE as the pragma-tuning baseline, and against
  optimized open-source designs at resource budgets of 0, 20, 50, 100 percent.

## Benchmarks and results

- 11 real-world programs from av1, libjpeg-turbo, libsodium, minimap2, HeteroRefactor's set, and two leetcode
  problems, from about 21 to 1,266 lines. Each run 20 times.
- Matches or beats the state of the art on 9 of 11. Loses on median_cut (18/20 vs 20/20) and argon2_fill_segment
  (13/20 vs 17/20). The 1,000-plus line av1 kernel barely improves because it is far from anything in memory.
- 6.51x geometric mean speedup over AutoDSE; 1.2x over optimized open-source designs with under 20 percent extra
  resources.
- Models: GPT-5-mini and GPT-5. 3 to 20 minutes per benchmark, average 10.

## What we take from it

1. Verify inside the loop with a testbench, and do not trust single-shot testbenches. We reuse the extracted
   design's own testbench instead of generating one, which sidesteps their inflation problem.
2. Deterministic tools plus an LLM beat either alone. In our flow the deterministic part is the pragma and type
   mapping table; the LLM handles restructuring.
3. A memory of generalized critiques helps, but only for constructs the static reference does not already cover.
   Our lessons file should hold only what the target's documentation does not say.
4. Report pass@k over repeated runs, as they do with 20 runs per benchmark.
5. Their failure on the largest kernel is a warning: distribution shift kills retrieval. Expect the same on the
   largest extracted designs.
