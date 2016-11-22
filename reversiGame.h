/* ========================================
 *
 * Carl Lindquist
 * Nov 15, 2016
 * CE 121
 * Final Project Game Logic
 * Based on the game Reversi/Othello
 * 
 * 
 * ========================================
*/
#ifndef _REVERSI_GAME_H_
#define _REVERSI_GAME_H_
    
//#include <project.h>
#include "ledDisplay.h"
    
//Change this value to either 4, 8, or 16
#define GAME_BOARD_SIZE 4
    

#define BOARD_VALUE_EMPTY 0
#define BOARD_VALUE_RED COLOR_RED
#define BOARD_VALUE_BLUE COLOR_BLUE
    
#define RED_PLAYER BOARD_VALUE_RED
#define BLUE_PLAYER BOARD_VALUE_BLUE


struct cursor {
    uint8 color;
    uint8 row;
    uint8 column;
} gameCursor;

uint8 player;
uint8 redScore;
uint8 blueScore;
    

void gameBoardInit(void);

uint8 gameUserInput(uint8 input);

uint8 gameUpdateScore();
    
void boardPlaceValue(uint8 value, uint8 row, uint8 column);

void boardUpdateDisplay(void);

    
    
#endif //_REVERSI_GAME_H_
//EOF
