library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.numeric_std.ALL;

-- OnScreenDisplay module, generates a character-based display synced with external
-- H and V sync signals.  (Essentially a simple genlock.)
-- Provides a Window and Pixel output, allowing the host to dim the display around
-- the OSD if need be.
-- Registers provided for the following:
--   0 W XPOS
--   1 W YPOS
--   2 W Pixel clock (firmware will guess this from the framing.)
--   3 R hframe - counting how many clocks HSync remains high
--   4 R vframe - ditto, but low.
-- 512 W Character buffer

entity OnScreenDisplay is
port(
	reset_n : in std_logic;
	clk : in std_logic;
	-- Video
	hsync_n : in std_logic;
	vsync_n : in std_logic;
	pixel : out std_logic;
	window : out std_logic;
	-- Registers
	addr : in std_logic_vector(9 downto 0);
	dat_in : in std_logic_vector(15 downto 0);
	dat_out : out std_logic_vector(15 downto 0);
	req : in std_logic
);
end entity;

architecture rtl of OnScreenDisplay is

-- Counted in terms of master clocks.
signal hframe : std_logic_vector(15 downto 0);
signal vframe : std_logic_vector(15 downto 0);
signal hcounter : unsigned(15 downto 0);
signal vcounter: unsigned(15 downto 0);

signal hsync_p : std_logic;
signal vsync_p : std_logic;
signal newline : std_logic;

-- Pixel clock generation

signal pixelclock : unsigned(3 downto 0);
signal pixelcounter : unsigned(3 downto 0);
signal pix : std_logic; -- Triggered momentarily at a pixel boundary

-- Pixel-clock-based signals
signal xpixelpos : unsigned(15 downto 0);
signal ypixelpos : unsigned(15 downto 0);
signal hwindowactive : std_logic;
signal vwindowactive : std_logic;
signal hactive : std_logic;
signal vactive : std_logic;

begin

-- Monitor hsync and count the pulse widths

process(clk,hsync_n)
begin
	if rising_edge(clk) then
		hsync_p<=hsync_n;
		hcounter<=hcounter+1;
		newline<='0';
		if hsync_n='1' then
			if hsync_p='0' then -- rising edge?
				hframe(15 downto 8)<=std_logic_vector(hcounter(11 downto 4));
				hcounter<=(others => '0'); -- Reset counter
			end if;
		else
			if hsync_p='1' then -- falling edge?
				hframe(7 downto 0)<=std_logic_vector(hcounter(11 downto 4));
				hcounter<=(others => '0'); -- Reset counter
				newline<='1';
			end if;		
		end if;
	end if;
end process;


-- Monitor newline and count the vsync pulses

process(clk,hsync_n)
begin
	if rising_edge(clk) then
		if newline='1' then
			vsync_p<=vsync_n;
			vcounter<=vcounter+1;
			if vsync_n='1' then
				if vsync_p='0' then -- rising edge?
					vframe(15 downto 8)<=std_logic_vector(vcounter(11 downto 4));
					vcounter<=(others => '0'); -- Reset counter
				end if;
			else
				if vsync_p='1' then -- falling edge?
					vframe(7 downto 0)<=std_logic_vector(vcounter(11 downto 4));
					vcounter<=(others => '0'); -- Reset counter
					newline<='1';
				end if;		
			end if;
		end if;
	end if;
end process;


-- Increment pixel counter and generate pixel pulse.

process(clk)
begin
	if rising_edge(clk) then
		if pixelcounter=pixelclock then
			pixelcounter<="0000";
			pix<='1';
		else
			pixelcounter<=pixelcounter+1;
			pix<='0';
		end if;
	end if;
end process;





end architecture;
	