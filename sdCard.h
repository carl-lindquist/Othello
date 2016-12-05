/* 
    Carl Lindquist
    Nov 29, 2016
    
    Interface for logging moves from the Reversi game 
    on the PSOC 5LP to an SD Card. 
*/
    
#ifndef SD_CARD_H
#define SD_CARD_H

#include <project.h>
    
    
/*
[desc]	Opens and overwrites a file called "Report.txt" on the optional SD Card
	
[ret]	Returns 1 for success, 0 otherwise
*/
uint8 sdStart(void);


/*
[desc]	Appends a formatted string describing a move to the file: "Report.txt" 

[id] Variable length player Id to signify the person making the move
[pFlag] Pass Flag value for the move, should be 0 or 1
[row] Row number of last move
[column] Column number of move
	
[ret]	Returns 1 for success, 0 otherwise
*/
uint8 sdWriteMove(char id[], uint8 pFlag, uint8 row, uint8 column);

    
#endif //SD_CARD_H
