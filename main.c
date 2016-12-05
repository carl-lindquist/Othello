/* 
    Carl Lindquist
    Nov 24, 2016
    CE 121 Final Project
    
    Main state machine to play Reversi on the PSOC5 LP
    Game begins as a simple shell output over USBUART.
    After successful connection to a remote player, typing start will
    begin the game. Use the "wasd" keys to control the cursor position, 
    return either places the piece or passes if "p" was pressed.

*/
#include <project.h>
#include <stdio.h>


#include "ledDisplay.h"
#include "usbProtocol.h"
#include "reversiGame.h"
#include "uartProtocol.h"
#include "sdCard.h"
#include "reversiShell.h"


int main()
{
    CyGlobalIntEnable;
    
    usbStart();
    LCD_Start();
    ledDisplayStart();
    uartProtocolStart();
    sdStart();
    
   
    uint8 byte = 0;

    usbSendString((uint8*)"-- Reversi by Cral --\r");
    usbSendString((uint8*)"Type 'help' to begin.\r");
    gameState = IDLE_STATE;
    
    
    for(;;) {
        switch(gameState) {
            
            case IDLE_STATE:
                byte = usbGetByte();
                runShell(byte);
                break;
            
            case START_STATE:
                gameBoardInit(playerIpAddress, enemyIpAddress);
                if(localPlayer == RED_PLAYER) {
                    gameState = PLAYER_TURN_STATE;
                } else {
                    uartSendTxPacket(0, 0, 0, 0);
                    gameState = ENEMY_TURN_STATE;
                }
                break;
            
            case PLAYER_TURN_STATE: 
                gameCursor.enable = 1;
                
                while(!gameLocalInput(byte)) {
                    byte = usbGetByte();
                    CyDelay(2);
                }
                displayScoreLCD();
                gameState = ENEMY_TURN_STATE;
                if(gameOver) {
                    gameState = GAME_OVER_STATE; 
                }
                break;
                
                
            case ENEMY_TURN_STATE:
                gameCursor.enable = 0;
                while(!gameRemoteInput()) {
                    uartTransmitPacketAgain();
                }
                displayScoreLCD();
                gameState = PLAYER_TURN_STATE;
                if(gameOver) {
                    gameState = GAME_OVER_STATE;   
                }
                break;
                
            case GAME_OVER_STATE:
                gameCursor.enable = 0;
                for(;;) {
                    uartTransmitPacketAgain();
                    if (usbGetByte() == 'R') {
                        uartSetRxMode(RX_MODE_SETUP);
                        gameState = IDLE_STATE;
                        break;
                    }
                }
                break;
        }
    }
    
    

    
    for(;;);
} //End of main

//EOF
