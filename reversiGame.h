/* 
    Carl Lindquist
    Nov 15, 2016
    
    Interface to play Reversi on the PSOC5 LP. Relies on state machine
    described in main.c

*/

#ifndef REVERSI_GAME_H
#define REVERSI_GAME_H
    
#include <project.h>
#include "ledDisplay.h"
    
//Change this value to either 4, 8, or 16
#define GAME_BOARD_SIZE 16
    

#define BOARD_VALUE_EMPTY COLOR_OFF
#define BOARD_VALUE_RED COLOR_RED
#define BOARD_VALUE_BLUE COLOR_BLUE
    
#define RED_PLAYER BOARD_VALUE_RED
#define BLUE_PLAYER BOARD_VALUE_BLUE

enum gamestates {
    IDLE_STATE,
    START_STATE,
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
uint8 gameOver;


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Initializes the gameboard. Changes the UART mode to parse packets according
        to game specification. 
        
[args]  If [arg1] is lower than [arg2], then the local player goes first.
*/
void gameBoardInit(uint32 localIpAddress, uint32 remoteIpAddress);


/*
[desc]  Analyzes a byte recieved (over USBUART) and uses it to play the game.
        
[input] A valid byte to be processed:
            'w'     move cursor up
            'a'     move cursor left
            's'     move cursor down
            'd'     move cursor right
            'h'     move cursor home
            'p'     stores input, if \r is pressed next player passes
            '\r'    
        
[ret]   1 if a local move was executed, 0 otherwise.
*/
uint8 gameLocalInput(uint8 input);


/*
[desc]  This function should be constantly called while expecting
        an enemy move to be received. 
        
[ret]   1 if a remote move was received and executed, 0 otherwise.
*/
uint8 gameRemoteInput(void);

/*
[desc]  Updates the LCD display with the current game score
*/
void displayScoreLCD(void);

    
/*
[desc]  Swaps the color of the localPlayer variable. 
        Used when playing a local game.
*/
void swapLocalPlayer(void);

    
#endif //REVERSI_GAME_H;
