-- fsm.vhd: Finite State Machine
-- Author(s):  IVan Sevcik
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity fsm is
port(
   CLK         : in  std_logic;
   RESET       : in  std_logic;

   -- Input signals
   KEY         : in  std_logic_vector(15 downto 0);
   CNT_OF      : in  std_logic;

   -- Output signals
   FSM_CNT_CE  : out std_logic;
   FSM_MX_MEM  : out std_logic;
   FSM_MX_LCD  : out std_logic;
   FSM_LCD_WR  : out std_logic;
   FSM_LCD_CLR : out std_logic
);
end entity fsm;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of fsm is
   type t_state is (TEST1, TEST2, TEST3_1, TEST3_2, TEST4_1, TEST4_2, TEST5_1, TEST5_2,
	TEST6_1, TEST6_2, TEST7_1, TEST7_2, TEST8_1, TEST8_2, TEST9_1, TEST9_2,  TEST10_1, TEST10_2,
	CONFIRM, WRONG, PRINT_CORRECT, PRINT_WRONG, FINISH);
   signal present_state, next_state : t_state;

begin
-- -------------------------------------------------------
sync_logic : process(RESET, CLK)
begin
   if (RESET = '1') then
      present_state <= TEST1;
   elsif (CLK'event AND CLK = '1') then
      present_state <= next_state;
   end if;
end process sync_logic;

-- Pristupove kody:
-- kod1 = 1157599492
-- kod2 = 1778238780
-- -------------------------------------------------------
next_state_logic : process(present_state, KEY, CNT_OF)
begin
   case (present_state) is
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST1 =>
		next_state <= TEST1;
		if(KEY(1) = '1') then
         next_state <= TEST2;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
	when TEST2 =>
		next_state <= TEST2;
		if(KEY(1) = '1') then
			next_state <= TEST3_1;
		elsif(KEY(7) = '1') then
			next_state <= TEST3_2;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST3_1 =>
		next_state <= TEST3_1;
		if(KEY(5) = '1') then
			next_state <= TEST4_1;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST3_2 =>
		next_state <= TEST3_2;
		if(KEY(7) = '1') then
			next_state <= TEST4_2;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST4_1 =>
		next_state <= TEST4_1;
		if(KEY(7) = '1') then
			next_state <= TEST5_1;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST4_2 =>
		next_state <= TEST4_2;
		if(KEY(8) = '1') then
			next_state <= TEST5_2;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST5_1 =>
		next_state <= TEST5_1;
		if(KEY(5) = '1') then
			next_state <= TEST6_1;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST5_2 =>
		next_state <= TEST5_2;
		if(KEY(2) = '1') then
			next_state <= TEST6_2;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST6_1 =>
		next_state <= TEST6_1;
		if(KEY(9) = '1') then
			next_state <= TEST7_1;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST6_2 =>
		next_state <= TEST6_2;
		if(KEY(3) = '1') then
			next_state <= TEST7_2;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST7_1 =>
		next_state <= TEST7_1;
		if(KEY(9) = '1') then
			next_state <= TEST8_1;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST7_2 =>
		next_state <= TEST7_2;
		if(KEY(8) = '1') then
			next_state <= TEST8_2;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST8_1 =>
		next_state <= TEST8_1; 
		if(KEY(4) = '1') then
			next_state <= TEST9_1;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST8_2 =>
		next_state <= TEST8_2; 
		if(KEY(7) = '1') then
			next_state <= TEST9_2;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST9_1 =>
		next_state <= TEST9_1; 
		if(KEY(9) = '1') then
			next_state <= TEST10_1;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST9_2 =>
		next_state <= TEST9_2; 
		if(KEY(8) = '1') then
			next_state <= TEST10_2;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST10_1 =>
		next_state <= TEST10_1; 
		if(KEY(2) = '1') then
			next_state <= CONFIRM;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when TEST10_2 =>
		next_state <= TEST10_2; 
		if(KEY(0) = '1') then
			next_state <= CONFIRM;
		elsif(KEY(15) = '1') then
			next_state <= PRINT_WRONG;
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when CONFIRM =>
		next_state <= CONFIRM; 
      if (KEY(15) = '1') then
         next_state <= PRINT_CORRECT; 
		elsif(or_reduce(KEY) /= '0') then
			next_state <= WRONG;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when WRONG =>
      next_state <= WRONG;
      if (KEY(15) = '1') then
         next_state <= PRINT_WRONG; 
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when PRINT_CORRECT =>
      next_state <= PRINT_CORRECT;
      if (CNT_OF = '1') then
         next_state <= FINISH;
      end if;
	-- - - - - - - - - - - - - - - - - - - - - - -
   when PRINT_WRONG =>
      next_state <= PRINT_WRONG;
      if (CNT_OF = '1') then
         next_state <= FINISH;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when FINISH =>
      next_state <= FINISH;
      if (KEY(15) = '1') then
         next_state <= TEST1; 
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when others =>
      next_state <= TEST1;
   end case;
end process next_state_logic;

-- -------------------------------------------------------
output_logic : process(present_state, KEY)
begin
   FSM_CNT_CE     <= '0';
   FSM_MX_LCD     <= '0';
   FSM_LCD_WR     <= '0';
   FSM_LCD_CLR    <= '0';

   case (present_state) is
   -- - - - - - - - - - - - - - - - - - - - - - -
   when PRINT_CORRECT =>
		FSM_MX_MEM 		<= '1';
      FSM_CNT_CE     <= '1';
      FSM_MX_LCD     <= '1';
      FSM_LCD_WR     <= '1';
	-- - - - - - - - - - - - - - - - - - - - - - -
   when PRINT_WRONG =>
		FSM_MX_MEM 		<= '0';
      FSM_CNT_CE     <= '1';
      FSM_MX_LCD     <= '1';
      FSM_LCD_WR     <= '1';
   -- - - - - - - - - - - - - - - - - - - - - - -
   when FINISH =>
      if (KEY(15) = '1') then
         FSM_LCD_CLR    <= '1';
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when others =>
		if (or_reduce(KEY(14 downto 0)) /= '0') then
         FSM_LCD_WR <= '1';
      end if;
      if (KEY(15) = '1') then
         FSM_LCD_CLR <= '1';
      end if;
   end case;
end process output_logic;

end architecture behavioral;

