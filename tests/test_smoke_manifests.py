import json
from pathlib import Path

import pytest

# Import models from the agent for schema validation
from hlsfactory_agent import HLSDesigns, SubComponents  # type: ignore


REPO_ROOT = Path(__file__).resolve().parents[1]
HLS_OUT_DIR = REPO_ROOT / "HLSDesigns"


def _load_json(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


@pytest.mark.parametrize(
    "design_dir_name",
    [
        "extracted_designs",
        "extracted_designs_2",
        "extracted_designs_smoke",
    ],
)
def test_designs_manifest_schema(design_dir_name: str) -> None:
    design_dir = HLS_OUT_DIR / design_dir_name
    if not design_dir.exists():
        pytest.skip(f"{design_dir} not present")
    manifest_path = design_dir / "designs.json"
    assert manifest_path.exists(), f"Missing manifest: {manifest_path}"
    manifest = _load_json(manifest_path)
    # Validate against Pydantic model
    HLSDesigns.model_validate(manifest)


@pytest.mark.parametrize(
    "design_dir_name",
    [
        "extracted_designs",
        "extracted_designs_2",
        "extracted_designs_smoke",
    ],
)
def test_subcomponents_schema(design_dir_name: str) -> None:
    design_dir = HLS_OUT_DIR / design_dir_name
    if not design_dir.exists():
        pytest.skip(f"{design_dir} not present")

    sub_dir = design_dir / "subcomponents"
    assert sub_dir.exists(), f"Missing subcomponents dir: {sub_dir}"

    # Ensure every JSON file parses and has a list of strings
    for json_path in sorted(sub_dir.glob("*.json")):
        data = _load_json(json_path)
        model = SubComponents.model_validate(data)
        assert isinstance(model.sub_components, list)
        for item in model.sub_components:
            assert isinstance(item, str), f"Non-string subcomponent in {json_path}"


