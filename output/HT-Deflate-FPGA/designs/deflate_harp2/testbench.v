`timescale 1ns/1ps

module deflate_harp2_tb;
    // Clock and reset signals
    reg clk;
    reg rst_n;
    
    // Input interface signals
    reg [511:0] input_data;
    reg input_valid;
    wire input_ready;
    reg [31:0] input_size;
    reg input_last;
    
    // Output interface signals
    wire [511:0] output_data;
    wire output_valid;
    reg output_ready;
    wire [31:0] output_size;
    wire output_last;
    wire [63:0] output_crc;
    
    // Test data
    reg [7:0] test_data [0:511];
    integer i;
    
    // Device under test instantiation
    deflate_top dut (
        .clk(clk),
        .rst_n(rst_n),
        .input_data(input_data),
        .input_valid(input_valid),
        .input_ready(input_ready),
        .input_size(input_size),
        .input_last(input_last),
        .output_data(output_data),
        .output_valid(output_valid),
        .output_ready(output_ready),
        .output_size(output_size),
        .output_last(output_last),
        .output_crc(output_crc)
    );
    
    // Clock generation
    initial begin
        clk = 0;
        forever #2.5 clk = ~clk; // 200MHz clock
    end
    
    // Test stimulus
    initial begin
        // Initialize
        rst_n = 0;
        input_data = 0;
        input_valid = 0;
        input_size = 0;
        input_last = 0;
        output_ready = 1;
        
        // Create sample test data
        for (i = 0; i < 512; i = i + 1) begin
            test_data[i] = i % 128;
        end
        
        // Reset sequence
        #20;
        rst_n = 1;
        #20;
        
        $display("HARP2 Deflate Test: Starting...");
        
        // Send test data (512 bytes)
        input_size = 512;
        input_last = 0;
        input_valid = 1;
        
        for (i = 0; i < 8; i = i + 1) begin
            input_data = {
                test_data[i*64+63], test_data[i*64+62], test_data[i*64+61], test_data[i*64+60],
                test_data[i*64+59], test_data[i*64+58], test_data[i*64+57], test_data[i*64+56],
                test_data[i*64+55], test_data[i*64+54], test_data[i*64+53], test_data[i*64+52],
                test_data[i*64+51], test_data[i*64+50], test_data[i*64+49], test_data[i*64+48],
                test_data[i*64+47], test_data[i*64+46], test_data[i*64+45], test_data[i*64+44],
                test_data[i*64+43], test_data[i*64+42], test_data[i*64+41], test_data[i*64+40],
                test_data[i*64+39], test_data[i*64+38], test_data[i*64+37], test_data[i*64+36],
                test_data[i*64+35], test_data[i*64+34], test_data[i*64+33], test_data[i*64+32],
                test_data[i*64+31], test_data[i*64+30], test_data[i*64+29], test_data[i*64+28],
                test_data[i*64+27], test_data[i*64+26], test_data[i*64+25], test_data[i*64+24],
                test_data[i*64+23], test_data[i*64+22], test_data[i*64+21], test_data[i*64+20],
                test_data[i*64+19], test_data[i*64+18], test_data[i*64+17], test_data[i*64+16],
                test_data[i*64+15], test_data[i*64+14], test_data[i*64+13], test_data[i*64+12],
                test_data[i*64+11], test_data[i*64+10], test_data[i*64+9], test_data[i*64+8],
                test_data[i*64+7], test_data[i*64+6], test_data[i*64+5], test_data[i*64+4],
                test_data[i*64+3], test_data[i*64+2], test_data[i*64+1], test_data[i*64]
            };
            @(posedge clk);
            #1;
            while (!input_ready) @(posedge clk);
        end
        
        input_valid = 0;
        input_last = 1;
        @(posedge clk);
        
        $display("HARP2 Deflate Test: Input complete");
        
        // Monitor for completion
        repeat (800) @(posedge clk);
        
        $display("HARP2 Deflate Test: Completed");
        $finish;
    end
    
    // Logging
    initial begin
        $monitor("[%0t] valid=%b data=%016h size=%d", $time, output_valid, output_data, output_size);
    end
    
    // Simulation dump
    initial begin
        $dumpfile("deflate_harp2_tb.vcd");
        $dumpvars(0, deflate_harp2_tb);
    end
    
endmodule
