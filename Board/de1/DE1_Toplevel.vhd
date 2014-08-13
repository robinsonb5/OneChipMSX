library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.numeric_std.ALL;

entity DE1_Toplevel is
	port
	(
		CLOCK_24		:	 in std_logic_vector(1 downto 0);
		CLOCK_27		:	 in std_logic_vector(1 downto 0);
		CLOCK_50		:	 in STD_LOGIC;
		EXT_CLOCK		:	 in STD_LOGIC;
		KEY		:	 in std_logic_vector(3 downto 0);
		SW		:	 in std_logic_vector(9 downto 0);
		HEX0		:	 out std_logic_vector(6 downto 0);
		HEX1		:	 out std_logic_vector(6 downto 0);
		HEX2		:	 out std_logic_vector(6 downto 0);
		HEX3		:	 out std_logic_vector(6 downto 0);
		LEDG		:	 out std_logic_vector(7 downto 0);
		LEDR		:	 out std_logic_vector(9 downto 0);
		UART_TXD		:	 out STD_LOGIC;
		UART_RXD		:	 in STD_LOGIC;
		DRAM_DQ		:	 inout std_logic_vector(15 downto 0);
		DRAM_ADDR		:	 out std_logic_vector(11 downto 0);
		DRAM_LDQM		:	 out STD_LOGIC;
		DRAM_UDQM		:	 out STD_LOGIC;
		DRAM_WE_N		:	 out STD_LOGIC;
		DRAM_CAS_N		:	 out STD_LOGIC;
		DRAM_RAS_N		:	 out STD_LOGIC;
		DRAM_CS_N		:	 out STD_LOGIC;
		DRAM_BA_0		:	 out STD_LOGIC;
		DRAM_BA_1		:	 out STD_LOGIC;
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
		SD_DAT		:	 inout STD_LOGIC;	-- in
		SD_DAT3		:	 inout STD_LOGIC; -- out
		SD_CMD		:	 out STD_LOGIC;
		SD_CLK		:	 out STD_LOGIC;
		TDI		:	 in STD_LOGIC;
		TCK		:	 in STD_LOGIC;
		TCS		:	 in STD_LOGIC;
		TDO		:	 out STD_LOGIC;
		I2C_SDAT		:	 inout STD_LOGIC;
		I2C_SCLK		:	 out STD_LOGIC;
		PS2_DAT		:	 inout STD_LOGIC;
		PS2_CLK		:	 inout STD_LOGIC;
		VGA_HS		:	 out STD_LOGIC;
		VGA_VS		:	 out STD_LOGIC;
		VGA_R		:	 out unsigned(3 downto 0);
		VGA_G		:	 out unsigned(3 downto 0);
		VGA_B		:	 out unsigned(3 downto 0);
		AUD_ADCLRCK		:	 out STD_LOGIC;
		AUD_ADCDAT		:	 in STD_LOGIC;
		AUD_DACLRCK		:	 out STD_LOGIC;
		AUD_DACDAT		:	 out STD_LOGIC;
		AUD_BCLK		:	 inout STD_LOGIC;
		AUD_XCK		:	 out STD_LOGIC;
		GPIO_0		:	 inout std_logic_vector(35 downto 0);
		GPIO_1		:	 inout std_logic_vector(35 downto 0)
	);
END entity;

architecture rtl of DE1_Toplevel is

signal reset : std_logic;
signal sysclk : std_logic;
signal slowclk : std_logic;
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

alias PS2_MDAT : std_logic is GPIO_1(19);
alias PS2_MCLK : std_logic is GPIO_1(18);

COMPONENT SEG7_LUT
	PORT
	(
		oSEG		:	 OUT STD_LOGIC_VECTOR(6 DOWNTO 0);
		iDIG		:	 IN STD_LOGIC_VECTOR(3 DOWNTO 0)
	);
END COMPONENT;

COMPONENT audio_top
	PORT
	(
		clk		:	 IN STD_LOGIC;
		rst_n		:	 IN STD_LOGIC;
		rdata		:	 IN SIGNED(15 DOWNTO 0);
		ldata		:	 IN SIGNED(15 DOWNTO 0);
		aud_bclk		:	 OUT STD_LOGIC;
		aud_daclrck		:	 OUT STD_LOGIC;
		aud_dacdat		:	 OUT STD_LOGIC;
		aud_xck		:	 OUT STD_LOGIC;
		i2c_sclk		:	 OUT STD_LOGIC;
		i2c_sdat		:	 INOUT STD_LOGIC
	);
END COMPONENT;

COMPONENT video_vga_dither
	generic (
		outbits : integer :=4
	);
	port (
		clk : in std_logic;
		hsync : in std_logic;
		vsync : in std_logic;
		vid_ena : in std_logic;
		iRed : in unsigned(7 downto 0);
		iGreen : in unsigned(7 downto 0);
		iBlue : in unsigned(7 downto 0);
		oRed : out unsigned(outbits-1 downto 0);
		oGreen : out unsigned(outbits-1 downto 0);
		oBlue : out unsigned(outbits-1 downto 0)
	);
end COMPONENT;

begin

--	All bidir ports tri-stated
FL_DQ <= (others => 'Z');
SRAM_DQ <= (others => 'Z');
I2C_SDAT	<= 'Z';
GPIO_0 <= (others => 'Z');
GPIO_1 <= (others => 'Z');

reset<=(not SW(0) xor KEY(0)) and pll_locked;

--hexdigit0 : component SEG7_LUT
--	port map (oSEG => HEX0, iDIG => hex(3 downto 0));
--hexdigit1 : component SEG7_LUT
--	port map (oSEG => HEX1, iDIG => hex(7 downto 4));
--hexdigit2 : component SEG7_LUT
--	port map (oSEG => HEX2, iDIG => hex(11 downto 8));
--hexdigit3 : component SEG7_LUT
--	port map (oSEG => HEX3, iDIG => hex(15 downto 12));

emsx_top : entity work.emsx_top
  port map(
    -- Clock, Reset ports
    CLOCK_50 => CLOCK_50,
    CLOCK_27 => CLOCK_27(0),

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
	 pMemClk => DRAM_CLK,
    pMemCke => DRAM_CKE,
    pMemCs_n => DRAM_CS_N,
    pMemRas_n => DRAM_RAS_N,
    pMemCas_n => DRAM_CAS_N,
    pMemWe_n => DRAM_WE_N,
    pMemUdq => DRAM_UDQM,
    pMemLdq => DRAM_LDQM,
    pMemBa1 => DRAM_BA_1,
    pMemBa0 => DRAM_BA_0,
    pMemAdr => DRAM_ADDR,
    pMemDat => DRAM_DQ,

    -- PS/2 keyboard ports
    pPs2Clk => PS2_CLK,
    pPs2Dat => PS2_DAT,
	 
--    -- Joystick ports (Port_A, Port_B)
--    pJoyA       : inout std_logic_vector( 5 downto 0):=(others=>'1');
--    pStrA       : out std_logic;
--    pJoyB       : inout std_logic_vector( 5 downto 0):=(others=>'1');
--    pStrB       : out std_logic;

    -- SD/MMC slot ports
    pSd_Ck => SD_CLK,
    pSd_Cm => SD_CMD,
--  pSd_Dt	    : inout std_logic_vector( 3 downto 0);  -- pin 1(D3), 9(D2), 8(D1), 7(D0)
    pSd_Dt3	=> SD_DAT3,
    pSd_Dt0	=> SD_DAT,


		-- DIP switch, Lamp ports
    pSW => KEY,
    pDip => SW,
    pLedG => LEDG,
    pLedR => LEDR,

    -- Video, Audio/CMT ports
    pDac_VR => vga_red(7 downto 2),
    pDac_VG => vga_green(7 downto 2),
    pDac_VB => vga_blue(7 downto 2),
--    pDac_S 		: out   std_logic;						-- Sound
--    pREM_out	: out   std_logic;						-- REM output; 1 - Tape On
--    pCMT_out	: out   std_logic;						-- CMT output
--    pCMT_in		: in    std_logic :='1';						-- CMT input

    pVideoHS_n => vga_hsync,
    pVideoVS_n => vga_vsync,

    -- DE1 7-SEG Display
    HEX0 => HEX0,
    HEX1 => HEX1,
    HEX2 => HEX2,
    HEX3 => HEX3,

    -- DE1 i2c
    I2C_SCLK => I2C_SCLK,
    I2C_SDAT => I2C_SDAT,

    AUD_ADCLRCK => AUD_ADCLRCK,
    AUD_ADCDAT	=> AUD_ADCDAT,
    AUD_XCK => AUD_XCK,
    AUD_DACLRCK => AUD_DACLRCK,
    AUD_DACDAT => AUD_DACDAT,
    AUD_BCLK => AUD_BCLK
);

	VGA_HS<=vga_hsync;
	VGA_VS<=vga_vsync;
	vga_window<='1';

	mydither : component video_vga_dither
		generic map(
			outbits => 4
		)
		port map(
			clk=>CLOCK_50, -- FIXME - sysclk
			hsync=>vga_hsync,
			vsync=>vga_vsync,
			vid_ena=>vga_window,
			iRed => unsigned(vga_red),
			iGreen => unsigned(vga_green),
			iBlue => unsigned(vga_blue),
			oRed => VGA_R,
			oGreen => VGA_G,
			oBlue => VGA_B
		);

--myaudio: component audio_top
--port map(
--  clk=>slowclk,
--  rst_n=>reset,
--  -- audio shifter,
--  rdata=>audio_r,
--  ldata=>audio_l,
--  aud_bclk=>AUD_BCLK, -- CODEC data clock
--  aud_daclrck=>AUD_DACLRCK, -- CODEC data clock
--  aud_dacdat=>AUD_DACDAT, -- CODEC data
--  aud_xck=>AUD_XCK, -- CODEC data clock
--  -- I2C audio config
--  i2c_sclk=>I2C_SCLK, -- CODEC config clock
--  i2c_sdat=>I2C_SDAT -- CODEC config data
--);

end architecture;
