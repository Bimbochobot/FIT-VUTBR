/*******************************************************************************
   main.c: main for breakout game
   Copyright (C) 2013 Brno University of Technology,
                      Faculty of Information Technology
	Author(s): Ivan Sevcik <xsevci50 AT stud.fit.vutbr.cz>

   LICENSE TERMS

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
   3. All advertising materials mentioning features or use of this software
      or firmware must display the following acknowledgement:

        This product includes software developed by the University of
        Technology, Faculty of Information Technology, Brno and its
        contributors.

   4. Neither the name of the Company nor the names of its contributors
      may be used to endorse or promote products derived from this
      software without specific prior written permission.

   This software or firmware is provided ``as is'', and any express or implied
   warranties, including, but not limited to, the implied warranties of
   merchantability and fitness for a particular purpose are disclaimed.
   In no event shall the company or contributors be liable for any
   direct, indirect, incidental, special, exemplary, or consequential
   damages (including, but not limited to, procurement of substitute
   goods or services; loss of use, data, or profits; or business
   interruption) however caused and on any theory of liability, whether
   in contract, strict liability, or tort (including negligence or
   otherwise) arising in any way out of the use of this software, even
   if advised of the possibility of such damage.

*******************************************************************************/

#include <fitkitlib.h>

/// Vypis uzivatelskej napovedy (prikaz "help")
void print_user_help(void)
{
	term_send_str("Hra Breakout:\n");
	term_send_str("Ciel: \n");
	term_send_str("	Znicit loptickou vsetky bloky v hornej casti a nenechat pri\n");
	term_send_str("	tom lopticku narazit do spodneho okraju obrazovky. Na odrazanie\n");
	term_send_str("	sa pouziva padlo.\n");
	term_send_str("Ovladanie:\n");
	term_send_str("	4-6	pohyb padla dolava - doprava\n");
	term_send_str("	5	pauza / spustenie\n");
	term_send_str("	#	autopilot\n");
	term_send_str("Prikazy terminalu:\n");
	term_send_str("	RESTART	- Spusti novu hru\n");
}

/// Spracovanie uzivatelskych prikazov
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
	if (strcmp7(cmd_ucase, "RESTART")){
		initialize_hardware();
	}
	else{
		return (CMD_UNKNOWN);
	}
	return USER_COMMAND;
}

/// Inicializacia periferii po naprogramovani FPGA
void fpga_initialized() {}

int main(void)
{
	initialize_hardware();
	int cnt = 0;
	while (1) 
	{
		delay_ms(1);  //zpozdeni 1ms
		cnt++;
		if(cnt == 500){
			flip_led_d6(); //invertovat LED
			cnt = 0;
		}
		terminal_idle();  // obsluha terminalu
	}
}
