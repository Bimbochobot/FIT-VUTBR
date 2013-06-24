-- breakout_top.vhd
-- Copyright (C) 2013 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Ivan Sevcik <xsevci50 AT stud.fit.vutbr.cz>
-- 
-- LICENSE TERMS
-- 
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions
-- are met:
-- 1. Redistributions of source code must retain the above copyright
--    notice, this list of conditions and the following disclaimer.
-- 2. Redistributions in binary form must reproduce the above copyright
--    notice, this list of conditions and the following disclaimer in
--    the documentation and/or other materials provided with the
--    distribution.
-- 3. All advertising materials mentioning features or use of this software
--    or firmware must display the following acknowledgement: 
--
--      This product includes software developed by the University of
--      Technology, Faculty of Information Technology, Brno and its 
--      contributors. 
--
-- 4. Neither the name of the Company nor the names of its contributors
--    may be used to endorse or promote products derived from this
--    software without specific prior written permission.
-- 
-- This software is provided ``as is'', and any express or implied
-- warranties, including, but not limited to, the implied warranties of
-- merchantability and fitness for a particular purpose are disclaimed.
-- In no event shall the company or contributors be liable for any
-- direct, indirect, incidental, special, exemplary, or consequential
-- damages (including, but not limited to, procurement of substitute
-- goods or services; loss of use, data, or profits; or business
-- interruption) however caused and on any theory of liability, whether
-- in contract, strict liability, or tort (including negligence or
-- otherwise) arising in any way out of the use of this software, even
-- if advised of the possibility of such damage.

library IEEE;
use IEEE.std_logic_1164.ALL;
use ieee.std_logic_unsigned.all;
use ieee.numeric_std.all;
use work.vga_controller_cfg.all;
use work.clkgen_cfg.all;
use work.utilities.all;

architecture arch_vga_mode of tlv_pc_ifc is
	
   signal irgb : std_logic_vector(8 downto 0);
   signal rrow : std_logic_vector(11 downto 0);
   signal rcol : std_logic_vector(11 downto 0);
	
	signal mouse_out	: std_logic_vector(7 downto 0);
	signal mouse_vld	: std_logic;
	signal mouse_err	: std_logic;
	signal mouseMoveX	: signed(8 downto 0) := "000000000";
	
	signal kbd_out	: std_logic_vector(15 downto 0);
	signal kbd_vld	: std_logic;
	
	-- Game area: 
	-- X: 32 - 512, middle at 272
	-- Y: 16 - 480, middle at 248
	constant ballDim : signed(4 downto 0) := to_signed(4, 5);
   signal ballRow : signed(rrow'length downto 0) := to_signed(446, rrow'length + 1);
	signal ballCol : signed(rcol'length downto 0) := to_signed(270, rcol'length + 1);
	
	signal ballMoveX : signed(3 downto 0) := to_signed(1, 4);
	signal ballMoveY : signed(3 downto 0) := to_signed(-1, 4);
	
	constant paddleWidth : signed(6 downto 0) := to_signed(50, 7);
	constant paddleHeight : signed(4 downto 0) := to_signed(4, 5);
	constant paddleRow : signed(rrow'length downto 0) := to_signed(450, rrow'length + 1);
	signal paddleCol : signed(rcol'length downto 0) := to_signed(247, rcol'length + 1);
	
	signal gameCLK : std_logic := '0';
	signal game_pause : std_logic := '1';
	signal autopilot : std_logic := '0';
	signal bricks_left : unsigned(9 downto 0) := to_unsigned(112, 10);
	type t_state is (PAUSE, MOVE, UPDATE, LOAD, RAM_READ, BOUNCE, RAM_WRITE, VICTORY, DEFEAT);
   signal gameState : t_state;

   -- Port A VGARAM
	signal vga_aram_addr    : std_logic_vector (10 downto 0); 
	signal vga_aram_dout    : std_logic_vector (7 downto 0); 
	signal vga_aram_din		: std_logic_vector (7 downto 0);
	signal vga_aram_we		: std_logic := '0';

   -- Port B VGARAM
   signal vga_bram_addr    : std_logic_vector (10 downto 0); 
   signal vga_bram_dout    : std_logic_vector (7 downto 0); 
  
   signal vga_mode  : std_logic_vector(60 downto 0); -- default 640x480x60
  
   component RAMB16_S9_S9
		generic (
			INIT_13 : std_logic_vector(255 downto 0);
			INIT_14 : std_logic_vector(255 downto 0);
			INIT_16 : std_logic_vector(255 downto 0);
			INIT_17 : std_logic_vector(255 downto 0);
			INIT_18 : std_logic_vector(255 downto 0);
			INIT_1A : std_logic_vector(255 downto 0)
		);
      port (
         DOA   : out std_logic_vector(7 downto 0);
			DOPA0	: out std_ulogic;
         ADDRA : in std_logic_vector(10 downto 0);
         CLKA  : in std_ulogic;
         DIA   : in std_logic_vector(7 downto 0);
			DIPA0	: in std_ulogic;
         ENA   : in std_ulogic;
         SSRA  : in std_ulogic;
         WEA   : in std_ulogic;

         DOB   : out std_logic_vector(7 downto 0);
			DOPB0	: out std_ulogic;
         ADDRB : in std_logic_vector(10 downto 0);
         CLKB  : in std_ulogic;
         DIB   : in std_logic_vector(7 downto 0);
			DIPB0	: in std_ulogic;
         ENB   : in std_ulogic;
         SSRB  : in std_ulogic;
         WEB   : in std_ulogic
      );
   end component;

component VGA_controller is 
      generic(
			REQ_DELAY : integer
		);
      port(
         CLK    : in std_logic; 
         RST    : in std_logic;
         ENABLE : in std_logic;
         MODE   : in std_logic_vector(60 downto 0);

         DATA_RED    : in std_logic_vector(8 downto 6);
         DATA_GREEN  : in std_logic_vector(5 downto 3);
         DATA_BLUE   : in std_logic_vector(2 downto 0);
         ADDR_COLUMN : out std_logic_vector(11 downto 0);
         ADDR_ROW    : out std_logic_vector(11 downto 0);

         VGA_RED   : out std_logic_vector;
         VGA_BLUE  : out std_logic_vector;
         VGA_GREEN : out std_logic_vector;
         VGA_HSYNC : out std_logic;
         VGA_VSYNC : out std_logic
      );
end component;

component PS2_controller is
	port (
		CLK		: in std_logic;
		RST		: in std_logic;
		DATA_IN	: in std_logic_vector(7 downto 0);
		WRITE_EN	: in std_logic;
		DATA_OUT	: out std_logic_vector(7 downto 0);
		DATA_VLD	: out std_logic;
		DATA_ERR	: out std_logic;
		
		PS2_CLK	: inout std_logic;
		PS2_DATA	: inout std_logic
	);
end component;

-- Keyboard 4x4
component keyboard_controller
	port(
		CLK      : in std_logic;
		RST      : in std_logic;

		DATA_OUT : out std_logic_vector(15 downto 0);
		DATA_VLD : out std_logic;

		KB_KIN   : out std_logic_vector(3 downto 0);
		KB_KOUT  : in  std_logic_vector(3 downto 0)
	);
end component;

for vga : VGA_controller use entity work.VGA_controller(arch_vga_controller);
for ps2ms : PS2_controller use entity work.PS2_controller(half);
for kbd : keyboard_controller use entity work.keyboard_controller;

begin
   -- Set graphical mode (640x480, 60 Hz refresh)
   setmode(r640x480x60, vga_mode);

   blkram_vram0: RAMB16_S9_S9
		generic map( -- Level definition
			INIT_13 => X"0000001816181618161816181618161816181618161816181618161816181600", -- 28 dest. blocks
			INIT_14 => X"0000002929290000292929000029000000000000290000292929000029292900", --  0 dest. blocks
			INIT_16 => X"0000001300130000111611161116111600001611161116111611000013001300", -- 20 dest. blocks
			INIT_17 => X"0000000014000000001116111611161113141116111611161100000000140000", -- 18 dest. blocks
			INIT_18 => X"0000001300130000000011161116111614131611161116110000000013001300", -- 18 dest. blocks
			INIT_1A => X"0000004A174A4A12124A1515154A4A141414144A4A1515154A12124A4A174A00"  -- 28 dest. blocks
		)
      port map (
         DOA             => vga_aram_dout,
			DOPA0				 => open,
         ADDRA           => vga_aram_addr,
         CLKA            => CLK,
         DIA             => vga_aram_din,
			DIPA0				 => '0',
         ENA             => '1',
         SSRA            => '0',
         WEA             => vga_aram_we,

         DOB             => vga_bram_dout,
			DOPB0				 => open,
         ADDRB           => vga_bram_addr,
         CLKB            => CLK,
         DIB             => (others => '0'),
			DIPB0				 => '0',
         ENB             => '1',
         SSRB            => '0',
         WEB             => '0'
      );
      
   -- VGA controller, delay 1 tact
   vga: VGA_controller
      generic map (REQ_DELAY => 1)
      port map (
         CLK    => CLK, 
         RST    => RESET,
         ENABLE => '1',
         MODE   => vga_mode,

         DATA_RED    => irgb(8 downto 6),
         DATA_GREEN  => irgb(5 downto 3),
         DATA_BLUE   => irgb(2 downto 0),
         ADDR_COLUMN => rcol,
         ADDR_ROW    => rrow,

         VGA_RED   => RED_V,
         VGA_BLUE  => BLUE_V,
         VGA_GREEN => GREEN_V,
         VGA_HSYNC => HSYNC_V,
         VGA_VSYNC => VSYNC_V
      );
	
	-- PS2 controller, will be used for mouse
	ps2ms: PS2_controller
		port map (
			CLK		=> SMCLK,
			RST		=> RESET,
			DATA_IN	=> (others => '0'),
			WRITE_EN	=> '0',
			DATA_OUT	=> mouse_out,
			DATA_VLD	=> mouse_vld,
			DATA_ERR	=> mouse_err,
			
			PS2_CLK	=> M_CLK,
			PS2_DATA	=> M_DATA
		);
		
	kbd: keyboard_controller
      port map(
         CLK      => CLK,
         RST      => RESET,
         
         DATA_OUT => kbd_out,         
         DATA_VLD => kbd_vld,

         KB_KIN   => KIN,
         KB_KOUT  => KOUT
      );

	mouse_process : process(mouse_vld)
	begin
		if rising_edge(mouse_vld) then
				-- mouseMoveX(0) <= mouse_out(0);
		end if;
	end process;
	
	-- Nahradzuje mys pokym nebude vyrieseny kontroler
	kbd_process : process(kbd_vld)
	begin
		if rising_edge(kbd_vld) then
			mouseMoveX <= "000000000";
			if kbd_out(9) = '1' then -- key 6 pressed
				mouseMoveX <= "000000010";
			elsif kbd_out(1) = '1' then -- key 4 pressed 
				mouseMoveX <= "111111110";
			elsif kbd_out(5) = '1' then -- key 5 pressed
				game_pause <= not game_pause;
			elsif kbd_out(11) = '1' then -- key # pressed
				autopilot <= not autopilot;
			end if;
		end if;
	end process;

	-- Assume 25 MHz CLK
	-- Optimal is 250 Hz per game cycle 
	-- Generate 250*18 = 4500 Hz ~= 5555 CLK ticks
	gameCLK_process : process(CLK)
		variable cnt : integer range 0 to 5555-1 := 0;
	begin
		if rising_edge(CLK) then
			if cnt = 0 then
				gameCLK <= not gameCLK;
			end if;
			cnt := cnt+1;
		end if;
	end process;
	
	-- It takes 18 gameCLK ticks to complete one game cycle
	game_logic: process(gameCLK)
		variable right : signed(rcol'length downto 0);
		variable bottom : signed(rrow'length downto 0);
		
		variable newX : signed(rcol'length downto 0);
		variable newY : signed(rrow'length downto 0);
		
		variable newPaddleCol : signed(rcol'length downto 0);
		
		variable blockX : std_logic_vector(4 downto 0);
		variable blockY : std_logic_vector(4 downto 0);
		variable newBlockX :  std_logic_vector(4 downto 0);
		variable newBlockY :  std_logic_vector(4 downto 0);
		
		variable readCnt : integer range 0 to 3 := 0;
		
		variable bounceFlip : std_logic := '0';
		variable bounceCase : std_logic_vector(3 downto 0) := "0000";
		
		type brickBufferType is array (0 to 2) of std_logic_vector(3 downto 0);
		variable brickBuffer : brickBufferType;
		variable brick : integer range 0 to 3 := 0;
	begin
		if rising_edge(gameCLK) then
			newX := ballCol + ballMoveX;
			newY := ballRow + ballMoveY;
			
			right  := newX + ballDim;
			bottom := newY + ballDim;
			
			blockX := std_logic_vector(ballCol(8 downto 4));
			blockY := std_logic_vector(ballRow(7 downto 3));
			
			newBlockX := std_logic_vector(newX(8 downto 4));
			newBlockY := std_logic_vector(newY(7 downto 3));
			
			if gameState = PAUSE then
				gameState <= MOVE;
				if game_pause = '1' then gameState <= PAUSE; end if;
			elsif gameState = MOVE then
				ballCol <= newX;
				ballRow <= newY;
				if autopilot = '1' then
					paddleCol <= newX;
				else
					newPaddleCol := paddleCol + mouseMoveX;
					if newPaddleCol < 32 then
						paddleCol <= to_signed(32, paddleCol'length);
					elsif newPaddleCol > 462 then -- max_x - paddle_width
						paddleCol <= to_signed(462, paddleCol'length);
					else
						paddleCol <= newPaddleCol;
					end if;
				end if;
				gameState <= UPDATE;
			elsif gameState = UPDATE then
				gameState <= LOAD;
			
				if bottom > 480 then
					-- Game Over --
					gameState <= DEFEAT;
				elsif bottom = paddleRow and right > paddleCol and (ballCol < (paddleCol + paddleWidth)) then
					ballMoveY <= not ballMoveY + 1;
				elsif ballRow < 16 then
					ballMoveY <= not ballMoveY + 1;
				end if;
				
				if right > 512 then
					ballMoveX <= not ballMoveX + 1;
				elsif ballCol < 32 then
					ballMoveX <= not ballMoveX + 1;
				end if;
				
			elsif gameState = LOAD then
				-- Set address to block being checked
				if newX(11 downto 9) = "000" and newY(11 downto 8) = "0000" then
					if(blockX /= newBlockX or blockY /= newBlockY) then
						-- Create rectangle overlay map
						if newBlockX < blockX 									then bounceCase(0) := '1'; else bounceCase(0) := '0'; end if;
						if std_logic_vector(right(8 downto 4)) > blockX then bounceCase(1) := '1'; else bounceCase(1) := '0'; end if;
						if newBlockY < blockY 									then bounceCase(2) := '1'; else bounceCase(2) := '0'; end if;
						if std_logic_vector(bottom(7 downto 3)) > blockY then bounceCase(3) := '1'; else bounceCase(3) := '0'; end if;
						case bounceCase is
						-- Side cases
						when "0001" => -- left
							case readCnt is
							when 0 	=> blockX := blockX - 1;
							when 1 	=> blockX := (others => '0'); blockY := (others => '0');
							when 2 	=> blockX := (others => '0'); blockY := (others => '0');
							when others => null;
							end case;
						when "0010" => -- right
							case readCnt is
							when 0 	=> blockX := blockX + 1;
							when 1 	=> blockX := (others => '0'); blockY := (others => '0');
							when 2 	=> blockX := (others => '0'); blockY := (others => '0');
							when others => null;
							end case;
						when "0100" => -- top
							case readCnt is
							when 0 	=> blockX := (others => '0'); blockY := (others => '0');
							when 1 	=> blockY := blockY - 1;
							when 2 	=> blockX := (others => '0'); blockY := (others => '0');
							when others => null;
							end case;
						when "1000" => -- bottom
							case readCnt is
							when 0 	=> blockX := (others => '0'); blockY := (others => '0');
							when 1 	=> blockY := blockY + 1;
							when 2 	=> blockX := (others => '0'); blockY := (others => '0');
							when others => null;
							end case;
						-- Corner cases
						when "0101" => -- top left
							case readCnt is
							when 0 	=> blockX := blockX - 1;
							when 1 	=> blockY := blockY - 1;
							when 2 	=> blockX := blockX - 1; blockY := blockY - 1;
							when others => null;
							end case;
						when "0110" => -- top right
							case readCnt is
							when 0 	=> blockX := blockX + 1;
							when 1 	=> blockY := blockY - 1;
							when 2 	=> blockX := blockX + 1; blockY := blockY - 1;
							when others => null;
							end case;
						when "1001" => -- bottom left
							case readCnt is
							when 0 	=> blockX := blockX - 1;
							when 1 	=> blockY := blockY + 1;
							when 2 	=> blockX := blockX - 1; blockY := blockY + 1;
							when others => null;
							end case;
						when "1010" => -- bottom right
							case readCnt is
							when 0 	=> blockX := blockX + 1;
							when 1 	=> blockY := blockY + 1;
							when 2 	=> blockX := blockX + 1; blockY := blockY + 1;
							when others => null;
							end case;
						when others => null;
						end case;
						vga_aram_addr <= "0" & blockY & (not(blockX));
					else
						vga_aram_addr <= "00000000000";
					end if;
				else
					vga_aram_addr <= "00000000000";
				end if;

				gameState <= RAM_READ;
			elsif gameState = RAM_READ then
				-- RAM is controlled by CLK signal, which is divided to get gameCLK
				-- therefore there have been enough ticks for VRAM to set DOA port
				-- Read block type and decide what should happen based on ball position
				brickBuffer(readCnt) := vga_aram_dout(7 downto 4);
				
				readCnt := readCnt + 1;
				if readCnt = 3 then
					readCnt := 0;
					gameState <= BOUNCE;
				else
					gameState <= LOAD;
				end if;
				
			elsif gameState = BOUNCE then
				
				if brickBuffer(0) /= "0000" then
					if brickBuffer(1) /= "0000" then
						if bounceFlip = '0' then
							ballMoveX <= not ballMoveX + 1;
							brick := 0;
						else
							ballMoveY <= not ballMoveY + 1;
							brick := 1;
						end if;
						bounceFlip := not bounceFlip;
					else
						ballMoveX <= not ballMoveX + 1;
						brick := 0;
					end if;
				elsif brickBuffer(1) /= "0000" then
					ballMoveY <= not ballMoveY + 1;
					brick := 1;
				elsif brickBuffer(2) /= "0000" then
					ballMoveX <= not ballMoveX + 1;
					ballMoveY <= not ballMoveY + 1;
					brick := 2;
				end if;
				
				case brickBuffer(brick) is
				when X"1" => 
					vga_aram_we <= '1';
					vga_aram_din <= (others => '0');
					bricks_left <= bricks_left - 1;
				when X"3" =>
					vga_aram_we <= '1';
					vga_aram_din <= X"1C";
				when X"4" =>
					vga_aram_we <= '1';
					vga_aram_din <= X"3B";
				when others => null;
				end case;
				
				-- Select brick address
				-- 1 corner case can process up to 3 bricks: corner and 2 sides
				-- Couple corner with side in clockwise fashion
				case bounceCase is
				when "0101" | "0100" => -- top left + top
					case brick is
					when 0 	=> blockX := blockX - 1;
					when 1 	=> blockY := blockY - 1;
					when 2 	=> blockX := blockX - 1; blockY := blockY - 1;
					when others => null;
					end case;
				when "0110" | "0010" => -- top right + right
					case brick is
					when 0 	=> blockX := blockX + 1;
					when 1 	=> blockY := blockY - 1;
					when 2 	=> blockX := blockX + 1; blockY := blockY - 1;
					when others => null;
					end case;
				when "1001" | "0001" => -- bottom left + left
					case brick is
					when 0 	=> blockX := blockX - 1;
					when 1 	=> blockY := blockY + 1;
					when 2 	=> blockX := blockX - 1; blockY := blockY + 1;
					when others => null;
					end case;
				when "1010" | "1000" => -- bottom right + bottom
					case brick is
					when 0 	=> blockX := blockX + 1;
					when 1 	=> blockY := blockY + 1;
					when 2 	=> blockX := blockX + 1; blockY := blockY + 1;
					when others => null;
					end case;
				when others => null;
				end case;
				vga_aram_addr <= "0" & blockY & (not(blockX));
				
				gameState <= RAM_WRITE;
			elsif gameState = RAM_WRITE then
				-- Again there will be enough ticks for VRAM to read DIA before
				-- write is disabled
				vga_aram_we <= '0';
				
				if bricks_left = 0 then
					gameState <= VICTORY;
				elsif bounceFlip = '0' then
					gameState <= PAUSE;
				else
					gameState <= BOUNCE;
				end if;
			end if;
		elsif gameState = VICTORY then
			-- Congratulations
		elsif gameState = DEFEAT then
			-- Good luck next time
		end if;
	end process;
	
	-- Convert request for VGA (row, column) into VideoRAM address
	-- 32x32 blocks with dimensions 32x16px
   vga_bram_addr <= "0" & rrow(7 downto 3) & (not rcol(8 downto 4));

	select_color: process(vga_bram_dout)
		variable x : signed(rcol'length downto 0);
		variable y : signed(rrow'length downto 0);
	begin
	-- block request
	if rcol(11 downto 9) /= "000" or rcol(8 downto 5) = "0000" or rrow(8 downto 4) = "00000" then
		-- eliminate requests not within range 32x32 blocks
			irgb <= "100100100";
		elsif rrow(11 downto 8) /= "0000" then
			irgb <= "000000000";
		else
			case vga_bram_dout(3 downto 0) is
			when X"0" => -- black
				irgb <= "000000000";
			when X"1" => -- red
				irgb <= "101000000";
			when X"2" => -- yellow
				irgb <= "101101000";
			when X"3" => -- orange
				irgb <= "111011000";
			when X"4" => -- green
				irgb <= "001100001";
			when X"5" => -- light blue
				irgb <= "000011111";
			when X"6" => -- blue
				irgb <= "000000111";
			when X"7" => -- brown
				irgb <= "100001000";
			when X"8" => -- greyish white
				irgb <= "101101101";
			when X"9" => -- steel grey
				irgb <= "010010011";
			when X"A" => -- purple
				irgb <= "011000011";
			when X"B" => -- magenta
				irgb <= "101000101";
			when X"C" => -- light pink
				irgb <= "111011111";
			when others => -- white
				irgb <= "111111111";
			end case;
		end if;
		
		x := signed(rcol) - ballCol;
		y := signed(rrow) - ballRow;
		if (x > 0 and x < ballDim) and (y > 0 and y < ballDim) then
			irgb <= "110110110";
		end if;
		
		x := signed(rcol) - paddleCol;
		y := signed(rrow) - paddleRow;
		if (x > 0 and x < paddleWidth) and (y > 0 and y < paddleHeight) then
			irgb <= "110110110";
		end if;
	end process;
end arch_vga_mode;
