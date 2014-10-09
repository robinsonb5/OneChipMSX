library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.numeric_std.ALL;
 
entity MIST_Toplevel is
	port
	(
		CLOCK_27		:	 in std_logic_vector(1 downto 0);
		
		LED			: 	out std_logic;

		UART_TX		:	 out STD_LOGIC;
		UART_RX		:	 in STD_LOGIC;

		SDRAM_DQ		:	 inout std_logic_vector(15 downto 0);
		SDRAM_A	:	 out std_logic_vector(12 downto 0);
		SDRAM_DQMH	:	 out STD_LOGIC;
		SDRAM_DQML	:	 out STD_LOGIC;
		SDRAM_nWE	:	 out STD_LOGIC;
		SDRAM_nCAS	:	 out STD_LOGIC;
		SDRAM_nRAS	:	 out STD_LOGIC;
		SDRAM_nCS	:	 out STD_LOGIC;
		SDRAM_BA		:	 out std_logic_vector(1 downto 0);
		SDRAM_CLK	:	 out STD_LOGIC;
		SDRAM_CKE	:	 out STD_LOGIC;

		SPI_DO	: inout std_logic;
		SPI_DI	: in std_logic;
		SPI_SCK		:	 in STD_LOGIC;
		SPI_SS2		:	 in STD_LOGIC; -- FPGA
		SPI_SS3		:	 in STD_LOGIC; -- OSD
		SPI_SS4		:	 in STD_LOGIC; -- "sniff" mode
		CONF_DATA0  : in std_logic; -- SPI_SS for user_io

		VGA_HS		:	buffer STD_LOGIC;
		VGA_VS		:	buffer STD_LOGIC;
		VGA_R		:	 out unsigned(5 downto 0);
		VGA_G		:	 out unsigned(5 downto 0);
		VGA_B		:	 out unsigned(5 downto 0);

		AUDIO_L : out std_logic;
		AUDIO_R : out std_logic
	);
END entity;

architecture rtl of MIST_Toplevel is

signal reset : std_logic;
signal pll_locked : std_logic;
signal fastclk : std_logic;
signal clk21m      : std_logic;
signal memclk      : std_logic;

signal audiol : std_logic_vector(15 downto 0);
signal audior : std_logic_vector(15 downto 0);

signal vga_tred : unsigned(7 downto 0);
signal vga_tgreen : unsigned(7 downto 0);
signal vga_tblue : unsigned(7 downto 0);
signal vga_window : std_logic;

-- user_io
signal buttons: std_logic_vector(1 downto 0);
signal status:  std_logic_vector(7 downto 0);
signal joy_0: std_logic_vector(5 downto 0);
signal joy_1: std_logic_vector(5 downto 0);
signal joyn_0: std_logic_vector(5 downto 0);
signal joyn_1: std_logic_vector(5 downto 0);
signal joy_ana_0: std_logic_vector(15 downto 0);
signal joy_ana_1: std_logic_vector(15 downto 0);
signal txd:     std_logic;
signal par_out_data: std_logic_vector(7 downto 0);
signal par_out_strobe: std_logic;

-- signals to connect sd card emulation with io controller
signal sd_lba:  std_logic_vector(31 downto 0);
signal sd_rd:   std_logic;
signal sd_wr:   std_logic;
signal sd_ack:  std_logic;
signal sd_conf: std_logic;
signal sd_sdhc: std_logic;
signal sd_allow_sdhc: std_logic;
signal sd_allow_sdhcD: std_logic;
signal sd_allow_sdhcD2: std_logic;
signal sd_allow_sdhc_changed: std_logic;
-- data from io controller to sd card emulation
signal sd_data_in: std_logic_vector(7 downto 0);
signal sd_data_in_strobe:  std_logic;
signal sd_data_out: std_logic_vector(7 downto 0);
signal sd_data_out_strobe:  std_logic;

-- sd card emulation
signal sd_cs:	std_logic;
signal sd_sck:	std_logic;
signal sd_sdi:	std_logic;
signal sd_sdo:	std_logic;

-- PS/2
signal ps2_clk : std_logic;
signal ps2counter : unsigned(10 downto 0);

-- PS/2 Keyboard
signal ps2_keyboard_clk_in : std_logic;
signal ps2_keyboard_dat_in : std_logic;
signal ps2_keyboard_clk_mix : std_logic;
signal ps2_keyboard_clk_out : std_logic;
signal ps2_keyboard_dat_out : std_logic;

-- PS/2 Mouse
signal ps2_mouse_clk_in : std_logic;
signal ps2_mouse_dat_in : std_logic;
signal ps2_mouse_clk_mix : std_logic;
signal ps2_mouse_clk_out : std_logic;
signal ps2_mouse_dat_out : std_logic;

-- Sigma Delta audio
COMPONENT hybrid_pwm_sd
	PORT
	(
		clk		:	 IN STD_LOGIC;
		n_reset		:	 IN STD_LOGIC;
		din		:	 IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		dout		:	 OUT STD_LOGIC
	);
END COMPONENT;

COMPONENT video_vga_dither
	GENERIC ( outbits : INTEGER := 4 );
	PORT
	(
		clk		:	 IN STD_LOGIC;
		hsync		:	 IN STD_LOGIC;
		vsync		:	 IN STD_LOGIC;
		vid_ena		:	 IN STD_LOGIC;
		iRed		:	 IN UNSIGNED(7 DOWNTO 0);
		iGreen		:	 IN UNSIGNED(7 DOWNTO 0);
		iBlue		:	 IN UNSIGNED(7 DOWNTO 0);
		oRed		:	 OUT UNSIGNED(outbits-1 DOWNTO 0);
		oGreen		:	 OUT UNSIGNED(outbits-1 DOWNTO 0);
		oBlue		:	 OUT UNSIGNED(outbits-1 DOWNTO 0)
	);
END COMPONENT;

function to_slv(s: string) return std_logic_vector is
    constant ss: string(1 to s'length) := s;
    variable rval: std_logic_vector(1 to 8 * s'length);
    variable p: integer;
    variable c: integer;
  
  begin  
    for i in ss'range loop
      p := 8 * i;
      c := character'pos(ss(i));
      rval(p - 7 to p) := std_logic_vector(to_unsigned(c,8));
    end loop;
    return rval;

end function;
  

component user_io 
	generic ( STRLEN : integer := 0 );
   port (
			  SPI_CLK, SPI_SS_IO, SPI_MOSI :in std_logic;
           SPI_MISO : out std_logic;
           conf_str : in std_logic_vector(8*STRLEN-1 downto 0);
           joystick_0 : out std_logic_vector(5 downto 0);
           joystick_1 : out std_logic_vector(5 downto 0);
           joystick_analog_0 : out std_logic_vector(15 downto 0);
           joystick_analog_1 : out std_logic_vector(15 downto 0);
           status: out std_logic_vector(7 downto 0);
           switches : out std_logic_vector(1 downto 0);
           buttons : out std_logic_vector(1 downto 0);
			  sd_lba : in std_logic_vector(31 downto 0);
			  sd_rd : in std_logic;
			  sd_wr : in std_logic;
			  sd_ack : out std_logic;
			  sd_conf : in std_logic;
			  sd_sdhc : in std_logic;
			  sd_dout : out std_logic_vector(7 downto 0);
			  sd_dout_strobe : out std_logic;
			  sd_din : in std_logic_vector(7 downto 0);
			  sd_din_strobe : out std_logic;
           ps2_clk : in std_logic;
           ps2_kbd_clk : out std_logic;
           ps2_kbd_data : out std_logic;
           ps2_mouse_clk : out std_logic;
           ps2_mouse_data : out std_logic;
			  serial_data : in std_logic_vector(7 downto 0);
           serial_strobe : in std_logic
      );
  end component user_io;
  
component mist_console
	generic ( CLKFREQ : integer := 100 );
   port (  clk 	:	in std_logic;
           n_reset:	in std_logic;
           ser_in :	in std_logic;
           par_out_data :	out std_logic_vector(7 downto 0);
           par_out_strobe :	out std_logic
  );
  end component mist_console;

component sd_card
   port (  io_lba 	: out std_logic_vector(31 downto 0);
			  io_rd  	: out std_logic;
			  io_wr  	: out std_logic;
			  io_ack 	: in std_logic;
			  io_sdhc 	: out std_logic;
			  io_conf 	: out std_logic;
			  io_din 	: in std_logic_vector(7 downto 0);
			  io_din_strobe : in std_logic;
			  io_dout 	: out std_logic_vector(7 downto 0);
			  io_dout_strobe : in std_logic;

			  allow_sdhc : in std_logic;
			  
           sd_cs 		:	in std_logic;
           sd_sck 	:	in std_logic;
           sd_sdi 	:	in std_logic;
           sd_sdo 	:	out std_logic
  );
  end component sd_card;

begin


  U00 : entity work.pll4x2
    port map(
		areset => '0',
      inclk0 => CLOCK_27(0),       -- 27 MHz external
      c0     => clk21m,         -- 21.43MHz internal (50*3/7)
      c1     => memclk,         -- 85.72MHz = 21.43MHz x 4
      c2     => SDRAM_CLK,        -- 85.72MHz external
		c3		=> fastclk,		-- ~110Mhz, for MUX clock
      locked => pll_locked
    );

SDRAM_A(12)<='0';

-- reset from IO controller
-- status bit 0 is always triggered by the i ocontroller on its own reset
-- button 1 is the core specfic button in the mists front
reset <= '0' when status(0)='1' or buttons(1)='1' else '1';

process(clk21m)
begin
--	ps2_keyboard_clk_mix <= ps2_keyboard_clk_in and (ps2_clk or ps2_keyboard_dat_out);
	ps2_keyboard_clk_mix <= ps2_keyboard_clk_in; -- and (ps2_clk or ps2_keyboard_dat_out);
	ps2_mouse_clk_mix <= ps2_mouse_clk_in; -- and (ps2_clk or ps2_mouse_dat_out);
	if rising_edge(clk21m) then
		ps2counter<=ps2counter+1;
		if ps2counter=1200 then
			ps2_clk<=not ps2_clk;
			ps2counter<=(others => '0');
		end if;
	end if;
end process;


emsx_top : entity work.Virtual_Toplevel
	generic map(
		mouse_fourbyte => '0',
		mouse_init => '0'
	)
  port map(
    -- Clock, Reset ports
		clk21m => clk21m,
		memclk => memclk,
		lock_n => reset,

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
    pMemCke => SDRAM_CKE,
    pMemCs_n => SDRAM_nCS,
    pMemRas_n => SDRAM_nRAS,
    pMemCas_n => SDRAM_nCAS,
    pMemWe_n => SDRAM_nWE,
    pMemUdq => SDRAM_DQMH,
    pMemLdq => SDRAM_DQML,
    pMemBa1 => SDRAM_BA(1),
    pMemBa0 => SDRAM_BA(0),
    pMemAdr => SDRAM_A(11 downto 0),
    pMemDat => SDRAM_DQ,

    -- PS/2 keyboard ports
	 pPs2Clk_out => ps2_keyboard_clk_out,
	 pPs2Dat_out => ps2_keyboard_dat_out,
	 pPs2Clk_in => ps2_keyboard_clk_mix,
	 pPs2Dat_in => ps2_keyboard_dat_in,

    -- PS/2 mouse ports
	 ps2m_clk_out => ps2_mouse_clk_out,
	 ps2m_dat_out => ps2_mouse_dat_out,
	 ps2m_clk_in => ps2_mouse_clk_mix,
	 ps2m_dat_in => ps2_mouse_dat_in,

	 pJoyA => joyn_0,
		pJoyB => joyn_1,
--    -- Joystick ports (Port_A, Port_B)
--    pJoyA => std_logic_vector(c64_joy1), --       : inout std_logic_vector( 5 downto 0):=(others=>'1');
--    pStrA       : out std_logic;
--    pJoyB => std_logic_vector(c64_joy2), --       : inout std_logic_vector( 5 downto 0):=(others=>'1');
--    pStrB       : out std_logic;

    -- SD/MMC slot ports
    pSd_Ck => sd_sck,
    pSd_Cm => sd_sdi,
--  pSd_Dt	    : inout std_logic_vector( 3 downto 0);  -- pin 1(D3), 9(D2), 8(D1), 7(D0)
    pSd_Dt3	=> sd_cs,
    pSd_Dt0	=> sd_sdo,

		-- DIP switch, Lamp ports
    pSW => "111"&reset,
    pDip => "0000111001",
    pLedG => open,
    pLedR => open,

    -- Video, Audio/CMT ports
    unsigned(pDac_VR) => vga_tred,
    unsigned(pDac_VG) => vga_tgreen,
    unsigned(pDac_VB) => vga_tblue,
--    pDac_S 		: out   std_logic;						-- Sound
--    pREM_out	: out   std_logic;						-- REM output; 1 - Tape On
--    pCMT_out	: out   std_logic;						-- CMT output
--    pCMT_in		: in    std_logic :='1';						-- CMT input

    pVideoHS_n => VGA_HS,
    pVideoVS_n => VGA_VS,

    -- DE1 7-SEG Display
    hex => open,

	 SOUND_L => audiol,
	 SOUND_R => audior,
	 CmtIn => '1',
	 
	 RS232_RxD => UART_RX,
	 RS232_TxD => txd
);

UART_TX <='1';

mist_console_d: component mist_console
	generic map
	( CLKFREQ => 86)
	port map
	(
		clk => memclk,
		n_reset => reset,
		ser_in => txd,
		par_out_data => par_out_data,
		par_out_strobe => par_out_strobe
	);

sd_card_d: component sd_card
	port map
	(
 		-- connection to io controller
 		io_lba => sd_lba,
 		io_rd  => sd_rd,
		io_wr  => sd_wr,
 		io_ack => sd_ack,
		io_conf => sd_conf,
		io_sdhc => sd_sdhc,
 		io_din => sd_data_in,
 		io_din_strobe => sd_data_in_strobe,
		io_dout => sd_data_out,
		io_dout_strobe => sd_data_out_strobe,
 
		allow_sdhc  => '0',   -- MSX does not support SDHC
		
 		-- connection to host
 		sd_cs  => sd_cs,
 		sd_sck => sd_sck,
		sd_sdi => sd_sdi,
		sd_sdo => sd_sdo		
	);

-- prevent joystick signals from being optimzed away
LED <= '0' when ((joy_ana_0 /= joy_ana_1) AND (joy_0 /= joy_1)) else '1';
	
user_io_d : user_io
    generic map (STRLEN => 1)
    port map (
      SPI_CLK => SPI_SCK,
      SPI_SS_IO => CONF_DATA0,
      SPI_MISO => SPI_DO,
      SPI_MOSI => SPI_DI,
      conf_str => "00000000",   -- no config string -> no osd
      status => status,
		
 		-- connection to io controller
		sd_lba  => sd_lba,
		sd_rd   => sd_rd,
		sd_wr   => sd_wr,
		sd_ack  => sd_ack,
		sd_sdhc => sd_sdhc,
		sd_conf => sd_conf,
 		sd_dout => sd_data_in,
 		sd_dout_strobe => sd_data_in_strobe,
		sd_din => sd_data_out,
		sd_din_strobe => sd_data_out_strobe,

      joystick_0 => joy_0,
      joystick_1 => joy_1,
      joystick_analog_0 => joy_ana_0,
      joystick_analog_1 => joy_ana_1,
--      switches => switches,
       BUTTONS => buttons,
		ps2_clk => ps2_clk,
      ps2_kbd_clk => ps2_keyboard_clk_in,
      ps2_kbd_data => ps2_keyboard_dat_in,
      ps2_mouse_clk => ps2_mouse_clk_in,
      ps2_mouse_data => ps2_mouse_dat_in,
 		serial_data => par_out_data,
 		serial_strobe => par_out_strobe
 );
 
-- swap, invert and remap joystick bits
 joyn_0 <= not joy_1(5) & not joy_1(4) & not joy_1(0) & not joy_1(1) & not joy_1(2) & not joy_1(3);
 joyn_1 <= not joy_0(5) & not joy_0(4) & not joy_0(0) & not joy_0(1) & not joy_0(2) & not joy_0(3);
 
vga_window<='1';
mydither : component video_vga_dither
	generic map (
		outbits => 6
	)
	port map (
		clk => fastclk,
		hsync => VGA_HS,
		vsync => VGA_VS,
		vid_ena => vga_window,
		iRed => vga_tred,
		iGreen => vga_tgreen,
		iBlue => vga_tblue,
		std_logic_vector(oRed) => VGA_R,
		std_logic_vector(oGreen) => VGA_G,
		std_logic_vector(oBlue) => VGA_B
	);
 

-- Do we have audio?  If so, instantiate a two DAC channels.
leftsd: component hybrid_pwm_sd
	port map
	(
		clk => fastclk,
		n_reset => reset,
		din => std_logic_vector(audiol),
		dout => AUDIO_L
	);
	
rightsd: component hybrid_pwm_sd
	port map
	(
		clk => fastclk,
		n_reset => reset,
		din => std_logic_vector(audior),
		dout => AUDIO_R
	);

end architecture;
