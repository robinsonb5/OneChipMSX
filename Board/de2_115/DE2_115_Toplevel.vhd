library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.numeric_std.ALL;


entity DE2_Toplevel is
	port
	(
--		CLOCK_24		:	 in std_logic_vector(1 downto 0);
--		CLOCK_27		:	 in std_logic;
		CLOCK_50		:	 in STD_LOGIC;
--		EXT_CLOCK		:	 in STD_LOGIC;

		-- Switches
		SW			:	in	std_logic_vector(17 downto 0);
		-- Buttons
		KEY			:	in	std_logic_vector(3 downto 0);
	
		-- 7 segment displays
		HEX0		:	 out std_logic_vector(6 downto 0);
		HEX1		:	 out std_logic_vector(6 downto 0);
		HEX2		:	 out std_logic_vector(6 downto 0);
		HEX3		:	 out std_logic_vector(6 downto 0);
		HEX4		:	out	std_logic_vector(6 downto 0);
		HEX5		:	out	std_logic_vector(6 downto 0);
		HEX6		:	out	std_logic_vector(6 downto 0);
		HEX7		:	out	std_logic_vector(6 downto 0);
	
		-- Red LEDs
		LEDR		:	out	std_logic_vector(17 downto 0);
		-- Green LEDs
		LEDG		:	out	std_logic_vector(8 downto 0);

		UART_TXD		:	 out STD_LOGIC;
		UART_RXD		:	 in STD_LOGIC;
		
		DRAM_DQ			:	 inout std_logic_vector(15 downto 0);
		DRAM_ADDR		:	 out std_logic_vector(11 downto 0);
--		DRAM_LDQM		:	 out STD_LOGIC;
		-- DRAM_DQM[0]		:	 out STD_LOGIC;
		-- DRAM_DQM[1]		:	 out STD_LOGIC;
		DRAM_DQM		:	 out std_logic_vector(1 downto 0);
--		DRAM_UDQM		:	 out STD_LOGIC;
		DRAM_WE_N		:	 out STD_LOGIC;
		DRAM_CAS_N		:	 out STD_LOGIC;
		DRAM_RAS_N		:	 out STD_LOGIC;
		DRAM_CS_N		:	 out STD_LOGIC;
--		DRAM_BA_0		:	 out STD_LOGIC;
--		DRAM_BA_1		:	 out STD_LOGIC;
		-- DRAM_BA[0]		:	 out STD_LOGIC;
		-- DRAM_BA[1]		:	 out STD_LOGIC;
		DRAM_BA			:	 out std_logic_vector(1 downto 0);
		DRAM_CLK		:	 out STD_LOGIC;
		DRAM_CKE		:	 out STD_LOGIC;
		
		FL_DQ		:	 inout std_logic_vector(7 downto 0);
		FL_ADDR		:	 out std_logic_vector(21 downto 0);
		FL_WE_N		:	 out STD_LOGIC;
		FL_RST_N		:	 out STD_LOGIC;
		FL_OE_N		:	 out STD_LOGIC;
		FL_CE_N		:	 out STD_LOGIC;
		
		SRAM_DQ		:	 inout std_logic_vector(15 downto 0);
		SRAM_ADDR		:	 out std_logic_vector(17 downto 0);
		SRAM_UB_N		:	 out STD_LOGIC;
		SRAM_LB_N		:	 out STD_LOGIC;
		SRAM_WE_N		:	 out STD_LOGIC;
		SRAM_CE_N		:	 out STD_LOGIC;
		SRAM_OE_N		:	 out STD_LOGIC;
		
		SD_DAT0		:	 in STD_LOGIC;	-- in
		SD_DAT3		:	 out STD_LOGIC; -- out
		SD_CMD		:	 out STD_LOGIC;
		SD_CLK		:	 out STD_LOGIC;
		
		-- TDI		:	 in STD_LOGIC;
		-- TCK		:	 in STD_LOGIC;
		-- TCS		:	 in STD_LOGIC;
		-- TDO		:	 out STD_LOGIC;
		
		I2C_SDAT		:	 inout STD_LOGIC;
		I2C_SCLK		:	 out STD_LOGIC;
		PS2_DAT		:	 inout STD_LOGIC;
		PS2_CLK		:	 inout STD_LOGIC;
		PS2_DAT2		:	 inout STD_LOGIC;
		PS2_CLK2		:	 inout STD_LOGIC;
		
		VGA_HS		:	 out STD_LOGIC;
		VGA_VS		:	 out STD_LOGIC;
		VGA_R		:	 out unsigned(7 downto 0);
		VGA_G		:	 out unsigned(7 downto 0);
		VGA_B		:	 out unsigned(7 downto 0);
		VGA_SYNC_N	:	 out STD_LOGIC;
		VGA_BLANK_N	:	 out STD_LOGIC;
		VGA_CLK	:	 out STD_LOGIC;
		
		AUD_ADCLRCK		:	 out STD_LOGIC;
		AUD_ADCDAT		:	 in STD_LOGIC;
		AUD_DACLRCK		:	 out STD_LOGIC;
		AUD_DACDAT		:	 out STD_LOGIC;
		AUD_BCLK		:	 inout STD_LOGIC;
		AUD_XCK		:	 out STD_LOGIC;

		Joya : inout std_logic_vector(5 downto 0);
		Joyb : inout std_logic_vector(5 downto 0)
	);
END entity;



architecture rtl of DE2_Toplevel is

  component a_codec
	port(
	  iCLK	    : in std_logic;
	  iSL       : in std_logic_vector(15 downto 0);	-- left chanel
	  iSR       : in std_logic_vector(15 downto 0);	-- right chanel
	  oAUD_XCK	: out std_logic;
	  oAUD_DATA : out std_logic;
	  oAUD_LRCK : out std_logic;
	  oAUD_BCK  : out std_logic;
	  iAUD_ADCDAT	: in std_logic;
	  oAUD_ADCLRCK	: out std_logic;
	  o_tape	: out std_logic	  
	);
  end component;

  component I2C_AV_Config
	port(
	  iCLK	    : in std_logic;
	  iRST_N    : in std_logic;
	  oI2C_SCLK : out std_logic;
	  oI2C_SDAT : inout std_logic
	);
  end component;

  component seg7_lut_4
	port(
	  oSEG0	  : out std_logic_vector(6 downto 0);
	  oSEG1   : out std_logic_vector(6 downto 0);
	  oSEG2   : out std_logic_vector(6 downto 0);
	  oSEG3   : out std_logic_vector(6 downto 0);
	  iDIG	  : in std_logic_vector(15 downto 0)
	);
  end component;

signal reset : std_logic;
signal clk21m      : std_logic;
signal memclk      : std_logic;
signal pll_locked : std_logic;

signal ps2m_clk_in : std_logic;
signal ps2m_clk_out : std_logic;
signal ps2m_dat_in : std_logic;
signal ps2m_dat_out : std_logic;

signal ps2k_clk_in : std_logic;
signal ps2k_clk_out : std_logic;
signal ps2k_dat_in : std_logic;
signal ps2k_dat_out : std_logic;

signal vga_red : std_logic_vector(7 downto 0);
signal vga_green : std_logic_vector(7 downto 0);
signal vga_blue : std_logic_vector(7 downto 0);
signal vga_window : std_logic;
signal vga_hsync : std_logic;
signal vga_vsync : std_logic;

signal audio_l : signed(15 downto 0);
signal audio_r : signed(15 downto 0);

signal hex : std_logic_vector(15 downto 0);

signal SOUND_L : std_logic_vector(15 downto 0);
signal SOUND_R : std_logic_vector(15 downto 0);
signal CmtIn : std_logic;

signal joy1 : std_logic_vector(5 downto 0);
signal joy2 : std_logic_vector(5 downto 0);

COMPONENT SEG7_LUT
	PORT
	(
		oSEG		:	 OUT STD_LOGIC_VECTOR(6 DOWNTO 0);
		iDIG		:	 IN STD_LOGIC_VECTOR(3 DOWNTO 0)
	);
END COMPONENT;


begin

--	All bidir ports tri-stated
FL_DQ <= (others => 'Z');
SRAM_DQ <= (others => 'Z');
I2C_SDAT	<= 'Z';

--ps2m_clk_out <='1';
--ps2m_dat_out <='1';

	-- PS2 keyboard & mouse
ps2m_dat_in<=PS2_DAT2;
PS2_DAT2 <= '0' when ps2m_dat_out='0' else 'Z';
ps2m_clk_in<=PS2_CLK2;
PS2_CLK2 <= '0' when ps2m_clk_out='0' else 'Z';

ps2k_dat_in<=PS2_DAT;
PS2_DAT <= '0' when ps2k_dat_out='0' else 'Z';
ps2k_clk_in<=PS2_CLK;
PS2_CLK <= '0' when ps2k_clk_out='0' else 'Z';


reset<=KEY(0) and pll_locked;


  U00 : entity work.pll_main
    port map(					-- for Altera DE2
		areset => not KEY(0),
		inclk0 => CLOCK_50,       -- 50 MHz external
		c0     => clk21m,         -- 21.43MHz internal (50*3/7)
		c1     => memclk,         -- 85.72MHz = 21.43MHz x 4
		c2     => DRAM_CLK,        -- 85.72MHz external
		locked => pll_locked

    );




emsx_top : entity work.Virtual_Toplevel
	generic map(
		mouse_fourbyte => '0',
		mouse_init => '1'
	)
	port map(
    -- Clock, Reset ports
--    CLOCK_50 => CLOCK_50,
--    CLOCK_27 => CLOCK_27(0),
		clk21m => clk21m,
		memclk => memclk,
		lock_n => pll_locked,

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
--	 pMemClk => DRAM_CLK,
    pMemCke => DRAM_CKE,
    pMemCs_n => DRAM_CS_N,
    pMemRas_n => DRAM_RAS_N,
    pMemCas_n => DRAM_CAS_N,
    pMemWe_n => DRAM_WE_N,
--    pMemUdq => DRAM_UDQM,
    pMemUdq => DRAM_DQM(1),
--    pMemLdq => DRAM_LDQM,
    pMemLdq => DRAM_DQM(0),
--    pMemBa1 => DRAM_BA_1,
    pMemBa1 => DRAM_BA(1),
--    pMemBa0 => DRAM_BA_0,
    pMemBa0 => DRAM_BA(0),
    pMemAdr => DRAM_ADDR,
    pMemDat => DRAM_DQ,

    -- PS/2 keyboard ports
	 pPs2Clk_out => ps2k_clk_out,
	 pPs2Dat_out => ps2k_dat_out,
	 pPs2Clk_in => ps2k_clk_in,
	 pPs2Dat_in => ps2k_dat_in,

	 -- PS/2 mouse ports
		ps2m_clk_in => ps2m_clk_in,
		ps2m_dat_in => ps2m_dat_in,
		ps2m_clk_out => ps2m_clk_out,
		ps2m_dat_out => ps2m_dat_out,
 
--    -- Joystick ports (Port_A, Port_B)
    pJoyA => joy1,
--    pStrA       : out std_logic;
    pJoyB => joy2,
--    pStrB       : out std_logic;

    -- SD/MMC slot ports
    pSd_Ck => SD_CLK,
    pSd_Cm => SD_CMD,
--  pSd_Dt	    : inout std_logic_vector( 3 downto 0);  -- pin 1(D3), 9(D2), 8(D1), 7(D0)
    pSd_Dt3	=> SD_DAT3,
    pSd_Dt0	=> SD_DAT0,


		-- DIP switch, Lamp ports
    pSW => KEY,
    pDip => SW( 9 downto 0),
    pLedG => LEDG( 7 downto 0),
    pLedR => LEDR( 9 downto 0),

    -- Video, Audio/CMT ports
    std_logic_vector(pDac_VR) => VGA_R(7 downto 0),
    std_logic_vector(pDac_VG) => VGA_G(7 downto 0),
    std_logic_vector(pDac_VB) => VGA_B(7 downto 0),
--    pDac_S 		: out   std_logic;						-- Sound
--    pREM_out	: out   std_logic;						-- REM output; 1 - Tape On
--    pCMT_out	: out   std_logic;						-- CMT output
--    pCMT_in		: in    std_logic :='1';						-- CMT input

    pVideoHS_n => VGA_HS,
    pVideoVS_n => VGA_VS,

    -- DE1 7-SEG Display
    hex => hex,

	 SOUND_L => SOUND_L,
	 SOUND_R => SOUND_R,
	 CmtIn => CmtIn,
	 
	 RS232_RxD => UART_RXD,
	 RS232_TxD => UART_TXD
);

-- Remap joystick directions
joy1<=joya(5)&joya(4)&joya(0)&joya(1)&joya(2)&joya(3);
joy2<=joyb(5)&joyb(4)&joyb(0)&joyb(1)&joyb(2)&joyb(3);

-- VGA_R(9 downto 8)<="00";
-- VGA_G(9 downto 8)<="00";
-- VGA_B(9 downto 8)<="00";
VGA_SYNC_N	<='0';
VGA_BLANK_N	<='1';
VGA_CLK<=memclk;

		
-- Hex display		
	U34: seg7_lut_4
    port map (HEX0,HEX1,HEX2,HEX3,hex);

-- Audio
		
  AUD_ADCLRCK	<= 'Z';
		
  U35: a_codec
	port map (
	  iCLK	  => clk21m,
	  iSL     => SOUND_L,
	  iSR     => SOUND_R,
	  oAUD_XCK  => AUD_XCK,
	  oAUD_DATA => AUD_DACDAT,
	  oAUD_LRCK => AUD_DACLRCK,
	  oAUD_BCK  => AUD_BCLK,
	  iAUD_ADCDAT => AUD_ADCDAT,
	  oAUD_ADCLRCK => AUD_ADCLRCK,
	  o_tape => CmtIn
	);

  U36: I2C_AV_Config
	port map (
	  iCLK	  => clk21m,
	  iRST_N  => reset,
	  oI2C_SCLK => I2C_SCLK,
	  oI2C_SDAT => I2C_SDAT
	);

end architecture;
