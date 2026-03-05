# self-attn-intrra

## Description
Self-attention with InTAR optimization

## Top Function
`send_inst_cc0`

## Source Files
- `self-attn-intrra-kernel.cpp`
- `self-attn-intrra-host.cpp`
- `self-attn.cpp`

## Usage
To synthesize this design with Vitis HLS:
```bash
vitis_hls -f run_hls.tcl
```

## Notes
This design was extracted from the InTAR repository.
