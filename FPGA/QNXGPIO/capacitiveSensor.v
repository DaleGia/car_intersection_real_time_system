

module capacitiveSensor (
	input				clock,
	input sensorReceive,
	output sensorSend,
	output sensorOutput,
	input		[31:0] thresholdVal,
	input		[31:0] delayVal
);

reg [31:0]	delay;
reg [31:0]	state;
reg [31:0] 	count;
reg sensorSend_reg;
reg sensorOutput_reg;
assign sensorSend = sensorSend_reg;
assign sensorOutput = sensorOutput_reg;

always@(posedge(clock))
	begin
		case(state)
			0:	begin
					count = 0;
					sensorOutput_reg = 1'b0;
					sensorSend_reg = 1'b0;
					state = 1;
				end
			1:	begin
					sensorSend_reg = 1'b1;
					count = 0;
					sensorOutput_reg = 1'b0;
					state = 2;
				end
			2:	begin
					if(count < thresholdVal)
						begin
							count = count + 1;
						end
					else
						begin
							state = 3;
						end
				end
			3:	begin
					sensorOutput_reg = ~sensorReceive;
					delay = delayVal;
					state = 4;
				end
			4:	begin
					sensorSend_reg = 1'b0;
					if(delay > 0)
						begin
							delay = delay - 1;
						end
					else
						begin
							state = 1;
						end
				end
		endcase
	end
	
endmodule
