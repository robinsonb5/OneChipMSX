// http://elm-chan.org/docs/mmc/mmc_e.html

module sd_card (
	input        clk,      // 125MHz
   input        n_reset,

	// link to user_io for io controller
	output [31:0] io_lba,
	output reg    io_rd,
	input			  io_ack,
	input	[7:0]	  io_din,
	input 		  io_din_strobe,
	
   input        sd_cs,
   input        sd_sck,
   input        sd_sdi,
   output reg   sd_sdo
); 

// set io_rd once read_state machine starts waiting (rising edge of req_io_rd)
// and clear it once io controller uploads something (io_ack==1) 
wire req_io_rd = (read_state == 3'd1);
wire io_rd_reset = io_ack || sd_cs;
always @(posedge req_io_rd or posedge io_rd_reset) begin
	if(io_rd_reset) io_rd <= 1'b0;
	else 		       io_rd <= 1'b1;
end


wire [31:0] OCR = 32'h40000000;  // bit30 = 1 -> high capaciry card (sdhc)
wire [7:0] read_data_token = 8'hfe;

localparam NCR=4;

reg cmd55;

// 0=idle, 1=wait for io ctrl, 2=wait for byte start, 2=send token, 3=send data, 4=send crc
reg [2:0] read_state;  

reg [6:0] sbuf; 
reg [7:0] cmd;
reg [2:0] bit_cnt;    // counts bits 0-7 0-7 ...
reg [7:0] byte_cnt;   // counts bytes
reg [7:0] cmd_cnt;    // counts command bytes

reg [7:0] lba0, lba1, lba2, lba3;
assign io_lba = { lba3, lba2, lba1, lba0 };

// the command crc is actually never evaluated
reg [7:0] crc;

reg [7:0] reply;
reg [7:0] reply0, reply1, reply2, reply3;
reg [3:0] reply_len;

reg [8:0] rx_in_cnt;
reg [8:0] rx_out_cnt;

// buffer to hold a single sector
reg [7:0] buffer [512:0];
wire [7:0] buffer_byte = buffer[rx_out_cnt];

reg io_ackD, io_ackD2;

// receiver for io data coming from io controller
always @(negedge io_din_strobe or posedge sd_cs) begin
	if(sd_cs == 1)
		rx_in_cnt <= 9'd0;
	else begin
		buffer[rx_in_cnt] <= io_din;
		rx_in_cnt <= rx_in_cnt + 9'd1;
	end
end

// spi transmitter
always@(negedge sd_sck or posedge sd_cs) begin
	if(sd_cs == 1) begin
	   sd_sdo <= 1'b1;
		read_state <= 3'd0;
	end else begin
		// bring io ack in local clock domain
		io_ackD <= io_ack;
		io_ackD2 <= io_ackD;
	

		// -------- catch read commmand and reset read state machine ------
		if(bit_cnt == 7) begin
			// starting new command: reset read_state
			if((cmd_cnt == 0) && (sbuf[6:5] == 2'b01))
				read_state <= 3'd0;
		
			// CMD17: READ_SINGLE_BLOCK
			else if((cmd_cnt == 5) && (cmd == 8'h51)) begin
				read_state <= 3'd1;      // trigger read
			end
		end


		// first byte returned is always core type, further bytes are 
		// command dependent
      if(byte_cnt < 6+NCR) begin
		  sd_sdo <= 1'b1;				// reply $ff -> wait
		end else begin
			if(byte_cnt == 6+NCR)
				sd_sdo <= reply[~bit_cnt];
				
			else if((reply_len > 0) && (byte_cnt == 6+NCR+1))
				sd_sdo <= reply0[~bit_cnt];
			else if((reply_len > 1) && (byte_cnt == 6+NCR+2))
				sd_sdo <= reply1[~bit_cnt];
			else if((reply_len > 2) && (byte_cnt == 6+NCR+3))
				sd_sdo <= reply2[~bit_cnt];
			else if((reply_len > 3) && (byte_cnt == 6+NCR+4))
				sd_sdo <= reply3[~bit_cnt];
					
			else
				sd_sdo <= 1'b1;
				
			// falling edge of io_ack signals end of incoming data stream
			if((read_state == 3'd1) && !io_ackD && io_ackD2) 
				read_state <= 3'd2;

			// wait for begin of new byte
			if((read_state == 3'd2) && (bit_cnt == 7))
				read_state <= 3'd3;

			if(read_state == 3'd3) begin
				sd_sdo <= read_data_token[~bit_cnt];
				if(bit_cnt == 7) begin
					read_state <= 3'd4;   // next: send data
					rx_out_cnt <= 9'd0;
				end
			end

			// send data
			if(read_state == 3'd4) begin
				sd_sdo <= buffer_byte[~bit_cnt];

				if(bit_cnt == 7) begin
					rx_out_cnt <= rx_out_cnt + 9'd1;
					if(rx_out_cnt == 511) begin
						read_state <= 3'd5;   // next: send crc
						rx_out_cnt <= 9'd0;
					end
				end
			end
			
			// send crc
			if(read_state == 3'd5) begin
				sd_sdo <= rx_out_cnt[~bit_cnt];

				if(bit_cnt == 7) begin
					rx_out_cnt <= rx_out_cnt + 9'd1;
					if(rx_out_cnt == 1)
						read_state <= 3'd0;  // return to idle state
				end
			end

		end
   end
end

// spi receiver  
always @(posedge sd_sck or posedge sd_cs) begin
	// cs is active low
	if(sd_cs == 1) begin
		bit_cnt <= 3'd0;
		byte_cnt <= 8'd0;
		cmd_cnt <= 8'd0;
	end else begin 
		sbuf[6:0] <= { sbuf[5:0], sd_sdi };
		bit_cnt <= bit_cnt + 3'd1;
		if((bit_cnt == 7)&&(byte_cnt != 255)) begin
			byte_cnt <= byte_cnt + 8'd1;			
			
			if(cmd_cnt == 0) begin
				// first byte of valid command is 01xxxxxx
				if(sbuf[6:5] == 2'b01) begin
					cmd_cnt <= 8'd1;			
					byte_cnt <= 8'd1;			
				end
			end else if(byte_cnt < 6)
				cmd_cnt <= byte_cnt + 8'd1;
			else
				// command counting stops after last command byte. 
				cmd_cnt <= 8'd0;
		end

		// finished reading command byte
      if(bit_cnt == 7) begin

			if((cmd_cnt == 0)&&(sbuf[6:5] == 2'b01)) begin
				cmd <= { sbuf, sd_sdi};

			   // set cmd55 flag if previous command was 55
			   cmd55 <= (cmd == 8'h77);
			end

			// parse additional command bytes
			if(cmd_cnt == 1) lba0 <= { sbuf, sd_sdi};
			if(cmd_cnt == 2) lba1 <= { sbuf, sd_sdi};
			if(cmd_cnt == 3) lba2 <= { sbuf, sd_sdi};
			if(cmd_cnt == 4) lba3 <= { sbuf, sd_sdi};			
			if(cmd_cnt == 5) crc  <= { sbuf, sd_sdi};
			
			// last byte received, evaluate
			if(cmd_cnt == 5) begin
				// default:
				reply <= 8'h04;     // illegal command
				reply_len <= 4'd0;  // no extra reply bytes
				
			
				// CMD0: GO_IDLE_STATE
				if(cmd == 8'h40)
					reply <= 8'h01;    // ok, busy

				// CMD1: SEND_IF_COND (V2 only)
				else if(cmd == 8'h48) begin
					reply <= 8'h01;    // ok, busy
					reply0 <= 8'h00;
					reply1 <= 8'h00;
					reply2 <= 8'h01;
					reply3 <= 8'hAA;
					reply_len <= 4'd4;
				end
				
				// CMD16: SET_BLOCKLEN
				else if(cmd == 8'h50) begin
				   // we only support a block size of 512
				   if(io_lba == 32'd512)
						reply <= 8'h00;    // ok
				   else
						reply <= 8'h40;    // parmeter error
				end

				// CMD17: READ_SINGLE_BLOCK
				else if(cmd == 8'h51) begin
					reply <= 8'h00;    // ok
				end

			   // SEND_OP_COND
			   else if(!cmd55 && (cmd == 8'h69))
					reply <= 8'h00;    // ok, not busy

			   // APP_SEND_OP_COND
			   else if(cmd55 && (cmd == 8'h69))
					reply <= 8'h00;    // ok, not busy
	
				// APP_COND
				else if(cmd == 8'h77)
					reply <= 8'h01;    // ok, busy

				// READ_OCR
				else if(cmd == 8'h7a) begin
					reply <= 8'h00;    // ok
					
					reply0 <= OCR[31:24];   // bit 30 = 1 -> high capacity card 
					reply1 <= OCR[23:16];
					reply2 <= OCR[15:8];
					reply3 <= OCR[7:0];
					reply_len <= 4'd4;
				end
				
			end
		end
	end
end

endmodule
