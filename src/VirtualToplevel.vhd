-- A wrapper in which the more invasive project changes - i.e. the extra
-- Control CPU - can be isolated from upstream as much as possible.

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity virtual_toplevel is
  port(
	clk21m : in std_logic;
	memclk : in std_logic;
	lock_n	: in std_logic;

    -- MSX cartridge slot ports
    pSltClk     : out std_logic;	-- pCpuClk returns here, for Z80, etc.
    pSltRst_n   : in std_logic :='1';		-- pCpuRst_n returns here
    pSltSltsl_n : inout std_logic:='1';
    pSltSlts2_n : inout std_logic:='1';
    pSltIorq_n  : inout std_logic:='1';
    pSltRd_n    : inout std_logic:='1';
    pSltWr_n    : inout std_logic:='1';
    pSltAdr     : inout std_logic_vector(15 downto 0):=(others=>'1');
    pSltDat     : inout std_logic_vector(7 downto 0):=(others=>'1');
    pSltBdir_n  : out std_logic;	-- Bus direction (not used in master mode)

    pSltCs1_n   : inout std_logic:='1';
    pSltCs2_n   : inout std_logic:='1';
    pSltCs12_n  : inout std_logic:='1';
    pSltRfsh_n  : inout std_logic:='1';
    pSltWait_n  : inout std_logic:='1';
    pSltInt_n   : inout std_logic:='1';
    pSltM1_n    : inout std_logic:='1';
    pSltMerq_n  : inout std_logic:='1';

    pSltRsv5    : out std_logic;            -- Reserved
    pSltRsv16   : out std_logic;            -- Reserved (w/ external pull-up)
    pSltSw1     : inout std_logic:='1';          -- Reserved (w/ external pull-up)
    pSltSw2     : inout std_logic:='1';          -- Reserved

    -- SDRAM DE1 ports
--    pMemClk     : out std_logic;            -- SD-RAM Clock
    pMemCke     : out std_logic;            -- SD-RAM Clock enable
    pMemCs_n    : out std_logic;            -- SD-RAM Chip select
    pMemRas_n   : out std_logic;            -- SD-RAM Row/RAS
    pMemCas_n   : out std_logic;            -- SD-RAM /CAS
    pMemWe_n    : out std_logic;            -- SD-RAM /WE
    pMemUdq     : out std_logic;            -- SD-RAM UDQM
    pMemLdq     : out std_logic;            -- SD-RAM LDQM
    pMemBa1     : out std_logic;            -- SD-RAM Bank select address 1
    pMemBa0     : out std_logic;            -- SD-RAM Bank select address 0
    pMemAdr     : out std_logic_vector(11 downto 0);    -- SD-RAM Address
    pMemDat     : inout std_logic_vector(15 downto 0):=(others=>'1');  -- SD-RAM Data

    -- PS/2 keyboard ports - in and out separated by AMR
    pPs2Clk_in     : in std_logic:='1';
    pPs2Dat_in     : in std_logic:='1';
    pPs2Clk_out     : out std_logic;
    pPs2Dat_out     : out std_logic;

    -- Joystick ports (Port_A, Port_B)
    pJoyA       : inout std_logic_vector( 5 downto 0):=(others=>'1');
    pStrA       : out std_logic;
    pJoyB       : inout std_logic_vector( 5 downto 0):=(others=>'1');
    pStrB       : out std_logic;

    -- SD/MMC slot ports
    pSd_Ck      : out std_logic;                        -- pin 5
    pSd_Cm      : out std_logic;                        -- pin 2
--  pSd_Dt	    : inout std_logic_vector( 3 downto 0);  -- pin 1(D3), 9(D2), 8(D1), 7(D0)
    pSd_Dt3	    : out std_logic;						-- pin 1
    pSd_Dt0	    : in std_logic;						-- pin 7

    -- DIP switch, Lamp ports
    pSW		    : in std_logic_vector( 3 downto 0);	    -- 0 - press; 1 - unpress
    pDip        : in std_logic_vector( 9 downto 0);     -- 0=ON,  1=OFF(default on shipment)
    pLedG       : out std_logic_vector( 7 downto 0);   	-- 0=OFF, 1=ON(green)
    pLedR	    : out std_logic_vector( 9 downto 0);    -- 0=OFF, 1=ON(red) ...Power & SD/MMC access lamp

    -- Video, Audio/CMT ports
    pDac_VR     : out std_logic_vector( 5 downto 0);  -- RGB_Red / Svideo_C
    pDac_VG     : out std_logic_vector( 5 downto 0);  -- RGB_Grn / Svideo_Y
    pDac_VB     : out std_logic_vector( 5 downto 0);  -- RGB_Blu / CompositeVideo
    pDac_S		: out   std_logic;						-- Sound
    pREM_out	: out   std_logic;						-- REM output; 1 - Tape On
    pCMT_out	: out   std_logic;						-- CMT output
    pCMT_in		: in    std_logic :='1';						-- CMT input

    pVideoHS_n  : out std_logic;                        -- Csync(RGB15K), HSync(VGA31K)
    pVideoVS_n  : out std_logic;                        -- Audio(RGB15K), VSync(VGA31K)

    -- Hex display
    hex	    : out std_logic_vector(15 downto 0);

	SOUND_L : out std_logic_vector(15 downto 0);
	SOUND_R : out std_logic_vector(15 downto 0);
	CmtIn : in std_logic;
	
	RS232_RxD : in std_logic;
	RS232_TxD : out std_logic
);
end entity;

architecture rtl of Virtual_Toplevel is

signal boot_req : std_logic;
signal boot_ack : std_logic;
signal boot_ack_ctrl : std_logic;
signal boot_data : std_logic_vector(7 downto 0);

signal host_reset_n : std_logic;
signal host_bootdone : std_logic;
signal host_divert_sdcard : std_logic;

signal host_sd_cmd : std_logic;
signal host_sd_dat3 : std_logic;
signal host_sd_clk : std_logic;
signal host_sd_dat : std_logic;

signal ctrl_sd_cmd : std_logic;
signal ctrl_sd_dat3 : std_logic;
signal ctrl_sd_clk : std_logic;

begin


pSd_Ck <= ctrl_sd_clk when host_divert_sdcard='1' else host_sd_clk;
pSd_Dt3 <= ctrl_sd_dat3 when host_divert_sdcard='1' else host_sd_dat3;
pSd_Cm <= ctrl_sd_cmd when host_divert_sdcard='1' else host_sd_cmd;
host_sd_dat <= '1' when host_divert_sdcard='1' else pSd_Dt0;

mymsx : entity work.emsx_top
  port map (
	clk21m => clk21m,
	memclk => memclk,
	lock_n => host_reset_n,

    -- MSX cartridge slot ports
    pSltClk => pSltClk,
    pSltRst_n => pSltRst_n,
    pSltSltsl_n => pSltSltsl_n,
    pSltSlts2_n => pSltSlts2_n,
    pSltIorq_n => pSltIorq_n,
    pSltRd_n => pSltRd_n,
    pSltWr_n => pSltWr_n,
    pSltAdr => pSltAdr,
    pSltDat => pSltDat,
    pSltBdir_n => pSltBdir_n,

    pSltCs1_n => pSltCs1_n,
    pSltCs2_n => pSltCs2_n,
    pSltCs12_n => pSltCs12_n,
    pSltRfsh_n => pSltRfsh_n,
    pSltWait_n => pSltWait_n,
    pSltInt_n => pSltInt_n,
    pSltM1_n => pSltM1_n,
    pSltMerq_n => pSltMerq_n,

    pSltRsv5 => pSltRsv5,
    pSltRsv16 => pSltRsv16,
    pSltSw1 => pSltSw1,
    pSltSw2 => pSltSw2,

    -- SDRAM DE1 ports
    pMemCke => pMemCke,
    pMemCs_n => pMemCs_n,
    pMemRas_n => pMemRas_n,
    pMemCas_n => pMemCas_n,
    pMemWe_n => pMemWe_n,
    pMemUdq => pMemUdq,
    pMemLdq => pMemLdq,
    pMemBa1 => pMemBa1,
    pMemBa0 => pMemBa0,
    pMemAdr => pMemAdr,
    pMemDat => pMemDat,

    -- PS/2 keyboard ports
    pPs2Clk_in => pPs2Clk_in,
    pPs2Dat_in => pPs2Dat_in,
    pPs2Clk_out => pPs2Clk_out,
    pPs2Dat_out => pPs2Dat_out,

    -- Joystick ports (Port_A, Port_B)
    pJoyA => pJoyA,
    pStrA => pStrA,
    pJoyB => pJoyB,
    pStrB => pStrB,

    -- SD/MMC slot ports
    pSd_Ck => host_sd_clk,
    pSd_Cm => host_sd_cmd,
    pSd_Dt3 => host_sd_dat3,
    pSd_Dt0 => host_sd_dat,
	 
    -- DIP switch, Lamp ports
    pSW => pSW,
    pDip => pDip,
    pLedG => pLedG,
    pLedR => pLedR,

    -- Video, Audio/CMT ports
    pDac_VR => pDac_VR,
    pDac_VG => pDac_VG,
    pDac_VB => pDac_VB,
    pDac_S => pDac_S,
    pREM_out => pREM_out,
    pCMT_out => pCMT_out,
    pCMT_in	=> pCMT_in,

    pVideoHS_n => pVideoHS_n,
    pVideoVS_n => pVideoVS_n,

    -- Hex display
    hex => hex,

	SOUND_L => SOUND_L,
	SOUND_R => SOUND_R,
	CmtIn => CmtIn,
	
	boot_req => boot_req,
	boot_ack => boot_ack,
	boot_data => boot_data
);

top : entity work.CtrlModule
	generic map(
		sysclk_frequency => 857
	)
	port map(
		clk => memclk,
		reset_n => lock_n,

		-- SD/MMC slot ports
		spi_clk => ctrl_sd_clk,
		spi_mosi => ctrl_sd_cmd,
		spi_cs => ctrl_sd_dat3,
		spi_miso => pSd_Dt0,
		 
		txd => RS232_TxD,
		rxd => RS232_RxD,
	 
		-- PS/2
		ps2k_clk_in => pPs2Clk_in,
		ps2k_dat_in => pPs2Dat_in,

		host_reset_n => host_reset_n,
		host_bootdone => host_bootdone,
		host_divert_sdcard => host_divert_sdcard,
		
		-- Host boot data
		host_bootdata => boot_data,
		host_bootdata_req => boot_req,
		host_bootdata_ack => boot_ack_ctrl

);

boot_ack<=boot_ack_ctrl or host_bootdone;  -- Once the ROM is fully transferred, ack any further reads immediately.

end architecture;

