/* 
    Carl Lindquist
    Nov 24, 2016
    CE 121 Final Project
    
    Main control code to run reversi
    for the PSOC 5LP
*/
#include <project.h>
#include <stdio.h>


#include "ledDisplay.h"
#include "usbProtocol.h"
#include "reversiGame.h"
#include "uartProtocol.h"
#include "sdCard.h"
#include "reversiShell.h"




void displayScoreLCD(void);


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
    usbSendString((uint8*)"Type 'help' to begin.\r\r");
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

//EOF
