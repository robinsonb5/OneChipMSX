## Generated SDC file "hello_led.out.sdc"

## Copyright (C) 1991-2011 Altera Corporation
## Your use of Altera Corporation's design tools, logic functions 
## and other software and tools, and its AMPP partner logic 
## functions, and any output files from any of the foregoing 
## (including device programming or simulation files), and any 
## associated documentation or information are expressly subject 
## to the terms and conditions of the Altera Program License 
## Subscription Agreement, Altera MegaCore Function License 
## Agreement, or other applicable license agreement, including, 
## without limitation, that your use is for the sole purpose of 
## programming logic devices manufactured by Altera and sold by 
## Altera or its authorized distributors.  Please refer to the 
## applicable agreement for further details.


## VENDOR  "Altera"
## PROGRAM "Quartus II"
## VERSION "Version 11.1 Build 216 11/23/2011 Service Pack 1 SJ Web Edition"

## DATE    "Fri Jul 06 23:05:47 2012"

##
## DEVICE  "EP3C25Q240C8"
##


#**************************************************************
# Time Information
#**************************************************************

set_time_format -unit ns -decimal_places 3



#**************************************************************
# Create Clock
#**************************************************************

create_clock -name {clk_50} -period 20.000 -waveform { 0.000 0.500 } [get_ports {CLOCK_50}]


#**************************************************************
# Create Generated Clock
#**************************************************************

derive_pll_clocks 
create_generated_clock -name sd1clk_pin -source [get_pins {U00|altpll_component|pll|clk[2]}] [get_ports {DRAM_CLK}]
create_generated_clock -name sysclk -source [get_pins {U00|altpll_component|pll|clk[1]}]
create_generated_clock -name slowclock -source [get_pins {U00|altpll_component|pll|clk[0]}]
create_generated_clock -name clkdiv -source [get_pins {U00|altpll_component|pll|clk[0]}] -divide_by 4 [get_keepers {virtual_toplevel:emsx_top|emsx_top:mymsx|clkdiv[0]}]
create_generated_clock -name Aud1 -source [get_pins {U00|altpll_component|pll|clk[0]}] -divide_by 3 [get_keepers {a_codec:U35|oAUD_XCK}]
create_generated_clock -name Aud2 -source [get_pins {U00|altpll_component|pll|clk[0]}] -divide_by 10 [get_keepers {a_codec:U35|oAUD_BCK}]
create_generated_clock -name Aud3 -source [get_pins {U00|altpll_component|pll|clk[0]}] -divide_by 10 [get_keepers {a_codec:U35|oAUD_DACLRCLK}]
create_generated_clock -name Aud4 -source [get_pins {U00|altpll_component|pll|clk[0]}] -divide_by 10 [get_keepers {a_codec:U35|oAUD_ADCLRCK}]
create_generated_clock -name Aud5 -source [get_pins {U00|altpll_component|pll|clk[0]}] -divide_by 30 [get_keepers {I2C_AV_Config:U36|mI2C_CTRL_CLK}]
create_generated_clock -name reset -source [get_pins {U00|altpll_component|pll|clk[0]}] [get_keepers {virtual_toplevel:emsx_top|emsx_top:mymsx|RstPower}]


#**************************************************************
# Set Clock Latency
#**************************************************************


#**************************************************************
# Set Clock Uncertainty
#**************************************************************

derive_clock_uncertainty;

#**************************************************************
# Set Input Delay
#**************************************************************

set_input_delay -clock sd1clk_pin -max 5.8 [get_ports DRAM_DQ*]
set_input_delay -clock sd1clk_pin -min 3.2 [get_ports DRAM_DQ*]

set_input_delay -clock sysclk -min 0.5 [get_ports SD_DAT*]
set_input_delay -clock sysclk -max 1.0 [get_ports SD_DAT*]

set_input_delay -clock slowclock -min 0.5 [get_ports GPIO*]
set_input_delay -clock slowclock -max 1.0 [get_ports GPIO*]

set_input_delay -clock slowclock -min 0.5 [get_ports KEY*]
set_input_delay -clock slowclock -max 1.0 [get_ports KEY*]

set_input_delay -clock slowclock -min 0.5 [get_ports PS2*]
set_input_delay -clock slowclock -max 1.0 [get_ports PS2*]

set_input_delay -clock slowclock -min 0.5 [get_ports AUD_ADCDAT]
set_input_delay -clock slowclock -max 1.0 [get_ports AUD_ADCDAT]
set_input_delay -clock slowclock -min 0.5 [get_ports I2C_SDAT]
set_input_delay -clock slowclock -max 1.0 [get_ports I2C_SDAT]

#**************************************************************
# Set Output Delay
#**************************************************************

set_output_delay -clock sd1clk_pin -max 1.5 [get_ports DRAM_*]
set_output_delay -clock sd1clk_pin -min -0.8 [get_ports DRAM_*]
set_output_delay -clock sd1clk_pin -max 0.5 [get_ports DRAM_CLK]
set_output_delay -clock sd1clk_pin -min 0.5 [get_ports DRAM_CLK]

set_output_delay -clock slowclock -min 0.5 [get_ports HEX*]
set_output_delay -clock slowclock -max 1.0 [get_ports HEX*]
set_output_delay -clock slowclock -min 0.5 [get_ports LED*]
set_output_delay -clock slowclock -max 1.0 [get_ports LED*]
set_output_delay -clock slowclock -min 0.5 [get_ports VGA*]
set_output_delay -clock slowclock -max 1.0 [get_ports VGA*]
set_output_delay -clock sysclk -min 0.5 [get_ports SD_*]
set_output_delay -clock sysclk -max 1.0 [get_ports SD_*]
set_output_delay -clock sysclk -min 0.5 [get_ports AUD_*]
set_output_delay -clock sysclk -max 1.0 [get_ports AUD_*]
set_output_delay -clock sysclk -min 0.5 [get_ports I2C_*]
set_output_delay -clock sysclk -max 1.0 [get_ports I2C_*]
set_output_delay -clock sysclk -min 0.5 [get_ports UART_*]
set_output_delay -clock sysclk -max 1.0 [get_ports UART_*]
set_output_delay -clock sysclk -min 0.5 [get_ports GPIO_*]
set_output_delay -clock sysclk -max 1.0 [get_ports GPIO_*]
set_output_delay -clock sysclk -min 0.5 [get_ports PS2_*]
set_output_delay -clock sysclk -max 1.0 [get_ports PS2_*]

#**************************************************************
# Set Clock Groups
#**************************************************************



#**************************************************************
# Set False Path
#**************************************************************

#**************************************************************
# Set Multicycle Path
#**************************************************************

#set_multicycle_path -from [get_clocks {mypll|altpll_component|auto_generated|pll1|clk[0]}] -to [get_clocks {sd2clk_pin}] -setup -end 2
#set_multicycle_path -from [get_clocks {mypll2|altpll_component|auto_generated|pll1|clk[0]}] -to [get_clocks {sd2clk_pin}] -setup -end 2

# set_multicycle_path -from [get_clocks {sd1clk_pin}] -to [get_clocks {U00|altpll_component|pll|clk[1]}] -setup -end 2

# set_multicycle_path -from {VirtualToplevel:myVirtualToplevel|*:myrom|*} -to {VirtualToplevel:myVirtualToplevel|zpu_core:zpu|*} -setup -end 2
# set_multicycle_path -from {VirtualToplevel:myVirtualToplevel|*:myrom|*} -to {VirtualToplevel:myVirtualToplevel|zpu_core:zpu|*} -hold -end 2

#**************************************************************
# Set Maximum Delay
#**************************************************************



#**************************************************************
# Set Minimum Delay
#**************************************************************



#**************************************************************
# Set Input Transition
#**************************************************************
