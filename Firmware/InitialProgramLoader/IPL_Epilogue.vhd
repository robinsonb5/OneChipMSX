	);
BEGIN

	PROCESS( CLK )
	BEGIN
		IF( CLK'EVENT AND CLK = '1' )THEN
			DBI <= IPL_DATA( CONV_INTEGER( ADR(8 DOWNTO 0) ) );  -- FIXME reduced from 9 to save blockram
		END IF;
	END PROCESS;
END RTL;
