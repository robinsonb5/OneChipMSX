
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- -----------------------------------------------------------------------

architecture rtl of chameleon2 is
   constant reset_cycles : integer := 131071;
	
-- System clocks

	signal clk21m : std_logic;
	signal memclk : std_logic;
	signal pll_locked : std_logic;
	signal ena_1mhz : std_logic;
	signal ena_1khz : std_logic;
	signal phi2 : std_logic;
	
-- Global signals
	signal reset : std_logic;
	signal n_reset : std_logic;

-- LEDs
	signal led_green : std_logic;
	signal led_red : std_logic;

-- Docking station
	signal no_clock : std_logic;
	signal docking_station : std_logic;
	signal phi_cnt : unsigned(7 downto 0);
	signal phi_end_1 : std_logic;
	
-- PS/2 Keyboard socket - used for second mouse
	signal ps2_keyboard_clk_in : std_logic;
	signal ps2_keyboard_dat_in : std_logic;
	signal ps2_keyboard_clk_out : std_logic;
	signal ps2_keyboard_dat_out : std_logic;

-- PS/2 Mouse
	signal ps2_mouse_clk_in: std_logic;
	signal ps2_mouse_dat_in: std_logic;
	signal ps2_mouse_clk_out: std_logic;
	signal ps2_mouse_dat_out: std_logic;
	
	signal sdram_req : std_logic := '0';
	signal sdram_ack : std_logic;
	signal sdram_we : std_logic := '0';
	signal sdram_a : unsigned(24 downto 0) := (others => '0');
	signal sdram_d : unsigned(7 downto 0);
	signal sdram_q : unsigned(7 downto 0);

	signal hsync : std_logic;
	signal vsync : std_logic;	
	signal wred : unsigned(7 downto 0);
	signal wgrn : unsigned(7 downto 0);
	signal wblu : unsigned(7 downto 0);

	-- Video
	signal vga_r: std_logic_vector(7 downto 0);
	signal vga_g: std_logic_vector(7 downto 0);
	signal vga_b: std_logic_vector(7 downto 0);
	signal vga_window : std_logic;
	signal vga_hsync : std_logic;
	signal vga_vsync : std_logic;
	
	
-- RS232 serial
	signal rs232_rxd : std_logic;
	signal rs232_txd : std_logic;

-- Sound
	signal audio_l : std_logic_vector(15 downto 0);
	signal audio_r : std_logic_vector(15 downto 0);

-- IO
	signal button_reset_n : std_logic;

	signal c64_keys : unsigned(63 downto 0);
	signal keys_valid : std_logic;
	signal c64_restore_key_n : std_logic;
	signal c64_nmi_n : std_logic;
	signal c64_joy1 : unsigned(5 downto 0);
	signal c64_joy2 : unsigned(5 downto 0);
	signal joystick3 : unsigned(5 downto 0);
	signal joystick4 : unsigned(5 downto 0);
	signal gp1_run : std_logic;
	signal gp1_select : std_logic;
	signal joy1 : unsigned(7 downto 0);
	signal joy2 : unsigned(7 downto 0);
	signal joy3 : unsigned(7 downto 0);
	signal joy4 : unsigned(7 downto 0);
	signal ir : std_logic;
	signal ir_d : std_logic;
	signal debug1 : std_logic_vector(31 downto 0);
	signal debug2 : std_logic_vector(31 downto 0);
	signal debug3 : std_logic_vector(31 downto 0);
	signal debug4 : std_logic_vector(31 downto 0);
	

	-- Sigma Delta audio
	COMPONENT hybrid_pwm_sd
	PORT
	(
		clk	:	IN STD_LOGIC;
		n_reset	:	IN STD_LOGIC;
		din	:	IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		dout	:	OUT STD_LOGIC
	);
	END COMPONENT;

	COMPONENT video_vga_dither
	GENERIC ( outbits : INTEGER := 4 );
	PORT
	(
		clk	:	IN STD_LOGIC;
		hsync	:	IN STD_LOGIC;
		vsync	:	IN STD_LOGIC;
		vid_ena	:	IN STD_LOGIC;
		iRed	:	IN UNSIGNED(7 DOWNTO 0);
		iGreen	:	IN UNSIGNED(7 DOWNTO 0);
		iBlue	:	IN UNSIGNED(7 DOWNTO 0);
		oRed	:	OUT UNSIGNED(outbits-1 DOWNTO 0);
		oGreen	:	OUT UNSIGNED(outbits-1 DOWNTO 0);
		oBlue	:	OUT UNSIGNED(outbits-1 DOWNTO 0)
	);
	END COMPONENT;

begin
-- -----------------------------------------------------------------------
-- Unused pins
-- -----------------------------------------------------------------------
	iec_clk_out <= '0';
	iec_atn_out <= '0';
	iec_dat_out <= '0';
	iec_srq_out <= '0';
	irq_out <= '0';
	nmi_out <= '0';
	usart_rx<='1';
	clock_ior <= '1';
	clock_iow <= '1';

	-- put these here?
	flash_cs <= '1';
	rtc_cs <= '0';
	

-- -----------------------------------------------------------------------
-- Reset
-- -----------------------------------------------------------------------
	myReset : entity work.gen_reset
		generic map (
			resetCycles => reset_cycles
		)
		port map (
			clk => clk21m,
			enable => '1',

			button => not reset_btn,
			reset => reset,
			nreset => n_reset
		);

-- -----------------------------------------------------------------------
-- 1 Mhz and 1 Khz clocks
-- -----------------------------------------------------------------------
	my1Mhz : entity work.chameleon_1mhz
		generic map (
			clk_ticks_per_usec => 85
		)
		port map (
			clk => memclk,
			ena_1mhz => ena_1mhz,
			ena_1mhz_2 => open
		);

	my1Khz : entity work.chameleon_1khz
		port map (
			clk => memclk,
			ena_1mhz => ena_1mhz,
			ena_1khz => ena_1khz
		);
	
-- -----------------------------------------------------------------------
-- PS2IEC multiplexer
-- -----------------------------------------------------------------------
	io_ps2iec_inst : entity work.chameleon2_io_ps2iec
		port map (
			clk => clk21m,

			ps2iec_sel => ps2iec_sel,
			ps2iec => ps2iec,

			ps2_mouse_clk => ps2_mouse_clk_in,
			ps2_mouse_dat => ps2_mouse_dat_in,
			ps2_keyboard_clk => ps2_keyboard_clk_in,
			ps2_keyboard_dat => ps2_keyboard_dat_in,

			iec_clk => open, -- iec_clk_in,
			iec_srq => open, -- iec_srq_in,
			iec_atn => open, -- iec_atn_in,
			iec_dat => open  -- iec_dat_in
		);

-- -----------------------------------------------------------------------
-- LED, PS2 and reset shiftregister
-- -----------------------------------------------------------------------
	io_shiftreg_inst : entity work.chameleon2_io_shiftreg
		port map (
			clk => clk21m,

			ser_out_clk => ser_out_clk,
			ser_out_dat => ser_out_dat,
			ser_out_rclk => ser_out_rclk,

			reset_c64 => reset,
			reset_iec => reset,
			ps2_mouse_clk => ps2_mouse_clk_out,
			ps2_mouse_dat => ps2_mouse_dat_out,
			ps2_keyboard_clk => ps2_keyboard_clk_out,
			ps2_keyboard_dat => ps2_keyboard_dat_out,
			led_green => led_green,
			led_red => led_red
		);

-- -----------------------------------------------------------------------
-- Chameleon IO, docking station and cartridge port
-- -----------------------------------------------------------------------
	chameleon2_io_blk : block
	begin
		chameleon2_io_inst : entity work.chameleon2_io
			generic map (
				enable_docking_station => true,
				enable_cdtv_remote => true,
				enable_c64_joykeyb => true,
				enable_c64_4player => true
			)
			port map (
				clk => memclk,
				ena_1mhz => ena_1mhz,
				phi2_n => phi2_n,
				dotclock_n => dotclk_n,

				reset => reset,

				ir_data => ir,
				ioef => ioef,
				romlh => romlh,

				dma_out => dma_out,
				game_out => game_out,
				exrom_out => exrom_out,

				ba_in => ba_in,
--				rw_in => rw_in,
				rw_out => rw_out,

				sa_dir => sa_dir,
				sa_oe => sa_oe,
				sa15_out => sa15_out,
				low_a => low_a,

				sd_dir => sd_dir,
				sd_oe => sd_oe,
				low_d => low_d,

				no_clock => no_clock,
				docking_station => docking_station,

				phi_cnt => phi_cnt,
				phi_end_1 => phi_end_1,

				joystick1 => c64_joy1,
				joystick2 => c64_joy2,
				joystick3 => joystick3,
				joystick4 => joystick4,
				keys => c64_keys,
				keys_valid => keys_valid,
--				restore_key_n => restore_n
				restore_key_n => open,
				debug1 => debug1,
				debug2 => debug2,
				debug3 => debug3
			);
	end block;

debug4 <= no_clock & docking_station & std_logic_vector(c64_keys(29 downto 0));
	
-- Synchronise IR signal
process (memclk)
begin
	if rising_edge(memclk) then
		ir_d<=ir_data;
		ir<=ir_d;
	end if;
end process;

		
--joy1<=not gp1_run & not gp1_select & (c64_joy1 and cdtv_joy1);
-- Update keypresses only when joystick is inactive.
process(memclk)
begin
	if rising_edge(memclk) then
		if keys_valid='1' then
			gp1_run<=c64_keys(11) and c64_keys(56);
			gp1_select<=c64_keys(60);
		end if;
	end if;
end process;

joy1<=gp1_run & gp1_select & c64_joy1;
joy2<="11" & c64_joy2;
joy3<="11" & joystick3;
joy4<="11" & joystick4;
	

  U00 : entity work.pll
    port map(
      inclk0 => clk50m,       -- 50 MHz external
      c0     => clk21m,         -- 21MHz internal
      c1     => memclk,         -- 84MHz = 21MHz x 4
      c2     => ram_clk,        -- 84MHz external
      locked => pll_locked
    );

ram_a(12)<='0';
	 
emsx_top : entity work.Virtual_Toplevel
  port map(
    -- Clock, Reset ports
		clk21m => clk21m,
		memclk => memclk,
		lock_n => n_reset,

--    -- MSX cartridge slot ports
--    pSltClk     : out std_logic;	-- pCpuClk returns here, for Z80, etc.
--    pSltRst_n   : in std_logic :='1';		-- pCpuRst_n returns here
--    pSltSltsl_n : inout std_logic:='1';
--    pSltSlts2_n : inout std_logic:='1';
--    pSltIorq_n  : inout std_logic:='1';
--    pSltRd_n    : inout std_logic:='1';
--    pSltWr_n    : inout std_logic:='1';
--    pSltAdr     : inout std_logic_vector(15 downto 0):=(others=>'1');
--    pSltDat     : inout std_logic_vector(7 downto 0):=(others=>'1');
--    pSltBdir_n  : out std_logic;	-- Bus direction (not used in master mode)
--
--    pSltCs1_n   : inout std_logic:='1';
--    pSltCs2_n   : inout std_logic:='1';
--    pSltCs12_n  : inout std_logic:='1';
--    pSltRfsh_n  : inout std_logic:='1';
--    pSltWait_n  : inout std_logic:='1';
--    pSltInt_n   : inout std_logic:='1';
--    pSltM1_n    : inout std_logic:='1';
--    pSltMerq_n  : inout std_logic:='1';
--
--    pSltRsv5    : out std_logic;            -- Reserved
--    pSltRsv16   : out std_logic;            -- Reserved (w/ external pull-up)
--    pSltSw1     : inout std_logic:='1';          -- Reserved (w/ external pull-up)
--    pSltSw2     : inout std_logic:='1';          -- Reserved

    -- SDRAM DE1 ports
--	 pMemClk => sd_clk,
    pMemCke => open,
    pMemCs_n => open,
    pMemRas_n => ram_ras,
    pMemCas_n => ram_cas,
    pMemWe_n => ram_we,
    pMemUdq => ram_udqm,
    pMemLdq => ram_ldqm,
    pMemBa1 => ram_ba(1),
    pMemBa0 => ram_ba(0),
    unsigned(pMemAdr) => ram_a(11 downto 0),
    unsigned(pMemDat) => ram_d,

    -- PS/2 keyboard ports
	 pPs2Clk_out => ps2_keyboard_clk_out,
	 pPs2Dat_out => ps2_keyboard_dat_out,
	 pPs2Clk_in => ps2_keyboard_clk_in,
	 pPs2Dat_in => ps2_keyboard_dat_in,

	 -- PS/2 mouse ports
		ps2m_clk_in => ps2_mouse_clk_in,
		ps2m_dat_in => ps2_mouse_dat_in,
		ps2m_clk_out => ps2_mouse_clk_out,
		ps2m_dat_out => ps2_mouse_dat_out,
	 
--    -- Joystick ports (Port_A, Port_B)
    pJoyA => std_logic_vector(joy1(5 downto 0)), --       : inout std_logic_vector( 5 downto 0):=(others=>'1');
--    pStrA       : out std_logic;
    pJoyB => std_logic_vector(joy2(5 downto 0)), --       : inout std_logic_vector( 5 downto 0):=(others=>'1');
--    pStrB       : out std_logic;

    -- SD/MMC slot ports
    pSd_Ck => spi_clk,
    pSd_Cm => spi_mosi,
--  pSd_Dt	    : inout std_logic_vector( 3 downto 0);  -- pin 1(D3), 9(D2), 8(D1), 7(D0)
    pSd_Dt3	=> mmc_cs,
    pSd_Dt0	=> spi_miso,

		-- DIP switch, Lamp ports
    pSW => "111"&n_reset,
    pDip => "0000111001",
    pLedG(0) => led_green,
    pLedR(0) => led_red,

    -- Video, Audio/CMT ports
    pDac_VR => vga_r,
    pDac_VG => vga_g,
    pDac_VB => vga_b,
--    pDac_S 		: out   std_logic;						-- Sound
--    pREM_out	: out   std_logic;						-- REM output; 1 - Tape On
--    pCMT_out	: out   std_logic;						-- CMT output
--    pCMT_in		: in    std_logic :='1';						-- CMT input

    pVideoHS_n => vga_hsync,
    pVideoVS_n => vga_vsync,

    -- DE1 7-SEG Display
    hex => open,

	 SOUND_L => audio_l,
	 SOUND_R => audio_r,
	 CmtIn => '1',
	 
	 RS232_RxD => rs232_rxd,
	 RS232_TxD => rs232_txd
);

	
-- Dither the video down to 5 bits per gun.
	vga_window<='1';
	hsync_n<= not vga_hsync;
	vsync_n<= not vga_vsync;	

	mydither : component video_vga_dither
		generic map(
			outbits => 5
		)
		port map(
			clk=>memclk,
			hsync=>vga_hsync,
			vsync=>vga_vsync,
			vid_ena=>vga_window,
			iRed => unsigned(vga_r),
			iGreen => unsigned(vga_g),
			iBlue => unsigned(vga_b),
			oRed => red,
			oGreen => grn,
			oBlue => blu
		);
	
leftsd: component hybrid_pwm_sd
	port map
	(
		clk => memclk,
		n_reset => n_reset,
		din(15) => not audio_l(15),
		din(14 downto 0) => std_logic_vector(audio_l(14 downto 0)),
		dout => sigma_l
	);
	
rightsd: component hybrid_pwm_sd
	port map
	(
		clk => memclk,
		n_reset => n_reset,
		din(15) => not audio_r(15),
		din(14 downto 0) => std_logic_vector(audio_r(14 downto 0)),
		dout => sigma_r
	);


end architecture;

