`timescale 1ns/1ps

module interface_top_tb;
    // Shell clock (250 MHz)
    reg clk_main;
    reset synchronized;
    reg rst_n;
    
    // PCIe interface (simulated)
    reg [7:0] pcie_tdata;
    reg pcie_tvalid;
    wire pcie_tready;
    reg pcie_tlast;
    
    // AXI-Lite control interface (from host)
    reg [31:0] s_axi_awaddr;
    reg [31:0] s_axi_wdata;
    reg s_axi_awvalid;
    reg s_axi_wvalid;
    wire s_axi_awready;
    wire s_axi_wready;
    
    // Status output
    wire [63:0] status_out;
    
    integer i;
    integer test_cycle;
    
    // Instantiate the shell
    cl_main dut (
        .clk_main(clk_main),
        .rst_main_n(rst_n),
        .pci_exp_txp(),
        .pci_exp_txn(),
        .pci_exp_rxp(32'b0),
        .pci_exp_rxn(32'b0),
        .sys_clk_p(clk_main),
        .sys_clk_n(~clk_main),
        .sys_rst_n(rst_n)
    );
    
    // Clock generation (250MHz PCIe base clock)
    initial begin
        clk_main = 0;
        forever #2 clk_main = ~clk_main;
    end
    
    // Reset sequence
    initial begin
        rst_n = 0;
        pcie_tdata = 0;
        pcie_tvalid = 0;
        pcie_tlast = 0;
        s_axi_awaddr = 32'h00000000;
        s_axi_wdata = 32'h00000000;
        s_axi_awvalid = 0;
        s_axi_wvalid = 0;
        test_cycle = 0;
        
        #20 rst_n = 1;
        #20;
        
        $display("Interface Shell Test Activating...");
        
        // DAC initialization sequence (simplified)
        repeat (10) @(posedge clk_main);
        
        // Informal test: select slot 0 as "compressor" and trigger start
        @(posedge clk_main);
        s_axi_awaddr = 32'h00010000; // slot-0 CSR
        s_axi_wdata  = 32'h00000001; // start
        s_axi_awvalid = 1; s_axi_wvalid = 1;
        @(posedge clk_main);
        repeat(8) @(posedge clk_main);
        s_axi_awvalid = 0; s_axi_wvalid = 0;
        
        $display("Shell-Control triggered");
        
        // Monitor responses for 1000 cycles
        repeat(1000) begin
            @(posedge clk_main);
            test_cycle++;
        end
        
        $display("Interface Shell Test: logs captured");
        $finish;
    end
    
    initial begin
        $monitor("[%0t] cycle=%0d status=%016h", $time, test_cycle, status_out);
    end
    
    initial begin
        $dumpfile("interface_top_tb.vcd");
        $dumpvars(0, interface_top_tb);
    end
    
endmodule
