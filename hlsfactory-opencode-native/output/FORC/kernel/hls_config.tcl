# HLS Configuration for Individual Modules

# Module: orcDecomp - ORC Header Processing
proc synth_orcDecomp {} {
    open_project orcDecomp_prj
    set_top DecompHead
    add_files orcDecomp.h
    add_files orc_proc.h
    open_solution "sol1"
    set_part {xcu280-fsvh2892-2L-e}
    create_clock -period 3.0 -name default
    config_interface -m_axi_addr64
    config_compile -pipeline_style stp
    csynth_design
    export_design -rtl verilog
    close_project
}

# Module: zlibTapa - Zlib Decompression
proc synth_zlibTapa {} {
    open_project zlibTapa_prj
    set_top huffmanDecoder
    add_files zlibTapa.h
    add_files orc_proc.h
    add_files fixed_codes.hpp
    open_solution "sol1"
    set_part {xcu280-fsvh2892-2L-e}
    create_clock -period 3.0 -name default
    config_interface -m_axi_addr64
    csynth_design
    export_design -rtl verilog
    close_project
}

# Module: orc_decoder - ORC Decoder
proc synth_orc_decoder {} {
    open_project orc_decoder_prj
    set_top load
    add_files orc_decoder.h
    add_files orc_proc.h
    open_solution "sol1"
    set_part {xcu280-fsvh2892-2L-e}
    create_clock -period 3.0 -name default
    config_interface -m_axi_addr64
    csynth_design
    export_design -rtl verilog
    close_project
}

# Module: orc_filter - Data Filtering
proc synth_orc_filter {} {
    open_project orc_filter_prj
    set_top FilterData
    add_files orc_filter.h
    add_files orc_proc.h
    open_solution "sol1"
    set_part {xcu280-fsvh2892-2L-e}
    create_clock -period 3.0 -name default
    config_interface -m_axi_addr64
    csynth_design
    export_design -rtl verilog
    close_project
}
