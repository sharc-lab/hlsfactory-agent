# UCLA-VAST/HP-FFT-HLS -> catapult

| design | oracle (orig testbench) | translation checks | syntax | testbench | agent wall s | tool calls | tokens in/out | cost USD |
|---|---|---|---|---|---|---|---|---|
| n1024_UF1 | pass | pass | ok | pass | 164 | 26 | 155106/16944 | 0.0281 |
| n1024_UF16 | pass | pass | ok | pass | 1227 | 53 | 236422/44765 | 0.0575 |
| n1024_UF2 | pass | pass | ok | pass | 547 | 24 | 177633/16917 | 0.0308 |
| n1024_UF32 | pass | pass | ok | pass | 561 | 31 | 114794/19436 | 0.0231 |
| n1024_UF4 | pass | pass | ok | pass | 407 | 26 | 39560/20185 | 0.0135 |
| n1024_UF8 | pass | pass | ok | pass | 386 | 25 | 114517/25378 | 0.0252 |
| n1024_no_StagePipeline | pass | pass | ok | pass | 180 | 27 | 53461/11254 | 0.0117 |
| n1024_original_C_style | pass | pass | ok | pass | 120 | 24 | 45430/6562 | 0.009 |
| n256_UF1 | pass | pass | ok | pass | 417 | 27 | 43947/18919 | 0.0128 |
| n256_UF16 | pass | pass | ok | pass | 1314 | 71 | 314189/63306 | 0.078 |
| n256_UF2 | pass | pass | ok | pass | 475 | 33 | 59079/21432 | 0.0167 |
| n256_UF32 | pass | pass | ok | pass | 327 | 28 | 53005/19449 | 0.0144 |
| n256_UF4 | pass | pass | ok | pass | 304 | 31 | 144001/19951 | 0.0273 |
| n256_UF8 | pass | pass | ok | pass | 261 | 29 | 42263/16667 | 0.0117 |
| n256_no_StagePipeline | pass | FAIL | ok | pass | 141 | 21 | 27364/8539 | 0.0068 |
| n256_original_C_style | pass | pass | ok | pass | 113 | 24 | 27307/6386 | 0.0062 |

Designs: 16. Oracle pass: 16. Translation pass: 15. Total agent cost: 0.3728 USD. Total agent wall time: 6944 s.

Each design folder holds `vitis/` (the extracted original), the target folder (translated output with run.tcl and
translation_report.md), `oracle.json`, `check_data.json`, and the agent session transcript.
