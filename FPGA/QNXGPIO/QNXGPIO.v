
module QNXGPIO (
	// Top Level Inputs into the system
	input 			CLOCK_50,	// System Clock - 50MHz input.
	input  [3:0]   SW,		   // 4 Key Switches.
	output [7:0]   LED,			// 7 LEDs for debugging purposes.
	
	input key0,
	input key1,
	
	// GPIO PORT - 16 Bits output
	output [15:0] gpio_0,
	
	// GPIO PORT - 16 Bits input
	input  [11:0] gpio_1,
	
	input	 [3:0] sensorReceive,
	output [3:0] sensorSend,
		
	output [14:0] hps_memory_mem_a, 
   output [2:0]  hps_memory_mem_ba, 
   output        hps_memory_mem_ck, 
   output        hps_memory_mem_ck_n, 
   output        hps_memory_mem_cke, 
   output        hps_memory_mem_cs_n, 
   output        hps_memory_mem_ras_n, 
   output        hps_memory_mem_cas_n, 
   output        hps_memory_mem_we_n, 
	output        hps_memory_mem_reset_n,
   inout  [39:0] hps_memory_mem_dq, 
   inout  [4:0]  hps_memory_mem_dqs, 
   inout  [4:0]  hps_memory_mem_dqs_n, 
   output        hps_memory_mem_odt, 
   output [4:0]  hps_memory_mem_dm, 
   input         hps_memory_oct_rzqin 
	
);

// Connections
wire main_clk = CLOCK_50;

wire [3:0] sOut;
// Capacitive Sensors
capacitiveSensor capSense0(
	.clock(CLOCK_50),
	.sensorReceive(sensorReceive[0]),
	.sensorSend(sensorSend[0]),
	.sensorOutput(sOut[0]),
	.thresholdVal(25000),
	.delayVal(3000)
);
capacitiveSensor capSense1(
	.clock(CLOCK_50),
	.sensorReceive(sensorReceive[1]),
	.sensorSend(sensorSend[1]),
	.sensorOutput(sOut[1]),
	.thresholdVal(25000),
	.delayVal(3000)
);
capacitiveSensor capSense2(
	.clock(CLOCK_50),
	.sensorReceive(sensorReceive[2]),
	.sensorSend(sensorSend[2]),
	.sensorOutput(sOut[2]),
	.thresholdVal(25000),
	.delayVal(3000)
);
capacitiveSensor capSense3(
	.clock(CLOCK_50),
	.sensorReceive(sensorReceive[3]),
	.sensorSend(sensorSend[3]),
	.sensorOutput(sOut[3]),
	.thresholdVal(25000),
	.delayVal(3000)
);

// Timed Latches
wire [3:0] sensorOutput;
timedLatch sensorLatch0(.clock(main_clk), .trigger(sOut[0]), .out(sensorOutput[0]));
timedLatch sensorLatch1(.clock(main_clk), .trigger(sOut[1]), .out(sensorOutput[1]));
timedLatch sensorLatch2(.clock(main_clk), .trigger(sOut[2]), .out(sensorOutput[2]));
timedLatch sensorLatch3(.clock(main_clk), .trigger(sOut[3]), .out(sensorOutput[3]));


wire [1:0] buttons;
timedLatch button0_latch(.clock(main_clk), .trigger(~key0), .out(buttons[0]));
timedLatch button1_latch(.clock(main_clk), .trigger(~key1), .out(buttons[1]));

wire [3:0] switches;
timedLatch sw0_latch(.clock(main_clk), .trigger(SW[0]), .out(switches[0]));
timedLatch sw1_latch(.clock(main_clk), .trigger(SW[1]), .out(switches[1]));
timedLatch sw2_latch(.clock(main_clk), .trigger(SW[2]), .out(switches[2]));
timedLatch sw3_latch(.clock(main_clk), .trigger(SW[3]), .out(switches[3]));


reg [7:0] LED_reg;
assign LED = LED_reg;
always@(*)
	begin
		LED_reg[3:0] = sensorOutput | switches;
		LED_reg[5:4] = buttons;
	end


soc_system soc ( 
	 .gpio_export_export   (),
	 .gpio_1_export_export ({gpio_1[9:0], buttons[1:0], sensorOutput[3:0]}), // gpio_1_export.export    (inputs)
    .gpio_0_export_export (gpio_0),  // gpio_0_export.export    (output)
	 .gpio_2_export_export (switches), 		// gpio_1_export.export    (inputs switches)

    .memory_mem_a        (hps_memory_mem_a), 
    .memory_mem_ba       (hps_memory_mem_ba), 
    .memory_mem_ck       (hps_memory_mem_ck), 
    .memory_mem_ck_n     (hps_memory_mem_ck_n), 
    .memory_mem_cke      (hps_memory_mem_cke), 
    .memory_mem_cs_n     (hps_memory_mem_cs_n), 
    .memory_mem_ras_n    (hps_memory_mem_ras_n), 
    .memory_mem_cas_n    (hps_memory_mem_cas_n), 
    .memory_mem_we_n     (hps_memory_mem_we_n), 
    .memory_mem_reset_n  (hps_memory_mem_reset_n), 
    .memory_mem_dq       (hps_memory_mem_dq), 
    .memory_mem_dqs      (hps_memory_mem_dqs), 
    .memory_mem_dqs_n    (hps_memory_mem_dqs_n), 
    .memory_mem_odt      (hps_memory_mem_odt), 
    .memory_mem_dm       (hps_memory_mem_dm), 
    .memory_oct_rzqin    (hps_memory_oct_rzqin), 
    .clk_clk (main_clk), 
    .reset_reset_n 		  (1'b1) 
);
endmodule
