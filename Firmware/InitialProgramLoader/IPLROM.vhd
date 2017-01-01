-- 
-- iplrom.vhd
--   initial program loader for Cyclone & EPCS (Altera)
--   Revision 1.00
-- 
-- Copyright (c) 2006 Kazuhiro Tsujikawa (ESE Artists' factory)
-- All rights reserved.
-- 
-- Redistribution and use of this source code or any derivative works, are 
-- permitted provided that the following conditions are met:
--
-- 1. Redistributions of source code must retain the above copyright notice, 
--    this list of conditions and the following disclaimer.
-- 2. Redistributions in binary form must reproduce the above copyright 
--    notice, this list of conditions and the following disclaimer in the 
--    documentation and/or other materials provided with the distribution.
-- 3. Redistributions may not be sold, nor may they be used in a commercial 
--    product or activity without specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
-- "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
-- TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
-- PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
-- CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
-- EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
-- PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
-- OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
-- WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
-- OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
-- ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--
-- 31th,March,2008
-- patch caro for load KANJI BASIC from SD/MMC
--
-- 13th,April,2008
-- patch t.hara for load SLOT0-1/SLOT0-3 from SD/MMC
--
-- 29th,April,2008
-- patch caro for load compressed BIOS from EPCS4
--

LIBRARY IEEE;
	USE IEEE.STD_LOGIC_1164.ALL;
	USE IEEE.STD_LOGIC_UNSIGNED.ALL;

ENTITY IPLROM IS
	PORT (
		CLK		: IN STD_LOGIC;
		ADR		: IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		DBI		: OUT STD_LOGIC_VECTOR(7 DOWNTO 0)
	);
END IPLROM;

ARCHITECTURE RTL OF IPLROM IS
	TYPE ROM_TYPE IS ARRAY (0 TO 511) OF STD_LOGIC_VECTOR(7 DOWNTO 0);  -- FIXME reduced from 767 to save blockram
	CONSTANT IPL_DATA : ROM_TYPE := (
		X"f3",X"01",X"00",X"02",X"11",X"00",X"f0",X"21",
		X"00",X"00",X"ed",X"b0",X"21",X"1c",X"f0",X"01",
		X"99",X"02",X"ed",X"b3",X"01",X"9a",X"20",X"ed",
		X"b3",X"c3",X"3e",X"f0",X"00",X"5a",X"00",X"00",
		X"00",X"00",X"0b",X"06",X"21",X"07",X"11",X"01",
		X"1b",X"03",X"33",X"01",X"1b",X"06",X"47",X"01",
		X"49",X"03",X"3d",X"06",X"40",X"06",X"0b",X"04",
		X"41",X"02",X"37",X"05",X"4d",X"07",X"31",X"ff",
		X"ff",X"3e",X"60",X"32",X"00",X"60",X"3a",X"00",
		X"40",X"3d",X"ca",X"54",X"f0",X"11",X"00",X"02",
		X"4b",X"cd",X"67",X"f0",X"af",X"32",X"00",X"60",
		X"3c",X"32",X"00",X"68",X"32",X"00",X"70",X"32",
		X"00",X"78",X"3e",X"c0",X"d3",X"a8",X"c7",X"06",
		X"18",X"3e",X"80",X"32",X"00",X"70",X"3c",X"32",
		X"00",X"78",X"3c",X"f5",X"c5",X"06",X"20",X"21",
		X"00",X"80",X"cd",X"84",X"f0",X"c1",X"e1",X"d8",
		X"7c",X"10",X"e8",X"c9",X"d5",X"c5",X"cb",X"23",
		X"cb",X"12",X"78",X"87",X"4f",X"06",X"00",X"e5",
		X"21",X"00",X"40",X"d1",X"7e",X"12",X"13",X"10",
		X"fb",X"0d",X"20",X"f8",X"3a",X"00",X"50",X"c1",
		X"e1",X"af",X"57",X"58",X"19",X"eb",X"89",X"4f",
		X"c9",
		others=>X"00"	);
BEGIN

	PROCESS( CLK )
	BEGIN
		IF( CLK'EVENT AND CLK = '1' )THEN
			DBI <= IPL_DATA( CONV_INTEGER( ADR(8 DOWNTO 0) ) );  -- FIXME reduced from 9 to save blockram
		END IF;
	END PROCESS;
END RTL;
