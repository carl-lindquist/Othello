/*
    Carl Lindquist
    Nov 15, 2016
    CE 121 Final Project
*/

#include <project.h>
#include "reversiGame.h"

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define HOME 4

#define DRAW COLOR_WHITE

CY_ISR_PROTO(cursorIsr);

uint8 board[GAME_BOARD_SIZE][GAME_BOARD_SIZE] = {};

uint8 gameOver = 0;
uint8 playerPressedP = 0;
uint8 passesInARow = 0;


void gameBoardInit(void) {
    CYGlobalIntEnable
    Cursor_Timer_Start();
    Cursor_Timer_Isr_StartEx(cursorIsr);
    
    player = RED_PLAYER;
    gameCursor.color = player;
    gameCursor.column = 1;
    gameCursor.row = 1;
    
    redScore = 2;
    blueScore = 2;
    
    
    uint8 row, column;
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
    
    boardUpdateDisplay();
}

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

uint8 indexInBounds(uint8 row, uint8 column) {
    return (0 <= row && row < GAME_BOARD_SIZE 
            && 0 <= column && column < GAME_BOARD_SIZE);
}

//Returns total number of pieces
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
uint8 executeMove(uint8 row, uint8 column) {
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

uint8 determineWinner(void) {
    return (blueScore == redScore) ?
        DRAW : (blueScore > redScore ? BLUE_PLAYER : RED_PLAYER);
}

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

uint8 gameUserInput(uint8 input) {
    if(gameOver) {
        return 1;    
    }
    switch(input) {
        case 'w' :
            moveCursor(UP);
            playerPressedP = 0;
            break;
        
        case 'a' :
            moveCursor(LEFT);
            playerPressedP = 0;
            break;
        
        case 's' :
            moveCursor(DOWN);
            playerPressedP = 0;
            break;
        
        case 'd' :
            moveCursor(RIGHT);
            playerPressedP = 0;
            break;
            
        case 'h' :
            moveCursor(HOME);
            playerPressedP = 0;
            break;
            
        case 'p' :
            playerPressedP = 1;
            break;
            
        case '\r' :
            if (playerPressedP) {
                playerPressedP = 0;
                if(++passesInARow == 2) {
                    gameOver = 1;    
                }
                if (player == RED_PLAYER) {
                    player = BLUE_PLAYER;
                } else {
                    player = RED_PLAYER;    
                }
                break;
            }
            if (executeMove(gameCursor.row, gameCursor.column)) {
                passesInARow = 0;
                if (gameUpdateScore() == GAME_BOARD_SIZE*GAME_BOARD_SIZE) {
                    gameOver = 1;
                }
                if (player == RED_PLAYER) {
                    player = BLUE_PLAYER;
                } else {
                    player = RED_PLAYER;    
                }
            }
            //send move to remote player
            break;
    }
    if(gameOver) {
        gameOverScreen(determineWinner());    
    }
    boardUpdateDisplay();
    return 0;
}

void boardPlaceValue(uint8 value, uint8 row, uint8 column) {
    board[row][column] = value;
}

void boardUpdateDisplay(void) {
    uint8 row, column;
    for(row=0; row < GAME_BOARD_SIZE; row++) {
        for(column=0; column <GAME_BOARD_SIZE; column++) {
            ledDisplayWriteColor(board[row][column], row, column);
        }
    }
    //cursor is indexed by natural numbers
    if (!gameOver) { //turn off cursor when game is over
        ledDisplayWriteColor(gameCursor.color, gameCursor.row - 1, gameCursor.column - 1);
    }
}


CY_ISR(cursorIsr) {
    if (gameCursor.color) {
        gameCursor.color = COLOR_OFF;
    } else if (player == RED_PLAYER) {
        gameCursor.color = COLOR_RED;
    } else {
        gameCursor.color = COLOR_BLUE;
    }
    boardUpdateDisplay();
    Cursor_Timer_ReadStatusRegister(); //Clear ISR
}


//EOF