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
#define GAME_BOARD_SIZE 16
    

#define BOARD_VALUE_EMPTY 0
#define BOARD_VALUE_RED COLOR_RED
#define BOARD_VALUE_BLUE COLOR_BLUE
    
#define RED_PLAYER BOARD_VALUE_RED
#define BLUE_PLAYER BOARD_VALUE_BLUE

enum gamestates {
    IDLE_STATE,
    START_STATE,
    //LOOK_FOR_OPPONENT_STATE, 
    //DECIDE_TURN_STATE,
    PLAYER_TURN_STATE,
    ENEMY_TURN_STATE,
    GAME_OVER_STATE
} gameState;

struct cursor {
    uint8 color;
    uint8 row;
    uint8 column;
    uint8 enable;
} gameCursor;

uint8 localPlayer;
uint8 redScore;
uint8 blueScore;

uint8 gameOver;
    

void gameBoardInit(uint32 localIpAddress, uint32 remoteIpAddress);

uint8 gameLocalInput(uint8 input);

uint8 gameRemoteInput(void);

uint8 gameUpdateScore();
    
//void boardPlaceValue(uint8 value, uint8 row, uint8 column);

//void boardUpdateDisplay(void);

//––––––––––––––––––––––––––––––  Test Functions  ––––––––––––––––––––––––––––––//

void swapLocalPlayer(void);

    
    
#endif //_REVERSI_GAME_H_
//EOF
