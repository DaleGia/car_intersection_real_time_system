

module timedLatch
(
	input clock,
	input trigger,
	output out
);

parameter delayVal = 25000000;	//placeholder value
reg [31:0] count;
reg out_reg;
assign out = out_reg;

always@(posedge(clock))
	begin
		if(trigger)
			begin
				out_reg = 1;
				count = delayVal;
			end
		else
			begin
				if(count > 0)
					begin
						count = count - 1;
					end
				else
					begin
						out_reg = 0;
					end
			end
	end
endmodule
