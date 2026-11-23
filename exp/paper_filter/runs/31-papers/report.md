# Paper-filter run: `31-papers`

Generated 2026-11-23 01:42. 31 papers; 8 repos handed off; 3 for review.

Status: ok 31  
Outcome: already_known 3, hls_found 7, needs_review 2, not_found 13, repo_no_hls 6

## By group

| group | papers | already_known | hls_found | needs_review | not_found | repo_no_hls | with handoff |
|---|---|---|---|---|---|---|---|
| with_repo | 3 | 2 | 1 | 0 | 0 | 0 | 1 |
| without_repo | 28 | 1 | 6 | 2 | 13 | 6 | 6 |

## Handed off to HLSFactory

Also in `handoff_repos.txt` (paste into `repo_urls` in `exp/run_base/run.py`).

| repo | provenance | HLS files | flags | paper | evidence |
|---|---|---|---|---|---|
| [breagen/MachSuite](https://github.com/breagen/MachSuite) | third_party_demo | 18 TCL projects |hls_via_tcl;third_party | A Reinforcement Learning Based Multi-objective Evolutionary… | paper_names_repo |
| [EPFL-LAP/dynamatic](https://github.com/EPFL-LAP/dynamatic) | third_party_demo | 460 |third_party | CRUSH: A Credit-Based Approach for Functional Unit Sharing … | paper_own_link; paper_names_repo; readme_has_doi_or_arxiv |
| [jiahaogai/LLM-for-HLS](https://github.com/jiahaogai/LLM-for-HLS) | own_found_by_search | 3 | | Exploring Code Language Models for Automated HLS-based Hard… | name_matches_system; contents_match_paper; contents_match_paper |
| [jianyicheng/HLS-benchmarks](https://github.com/jianyicheng/HLS-benchmarks) | third_party_demo | 1 |third_party | A Speculative Loop Pipeline Framework with Accurate Path Mo… | paper_names_repo; name_matches_system; description_matches_abstract |
| [Lucaz97/Agentic-HLS](https://github.com/Lucaz97/Agentic-HLS) | own_found_by_search | 336 | | Can Reasoning Models Reason about Hardware? An Agentic HLS … | git_log_authors_match; owner_matches_affiliation; name_matches_system |
| [MasLiang/AutoClock](https://github.com/MasLiang/AutoClock) | own_in_paper | 58 | | AutoClock: Automated Clock Management for Power-Efficient H… | paper_own_link; readme_has_title; git_log_authors_match |
| [UT-LCA/ML4Accel-Dataset](https://github.com/UT-LCA/ML4Accel-Dataset) | third_party_demo | 150 |third_party | Exploring Code Language Models for Automated HLS-based Hard… | paper_names_repo; paper_own_link |
| [zhaijiaqi/AccelES](https://github.com/zhaijiaqi/AccelES) | own_found_by_search | 5 | | AccelES: Accelerating Top-K SpMV for Embedding Similarity v… | owner_matches_affiliation; readme_has_title; git_log_authors_match |

## Needs a human look

| repo | paper | reasons | flags |
|---|---|---|---|
| [UCLA-DM/HLSyn](https://github.com/UCLA-DM/HLSyn) | Exploring Code Language Models for Automated HLS-based Hard… | needs_review | fork;hls_maybe_in_archive;third_party |
| [aferikoglou/CollectiveHLS](https://github.com/aferikoglou/CollectiveHLS) | CollectiveHLS: A Collaborative Approach to High-Level Synth… | weak_hls_markers_only;needs_review | weak_hls_markers_only |
| [ncsys-lab/FUSE](https://github.com/ncsys-lab/FUSE) | Efficient Optimization with Encoded Ising Models | needs_review | hls_maybe_in_archive |

## All papers

| group | paper | outcome | repos (outcome) | problems | |
|---|---|---|---|---|---|
| with_repo | AutoClock: Automated Clock Management for Power-Efficient HLS Designs… | hls_found | MasLiang/AutoClock (hls_found, handoff) |  | [transcript](work/AutoClock_Automated_Clock_Management_for_Power-Efficient_HLS_Designs_on_FPGAs/run_area/.pi/sessions/2026-11-23T05-54-40-924Z_01a206f8-889b-76b1-b5f8-c2e79c06d796.html) |
| with_repo | AutoNTT: Automatic Architecture Design and Exploration for Number The… | already_known | SFU-HiAccel/AutoNTT (already_known) |  | [transcript](work/AutoNTT_Automatic_Architecture_Design_and_Exploration_for_Number_Theoretic_Trans-906c9404/run_area/.pi/sessions/2026-11-23T05-54-40-876Z_01a206f8-886b-72f3-b0fb-07e9e99d6d52.html) |
| with_repo | InTAR: Inter-Task Auto-Reconfigurable Accelerator Design for High Dat… | already_known | OswaldHe/InTAR (already_known) |  | [transcript](work/InTAR_Inter-Task_Auto-Reconfigurable_Accelerator_Design_for_High_Data_Volume_Var-79bd66b0/run_area/.pi/sessions/2026-11-23T05-54-40-426Z_01a206f8-86a9-77d7-ba4c-b20b28a81c61.html) |
| without_repo | Exploring Code Language Models for Automated HLS-based Hardware Gener… | hls_found | jiahaogai/LLM-for-HLS (hls_found, handoff), UCLA-DM/HLSyn (needs_review), UT-LCA/ML4Accel-Dataset (hls_found, handoff) |  | [transcript](work/3658617.3697616/run_area/.pi/sessions/2026-11-23T06-30-58-595Z_01a20719-c322-7746-afb5-d25037dab5d3.html) |
| without_repo | CRUSH: A Credit-Based Approach for Functional Unit Sharing in Dynamic… | hls_found | EPFL-LAP/dynamatic (hls_found, handoff) |  | [transcript](work/3669940.3707273/run_area/.pi/sessions/2026-11-23T05-54-40-622Z_01a206f8-876d-7080-bff3-4bbbd8bebb93.html) |
| without_repo | CollectiveHLS: A Collaborative Approach to High-Level Synthesis Desig… | needs_review | aferikoglou/CollectiveHLS (needs_review) |  | [transcript](work/3702005/run_area/.pi/sessions/2026-11-23T05-54-39-820Z_01a206f8-844b-708c-85d8-53fc76f40e97.html) |
| without_repo | A Speculative Loop Pipeline Framework with Accurate Path Modeling for… | hls_found | jianyicheng/HLS-benchmarks (hls_found, handoff) |  | [transcript](work/3705732/run_area/.pi/sessions/2026-11-23T05-55-51-985Z_01a206f9-9e30-737f-ba23-383468e5fd80.html) |
| without_repo | An Empirical Comparision of LLM-based Hardware Design and High-level … | not_found |  |  | [transcript](work/3706628.3708861/run_area/.pi/sessions/2026-11-23T05-56-30-035Z_01a206fa-32d2-75a3-8543-c3393a2e0541.html) |
| without_repo | Dynamic Loop Fusion in High-Level Synthesis. | repo_no_hls | robertszafa/elastic-sycl-hls (repo_no_hls) |  | [transcript](work/3706628.3708871/run_area/.pi/sessions/2026-11-23T05-56-39-062Z_01a206fa-5615-717b-afd0-eacf3e2f9dd3.html) |
| without_repo | ADVISOR: Approximate Computing-frienDly High-LeVel Synthesis DesIgn S… | already_known | DARClab-UTD/S2CBench (already_known), ehw-fit/evoapproxlib (repo_no_hls) |  | [transcript](work/ADVISOR_Approximate_Computing-frienDly_High-LeVel_Synthesis_DesIgn_Space_ExplORe-90715d9a/run_area/.pi/sessions/2026-11-23T05-56-43-614Z_01a206fa-67dd-7201-b242-3f04c795cd95.html) |
| without_repo | APPLE-DSE: Asynchronous Parallel Pareto Set Learning for Microarchite… | repo_no_hls | riscv-software-src/riscv-tests (repo_no_hls) | 1 | [transcript](work/APPLE-DSE_Asynchronous_Parallel_Pareto_Set_Learning_for_Microarchitecture_Design-96742203/run_area/.pi/sessions/2026-11-23T05-58-16-085Z_01a206fb-d114-775f-92fa-4edb29059f6b.html) |
| without_repo | A FeFET-Based Compute-in-Memory Architecture on FPGA for Neural Netwo… | not_found |  |  | [transcript](work/A_FeFET-Based_Compute-in-Memory_Architecture_on_FPGA_for_Neural_Network_Inferenc-0eb75663/run_area/.pi/sessions/2026-11-23T05-58-30-582Z_01a206fc-09b5-7201-afa8-a20df1bbb7a4.html) |
| without_repo | A Post-Implementation Performance Prediction Method with HLS Optimiza… | not_found |  |  | [transcript](work/A_Post-Implementation_Performance_Prediction_Method_with_HLS_Optimization_Direct-5e1187ad/run_area/.pi/sessions/2026-11-23T05-59-16-132Z_01a206fc-bba3-7070-8382-37effd4b25c6.html) |
| without_repo | A Reinforcement Learning Based Multi-objective Evolutionary Algorithm… | hls_found | 23ydqian/QLearning_MOEA (repo_no_hls), breagen/MachSuite (hls_found, handoff), lorenzoferretti/db4hls (repo_no_hls) |  | [transcript](work/A_Reinforcement_Learning_Based_Multi-objective_Evolutionary_Algorithm_for_High-l-5585df5e/run_area/.pi/sessions/2026-11-23T06-28-18-714Z_01a20717-529a-72fa-9f9a-857e90eb5d12.html) |
| without_repo | A Systematic Methodology for Time-multiplexing Algorithms on a Reconf… | not_found |  |  | [transcript](work/A_Systematic_Methodology_for_Time-multiplexing_Algorithms_on_a_Reconfigurable_Sy-bde51c71/run_area/.pi/sessions/2026-11-23T06-01-31-561Z_01a206fe-cca8-725d-b7e5-8dcc1dda582a.html) |
| without_repo | AccelES: Accelerating Top-K SpMV for Embedding Similarity via Low-bit… | hls_found | zhaijiaqi/AccelES (hls_found, handoff) | 1 | [transcript](work/AccelES_Accelerating_Top-K_SpMV_for_Embedding_Similarity_via_Low-bit_Pruning/run_area/.pi/sessions/2026-11-23T06-03-14-128Z_01a20700-5d4f-73f9-b6e0-cc5f64278a7f.html) |
| without_repo | AttentionLib: A Scalable Optimization Framework for Automated Attenti… | not_found |  |  | [transcript](work/AttentionLib_A_Scalable_Optimization_Framework_for_Automated_Attention_Accelerat-640628cb/run_area/.pi/sessions/2026-11-23T06-03-19-317Z_01a20700-7194-7562-b0ce-a6c35fed2d97.html) |
| without_repo | Automated Bitstream-Level Cost-Reliability Design-Space Exploration f… | repo_no_hls | byuccl/bfat (repo_no_hls), f4pga/prjxray (repo_no_hls), olofk/serv (repo_no_hls), stnolting/neorv32 (repo_no_hls) | 3 | [transcript](work/Automated_Bitstream-Level_Cost-Reliability_Design-Space_Exploration_for_SRAM-Bas-81239172/run_area/.pi/sessions/2026-11-23T06-04-33-748Z_01a20701-9453-773b-9911-2d0c9c904767.html) |
| without_repo | Banked Memories for Soft SIMT Processors | not_found |  |  | [transcript](work/Banked_Memories_for_Soft_SIMT_Processors/run_area/.pi/sessions/2026-11-23T06-04-44-562Z_01a20701-be91-74b8-9959-9e562bbd3919.html) |
| without_repo | CHESS: Compiling Homomorphic Encryption with Scheme Switching | repo_no_hls | TrustworthyComputing/CHESS (repo_no_hls) | 1 | [transcript](work/CHESS_Compiling_Homomorphic_Encryption_with_Scheme_Switching/run_area/.pi/sessions/2026-11-23T06-05-07-321Z_01a20702-1778-7446-9830-95a1bac318b6.html) |
| without_repo | Can Reasoning Models Reason about Hardware? An Agentic HLS Perspective | hls_found | Lucaz97/Agentic-HLS (hls_found, handoff) |  | [transcript](work/Can_Reasoning_Models_Reason_about_Hardware_An_Agentic_HLS_Perspective/run_area/.pi/sessions/2026-11-23T06-05-11-538Z_01a20702-27f1-7552-a641-597d6978e774.html) |
| without_repo | Cayman: Custom Accelerator Generation with Control Flow and Data Acce… | not_found |  |  | [transcript](work/Cayman_Custom_Accelerator_Generation_with_Control_Flow_and_Data_Access_Optimizat-e72c9bc5/run_area/.pi/sessions/2026-11-23T06-06-03-466Z_01a20702-f2c9-711f-87e3-d351cb1b9482.html) |
| without_repo | Chronbench: An Incremental HDL Benchmark Suite | repo_no_hls | ZakSN/chronbench (repo_no_hls), DanieleParravicini/regex_coprocessor (repo_no_hls), openhwfoundation/cva5 (repo_no_hls), ZipCPU/zipcpu (repo_no_hls), jotego/jt12 (repo_no_hls), vortexgpgpu/vortex (repo_no_hls) | 2 | [transcript](work/Chronbench_An_Incremental_HDL_Benchmark_Suite/run_area/.pi/sessions/2026-11-23T06-06-13-778Z_01a20703-1b11-7316-96dd-91c763aaf7a3.html) |
| without_repo | DE2: SAT-Based Sequential Logic Decryption with a Functional Descript… | not_found |  |  | [transcript](work/DE2_SAT-Based_Sequential_Logic_Decryption_with_a_Functional_Description/run_area/.pi/sessions/2026-11-23T06-06-31-516Z_01a20703-605b-766d-8a77-b8917c03cb90.html) |
| without_repo | Dataflow Optimized Reconfigurable Acceleration for FEM-Based CFD Simu… | not_found |  |  | [transcript](work/Dataflow_Optimized_Reconfigurable_Acceleration_for_FEM-Based_CFD_Simulations/run_area/.pi/sessions/2026-11-23T06-08-22-267Z_01a20705-10fa-7713-9c24-e93a5bfa21ea.html) |
| without_repo | DyREM: Dynamically Mitigating Quantum Readout Error with Embedded Acc… | not_found |  |  | [transcript](work/DyREM_Dynamically_Mitigating_Quantum_Readout_Error_with_Embedded_Accelerator/run_area/.pi/sessions/2026-11-23T06-08-50-543Z_01a20705-7f6e-77fb-823a-4636a5dac9ef.html) |
| without_repo | Efficient Digital Beamforming for Satellite Payloads Using a 2D FFT-B… | not_found |  |  | [transcript](work/Efficient_Digital_Beamforming_for_Satellite_Payloads_Using_a_2D_FFT-Based_Parall-f4d5c534/run_area/.pi/sessions/2026-11-23T06-10-06-068Z_01a20706-a673-7778-83eb-e35c0a0f28e3.html) |
| without_repo | Efficient Optimization with Encoded Ising Models | needs_review | ncsys-lab/FUSE (needs_review) | 1 | [transcript](work/Efficient_Optimization_with_Encoded_Ising_Models/run_area/.pi/sessions/2026-11-23T06-33-36-262Z_01a2071c-2b06-7152-b2b0-c2b91af81856.html) |
| without_repo | Efficient and Scalable Architectures for Multi-level Superconducting … | repo_no_hls | satvikmaurya/HERQULES (repo_no_hls) |  | [transcript](work/Efficient_and_Scalable_Architectures_for_Multi-level_Superconducting_Qubit_Reado-568389fd/run_area/.pi/sessions/2026-11-23T06-11-35-585Z_01a20708-0420-7417-aa31-26200f207390.html) |
| without_repo | Exploiting Power Side-Channel Vulnerabilities in XGBoost Accelerator | not_found |  |  | [transcript](work/Exploiting_Power_Side-Channel_Vulnerabilities_in_XGBoost_Accelerator/run_area/.pi/sessions/2026-11-23T06-11-38-192Z_01a20708-0e4f-7431-944b-4bd62ad9346c.html) |
| without_repo | RPS-KNN: An Ultra-Fast FPGA Accelerator of Range-Projection-Structure… | not_found |  |  | [transcript](work/RPS-KNN_An_Ultra-Fast_FPGA_Accelerator_of_Range-Projection-Structure_K-Nearest-N-505efba4/run_area/.pi/sessions/2026-11-23T06-12-00-247Z_01a20708-6476-7028-906d-0394f97727d4.html) |

## API usage

Proxy calls: code_search 83, core 307, search 360, web 56.  
Exa queries billed (not cached): 50, about $0.35.  
LLM token cost is not tracked yet (see OpenRouter usage).

## Files in this folder

- `summary.csv`: one row per paper. `handoff.csv` / `handoff_repos.txt`: repos for HLSFactory.
- `review_list.csv`: repos needing a human look, with evidence quotes. `stats.json`: counts.
- `papers/<id>.json`: full per-paper record (evidence, HLS scan, flags, problems).
- `work/<id>/`: prompt, agent output, PDF pre-pass, pi transcript (`run_area/.pi/sessions/*.html`).
- `run.log` (with `--detach`), `proxy_calls.jsonl`, `registry.sqlite`: logs and internal state.
