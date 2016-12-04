/* 
    Carl Lindquist
    Dec 1, 2016
    
    A shell for communication between the PSOC 5LP
    via USBUART for Reversi.
*/
    
#include "reversiShell.h"
#include "reversiGame.h"
#include "usbProtocol.h"
#include "uartProtocol.h"

#include <stdio.h>

#define SHELL_BUFFER_SIZE 64
#define COMMAND_LENGTH 16
#define ARGUMENT_LENGTH 16

uint8 bufferCount = 0;
char buffer[SHELL_BUFFER_SIZE];

//–––––– Private Declarations ––––––//

void runCommand(void);
uint8 compareStrings(char string1[], char string2[]);
uint8 determineCommand(char command[]);

//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void runShell(uint8 byte) {
    if(byte) {
        if((char)byte == '\r') {
            rxPrintAllowed = 1;
            buffer[bufferCount] = '\0';
            runCommand();
            buffer[0] = '\0';
            bufferCount = 0;
        } else {
            uartClearRxBuffer();
            if(bufferCount == 0) {
                usbSendByte('\n');
            }
            buffer[bufferCount++] = byte;
            rxPrintAllowed = 0;
        }
        usbSendByte(byte);
    }
}

    
//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

enum commands {
    ADVERTISE = 1,
    CONNECT,
    DISCONNECT,
    STATUS,
    START,
    HELP,
    HELLO,
};

void runCommand(void) {
    char command[COMMAND_LENGTH] = {};
    char argument[ARGUMENT_LENGTH] = {};
    
    if (sscanf(buffer, "%s", command)) {
        switch (determineCommand(command)) {
            
            case ADVERTISE:
                usbSendString((uint8*)"\r   Advertise command sent.");
                sscanf(buffer, "%*s%s", argument);
                uartSendAdvertise(argument);
                uartLoadId(PLAYER, argument);
                break;
                
            case CONNECT:
                usbSendString((uint8*)"\r   Connect command sent.");
                sscanf(buffer, "%*s%s", argument);
                uartSendConnect(argument);
                break;
                
            case DISCONNECT:
                usbSendString((uint8*)"\r   Disconnect command sent.");
                enemyIpAddress = 0;
                uartSendDisconnect();
                break;
                
            case STATUS:
                usbSendString((uint8*)"\r   Status:");
                usbSendString((uint8*)"\r       Player Ip Address: ");
                if(playerIpAddress) {
                    char outString[16];
                    sprintf(outString, "%lu", (uint32)playerIpAddress);
                    usbSendString((uint8*)outString);
                } else {
                    usbSendString((uint8*)"not found");
                }
                usbSendString((uint8*)"\r       Enemy Ip Address: ");
                if(enemyIpAddress) {
                    char outString[16];
                    sprintf(outString, "%lu", (uint32)enemyIpAddress);
                    usbSendString((uint8*)outString);
                } else {
                    usbSendString((uint8*)"not found");
                }
                break;
                
            case START:
                usbSendString((uint8*)"\r   Play!");
                gameState = START_STATE;
                break;
                
            case HELP:
                usbSendString((uint8*)"\r\rUsage: advertise connect start.\r");
                break;
                
            case HELLO:
                usbSendString((uint8*)"\r   Hey there :)");
                break;
                
            default:
                break;
        } 
    }
}

uint8 determineCommand(char command[]) {
    if(compareStrings(command, "hello")) {
        return HELLO;    
    } else if(compareStrings(command, "start")) {
        return START;
    } else if(compareStrings(command, "advertise") ||compareStrings(command, "a")) {
        return ADVERTISE;
    } else if(compareStrings(command, "help")) {
        return HELP;
    } else if(compareStrings(command, "status")) {
        return STATUS;
    } else if(compareStrings(command, "connect")) {
        return CONNECT;
    } else if(compareStrings(command, "disconnect")) {
        return DISCONNECT;
    } else {
        return 0;
    }
}

uint8 compareStrings(char string1[], char string2[]) {
    uint8 string1Length;
    uint8 string2Length;
    for(string1Length=0; string1[string1Length]; string1Length++);
    for(string2Length=0; string2[string2Length]; string2Length++);
    if(string1Length != string2Length) {
        return 0;    
    }
    uint8 i = 0;
    while(i < string1Length) {
        if(string1[i] != string2[i]) {
            return 0;    
        }
        i++;
    }
    return i;
}

//EOF