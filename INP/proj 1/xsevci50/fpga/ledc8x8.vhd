library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity ledc8x8 is
    port(
        RESET, SMCLK    : in    std_logic;
        ROW, LED        : out   std_logic_vector(0 to 7));
end ledc8x8;

architecture behv of ledc8x8 is
    signal ce       : std_logic := '0';
    signal switch   : std_logic := '0';
    signal row_cnt  : std_logic_vector(7 downto 0) := "10000000";
    signal row_LEDS : std_logic_vector(7 downto 0) := (others => '0');
    signal ctrl_cnt : std_logic_vector(21 downto 0) := (others => '0');
begin
    -- Control counter
    process (SMCLK, RESET)
    begin
        if RESET = '1' then
            ctrl_cnt <= (others => '0');
            ce <= '0';
            switch <= '0';
        
        elsif rising_edge(SMCLK) then
            -- Increment counter
            ctrl_cnt <= ctrl_cnt + 1;
            
            -- SMCLK modulation to SMCLK / 256
            if ctrl_cnt(7 downto 0) = X"FF" then
                ce <= '1';
            else
                ce <= '0';
            end if;
            
            -- Blinking effect
            if ctrl_cnt(21) = '1' then
                switch <= '1';
            else
                switch <= '0';
            end if;
        end if;
    end process;
    
    -- Row counter
    process (SMCLK, RESET)
    begin
        if RESET = '1' then
            -- Initialize to first row
            row_cnt <= "10000000";        
        
        elsif rising_edge(SMCLK) then
            if ce = '1' then
                -- Roll counter
                row_cnt <= row_cnt(0) & row_cnt(7 downto 1);
            end if;
        end if;
    end process;
    
    -- Lead row_cnt directly to the output
    ROW <= row_cnt;

    -- Decoder
    process (row_cnt)
    begin
        case row_cnt is
            when "10000000" => row_LEDS <= "00011111";
            when "01000000" => row_LEDS <= "10111111";
            when "00100000" => row_LEDS <= "10111001"; 
            when "00010000" => row_LEDS <= "10110110"; 
            when "00001000" => row_LEDS <= "10111011"; 
            when "00000100" => row_LEDS <= "00011101"; 
            when "00000010" => row_LEDS <= "11110110"; 
            when "00000001" => row_LEDS <= "11111001"; 
            when others     => row_LEDS <= "11111111";
        end case;
    end process;

    -- Blinker
    process (row_LEDS)
    begin
        if switch = '1' then
            LED <= "11111111";
        else
            LED <= row_LEDS;
        end if;
    end process;
end;