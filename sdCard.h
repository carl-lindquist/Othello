/* 
    Carl Lindquist
    Nov 29, 2016
    
    Interface for logging moves from the Reversi game 
    on the PSOC 5LP to an SD Card
*/
    
#ifndef SD_CARD_H
#define SD_CARD_H

#include <project.h>
    
static const char sdVolName[] = "FILLMEUP";
    
//Returns 1 for successful init, 0 otherwise
uint8 sdStart(void);

//Returns 1 for successful write, 0 otherwise
uint8 sdWriteMove(char id[], uint8 pFlag, uint8 row, uint8 column);

//Returns 1 for successful close, 0 otherwise
uint8 sdClose(void);

    
#endif
