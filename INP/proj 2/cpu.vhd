-- cpu.vhd: Simple 8-bit CPU (BrainFuck interpreter)
-- Copyright (C) 2013 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Zdenek Vasicek <vasicek AT fit.vutbr.cz>
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet ROM
   CODE_ADDR : out std_logic_vector(11 downto 0); -- adresa do pameti
   CODE_DATA : in std_logic_vector(7 downto 0);   -- CODE_DATA <- rom[CODE_ADDR] pokud CODE_EN='1'
   CODE_EN   : out std_logic;                     -- povoleni cinnosti
   
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(9 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_RDWR  : out std_logic;                    -- cteni (1) / zapis (0)
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA <- stav klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna
   IN_REQ    : out std_logic;                     -- pozadavek na vstup data
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- LCD je zaneprazdnen (1), nelze zapisovat
   OUT_WE   : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is

-- Shift register for storing return addresses (16 items capacity)
type t_ipBuffer is array (15 downto 0) of std_logic_vector(11 downto 0);
signal ipBuffer: t_ipBuffer := (others => (others => '0'));

-- Braces counter for skipping until ] brace
-- Max is 16 == 2^4
signal braces : std_logic_vector(3 downto 0) := (others => '0');

-- CPU FSM states and registers
type t_cpuState is (loadi, waiti, evali, loadv, waitv, evalv, savev, beginw, endw, skipw, writev, readv, incPC, halt);
signal cpuState : t_cpuState;

-- Program counter for indexing instructions in ROM
signal ip : std_logic_vector(11 downto 0) := (others => '0');

-- Data pointer for addressing data in RAM
signal ptr : std_logic_vector(9 downto 0) := (others => '0');

-- Value modifier switch
signal valInc : std_logic := '0';

-- Temporary value register
signal valTmp : std_logic_vector(7 downto 0) := (others => '0');

begin

    process (CLK, RESET)
    begin
        -- Reinitialize CPU
        if RESET = '1' then
            braces <= (others => '0');
            ip <= (others => '0');
            ptr <= (others => '0');
            cpuState <= loadi;
     
        -- CPU works on CLK's rising edge
        elsif rising_edge(CLK) and EN = '1' then
            CODE_EN <= '0';
            DATA_RDWR <= '0';
            DATA_EN <= '0';
            IN_REQ <= '0';
            OUT_WE <= '0';
            
            case cpuState is            
                -- Load new instruction from ROM (available in 2 ticks)
                when loadi =>
                    CODE_ADDR <= ip;
                    CODE_EN <= '1';
                    cpuState <= waiti;
                    
                -- Waits for ROM to update CODE_DATA
                when waiti =>
                    if braces = 0 then
                        cpuState <= evali;
                    else
                        cpuState <= skipw;
                    end if;
                
                -- Evaluate loaded instruction
                when evali =>
                    case CODE_DATA is
                        -- Instruction >
                        when X"3E" =>
                            ptr <= ptr + 1;
                            cpuState <= incPC;
                            
                        -- Instruction <
                        when X"3C" =>
                            ptr <= ptr - 1;
                            cpuState <= incPC;
                            
                        -- Instruction +
                        when X"2B" =>
                            cpuState <= loadv;
                            
                        -- Instruction -
                        when X"2D" =>
                            cpuState <= loadv;
                            
                        -- Instruction [
                        when X"5B" =>
                            cpuState <= loadv;
                            
                        -- Instruction ]
                        when X"5D" =>
                            cpuState <= loadv;
                            
                        -- Instruction .
                        when X"2E" =>
                            cpuState <= loadv;
                            
                        -- Instruction ,
                        when X"2C" =>
                            cpuState <= readv;
                        
                        -- Halt instruction
                        when X"00" =>
                            cpuState <= halt;
                            
                        -- Ignore other characters
                        when others =>
                            cpuState <= incPC;
                    end case;
                    
                -- Load value from RAM
                when loadv =>
                    -- Load value from RAM at ptr (available in next 2 ticks)
                    DATA_ADDR <= ptr;
                    DATA_RDWR <= '1';
                    DATA_EN <= '1';
                    cpuState <= waitv;
                    
                -- Wait for RAM to update DATA_RDATA
                when waitv =>
                    case CODE_DATA is
                        -- Instruction +
                        when X"2B" =>
                            -- Set switch to incrementing
                            valInc <= '1';
                            cpuState <= evalv;
                            
                        -- Instruction -
                        when X"2D" =>
                            -- Set switch to decrementing
                            valInc <= '0';
                            cpuState <= evalv;
                            
                        -- Instruction [
                        when X"5B" =>
                            cpuState <= beginw;
                            
                        -- Instruction ]
                        when X"5D" =>
                            cpuState <= endw;
                            
                        -- Instruction .
                        when X"2E" =>
                            cpuState <= writev;
                            
                        -- Shouldn't be anything else than above
                        when others =>
                            null;
                            
                    end case;
                
                -- Evaluate new value
                when evalv =>
                    -- Compute new value into temporary register
                    if valInc = '1' then
                        valTmp <= DATA_RDATA + 1; 
                    else
                        valTmp <= DATA_RDATA - 1;
                    end if;
                    
                    cpuState <= savev;
                
                -- Save value to RAM
                when savev =>
                    -- Save value to RAM at ptr (will be saved in next tick)
                    DATA_ADDR <= ptr;
                    DATA_RDWR <= '0';
                    DATA_WDATA <= valTmp;
                    DATA_EN <= '1';
                    cpuState <= incPC;
                    
                -- Begin while
                when beginw =>
                    -- If value at ptr is not 0, iterate
                    if DATA_RDATA = 0 then
                        -- Add this brace to counter for skipping
                        braces <= braces + 1;
                    else
                        -- Shift buffer to the left and write current at the end ip
                        ipBuffer <= ipBuffer(14 downto 0) & ip;
                    end if;
                    cpuState <= incPC;
                
                -- End while
                when endw =>
                    -- If value at ptr is not 0, iterate
                    if DATA_RDATA = 0 then
                        -- Shift buffer to the right and leave the begin initialized to 0
                        ipBuffer <= "00000000000" & ipBuffer(15 downto 1);
                    else
                        -- Read ip from buffer
                        ip <= ipBuffer(0);
                    end if;
                    cpuState <= incPC;
                
                -- Skip code in "false" while.
                -- We now need to count braces as there's no "stop" in ipBuffer that would
                -- tell us when this while matching ] is found.
                when skipw =>
                    case CODE_DATA is
                        -- Instruction [
                        when X"5B" =>
                            -- Increase braces counter
                            braces <= braces + 1;
                            
                        -- Instruction ]
                        when X"5D" =>
                            -- Decrease braces counter
                            braces <= braces - 1;
                        
                        -- Don't perform other instructions
                        when others =>
                            null;
                    end case;
                    cpuState <= incPC;
                    
                -- Write value to output
                when writev =>
                    cpuState <= writev;
                    if OUT_BUSY = '0' then
                        OUT_DATA <= DATA_RDATA;
                        OUT_WE <= '1';
                        cpuState <= incPC;
                    end if;
                  
                -- Read value from input
                when readv =>
                    IN_REQ <= '1';
                    cpuState <= readv;
                    if IN_VLD = '1' then
                        valTmp <= IN_DATA;
                        cpuState <= savev;
                    end if;
                
                -- Increment program counter
                when incPC =>
                    ip <= ip + 1;
                    cpuState <= loadi;
                
                when others =>
                    null;
             end case;
        end if;
    end process;

end behavioral;

