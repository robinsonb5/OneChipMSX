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
	TYPE ROM_TYPE IS ARRAY (0 TO 767) OF STD_LOGIC_VECTOR(7 DOWNTO 0);
	CONSTANT IPL_DATA : ROM_TYPE := (
		X"f3",X"18",X"03",X"c3",X"3e",X"f1",X"01",X"00",
		X"02",X"11",X"00",X"f0",X"21",X"00",X"00",X"ed",
		X"b0",X"21",X"21",X"f0",X"01",X"99",X"02",X"ed",
		X"b3",X"01",X"9a",X"20",X"ed",X"b3",X"c3",X"43",
		X"f0",X"00",X"5a",X"00",X"00",X"00",X"00",X"0b",
		X"06",X"21",X"07",X"11",X"01",X"1b",X"03",X"33",
		X"01",X"1b",X"06",X"47",X"01",X"49",X"03",X"3d",
		X"06",X"40",X"06",X"0b",X"04",X"41",X"02",X"37",
		X"05",X"4d",X"07",X"31",X"ff",X"ff",X"3e",X"40",
		X"32",X"00",X"60",X"01",X"00",X"01",X"11",X"00",
		X"00",X"21",X"00",X"c0",X"cd",X"03",X"f0",X"38",
		X"22",X"cd",X"6f",X"f1",X"38",X"13",X"cd",X"90",
		X"f1",X"38",X"18",X"d5",X"c5",X"06",X"01",X"21",
		X"00",X"c0",X"cd",X"03",X"f0",X"c1",X"d1",X"38",
		X"0a",X"cd",X"a8",X"f1",X"38",X"05",X"cd",X"a0",
		X"f0",X"18",X"12",X"21",X"bd",X"f0",X"22",X"04",
		X"f0",X"3e",X"60",X"32",X"00",X"60",X"11",X"00",
		X"02",X"4b",X"cd",X"a0",X"f0",X"af",X"32",X"00",
		X"60",X"3c",X"32",X"00",X"68",X"32",X"00",X"70",
		X"32",X"00",X"78",X"3e",X"c0",X"d3",X"a8",X"c7",
		X"06",X"10",X"3e",X"80",X"32",X"00",X"70",X"3c",
		X"32",X"00",X"78",X"3c",X"f5",X"c5",X"06",X"20",
		X"21",X"00",X"80",X"cd",X"03",X"f0",X"c1",X"e1",
		X"d8",X"7c",X"10",X"e8",X"c9",X"d5",X"c5",X"cb",
		X"23",X"cb",X"12",X"78",X"87",X"4f",X"06",X"00",
		X"e5",X"21",X"00",X"40",X"36",X"03",X"72",X"73",
		X"70",X"7e",X"d1",X"7e",X"12",X"13",X"10",X"fb",
		X"0d",X"20",X"f8",X"3a",X"00",X"50",X"c1",X"e1",
		X"af",X"57",X"58",X"19",X"eb",X"89",X"4f",X"c9",
		X"7e",X"cb",X"23",X"cb",X"12",X"cb",X"11",X"70",
		X"71",X"72",X"73",X"36",X"00",X"36",X"95",X"7e",
		X"06",X"10",X"7e",X"fe",X"ff",X"3f",X"d0",X"10",
		X"f9",X"37",X"c9",X"06",X"0a",X"3a",X"00",X"50",
		X"10",X"fb",X"01",X"00",X"40",X"59",X"51",X"cd",
		X"e8",X"f0",X"d8",X"e6",X"f7",X"fe",X"01",X"37",
		X"c0",X"06",X"77",X"cd",X"e8",X"f0",X"e6",X"04",
		X"28",X"07",X"06",X"41",X"cd",X"e8",X"f0",X"18",
		X"05",X"06",X"69",X"cd",X"e8",X"f0",X"d8",X"fe",
		X"01",X"28",X"e6",X"b7",X"c8",X"37",X"c9",X"cd",
		X"03",X"f1",X"c1",X"d1",X"e1",X"d8",X"e5",X"d5",
		X"c5",X"06",X"51",X"21",X"00",X"40",X"cd",X"e8",
		X"f0",X"38",X"ec",X"c1",X"d1",X"e1",X"b7",X"37",
		X"c0",X"d5",X"c5",X"eb",X"01",X"00",X"02",X"21",
		X"00",X"40",X"7e",X"fe",X"fe",X"20",X"fb",X"ed",
		X"b0",X"eb",X"1a",X"c1",X"1a",X"d1",X"13",X"7a",
		X"b3",X"20",X"01",X"0c",X"10",X"d0",X"c9",X"21",
		X"00",X"c0",X"01",X"80",X"00",X"3e",X"46",X"ed",
		X"b1",X"28",X"02",X"b7",X"c9",X"e5",X"56",X"23",
		X"5e",X"21",X"54",X"41",X"b7",X"ed",X"52",X"e1",
		X"20",X"eb",X"0e",X"00",X"59",X"51",X"37",X"c9",
		X"06",X"04",X"21",X"c6",X"c1",X"e5",X"5e",X"23",
		X"56",X"23",X"4e",X"79",X"b2",X"b3",X"e1",X"c0",
		X"11",X"10",X"00",X"19",X"10",X"ef",X"37",X"c9",
		X"dd",X"21",X"00",X"c0",X"dd",X"6e",X"0e",X"dd",
		X"66",X"0f",X"79",X"19",X"ce",X"00",X"4f",X"dd",
		X"5e",X"11",X"dd",X"56",X"12",X"7b",X"e6",X"0f",
		X"06",X"04",X"cb",X"3a",X"cb",X"1b",X"10",X"fa",
		X"b7",X"28",X"01",X"13",X"d5",X"dd",X"46",X"10",
		X"dd",X"5e",X"16",X"dd",X"56",X"17",X"79",X"19",
		X"ce",X"00",X"10",X"fb",X"d1",X"19",X"eb",X"4f",
		X"d5",X"c5",X"06",X"01",X"21",X"00",X"c0",X"cd",
		X"03",X"f0",X"d8",X"2a",X"00",X"c0",X"11",X"41",
		X"42",X"b7",X"ed",X"52",X"c1",X"d1",X"c8",X"37",
		X"c9",X"00",X"00",X"00",X"00",X"00",X"00",X"00",
		others=>X"00"	);
BEGIN

	PROCESS( CLK )
	BEGIN
		IF( CLK'EVENT AND CLK = '1' )THEN
			DBI <= IPL_DATA( CONV_INTEGER( ADR(9 DOWNTO 0) ) );
		END IF;
	END PROCESS;
END RTL;
