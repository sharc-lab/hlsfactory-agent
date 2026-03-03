`timescale 1ns/1ps

module deflate_aws_tb;
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
    reg [7:0] test_data [0:1023];
    integer i;
    integer cycle_count;
    
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
        forever #2 clk = ~clk; // 250MHz clock
    end
    
    // Test stimulus
    initial begin
        // Initialize signals
        rst_n = 0;
        input_data = 0;
        input_valid = 0;
        input_size = 0;
        input_last = 0;
        output_ready = 1;
        cycle_count = 0;
        
        // Fill test data - simple repeating pattern
        for (i = 0; i < 1024; i = i + 1) begin
            test_data[i] = i % 256;
        end
        
        // Wait for reset
        #10;
        rst_n = 1;
        #10;
        
        // Start compression test
        $display("Starting deflate compression test...");
        
        // Send test data
        input_size = 1024;
        input_last = 0;
        input_valid = 1;
        
        for (i = 0; i < 16; i = i + 1) begin
            input_data = {test_data[i*8+7], test_data[i*8+6], test_data[i*8+5], test_data[i*8+4],
                          test_data[i*8+3], test_data[i*8+2], test_data[i*8+1], test_data[i*8]};
            @(posedge clk);
            #1;
            while (!input_ready) @(posedge clk);
        end
        
        input_valid = 0;
        input_last = 1;
        @(posedge clk);
        
        // Wait for compression to complete
        $display("Waiting for compression completion...");
        repeat (1000) @(posedge clk);
        
        $display("Compression test completed");
        $finish;
    end
    
    // Monitor output
    initial begin
        $monitor("Time: %0t, output_valid: %b, output_data: %h, output_size: %0d", 
                 $time, output_valid, output_data, output_size);
    end
    
    // Timeout watchdog
    initial begin
        #100000;
        $display("Testbench timeout");
        $finish;
    end
    
    // Generate VCD file for waveform viewing
    initial begin
        $dumpfile("deflate_aws_tb.vcd");
        $dumpvars(0, deflate_aws_tb);
    end
    
endmodule
