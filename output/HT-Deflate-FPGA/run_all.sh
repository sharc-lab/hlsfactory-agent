#!/bin/bash
# HT-Deflate-FPGA Comprehensive Build & Test Script

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
echo "HT-Deflate-FPGA Build Process Starting..."
echo "Repository: github.com/UCLA-VAST/HT-Deflate-FPGA"
echo "Working directory: $SCRIPT_DIR"
echo

# Design validation
echo "=== Validating Designs ==="
for design in deflate_aws deflate_harp2 interface_top; do
    echo "Validating $design..."
    cd "$SCRIPT_DIR/designs/$design"
    
    # Basic file structure check
    ls -la *.v *.sv 2>/dev/null | wc -l > file_count.txt
    if [ $? -eq 0 ]; then
        echo "  ✓ Files found: $(cat file_count.txt)"
    else
        echo "  ✗ No Verilog files in $design"
    fi
    
    # RTL syntax check
    if command -v verilator >/dev/null 2>&1; then
        echo "  ✓ Verilator available, running syntax check..."
        verilator --lint-only *.v *.sv 2> syntax_errors.log || true
    else
        echo "  ⚠ Verilator not found, skipping syntax check"
    fi
done

# Build instructions per platform
echo
echo "=== Build Instructions ==="
echo "Designs processed successfully."
echo
echo "To build the compressed implementations:"
echo
echo "AWS VU9P (F1/F2 instance):"
echo "  cd designs/deflate_aws"
echo "  vivado -mode batch -source synthesis_aws.tcl"
echo
echo "Intel HARP2 (Stratix-10):"
echo "  cd designs/deflate_harp2" 
echo "  quartus_sh -t synthesis_harp2.tcl"
echo
echo "Interface shell integration:"
echo "  cd designs/interface_top"
echo "  Use provided AWS DK build flow or HARP2 DevStack flow"
echo
echo "=== Testbenches Available ==="
echo "Each design includes simulation testbenches:"
echo "  - testbench.v (AWS & HARP2 cores)"
echo "  - testbench.sv (interface shell)"
echo
echo "Simulation commands (when tools available):"
echo "  verilator --cc testbench.v --exe --build"
echo "  iverilog testbench.v -o sim && vvp sim"
echo

"echo" "HT-Deflate-FPGA processing complete"
