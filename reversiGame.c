  /*
    Carl Lindquist
    Nov 15, 2016
    
    Control code to play Reversi on the PSOC5 LP.
*/


#include <project.h>
#include "reversiGame.h"
#include "uartProtocol.h"
#include "usbProtocol.h"
#include "sdCard.h"
#include <stdio.h>

#define DRAW COLOR_WHITE

enum directions {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    HOME,
};


uint8 board[GAME_BOARD_SIZE][GAME_BOARD_SIZE] = {};

uint8 localPlayerPressedP;
uint8 passesInARow;
uint8 localTurnCount;
uint8 remoteTurnCount;
uint8 redScore;
uint8 blueScore;


//––––––  Private Declarations  ––––––//

void boardUpdateDisplay(void);
void boardPlaceValue(uint8 value, uint8 row, uint8 column);
uint8 gameUpdateScore();
void moveCursor(uint8 direction);
uint8 indexInBounds(uint8 row, uint8 column);
uint8 executeMove(uint8 row, uint8 column, uint8 player);
uint8 determineWinner(void);
void gameOverScreen(uint8 winner);
void boardPlaceValue(uint8 value, uint8 row, uint8 column);

CY_ISR_PROTO(cursorIsr);


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void gameBoardInit(uint32 localIpAddress, uint32 remoteIpAddress) {
    CYGlobalIntEnable
    Cursor_Timer_Start();
    Cursor_Timer_Isr_StartEx(cursorIsr);

    
    if(localIpAddress < remoteIpAddress) {
        localPlayer = RED_PLAYER;    
    } else {
        localPlayer = BLUE_PLAYER;    
    }

    
    gameOver = 0;
    localTurnCount = 0;
    remoteTurnCount = 0;
    gameCursor.enable = 1;
    gameCursor.color = localPlayer;
    gameCursor.column = 1;
    gameCursor.row = 1;
    localPlayerPressedP = 0;
    passesInARow = 0;
    
    redScore = 2;
    blueScore = 2;
    
    uint8 row, column;
    ledDisplayClearDisplay();
    for(row=0; row < GAME_BOARD_SIZE; row++) {
        for(column=0; column < GAME_BOARD_SIZE; column++) {
            ledDisplayWriteColor(COLOR_OFF, row, column);
        }
    }
    //Load border
    for(row=GAME_BOARD_SIZE, column=0; column <= GAME_BOARD_SIZE; column++) {
        ledDisplayWriteColor(COLOR_PURPLE, row, column);
    }
    for(row=0, column=GAME_BOARD_SIZE; row <= GAME_BOARD_SIZE; row++) {
        ledDisplayWriteColor(COLOR_PURPLE, row, column);
    }

    row = GAME_BOARD_SIZE / 2;
    column = GAME_BOARD_SIZE / 2;
    //The following place the middle pieces, very unreadable
    boardPlaceValue(BOARD_VALUE_BLUE, row--, column--);
    boardPlaceValue(BOARD_VALUE_BLUE, row++, column);
    boardPlaceValue(BOARD_VALUE_RED, row--, column++);
    boardPlaceValue(BOARD_VALUE_RED, row, column);
    
    uartSetRxMode(RX_MODE_GAMEPLAY);
    
    boardUpdateDisplay();
}

uint8 gameLocalInput(uint8 input) {
    uint8 ret = 0;
    if(gameOver) {
        return ret;    
    }
    switch(input) {
        case 'w' :
            moveCursor(UP);
            localPlayerPressedP = 0;
            break;
        
        case 'a' :
            moveCursor(LEFT);
            localPlayerPressedP = 0;
            break;
        
        case 's' :
            moveCursor(DOWN);
            localPlayerPressedP = 0;
            break;
        
        case 'd' :
            moveCursor(RIGHT);
            localPlayerPressedP = 0;
            break;
            
        case 'h' :
            moveCursor(HOME);
            localPlayerPressedP = 0;
            break;
            
        case 'p' :
            localPlayerPressedP = 1;
            break;
            
        case '\r' :
            if(localPlayerPressedP) {
                localPlayerPressedP = 0;
                passesInARow++;
                localTurnCount++;
                uartSendTxPacket(localTurnCount, 1, gameCursor.row, gameCursor.column);
                sdWriteMove((char*)playerId, 1, gameCursor.row, gameCursor.column);
                ret = 1;
                break; //dont execute move if player passed
            }
            if (executeMove(gameCursor.row, gameCursor.column, localPlayer)) {
                passesInARow = 0;
                localTurnCount++;
                uartSendTxPacket(localTurnCount, 0, gameCursor.row, gameCursor.column);
                sdWriteMove((char*)playerId, 0, gameCursor.row, gameCursor.column);
                ret = 1;
            }
            break;
    }
    if(gameUpdateScore() == (uint8)(GAME_BOARD_SIZE*GAME_BOARD_SIZE) || passesInARow == 2) {
        gameOver = 1;
        gameCursor.enable = 0;
        gameOverScreen(determineWinner()); 
        uartSendTxPacket(localTurnCount, 1, rxPacket.row, rxPacket.column);
    }
    boardUpdateDisplay();
    return ret;
}

uint8 gameRemoteInput(void) {
    uint8 ret = 0;
    uint8 enemy = RED_PLAYER;
    if(gameOver || !uartParseRxPacket()) {
        return ret; 
    }
    
    if (rxPacket.seq == remoteTurnCount + 1) {
        if(rxPacket.passFlag) {
            passesInARow++;
            uartPrintRxPacket();
            sdWriteMove((char*)enemyId, 1, rxPacket.row, rxPacket. column);
            remoteTurnCount++;
            ret = 1;
        } else {
            if(localPlayer == RED_PLAYER) {
                enemy = BLUE_PLAYER;
            }
            if(executeMove(rxPacket.row, rxPacket.column, enemy)) {
                uartPrintRxPacket();
                sdWriteMove((char*)enemyId, 0, rxPacket.row, rxPacket. column);
                remoteTurnCount++;
                passesInARow = 0;
                ret = 1;
            }
        }
    }
    if(passesInARow == 2 || gameUpdateScore() == (uint8)(GAME_BOARD_SIZE*GAME_BOARD_SIZE)) {
        gameOver = 1;
        gameOverScreen(determineWinner()); 
    }
    boardUpdateDisplay();
    return ret;
}

void displayScoreLCD(void) {
    LCD_ClearDisplay();
    char outString[16] = {};
    sprintf(outString, "Red: %d", redScore);
    LCD_PrintString(outString);
    LCD_Position(1,0);
    outString[0] = '\0';
    sprintf(outString, "Blue: %d", blueScore);
    LCD_PrintString(outString);
}

void swapLocalPlayer(void) {
    if (localPlayer == RED_PLAYER) {
         localPlayer = BLUE_PLAYER;
    } else {
        localPlayer = RED_PLAYER;    
    }
}


//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

//Moves cursor up, down, left, right, or home depending on input
void moveCursor(uint8 direction) {
    if (direction == UP) {
        if(gameCursor.row == 1) {
            gameCursor.row = GAME_BOARD_SIZE;
        } else {
            gameCursor.row--;   
        }
    } else if (direction == DOWN) {
        if(gameCursor.row == GAME_BOARD_SIZE) {
            gameCursor.row = 1;
        } else {
            gameCursor.row++;   
        }
    } else if (direction == LEFT) {
        if(gameCursor.column == 1) {
            gameCursor.column = GAME_BOARD_SIZE;
        } else {
            gameCursor.column--;   
        }
    } else if (direction == RIGHT) {
        if(gameCursor.column == GAME_BOARD_SIZE) {
            gameCursor.column = 1;
        } else {
            gameCursor.column++;   
        }
    } else {
        gameCursor.row = 1;
        gameCursor.column = 1;
    }
}

//Returns 1 if index is in bounds, given a row and column
uint8 indexInBounds(uint8 row, uint8 column) {
    return (0 <= row && row < GAME_BOARD_SIZE 
            && 0 <= column && column < GAME_BOARD_SIZE);
}

//Updates global score counters and returns total number of pieces
uint8 gameUpdateScore() {
    uint8 row, column;
    blueScore = 0;
    redScore = 0;
    for(row=0; row < GAME_BOARD_SIZE; row++) {
        for(column=0; column < GAME_BOARD_SIZE; column++) {
            if(board[row][column] == BOARD_VALUE_RED) {
                redScore++;    
            } else if(board[row][column] == BOARD_VALUE_BLUE) {
                blueScore++;    
            }
        }
    }
    return (redScore + blueScore);
}

//returns number of successful directions for success, 0 for failure
uint8 executeMove(uint8 row, uint8 column, uint8 player) {
    uint8 success = 0;
    row--; //change to zero indexing
    column--;
    if(board[row][column] != BOARD_VALUE_EMPTY) {
        return success; //Exit if the move is on another tile
    }
    //Discern enemy color
    uint8 enemy = RED_PLAYER;
    if(player == RED_PLAYER) {
        enemy = BLUE_PLAYER;
    }
    
    int i,j;
    
    //upleft diagonal, minus 1 each for upleft
    i=row-1;
    j=column-1;
    if (indexInBounds(i, j) && board[i][j] == enemy) { //check adjacent
        for(; indexInBounds(i, j); i--, j--) { //continue diagonal
            if (board[i][j] == player) {
                //go back and flip inbetween pieces
                for(; i <= row && j <= column; i++, j++) {
                    board[i][j] = player;
                }
                success++;
                break;
            } else if (board[i][j] == BOARD_VALUE_EMPTY) {
                break;    
            }
        }
    }
    
    //upwards line
    i=row-1; 
    j=column;
    if (indexInBounds(i, j) && board[i][j] == enemy) { //check adjacent
        for(; indexInBounds(i, j); i--) { //continue line
            if (board[i][j] == player) {
                //go back and flip inbetween pieces
                for(; i <= row; i++) {
                    board[i][j] = player;
                }
                success++;
                break;
            } else if (board[i][j] == BOARD_VALUE_EMPTY) {
                break;    
            }
        }
    }
    
    //upright diagonal
    i=row-1;
    j=column+1;
    if (indexInBounds(i, j) && board[i][j] == enemy) { //check adjacent
        for(; indexInBounds(i, j); i--, j++) { //continue diagonal
            if (board[i][j] == player) {
                //go back and flip inbetween pieces
                for(; i <= row && j >= column; i++, j--) {
                    board[i][j] = player;
                }
                success++;
                break;
            } else if (board[i][j] == BOARD_VALUE_EMPTY) {
                break;    
            }
        }
    }
    
    //right line
    i=row;
    j=column+1;
    if (indexInBounds(i, j) && board[i][j] == enemy) { //check adjacent
        for(; indexInBounds(i, j); j++) { //continue line
            if (board[i][j] == player) {
                //go back and flip inbetween pieces
                for(; j >= column; j--) {
                    board[i][j] = player;
                }
                success++;
                break;
            } else if (board[i][j] == BOARD_VALUE_EMPTY) {
                break;    
            }
        }
    }
    
    //downright diagonal
    i=row+1;
    j=column+1;
    if (indexInBounds(i, j) && board[i][j] == enemy) { //check adjacent
        for(; indexInBounds(i, j); i++, j++) { //continue diagonal
            if (board[i][j] == player) {
                //go back and flip inbetween pieces
                for(; i >= row && j >= column; i--, j--) {
                    board[i][j] = player;
                }
                success++;
                break;
            } else if (board[i][j] == BOARD_VALUE_EMPTY) {
                break;    
            }
        }
    }
    
    //down line
    i=row+1; 
    j=column;
    if (indexInBounds(i, j) && board[i][j] == enemy) { //check adjacent
        for(; indexInBounds(i, j); i++) { //continue line
            if (board[i][j] == player) {
                //go back and flip inbetween pieces
                for(; i >= row; i--) {
                    board[i][j] = player;
                }
                success++;
                break;
            } else if (board[i][j] == BOARD_VALUE_EMPTY) {
                break;    
            }
        }
    }
    
    //downleft diagonal
    i=row+1;
    j=column-1;
    if (indexInBounds(i, j) && board[i][j] == enemy) { //check adjacent
        for(; indexInBounds(i, j); i++, j--) { //continue diagonal
            if (board[i][j] == player) {
                //go back and flip inbetween pieces
                for(; i >= row && j <= column; i--, j++) {
                    board[i][j] = player;
                }
                success++;
                break;
            } else if (board[i][j] == BOARD_VALUE_EMPTY) {
                break;    
            }
        }
    }
    
    //Left line
    i=row;
    j=column-1;
    if (indexInBounds(i, j) && board[i][j] == enemy) { //check adjacent
        for(; indexInBounds(i, j); j--) { //continue diagonal
            if (board[i][j] == player) {
                //go back and flip inbetween pieces
                for(; j <= column; j++) {
                    board[i][j] = player;
                }
                success++;
                break;
            } else if (board[i][j] == BOARD_VALUE_EMPTY) {
                break;    
            }
        }
    }

    return success;
}

//Returns the player with the most pieces
uint8 determineWinner(void) {
    return (blueScore == redScore) ?
        DRAW : (blueScore > redScore ? BLUE_PLAYER : RED_PLAYER);
}

//Loads indicator for victor into the display
void gameOverScreen(uint8 winner) {
    uint8 row, column;
    uint8 rArray[6][3] = {
        {1, 1, 0},
        {1, 0, 1},
        {1, 1, 0},
        {1, 0, 1},
        {1, 0, 1},
        {1, 0, 1}};
    
    uint8 bArray[6][3] = {
        {1, 1, 0},
        {1, 0, 1},
        {1, 1, 0},
        {1, 0, 1},
        {1, 0, 1},
        {1, 1, 0}};
    
    uint8 dArray[6][3] = {
        {1, 1, 0},
        {1, 0, 1},
        {1, 0, 1},
        {1, 0, 1},
        {1, 0, 1},
        {1, 1, 0}};
            
    for(row = 0; row < 6; row++) {
        for(column=0; column < 3; column++) {
            if(winner == RED_PLAYER && rArray[row][column]) {
                ledDisplayWriteColor(winner, row + 5, column + 22);
            } else if(winner == BLUE_PLAYER && bArray[row][column]) {
                ledDisplayWriteColor(winner, row + 5, column + 22);
            } else if(winner == DRAW && dArray[row][column]) {
                ledDisplayWriteColor(winner, row + 5, column + 22);
            }
        }
    }
}

//Places a piece into the board array
void boardPlaceValue(uint8 value, uint8 row, uint8 column) {
    board[row][column] = value;
}

//Loads the gameboard and cursor into the ledDisplay
void boardUpdateDisplay(void) {
    uint8 row, column;
    for(row=0; row < GAME_BOARD_SIZE; row++) {
        for(column=0; column <GAME_BOARD_SIZE; column++) {
            ledDisplayWriteColor(board[row][column], row, column);
        }
    }
    //cursor is indexed by natural numbers
    if (gameCursor.enable) { 
        ledDisplayWriteColor(gameCursor.color, gameCursor.row - 1, gameCursor.column - 1);
    }
}

//Oscillates cursor every 500ms
CY_ISR(cursorIsr) {
    if (gameCursor.color) {
        gameCursor.color = COLOR_OFF;
    } else if (localPlayer == RED_PLAYER) {
        gameCursor.color = COLOR_RED;
    } else if (localPlayer == BLUE_PLAYER) {
        gameCursor.color = COLOR_BLUE;
    }
    boardUpdateDisplay();
    Cursor_Timer_ReadStatusRegister(); //Clear ISR
}

//EOF