-- utilities.vhd
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
use IEEE.STD_LOGIC_1164.all;

package utilities is

-- Declare functions and procedure
function revert(a: in std_logic_vector) return std_logic_vector;

end utilities;

package body utilities is

function revert(a: in std_logic_vector) return std_logic_vector is
	variable result: std_logic_vector(a'REVERSE_RANGE);
begin
	for i in a'RANGE loop
		result(i) := a(i);
	end loop;
	return result;
end;
 
end utilities;
