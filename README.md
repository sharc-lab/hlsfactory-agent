# HLSFactory-Agent

HLSFactory-Agent is an open-source tool for automatically extracting standalone HLS designs from academic and open-source repositories.

It helps identify HLS kernels, collect the files needed to build them, and package each design into a standalone Vitis HLS-compatible project when possible.

## Features

- Finds HLS kernels and top-level functions
- Extracts each design into its own folder
- Copies required source files, headers, and test data
- Adapts non-Vitis HLS code when possible
- Generates a `synth.tcl` script for each design
- Saves logs, session traces, and HTML transcripts for debugging

The repository also includes scripts for finding HLS-related papers and repositories from conference proceedings and metadata sources.

## Repository Layout

```text
hlsfactory_agent/      Core agent code
docker_images/         Docker image used to run the agent
exp/run_base/          Example batch run and analysis scripts
hls_source_indexing/   Scripts and data for indexing HLS-related papers
````

## Requirements

* Python 3.12+
* Docker
* Git
* OpenRouter API key
* Vitis HLS, for validating extracted designs

## Setup

Clone the repository:

```sh
git clone https://github.com/sharc-lab/hlsfactory-agent.git
cd hlsfactory-agent
```

Build the Docker image:

```sh
cd docker_images
./build.sh
```

This creates the Docker image:

```sh
hlsfactory-agent
```

Create a `.env` file in the repository root with your OpenRouter API key:

```env
OPENROUTER_API_KEY=your_openrouter_key
```

## Running the Agent Demo

The main example workflow is:

```sh
python exp/run_base/run.py
```

This script clones a predefined list of repositories, runs HLSFactory-Agent inside Docker, and writes outputs to:

```text
exp/run_base/runs/
```

For each repository, the agent:

1. Analyzes the repository structure
2. Finds possible HLS designs
3. Creates one folder per design
4. Copies source files, headers, and test data
5. Converts code toward Vitis HLS compatibility when needed
6. Generates a `synth.tcl` script
7. Saves logs and session transcripts

You can modify this script to attempt to extract HLS designs from repositories you are intsted in.

## Outputs

Each run produces:

* Extracted standalone design folders
* `run_data.json`
* Pi session logs
* HTML session transcripts

Outputs are stored under:

```text
exp/run_base/runs/
```

## Analyzing Results

After running extraction jobs, analyze the outputs with:

```sh
python exp/run_base/analyze_output_designs.py
```

Summary figures are written to:

```text
exp/run_base/figures/
```

## HLS Source Indexing

The `hls_source_indexing/` directory contains scripts for finding candidate HLS papers and repositories.

The indexing workflow searches venues such as:

```text
FPGA, FCCM, FPL, FPT, HEART, TRETS, DAC, ICCAD, ASP-DAC,
DATE, MLCAD, ICLAD, GLVLSI, HOST, TCAD, ISCA, MICRO,
HPCA, ESWEEK, MLSYS, ASPLOS
```
