/* 
    Carl Lindquist
    Dec 1, 2016
    
    A shell for communication between the PSOC 5LP
    via USBUART for Reversi. Main should constantly send
    input bytes from the USBUART to runShell(),
    which will modify game values depending on input.
*/
    
#ifndef SHELL_H
#define SHELL_H

#include <project.h>


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Input bytes are buffered until '\r' received or buffer is full. Upon execute,
		shell checks buffer for a valid command.

		Valid commands:
			advertise [playerId]
    		connect [ipAddress in dotted deimal notation] 
    		disconnect
	    	status
	    	start
	    	help
	    	hello
        
[byte]	Bytes to be buffered, '\r' executes buffer.
*/
void runShell(uint8 byte);
    
    
#endif //SHELL_H
