// soc_system.v

// Generated using ACDS version 17.1 590

`timescale 1 ps / 1 ps
module soc_system (
		input  wire        clk_clk,              //           clk.clk
		output wire [15:0] gpio_0_export_export, // gpio_0_export.export
		input  wire [15:0] gpio_1_export_export, // gpio_1_export.export
		input  wire [3:0]  gpio_2_export_export, // gpio_2_export.export
		output wire [7:0]  gpio_export_export,   //   gpio_export.export
		output wire [12:0] memory_mem_a,         //        memory.mem_a
		output wire [2:0]  memory_mem_ba,        //              .mem_ba
		output wire        memory_mem_ck,        //              .mem_ck
		output wire        memory_mem_ck_n,      //              .mem_ck_n
		output wire        memory_mem_cke,       //              .mem_cke
		output wire        memory_mem_cs_n,      //              .mem_cs_n
		output wire        memory_mem_ras_n,     //              .mem_ras_n
		output wire        memory_mem_cas_n,     //              .mem_cas_n
		output wire        memory_mem_we_n,      //              .mem_we_n
		output wire        memory_mem_reset_n,   //              .mem_reset_n
		inout  wire [7:0]  memory_mem_dq,        //              .mem_dq
		inout  wire        memory_mem_dqs,       //              .mem_dqs
		inout  wire        memory_mem_dqs_n,     //              .mem_dqs_n
		output wire        memory_mem_odt,       //              .mem_odt
		output wire        memory_mem_dm,        //              .mem_dm
		input  wire        memory_oct_rzqin,     //              .oct_rzqin
		input  wire        reset_reset_n         //         reset.reset_n
	);

	wire   [1:0] hps_0_h2f_lw_axi_master_awburst;       // hps_0:h2f_lw_AWBURST -> mm_interconnect_0:hps_0_h2f_lw_axi_master_awburst
	wire   [3:0] hps_0_h2f_lw_axi_master_arlen;         // hps_0:h2f_lw_ARLEN -> mm_interconnect_0:hps_0_h2f_lw_axi_master_arlen
	wire   [3:0] hps_0_h2f_lw_axi_master_wstrb;         // hps_0:h2f_lw_WSTRB -> mm_interconnect_0:hps_0_h2f_lw_axi_master_wstrb
	wire         hps_0_h2f_lw_axi_master_wready;        // mm_interconnect_0:hps_0_h2f_lw_axi_master_wready -> hps_0:h2f_lw_WREADY
	wire  [11:0] hps_0_h2f_lw_axi_master_rid;           // mm_interconnect_0:hps_0_h2f_lw_axi_master_rid -> hps_0:h2f_lw_RID
	wire         hps_0_h2f_lw_axi_master_rready;        // hps_0:h2f_lw_RREADY -> mm_interconnect_0:hps_0_h2f_lw_axi_master_rready
	wire   [3:0] hps_0_h2f_lw_axi_master_awlen;         // hps_0:h2f_lw_AWLEN -> mm_interconnect_0:hps_0_h2f_lw_axi_master_awlen
	wire  [11:0] hps_0_h2f_lw_axi_master_wid;           // hps_0:h2f_lw_WID -> mm_interconnect_0:hps_0_h2f_lw_axi_master_wid
	wire   [3:0] hps_0_h2f_lw_axi_master_arcache;       // hps_0:h2f_lw_ARCACHE -> mm_interconnect_0:hps_0_h2f_lw_axi_master_arcache
	wire         hps_0_h2f_lw_axi_master_wvalid;        // hps_0:h2f_lw_WVALID -> mm_interconnect_0:hps_0_h2f_lw_axi_master_wvalid
	wire  [20:0] hps_0_h2f_lw_axi_master_araddr;        // hps_0:h2f_lw_ARADDR -> mm_interconnect_0:hps_0_h2f_lw_axi_master_araddr
	wire   [2:0] hps_0_h2f_lw_axi_master_arprot;        // hps_0:h2f_lw_ARPROT -> mm_interconnect_0:hps_0_h2f_lw_axi_master_arprot
	wire   [2:0] hps_0_h2f_lw_axi_master_awprot;        // hps_0:h2f_lw_AWPROT -> mm_interconnect_0:hps_0_h2f_lw_axi_master_awprot
	wire  [31:0] hps_0_h2f_lw_axi_master_wdata;         // hps_0:h2f_lw_WDATA -> mm_interconnect_0:hps_0_h2f_lw_axi_master_wdata
	wire         hps_0_h2f_lw_axi_master_arvalid;       // hps_0:h2f_lw_ARVALID -> mm_interconnect_0:hps_0_h2f_lw_axi_master_arvalid
	wire   [3:0] hps_0_h2f_lw_axi_master_awcache;       // hps_0:h2f_lw_AWCACHE -> mm_interconnect_0:hps_0_h2f_lw_axi_master_awcache
	wire  [11:0] hps_0_h2f_lw_axi_master_arid;          // hps_0:h2f_lw_ARID -> mm_interconnect_0:hps_0_h2f_lw_axi_master_arid
	wire   [1:0] hps_0_h2f_lw_axi_master_arlock;        // hps_0:h2f_lw_ARLOCK -> mm_interconnect_0:hps_0_h2f_lw_axi_master_arlock
	wire   [1:0] hps_0_h2f_lw_axi_master_awlock;        // hps_0:h2f_lw_AWLOCK -> mm_interconnect_0:hps_0_h2f_lw_axi_master_awlock
	wire  [20:0] hps_0_h2f_lw_axi_master_awaddr;        // hps_0:h2f_lw_AWADDR -> mm_interconnect_0:hps_0_h2f_lw_axi_master_awaddr
	wire   [1:0] hps_0_h2f_lw_axi_master_bresp;         // mm_interconnect_0:hps_0_h2f_lw_axi_master_bresp -> hps_0:h2f_lw_BRESP
	wire         hps_0_h2f_lw_axi_master_arready;       // mm_interconnect_0:hps_0_h2f_lw_axi_master_arready -> hps_0:h2f_lw_ARREADY
	wire  [31:0] hps_0_h2f_lw_axi_master_rdata;         // mm_interconnect_0:hps_0_h2f_lw_axi_master_rdata -> hps_0:h2f_lw_RDATA
	wire         hps_0_h2f_lw_axi_master_awready;       // mm_interconnect_0:hps_0_h2f_lw_axi_master_awready -> hps_0:h2f_lw_AWREADY
	wire   [1:0] hps_0_h2f_lw_axi_master_arburst;       // hps_0:h2f_lw_ARBURST -> mm_interconnect_0:hps_0_h2f_lw_axi_master_arburst
	wire   [2:0] hps_0_h2f_lw_axi_master_arsize;        // hps_0:h2f_lw_ARSIZE -> mm_interconnect_0:hps_0_h2f_lw_axi_master_arsize
	wire         hps_0_h2f_lw_axi_master_bready;        // hps_0:h2f_lw_BREADY -> mm_interconnect_0:hps_0_h2f_lw_axi_master_bready
	wire         hps_0_h2f_lw_axi_master_rlast;         // mm_interconnect_0:hps_0_h2f_lw_axi_master_rlast -> hps_0:h2f_lw_RLAST
	wire         hps_0_h2f_lw_axi_master_wlast;         // hps_0:h2f_lw_WLAST -> mm_interconnect_0:hps_0_h2f_lw_axi_master_wlast
	wire   [1:0] hps_0_h2f_lw_axi_master_rresp;         // mm_interconnect_0:hps_0_h2f_lw_axi_master_rresp -> hps_0:h2f_lw_RRESP
	wire  [11:0] hps_0_h2f_lw_axi_master_awid;          // hps_0:h2f_lw_AWID -> mm_interconnect_0:hps_0_h2f_lw_axi_master_awid
	wire  [11:0] hps_0_h2f_lw_axi_master_bid;           // mm_interconnect_0:hps_0_h2f_lw_axi_master_bid -> hps_0:h2f_lw_BID
	wire         hps_0_h2f_lw_axi_master_bvalid;        // mm_interconnect_0:hps_0_h2f_lw_axi_master_bvalid -> hps_0:h2f_lw_BVALID
	wire   [2:0] hps_0_h2f_lw_axi_master_awsize;        // hps_0:h2f_lw_AWSIZE -> mm_interconnect_0:hps_0_h2f_lw_axi_master_awsize
	wire         hps_0_h2f_lw_axi_master_awvalid;       // hps_0:h2f_lw_AWVALID -> mm_interconnect_0:hps_0_h2f_lw_axi_master_awvalid
	wire         hps_0_h2f_lw_axi_master_rvalid;        // mm_interconnect_0:hps_0_h2f_lw_axi_master_rvalid -> hps_0:h2f_lw_RVALID
	wire         mm_interconnect_0_pio_0_s1_chipselect; // mm_interconnect_0:pio_0_s1_chipselect -> pio_0:chipselect
	wire  [31:0] mm_interconnect_0_pio_0_s1_readdata;   // pio_0:readdata -> mm_interconnect_0:pio_0_s1_readdata
	wire   [2:0] mm_interconnect_0_pio_0_s1_address;    // mm_interconnect_0:pio_0_s1_address -> pio_0:address
	wire         mm_interconnect_0_pio_0_s1_write;      // mm_interconnect_0:pio_0_s1_write -> pio_0:write_n
	wire  [31:0] mm_interconnect_0_pio_0_s1_writedata;  // mm_interconnect_0:pio_0_s1_writedata -> pio_0:writedata
	wire  [31:0] mm_interconnect_0_pio_1_s1_readdata;   // pio_1:readdata -> mm_interconnect_0:pio_1_s1_readdata
	wire   [1:0] mm_interconnect_0_pio_1_s1_address;    // mm_interconnect_0:pio_1_s1_address -> pio_1:address
	wire         mm_interconnect_0_pio_2_s1_chipselect; // mm_interconnect_0:pio_2_s1_chipselect -> pio_2:chipselect
	wire  [31:0] mm_interconnect_0_pio_2_s1_readdata;   // pio_2:readdata -> mm_interconnect_0:pio_2_s1_readdata
	wire   [2:0] mm_interconnect_0_pio_2_s1_address;    // mm_interconnect_0:pio_2_s1_address -> pio_2:address
	wire         mm_interconnect_0_pio_2_s1_write;      // mm_interconnect_0:pio_2_s1_write -> pio_2:write_n
	wire  [31:0] mm_interconnect_0_pio_2_s1_writedata;  // mm_interconnect_0:pio_2_s1_writedata -> pio_2:writedata
	wire  [31:0] mm_interconnect_0_pio_3_s1_readdata;   // pio_3:readdata -> mm_interconnect_0:pio_3_s1_readdata
	wire   [1:0] mm_interconnect_0_pio_3_s1_address;    // mm_interconnect_0:pio_3_s1_address -> pio_3:address
	wire         rst_controller_reset_out_reset;        // rst_controller:reset_out -> [mm_interconnect_0:pio_0_reset_reset_bridge_in_reset_reset, pio_0:reset_n, pio_1:reset_n, pio_2:reset_n, pio_3:reset_n]
	wire         rst_controller_001_reset_out_reset;    // rst_controller_001:reset_out -> mm_interconnect_0:hps_0_h2f_lw_axi_master_agent_clk_reset_reset_bridge_in_reset_reset
	wire         hps_0_h2f_reset_reset;                 // hps_0:h2f_rst_n -> rst_controller_001:reset_in0

	soc_system_hps_0 #(
		.F2S_Width (0),
		.S2F_Width (0)
	) hps_0 (
		.mem_a          (memory_mem_a),                    //            memory.mem_a
		.mem_ba         (memory_mem_ba),                   //                  .mem_ba
		.mem_ck         (memory_mem_ck),                   //                  .mem_ck
		.mem_ck_n       (memory_mem_ck_n),                 //                  .mem_ck_n
		.mem_cke        (memory_mem_cke),                  //                  .mem_cke
		.mem_cs_n       (memory_mem_cs_n),                 //                  .mem_cs_n
		.mem_ras_n      (memory_mem_ras_n),                //                  .mem_ras_n
		.mem_cas_n      (memory_mem_cas_n),                //                  .mem_cas_n
		.mem_we_n       (memory_mem_we_n),                 //                  .mem_we_n
		.mem_reset_n    (memory_mem_reset_n),              //                  .mem_reset_n
		.mem_dq         (memory_mem_dq),                   //                  .mem_dq
		.mem_dqs        (memory_mem_dqs),                  //                  .mem_dqs
		.mem_dqs_n      (memory_mem_dqs_n),                //                  .mem_dqs_n
		.mem_odt        (memory_mem_odt),                  //                  .mem_odt
		.mem_dm         (memory_mem_dm),                   //                  .mem_dm
		.oct_rzqin      (memory_oct_rzqin),                //                  .oct_rzqin
		.h2f_rst_n      (hps_0_h2f_reset_reset),           //         h2f_reset.reset_n
		.h2f_lw_axi_clk (clk_clk),                         //  h2f_lw_axi_clock.clk
		.h2f_lw_AWID    (hps_0_h2f_lw_axi_master_awid),    // h2f_lw_axi_master.awid
		.h2f_lw_AWADDR  (hps_0_h2f_lw_axi_master_awaddr),  //                  .awaddr
		.h2f_lw_AWLEN   (hps_0_h2f_lw_axi_master_awlen),   //                  .awlen
		.h2f_lw_AWSIZE  (hps_0_h2f_lw_axi_master_awsize),  //                  .awsize
		.h2f_lw_AWBURST (hps_0_h2f_lw_axi_master_awburst), //                  .awburst
		.h2f_lw_AWLOCK  (hps_0_h2f_lw_axi_master_awlock),  //                  .awlock
		.h2f_lw_AWCACHE (hps_0_h2f_lw_axi_master_awcache), //                  .awcache
		.h2f_lw_AWPROT  (hps_0_h2f_lw_axi_master_awprot),  //                  .awprot
		.h2f_lw_AWVALID (hps_0_h2f_lw_axi_master_awvalid), //                  .awvalid
		.h2f_lw_AWREADY (hps_0_h2f_lw_axi_master_awready), //                  .awready
		.h2f_lw_WID     (hps_0_h2f_lw_axi_master_wid),     //                  .wid
		.h2f_lw_WDATA   (hps_0_h2f_lw_axi_master_wdata),   //                  .wdata
		.h2f_lw_WSTRB   (hps_0_h2f_lw_axi_master_wstrb),   //                  .wstrb
		.h2f_lw_WLAST   (hps_0_h2f_lw_axi_master_wlast),   //                  .wlast
		.h2f_lw_WVALID  (hps_0_h2f_lw_axi_master_wvalid),  //                  .wvalid
		.h2f_lw_WREADY  (hps_0_h2f_lw_axi_master_wready),  //                  .wready
		.h2f_lw_BID     (hps_0_h2f_lw_axi_master_bid),     //                  .bid
		.h2f_lw_BRESP   (hps_0_h2f_lw_axi_master_bresp),   //                  .bresp
		.h2f_lw_BVALID  (hps_0_h2f_lw_axi_master_bvalid),  //                  .bvalid
		.h2f_lw_BREADY  (hps_0_h2f_lw_axi_master_bready),  //                  .bready
		.h2f_lw_ARID    (hps_0_h2f_lw_axi_master_arid),    //                  .arid
		.h2f_lw_ARADDR  (hps_0_h2f_lw_axi_master_araddr),  //                  .araddr
		.h2f_lw_ARLEN   (hps_0_h2f_lw_axi_master_arlen),   //                  .arlen
		.h2f_lw_ARSIZE  (hps_0_h2f_lw_axi_master_arsize),  //                  .arsize
		.h2f_lw_ARBURST (hps_0_h2f_lw_axi_master_arburst), //                  .arburst
		.h2f_lw_ARLOCK  (hps_0_h2f_lw_axi_master_arlock),  //                  .arlock
		.h2f_lw_ARCACHE (hps_0_h2f_lw_axi_master_arcache), //                  .arcache
		.h2f_lw_ARPROT  (hps_0_h2f_lw_axi_master_arprot),  //                  .arprot
		.h2f_lw_ARVALID (hps_0_h2f_lw_axi_master_arvalid), //                  .arvalid
		.h2f_lw_ARREADY (hps_0_h2f_lw_axi_master_arready), //                  .arready
		.h2f_lw_RID     (hps_0_h2f_lw_axi_master_rid),     //                  .rid
		.h2f_lw_RDATA   (hps_0_h2f_lw_axi_master_rdata),   //                  .rdata
		.h2f_lw_RRESP   (hps_0_h2f_lw_axi_master_rresp),   //                  .rresp
		.h2f_lw_RLAST   (hps_0_h2f_lw_axi_master_rlast),   //                  .rlast
		.h2f_lw_RVALID  (hps_0_h2f_lw_axi_master_rvalid),  //                  .rvalid
		.h2f_lw_RREADY  (hps_0_h2f_lw_axi_master_rready)   //                  .rready
	);

	soc_system_pio_0 pio_0 (
		.clk        (clk_clk),                               //                 clk.clk
		.reset_n    (~rst_controller_reset_out_reset),       //               reset.reset_n
		.address    (mm_interconnect_0_pio_0_s1_address),    //                  s1.address
		.write_n    (~mm_interconnect_0_pio_0_s1_write),     //                    .write_n
		.writedata  (mm_interconnect_0_pio_0_s1_writedata),  //                    .writedata
		.chipselect (mm_interconnect_0_pio_0_s1_chipselect), //                    .chipselect
		.readdata   (mm_interconnect_0_pio_0_s1_readdata),   //                    .readdata
		.out_port   (gpio_export_export)                     // external_connection.export
	);

	soc_system_pio_1 pio_1 (
		.clk      (clk_clk),                             //                 clk.clk
		.reset_n  (~rst_controller_reset_out_reset),     //               reset.reset_n
		.address  (mm_interconnect_0_pio_1_s1_address),  //                  s1.address
		.readdata (mm_interconnect_0_pio_1_s1_readdata), //                    .readdata
		.in_port  (gpio_1_export_export)                 // external_connection.export
	);

	soc_system_pio_2 pio_2 (
		.clk        (clk_clk),                               //                 clk.clk
		.reset_n    (~rst_controller_reset_out_reset),       //               reset.reset_n
		.address    (mm_interconnect_0_pio_2_s1_address),    //                  s1.address
		.write_n    (~mm_interconnect_0_pio_2_s1_write),     //                    .write_n
		.writedata  (mm_interconnect_0_pio_2_s1_writedata),  //                    .writedata
		.chipselect (mm_interconnect_0_pio_2_s1_chipselect), //                    .chipselect
		.readdata   (mm_interconnect_0_pio_2_s1_readdata),   //                    .readdata
		.out_port   (gpio_0_export_export)                   // external_connection.export
	);

	soc_system_pio_3 pio_3 (
		.clk      (clk_clk),                             //                 clk.clk
		.reset_n  (~rst_controller_reset_out_reset),     //               reset.reset_n
		.address  (mm_interconnect_0_pio_3_s1_address),  //                  s1.address
		.readdata (mm_interconnect_0_pio_3_s1_readdata), //                    .readdata
		.in_port  (gpio_2_export_export)                 // external_connection.export
	);

	soc_system_mm_interconnect_0 mm_interconnect_0 (
		.hps_0_h2f_lw_axi_master_awid                                        (hps_0_h2f_lw_axi_master_awid),          //                                       hps_0_h2f_lw_axi_master.awid
		.hps_0_h2f_lw_axi_master_awaddr                                      (hps_0_h2f_lw_axi_master_awaddr),        //                                                              .awaddr
		.hps_0_h2f_lw_axi_master_awlen                                       (hps_0_h2f_lw_axi_master_awlen),         //                                                              .awlen
		.hps_0_h2f_lw_axi_master_awsize                                      (hps_0_h2f_lw_axi_master_awsize),        //                                                              .awsize
		.hps_0_h2f_lw_axi_master_awburst                                     (hps_0_h2f_lw_axi_master_awburst),       //                                                              .awburst
		.hps_0_h2f_lw_axi_master_awlock                                      (hps_0_h2f_lw_axi_master_awlock),        //                                                              .awlock
		.hps_0_h2f_lw_axi_master_awcache                                     (hps_0_h2f_lw_axi_master_awcache),       //                                                              .awcache
		.hps_0_h2f_lw_axi_master_awprot                                      (hps_0_h2f_lw_axi_master_awprot),        //                                                              .awprot
		.hps_0_h2f_lw_axi_master_awvalid                                     (hps_0_h2f_lw_axi_master_awvalid),       //                                                              .awvalid
		.hps_0_h2f_lw_axi_master_awready                                     (hps_0_h2f_lw_axi_master_awready),       //                                                              .awready
		.hps_0_h2f_lw_axi_master_wid                                         (hps_0_h2f_lw_axi_master_wid),           //                                                              .wid
		.hps_0_h2f_lw_axi_master_wdata                                       (hps_0_h2f_lw_axi_master_wdata),         //                                                              .wdata
		.hps_0_h2f_lw_axi_master_wstrb                                       (hps_0_h2f_lw_axi_master_wstrb),         //                                                              .wstrb
		.hps_0_h2f_lw_axi_master_wlast                                       (hps_0_h2f_lw_axi_master_wlast),         //                                                              .wlast
		.hps_0_h2f_lw_axi_master_wvalid                                      (hps_0_h2f_lw_axi_master_wvalid),        //                                                              .wvalid
		.hps_0_h2f_lw_axi_master_wready                                      (hps_0_h2f_lw_axi_master_wready),        //                                                              .wready
		.hps_0_h2f_lw_axi_master_bid                                         (hps_0_h2f_lw_axi_master_bid),           //                                                              .bid
		.hps_0_h2f_lw_axi_master_bresp                                       (hps_0_h2f_lw_axi_master_bresp),         //                                                              .bresp
		.hps_0_h2f_lw_axi_master_bvalid                                      (hps_0_h2f_lw_axi_master_bvalid),        //                                                              .bvalid
		.hps_0_h2f_lw_axi_master_bready                                      (hps_0_h2f_lw_axi_master_bready),        //                                                              .bready
		.hps_0_h2f_lw_axi_master_arid                                        (hps_0_h2f_lw_axi_master_arid),          //                                                              .arid
		.hps_0_h2f_lw_axi_master_araddr                                      (hps_0_h2f_lw_axi_master_araddr),        //                                                              .araddr
		.hps_0_h2f_lw_axi_master_arlen                                       (hps_0_h2f_lw_axi_master_arlen),         //                                                              .arlen
		.hps_0_h2f_lw_axi_master_arsize                                      (hps_0_h2f_lw_axi_master_arsize),        //                                                              .arsize
		.hps_0_h2f_lw_axi_master_arburst                                     (hps_0_h2f_lw_axi_master_arburst),       //                                                              .arburst
		.hps_0_h2f_lw_axi_master_arlock                                      (hps_0_h2f_lw_axi_master_arlock),        //                                                              .arlock
		.hps_0_h2f_lw_axi_master_arcache                                     (hps_0_h2f_lw_axi_master_arcache),       //                                                              .arcache
		.hps_0_h2f_lw_axi_master_arprot                                      (hps_0_h2f_lw_axi_master_arprot),        //                                                              .arprot
		.hps_0_h2f_lw_axi_master_arvalid                                     (hps_0_h2f_lw_axi_master_arvalid),       //                                                              .arvalid
		.hps_0_h2f_lw_axi_master_arready                                     (hps_0_h2f_lw_axi_master_arready),       //                                                              .arready
		.hps_0_h2f_lw_axi_master_rid                                         (hps_0_h2f_lw_axi_master_rid),           //                                                              .rid
		.hps_0_h2f_lw_axi_master_rdata                                       (hps_0_h2f_lw_axi_master_rdata),         //                                                              .rdata
		.hps_0_h2f_lw_axi_master_rresp                                       (hps_0_h2f_lw_axi_master_rresp),         //                                                              .rresp
		.hps_0_h2f_lw_axi_master_rlast                                       (hps_0_h2f_lw_axi_master_rlast),         //                                                              .rlast
		.hps_0_h2f_lw_axi_master_rvalid                                      (hps_0_h2f_lw_axi_master_rvalid),        //                                                              .rvalid
		.hps_0_h2f_lw_axi_master_rready                                      (hps_0_h2f_lw_axi_master_rready),        //                                                              .rready
		.clk_0_clk_clk                                                       (clk_clk),                               //                                                     clk_0_clk.clk
		.hps_0_h2f_lw_axi_master_agent_clk_reset_reset_bridge_in_reset_reset (rst_controller_001_reset_out_reset),    // hps_0_h2f_lw_axi_master_agent_clk_reset_reset_bridge_in_reset.reset
		.pio_0_reset_reset_bridge_in_reset_reset                             (rst_controller_reset_out_reset),        //                             pio_0_reset_reset_bridge_in_reset.reset
		.pio_0_s1_address                                                    (mm_interconnect_0_pio_0_s1_address),    //                                                      pio_0_s1.address
		.pio_0_s1_write                                                      (mm_interconnect_0_pio_0_s1_write),      //                                                              .write
		.pio_0_s1_readdata                                                   (mm_interconnect_0_pio_0_s1_readdata),   //                                                              .readdata
		.pio_0_s1_writedata                                                  (mm_interconnect_0_pio_0_s1_writedata),  //                                                              .writedata
		.pio_0_s1_chipselect                                                 (mm_interconnect_0_pio_0_s1_chipselect), //                                                              .chipselect
		.pio_1_s1_address                                                    (mm_interconnect_0_pio_1_s1_address),    //                                                      pio_1_s1.address
		.pio_1_s1_readdata                                                   (mm_interconnect_0_pio_1_s1_readdata),   //                                                              .readdata
		.pio_2_s1_address                                                    (mm_interconnect_0_pio_2_s1_address),    //                                                      pio_2_s1.address
		.pio_2_s1_write                                                      (mm_interconnect_0_pio_2_s1_write),      //                                                              .write
		.pio_2_s1_readdata                                                   (mm_interconnect_0_pio_2_s1_readdata),   //                                                              .readdata
		.pio_2_s1_writedata                                                  (mm_interconnect_0_pio_2_s1_writedata),  //                                                              .writedata
		.pio_2_s1_chipselect                                                 (mm_interconnect_0_pio_2_s1_chipselect), //                                                              .chipselect
		.pio_3_s1_address                                                    (mm_interconnect_0_pio_3_s1_address),    //                                                      pio_3_s1.address
		.pio_3_s1_readdata                                                   (mm_interconnect_0_pio_3_s1_readdata)    //                                                              .readdata
	);

	altera_reset_controller #(
		.NUM_RESET_INPUTS          (1),
		.OUTPUT_RESET_SYNC_EDGES   ("deassert"),
		.SYNC_DEPTH                (2),
		.RESET_REQUEST_PRESENT     (0),
		.RESET_REQ_WAIT_TIME       (1),
		.MIN_RST_ASSERTION_TIME    (3),
		.RESET_REQ_EARLY_DSRT_TIME (1),
		.USE_RESET_REQUEST_IN0     (0),
		.USE_RESET_REQUEST_IN1     (0),
		.USE_RESET_REQUEST_IN2     (0),
		.USE_RESET_REQUEST_IN3     (0),
		.USE_RESET_REQUEST_IN4     (0),
		.USE_RESET_REQUEST_IN5     (0),
		.USE_RESET_REQUEST_IN6     (0),
		.USE_RESET_REQUEST_IN7     (0),
		.USE_RESET_REQUEST_IN8     (0),
		.USE_RESET_REQUEST_IN9     (0),
		.USE_RESET_REQUEST_IN10    (0),
		.USE_RESET_REQUEST_IN11    (0),
		.USE_RESET_REQUEST_IN12    (0),
		.USE_RESET_REQUEST_IN13    (0),
		.USE_RESET_REQUEST_IN14    (0),
		.USE_RESET_REQUEST_IN15    (0),
		.ADAPT_RESET_REQUEST       (0)
	) rst_controller (
		.reset_in0      (~reset_reset_n),                 // reset_in0.reset
		.clk            (clk_clk),                        //       clk.clk
		.reset_out      (rst_controller_reset_out_reset), // reset_out.reset
		.reset_req      (),                               // (terminated)
		.reset_req_in0  (1'b0),                           // (terminated)
		.reset_in1      (1'b0),                           // (terminated)
		.reset_req_in1  (1'b0),                           // (terminated)
		.reset_in2      (1'b0),                           // (terminated)
		.reset_req_in2  (1'b0),                           // (terminated)
		.reset_in3      (1'b0),                           // (terminated)
		.reset_req_in3  (1'b0),                           // (terminated)
		.reset_in4      (1'b0),                           // (terminated)
		.reset_req_in4  (1'b0),                           // (terminated)
		.reset_in5      (1'b0),                           // (terminated)
		.reset_req_in5  (1'b0),                           // (terminated)
		.reset_in6      (1'b0),                           // (terminated)
		.reset_req_in6  (1'b0),                           // (terminated)
		.reset_in7      (1'b0),                           // (terminated)
		.reset_req_in7  (1'b0),                           // (terminated)
		.reset_in8      (1'b0),                           // (terminated)
		.reset_req_in8  (1'b0),                           // (terminated)
		.reset_in9      (1'b0),                           // (terminated)
		.reset_req_in9  (1'b0),                           // (terminated)
		.reset_in10     (1'b0),                           // (terminated)
		.reset_req_in10 (1'b0),                           // (terminated)
		.reset_in11     (1'b0),                           // (terminated)
		.reset_req_in11 (1'b0),                           // (terminated)
		.reset_in12     (1'b0),                           // (terminated)
		.reset_req_in12 (1'b0),                           // (terminated)
		.reset_in13     (1'b0),                           // (terminated)
		.reset_req_in13 (1'b0),                           // (terminated)
		.reset_in14     (1'b0),                           // (terminated)
		.reset_req_in14 (1'b0),                           // (terminated)
		.reset_in15     (1'b0),                           // (terminated)
		.reset_req_in15 (1'b0)                            // (terminated)
	);

	altera_reset_controller #(
		.NUM_RESET_INPUTS          (1),
		.OUTPUT_RESET_SYNC_EDGES   ("deassert"),
		.SYNC_DEPTH                (2),
		.RESET_REQUEST_PRESENT     (0),
		.RESET_REQ_WAIT_TIME       (1),
		.MIN_RST_ASSERTION_TIME    (3),
		.RESET_REQ_EARLY_DSRT_TIME (1),
		.USE_RESET_REQUEST_IN0     (0),
		.USE_RESET_REQUEST_IN1     (0),
		.USE_RESET_REQUEST_IN2     (0),
		.USE_RESET_REQUEST_IN3     (0),
		.USE_RESET_REQUEST_IN4     (0),
		.USE_RESET_REQUEST_IN5     (0),
		.USE_RESET_REQUEST_IN6     (0),
		.USE_RESET_REQUEST_IN7     (0),
		.USE_RESET_REQUEST_IN8     (0),
		.USE_RESET_REQUEST_IN9     (0),
		.USE_RESET_REQUEST_IN10    (0),
		.USE_RESET_REQUEST_IN11    (0),
		.USE_RESET_REQUEST_IN12    (0),
		.USE_RESET_REQUEST_IN13    (0),
		.USE_RESET_REQUEST_IN14    (0),
		.USE_RESET_REQUEST_IN15    (0),
		.ADAPT_RESET_REQUEST       (0)
	) rst_controller_001 (
		.reset_in0      (~hps_0_h2f_reset_reset),             // reset_in0.reset
		.clk            (clk_clk),                            //       clk.clk
		.reset_out      (rst_controller_001_reset_out_reset), // reset_out.reset
		.reset_req      (),                                   // (terminated)
		.reset_req_in0  (1'b0),                               // (terminated)
		.reset_in1      (1'b0),                               // (terminated)
		.reset_req_in1  (1'b0),                               // (terminated)
		.reset_in2      (1'b0),                               // (terminated)
		.reset_req_in2  (1'b0),                               // (terminated)
		.reset_in3      (1'b0),                               // (terminated)
		.reset_req_in3  (1'b0),                               // (terminated)
		.reset_in4      (1'b0),                               // (terminated)
		.reset_req_in4  (1'b0),                               // (terminated)
		.reset_in5      (1'b0),                               // (terminated)
		.reset_req_in5  (1'b0),                               // (terminated)
		.reset_in6      (1'b0),                               // (terminated)
		.reset_req_in6  (1'b0),                               // (terminated)
		.reset_in7      (1'b0),                               // (terminated)
		.reset_req_in7  (1'b0),                               // (terminated)
		.reset_in8      (1'b0),                               // (terminated)
		.reset_req_in8  (1'b0),                               // (terminated)
		.reset_in9      (1'b0),                               // (terminated)
		.reset_req_in9  (1'b0),                               // (terminated)
		.reset_in10     (1'b0),                               // (terminated)
		.reset_req_in10 (1'b0),                               // (terminated)
		.reset_in11     (1'b0),                               // (terminated)
		.reset_req_in11 (1'b0),                               // (terminated)
		.reset_in12     (1'b0),                               // (terminated)
		.reset_req_in12 (1'b0),                               // (terminated)
		.reset_in13     (1'b0),                               // (terminated)
		.reset_req_in13 (1'b0),                               // (terminated)
		.reset_in14     (1'b0),                               // (terminated)
		.reset_req_in14 (1'b0),                               // (terminated)
		.reset_in15     (1'b0),                               // (terminated)
		.reset_req_in15 (1'b0)                                // (terminated)
	);

endmodule
