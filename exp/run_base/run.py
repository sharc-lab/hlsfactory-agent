from pathlib import Path

from dotenv import dotenv_values
from joblib import Parallel, delayed

from hlsfactory_agent.core import HLSFactoryAgentRun
from hlsfactory_agent.utils import check_key

repo_urls = [
    "AlexMontgomerie/fpgaconvnet-hls",
    "DARClab-UTD/S2CBench",
    "ETHZ-DYNAMO/balor",
    "FedericoSerafini/HLS-CNN",
    "KastnerRG/Spector-HLS",
    "OswaldHe/InTAR",
    "SFU-HiAccel/AutoNTT",
    "SFU-HiAccel/BitBlender",
    "SFU-HiAccel/CHIP-KNN",
    "SFU-HiAccel/FORC",
    "SFU-HiAccel/HiSpMV",
    "SFU-HiAccel/SyncNN",
    "SFU-HiAccel/blaze",
    "SFU-HiAccel/pasta",
    "SFU-HiAccel/SERI",
    "TurakhiaLab/DP-HLS",
    "UCLA-VAST/CLINK",
    "UCLA-VAST/HP-FFT-HLS",
    "UIUC-ChenLab/ScaleHLS-HIDA",
    "Xtra-Computing/ThunderGP",
    "ZongyueQin/ProgSG",
    "bsc-loca/PQC-Crystals-HLS-Accelerators",
    "icl-utk-edu/hpcc",
    "robertoBosio/NN2FPGA",
    "spcl/gemm_hls",
    "ECASLab/hls-fpga-accelerators",
]

API_KEY_OPENROUTER = check_key(dotenv_values(".env")["OPENROUTER_API_KEY"])
MODEL = "deepseek/deepseek-v4-flash"

DIR_CURRENT = Path(__file__).resolve().parent

DIR_RUNS = DIR_CURRENT / "runs"
DIR_RUNS.mkdir(parents=True, exist_ok=True)


def run_single(repo_url: str):
    repo_id = repo_url
    repo_id_sanitized = repo_id.replace("/", "__")
    run_id = f"hlsfactory-agent-{repo_id_sanitized}"
    dir_run = DIR_RUNS / run_id
    run = HLSFactoryAgentRun(run_id, repo_id, dir_run, MODEL, API_KEY_OPENROUTER)
    run.run()

N_JOBS = 64
Parallel(n_jobs=N_JOBS)(delayed(run_single)(repo_url) for repo_url in repo_urls)