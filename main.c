/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include <stdio.h>

//#include "reversiGame.h"
#include "ledDisplay.h"
#include "usbProtocol.h"
#include "reversiGame.h"
#include "uartProtocol.h"


int main()
{
    CyGlobalIntEnable;
    
    usbStart();
    LCD_Start();
    ledDisplayStart();
    gameBoardInit();
    uartProtocolStart();
    
    uartUpdateTxPacket(123, 1, 3, 15);
    CyDelay(100);
    uartPrintTxPacket();
    
    

    uint8 byte = 0;
    char outString[16] = {};
    for(;;) {
        byte = usbGetByte();
        if (byte) {
            gameUserInput(byte);
            if (byte == '\r') {
                LCD_ClearDisplay();
                outString[0] = '\0';
                sprintf(outString, "Red: %d", redScore);
                LCD_PrintString(outString);
                LCD_Position(1,0);
                outString[0] = '\0';
                sprintf(outString, "Blue: %d", blueScore);
                LCD_PrintString(outString);
            }
        }
    }
    
    
} //End of main


/* [] END OF FILE */
