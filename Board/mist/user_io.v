//
// user_io.v
//
// user_io for the MiST board
// http://code.google.com/p/mist-board/
//
// Copyright (c) 2014 Till Harbaum <till@harbaum.org>
//
// This source file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This source file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

// parameter STRLEN and the actual length of conf_str have to match
 
module user_io #(parameter STRLEN=0) (
	input [(8*STRLEN)-1:0] conf_str,

	input      		SPI_CLK,
	input      		SPI_SS_IO,
	output     		reg SPI_MISO,
	input      		SPI_MOSI,
	
	output [5:0] 	JOY0,
	output [5:0] 	JOY1,
	output [1:0] 	BUTTONS,
	output [1:0] 	SWITCHES,

	output reg [7:0]   status,

	// connection to sd card emulation
	input [31:0]  sd_lba,
	input         sd_rd,
	output reg	  sd_ack,
	output reg [7:0]  sd_dout,
	output reg	  sd_dout_strobe,
	

	// ps2 keyboard emulation
	input 	  		clk,					// 12-16khz provided by core
	output	 		ps2_clk,
	output reg 		ps2_data,

	// serial com port 
	input [7:0]		serial_data,
	input				serial_strobe
);

reg [6:0]         sbuf;
reg [7:0]         cmd;
reg [2:0] 	      bit_cnt;    // counts bits 0-7 0-7 ...
reg [7:0]         byte_cnt;   // counts bytes
reg [5:0]         joystick0;
reg [5:0]         joystick1;
reg [3:0] 	      but_sw;

assign JOY0 = joystick0;
assign JOY1 = joystick1;
assign BUTTONS = but_sw[1:0];
assign SWITCHES = but_sw[3:2]; 

// this variant of user_io is for 8 bit cores (type == a4) only
wire [7:0] core_type = 8'ha4;

wire [7:0] sd_cmd = { 4'h5, 3'b000, sd_rd };

// drive MISO only when transmitting core id
always@(negedge SPI_CLK or posedge SPI_SS_IO) begin
	if(SPI_SS_IO == 1) begin
	   SPI_MISO <= 1'bZ;
	end else begin
		// first byte returned is always core type, further bytes are 
		// command dependent
      if(byte_cnt == 0) begin
		  SPI_MISO <= core_type[~bit_cnt];
		end else begin
			// reading serial fifo
		   if(cmd == 8'h05) begin
				// send alternating flag byte and data
				if(byte_cnt[0]) 	SPI_MISO <= serial_out_status[~bit_cnt];
				else					SPI_MISO <= serial_out_byte[~bit_cnt];
			end
			
			// reading config string
		   else if(cmd == 8'h14) begin
				// returning a byte from string
				if(byte_cnt < STRLEN + 1)
					SPI_MISO <= conf_str[{STRLEN - byte_cnt,~bit_cnt}];
				else
					SPI_MISO <= 1'b0;
			end
			
			// reading sd card status
		   else if(cmd == 8'h16) begin
				if(byte_cnt == 1)
					SPI_MISO <= sd_cmd[~bit_cnt];
				else if((byte_cnt >= 2) && (byte_cnt < 6))
					SPI_MISO <= sd_lba[{byte_cnt-2, ~bit_cnt}];
				else
					SPI_MISO <= 1'b0;
			end
			
			else
				SPI_MISO <= 1'b0;
		end
   end
end

// 8 byte fifo to store ps2 bytes
localparam PS2_FIFO_BITS = 3;
reg [7:0] ps2_fifo [(2**PS2_FIFO_BITS)-1:0];
reg [PS2_FIFO_BITS-1:0] ps2_wptr;
reg [PS2_FIFO_BITS-1:0] ps2_rptr;

// ps2 transmitter state machine
reg [3:0] ps2_tx_state;
reg [7:0] ps2_tx_byte;
reg ps2_parity;

assign ps2_clk = clk || (ps2_tx_state == 0);

// ps2 transmitter
// Takes a byte from the FIFO and sends it in a ps2 compliant serial format.
reg ps2_r_inc;
always@(posedge clk) begin
	ps2_r_inc <= 1'b0;
	
	if(ps2_r_inc)
		ps2_rptr <= ps2_rptr + 1;

	// transmitter is idle?
	if(ps2_tx_state == 0) begin
		// data in fifo present?
		if(ps2_wptr != ps2_rptr) begin
			// load tx register from fifo
			ps2_tx_byte <= ps2_fifo[ps2_rptr];
			ps2_r_inc <= 1'b1;
			
			// reset parity
			ps2_parity <= 1'b1;
			
			// start transmitter
			ps2_tx_state <= 4'd1;

			// put start bit on data line
			ps2_data <= 1'b0;			// start bit is 0
		end
	end else begin
	
		// transmission of 8 data bits
		if((ps2_tx_state >= 1)&&(ps2_tx_state < 9)) begin
			ps2_data <= ps2_tx_byte[0];			  // data bits
			ps2_tx_byte[6:0] <= ps2_tx_byte[7:1]; // shift down
			if(ps2_tx_byte[0]) 
				ps2_parity <= !ps2_parity;
		end

		// transmission of parity
		if(ps2_tx_state == 9)
			ps2_data <= ps2_parity;
			
		// transmission of stop bit
		if(ps2_tx_state == 10)
			ps2_data <= 1'b1;			// stop bit is 1

		// advance state machine
		if(ps2_tx_state < 11)
			ps2_tx_state <= ps2_tx_state + 4'd1;
		else	
			ps2_tx_state <= 4'd0;
	
	end
end
  
// fifo to receive serial data from core to be forwarded to io controller

// 16 byte fifo to store serial bytes
localparam SERIAL_OUT_FIFO_BITS = 6;
reg [7:0] serial_out_fifo [(2**SERIAL_OUT_FIFO_BITS)-1:0];
reg [SERIAL_OUT_FIFO_BITS-1:0] serial_out_wptr;
reg [SERIAL_OUT_FIFO_BITS-1:0] serial_out_rptr;
 
wire serial_out_data_available = serial_out_wptr != serial_out_rptr;
wire [7:0] serial_out_byte = serial_out_fifo[serial_out_rptr] /* synthesis keep */;
wire [7:0] serial_out_status = { 7'b1000000, serial_out_data_available};

// status[0] is reset signal from io controller and is thus used to flush
// the fifo
always @(posedge serial_strobe or posedge status[0]) begin
	if(status[0] == 1) begin
		serial_out_wptr <= 0;
	end else begin 
		serial_out_fifo[serial_out_wptr] <= serial_data;
		serial_out_wptr <= serial_out_wptr + 1;
	end
end 

always@(negedge SPI_CLK or posedge status[0]) begin
	if(status[0] == 1) begin
		serial_out_rptr <= 0;
	end else begin
		if((byte_cnt != 0) && (cmd == 8'h05)) begin
			// read last bit -> advance read pointer
			if((bit_cnt == 7) && !byte_cnt[0] && serial_out_data_available)
				serial_out_rptr <= serial_out_rptr + 1;
		end
	end
end

// SPI receiver
always@(posedge SPI_CLK or posedge SPI_SS_IO) begin

	if(SPI_SS_IO == 1) begin
	   bit_cnt <= 3'd0;
	   byte_cnt <= 8'd0;
		sd_ack <= 1'b0;
		sd_dout_strobe <= 1'b0;
	end else begin
		sd_dout_strobe <= 1'b0;
		sbuf[6:0] <= { sbuf[5:0], SPI_MOSI };
		bit_cnt <= bit_cnt + 3'd1;
		if((bit_cnt == 7)&&(byte_cnt != 8'd255)) 
			byte_cnt <= byte_cnt + 8'd1;

		// finished reading command byte
      if(bit_cnt == 7) begin
			if(byte_cnt == 0)
				cmd <= { sbuf, SPI_MOSI};
			else begin
				if(cmd == 8'h01)
					but_sw <= { sbuf[2:0], SPI_MOSI }; 

				if(cmd == 8'h02)
					joystick0 <= { sbuf[4:0], SPI_MOSI };
				 
				if(cmd == 8'h03)
					joystick1 <= { sbuf[4:0], SPI_MOSI };
				 
				if(cmd == 8'h05) begin
					// store incoming keyboard bytes in 
					ps2_fifo[ps2_wptr] <= { sbuf, SPI_MOSI }; 
					ps2_wptr <= ps2_wptr + 1;
				end
				
				if(cmd == 8'h15)
					status <= { sbuf[4:0], SPI_MOSI };
				
				// send sector IO -> FPGA
				if(cmd == 8'h17) begin
					// flag that download begins
					sd_dout <= { sbuf, SPI_MOSI};
					sd_ack <= 1'b1;
					sd_dout_strobe <= 1'b1;
				end
			end
		end
	end
end
   
endmodule
