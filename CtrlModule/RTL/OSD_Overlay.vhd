library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.numeric_std.ALL;

entity OSD_Overlay is
port (
	clk : in std_logic;
	red_in : in std_logic_vector(7 downto 0);
	green_in : in std_logic_vector(7 downto 0);
	blue_in : in std_logic_vector(7 downto 0);
	window_in : in std_logic;
	osd_window_in : in std_logic;
	osd_pixel_in : in std_logic;
	red_out : out std_logic_vector(7 downto 0);
	green_out : out std_logic_vector(7 downto 0);
	blue_out : out std_logic_vector(7 downto 0);
	window_out : out std_logic
);
end entity;

architecture RTL of OSD_Overlay is
begin

	process(clk)
	begin
	
		if rising_edge(clk) then
			window_out<=window_in;
			
			if osd_window_in='1' then
				red_out<=osd_pixel_in&osd_pixel_in&red_in(5 downto 0);
				green_out<=osd_pixel_in&osd_pixel_in&green_in(5 downto 0);
				blue_out<=osd_pixel_in&'1'&blue_in(5 downto 0);
			else
				red_out<=red_in;
				green_out<=green_in;
				blue_out<=blue_in;
			end if;
			
		end if;
	
	end process;

end architecture;
