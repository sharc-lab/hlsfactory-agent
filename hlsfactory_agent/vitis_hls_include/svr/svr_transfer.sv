// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.

`ifndef SVR_TRANSFER__SV
    `define SVR_TRANSFER__SV

    class svr_transfer#(int DATA_WIDTH = 32) extends uvm_sequence_item;
        rand logic [DATA_WIDTH - 1: 0] data;
        rand int delay;
        
        constraint data_range {
            data >= 0;
            data <= 1000;
        }
        
        constraint delay_range {
            delay >= -30;
            delay <= 30;
        }

        `uvm_object_param_utils_begin (svr_transfer#(DATA_WIDTH))
        `uvm_field_int(data , UVM_ALL_ON)
        `uvm_field_int(delay, UVM_ALL_ON|UVM_DEC)
        `uvm_object_utils_end

        function new (string name = "svr_transfer");
            super.new();
        endfunction

    endclass

    class svr_readwrite_transfer#(int DATA_WIDTH = 32) extends svr_transfer#(DATA_WIDTH);
        bit isupdate;
        `uvm_object_param_utils_begin (svr_readwrite_transfer#(DATA_WIDTH))
            `uvm_field_int(isupdate , UVM_ALL_ON)
        `uvm_object_utils_end
        function new (string name = "svr_readwrite_transfer");
            super.new(name);
            isupdate = 0;
            delay = 0;
        endfunction
    endclass
`endif 
