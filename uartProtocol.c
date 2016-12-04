/*
   Carl Lindquist
   Nov 15, 2016
   UART control code for the ce 121 Final Project
   Protocol specified in lab manual
*/

#include "uartProtocol.h"
#include "usbProtocol.h"
#include <stdio.h>


#define MAX_PACKET_LENGTH 19
#define ID_INDEX 2
#define BUFFER_SIZE 128
#define STRING_SIZE 64


uint8 rxMode;


CY_ISR_PROTO(txIsr);
CY_ISR_PROTO(rxIsr);
CY_ISR_PROTO(timer500msIsr);

/* 
–––––––––––  Packet Structure  –––––––––––
(0x55) (0xaa) (1-8B ID) (0x20) (3B Move#)
(pFlag) (2B row) (2B column)
*/

uint8 txPacketArr[MAX_PACKET_LENGTH + 1] = {}; // +1 for null terminating char
uint8 rxPacketArr[MAX_PACKET_LENGTH + 1] = {};
uint8 rxBuffer[BUFFER_SIZE];
uint8 txBuffer[BUFFER_SIZE];
uint8 txCount;
uint8 rxCount;
uint8 transmitAllowed;



uint8 playerPacketLength;
uint8 playerIdLength;
uint8 playerSeqIndex;
uint8 playerPFlagIndex;
uint8 playerRowIndex;
uint8 playerColumnIndex;


uint8 enemyPacketLength;
uint8 enemyIdLength;
uint8 enemySeqIndex;
uint8 enemyPFlagIndex;
uint8 enemyRowIndex;
uint8 enemyColumnIndex;


//–––––––––– Private Declarations ––––––––––//

void setIndices(uint8 playerOrEnemy);
void uartUpdateTxPacket(uint8 seq, uint8 passFlag, uint8 row, uint8 column);
uint64 stringToNumber(uint8 string[], uint8 length);
void uartPrintPacket(uint8 packet[]);
void uartPrintTxPacket(void);
void uartTestRxPacket(void);
//void uartTransmit(void);



//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void uartProtocolStart(void) {
    CyGlobalIntEnable 
    UART_Tx_Isr_StartEx(txIsr);
    UART_Rx_Isr_StartEx(rxIsr);
    Timer_500ms_Start();
    Timer_500ms_Isr_StartEx(timer500msIsr);
    txCount = 0;
    rxCount = 0;
    transmitAllowed = 0;
    uartSetRxMode(RX_MODE_SETUP);
    rxPrintAllowed = 1;
    UART_Start();
    UART_Tx_Isr_Disable();
    
    txPacketArr[0] = 0x55;
    txPacketArr[1] = 0xaa;

    playerIdLength = 0;
    playerIpAddress = 0;
    enemyIdLength = 0;
    enemyIpAddress = 0;
}

void uartLoadId(uint8 playerOrEnemy, char id[]) {
    if(playerOrEnemy == PLAYER) {
        sprintf(playerId, "%s", id);
        sprintf((char*)&txPacketArr[ID_INDEX], "%s ", id);
        setIndices(PLAYER);
    } else {
        sprintf(enemyId, "%s", id);
        setIndices(ENEMY);
    }
}

void uartStoreIpAddress(uint8 playerOrEnemy, char dottedIpAddress[]) {
    uint32 dec3, dec2, dec1, dec0;
    sscanf(dottedIpAddress, "%u.%u.%u.%u", (int*)&dec3, (int*)&dec2, (int*)&dec1, (int*)&dec0);

    if(playerOrEnemy == PLAYER) {
        playerIpAddress = (dec3 << 24) + (dec2 << 16) + (dec1 << 8) + (dec0); 
    } else {
        enemyIpAddress = (dec3 << 24) + (dec2 << 16) + (dec1 << 8) + (dec0);
    }
}

void uartSetRxMode(uint8 mode) {
    rxMode = mode;
    uartClearRxBuffer();
}

void uartSendAdvertise(char id[]) {
    char outString[30] = {};
    sprintf(outString, "advertise %s\n", id);
    uartSendString((uint8*)outString);
}

void uartSendConnect(char ipAddress[]) {
    char outString[30] = {};
    sprintf(outString, "connect %s\n", ipAddress);
    uartClearRxBuffer();
    uartSendString((uint8*)outString);
}

void uartSendDisconnect(void) {
    char outString[30] = {};
    sprintf(outString, "disconnect\n");
    uartSendString((uint8*)outString);
}


void uartSendString(uint8 string[]) {
    uint8 length = 0;
    while(string[length]) {
        txBuffer[length] = string[length];
        length++;
    }
    txBuffer[length] = '\0';
    UART_PutString((char*)txBuffer);
//    UART_Tx_Isr_Enable();
//    UART_PutChar(' ');
//    while(txCount < length){
//        
//    }
//    UART_Tx_Isr_Disable();
    txCount = 0;
}


void uartSendTxPacket(uint8 seq, uint8 passFlag, uint8 row, uint8 column) {
    sprintf((char*)&txPacketArr[playerSeqIndex], "%03d", (int)seq);
     
    txPacketArr[playerPFlagIndex] = passFlag + 0x30; 

    sprintf((char*)&txPacketArr[playerRowIndex], "%02d", row);
    sprintf((char*)&txPacketArr[playerColumnIndex], "%02d", column);
    txPacketArr[MAX_PACKET_LENGTH] = '\0';
    
    char outString[30] = {};
    sprintf(outString, "data %s \n", txPacketArr);
    uartSendString((uint8*)outString);
    usbSendString(txPacketArr);
}


void uartTransmitPacketAgain(void) {
    if(transmitAllowed) {
        char outString[30] = {};
        sprintf(outString, "data %s \n", txPacketArr);
        uartSendString((uint8*)outString);
        
        transmitAllowed = 0;
        usbSendString(txPacketArr);
    }     
}


uint8 uartValidPacketReceived(void) {
    uint8 i;
    uint8 ret = 1;
   
//    if(rxCount < MAX_PACKET_LENGTH - 1) {
//        return 0;    
//    }
    if(enemyIdLength == 0) {
        usbSendString((uint8*)"ERROR: Enemy Id Uninitialized\n");
        return 0;
    }
    if (rxBuffer[0] != 0x55 || rxBuffer[1] != 0xaa) {
        ret = 0;
    }
    for(i=0; i < enemyIdLength; i++) { 
        if(rxBuffer[i + ID_INDEX] != enemyId[i]) {
            usbSendString((uint8*)"ERROR: Bad ID:");
            usbSendString((uint8*)enemyId);
            usbSendString((uint8*)"\n");
            ret = 0;
            break;
        }
    }
    uint8 check = stringToNumber(&rxBuffer[enemySeqIndex], 3);
    if(check > 255) { // seq check
        usbSendString((uint8*)"ERROR: Bad SEQ\n");
        ret = 0;   
    }
    check = stringToNumber(&rxBuffer[enemyRowIndex], 2);
    if(check > 16) {
        usbSendString((uint8*)"ERROR: Bad Row\n"); 
        ret = 0;
    }
    check = stringToNumber(&rxBuffer[enemyColumnIndex], 2);
    if(check > 16) {
        usbSendString((uint8*)"ERROR: Bad column\n"); 
        ret = 0;
    }
    if(ret) {
//        usbSendString((uint8*)"Good Packet: ");
//        usbSendString(rxBuffer);
//        usbSendString((uint8*)"\n");
        sprintf((char*)rxPacketArr, "%s", rxBuffer);
    } else {
        usbSendString((uint8*)"Bad Packet: ");
        usbSendString(rxBuffer);
        usbSendString((uint8*)"\n");    
    }
    return ret;
}


void uartClearRxBuffer(void) {
    uint8 i = 0;  
    for(; i < BUFFER_SIZE; i++) {
        rxBuffer[i] = '\0';    
    }
}


uint8 uartParseRxPacket(void) {
    if(!uartValidPacketReceived()) {
        return 0;    
    }
    sprintf((char*)rxPacket.id, "%.*s", enemyIdLength, &rxPacketArr[ID_INDEX]);

    
    rxPacket.seq = stringToNumber(&rxPacketArr[enemySeqIndex], 3); //Length of seq string


    if(rxPacketArr[enemyPFlagIndex] == '0') {
        rxPacket.passFlag = 0;
    } else {
        rxPacket.passFlag = 1;    
    }

    rxPacket.row = stringToNumber(&rxPacketArr[enemyRowIndex], 2);
    rxPacket.column = stringToNumber(&rxPacketArr[enemyColumnIndex], 2);
    rxPacket.packetReady = 1;
    //uartClearRxBuffer();
    return 1;
}


void uartTestRxPacket(void) {
    rxPacketArr[0] = 0x55;
    rxPacketArr[1] = 0xaa;

    sprintf((char*)&rxPacketArr[ID_INDEX], "%s", "ENEMY ");
    setIndices(ENEMY);

    sprintf((char*)&rxPacketArr[enemySeqIndex], "%03d", 110);

    rxPacketArr[enemyPFlagIndex] = '0';

    sprintf((char*)&rxPacketArr[enemyRowIndex], "%02d", 14);
    sprintf((char*)&rxPacketArr[enemyColumnIndex], "%02d", 1); 
    uartPrintPacket(rxPacketArr);
}


void uartPrintRxPacket(void) {
    usbSendString((uint8*)"\rGood packet received: ");
    char outString[MAX_PACKET_LENGTH + 1] = {};
    sprintf(outString, "%s\r", rxPacketArr);
    usbSendString((uint8*)outString);
}


//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

void setIndices(uint8 playerOrEnemy) {    
    uint8 i;
    if(playerOrEnemy == PLAYER) {
        playerIdLength = 0;
        i = 0;
        while(playerId[i] != '\0' && i < 8) {
            playerIdLength++;
            i++;
        }
        
        playerSeqIndex = ID_INDEX + playerIdLength + 1;
        playerPFlagIndex = playerSeqIndex + 3;
        playerRowIndex = playerPFlagIndex + 1;
        playerColumnIndex = playerRowIndex + 2;
        playerPacketLength = MAX_PACKET_LENGTH - (8 - playerIdLength);
    } else if (playerOrEnemy == ENEMY) {
        i = 0;
        while(enemyId[i] && i < 8) {
            enemyIdLength++;
            i++;
        }

        enemySeqIndex = ID_INDEX + enemyIdLength + 1;
        enemyPFlagIndex = enemySeqIndex + 3;
        enemyRowIndex = enemyPFlagIndex + 1;
        enemyColumnIndex = enemyRowIndex + 2;
        enemyPacketLength = MAX_PACKET_LENGTH - (8 - enemyIdLength);
    } 
}


void uartPrintPacket(uint8 packet[]) {
    char outString[MAX_PACKET_LENGTH + 1] = {};
    sprintf(outString, "%s\r", packet);
    //printf("%s\n", outString);
    //LCD_PrintString(outString);
    usbSendString((uint8*)outString);
}


//Returns decimal number array to single num ("123" -> 123)
uint64 stringToNumber(uint8 string[], uint8 length) {
    uint8 i;
    uint32 multiplier = 1;
    uint64 number = 0;
    for(i=length; i > 0; i--) {
        number += multiplier * (string[i - 1] - 0x30);
        multiplier *= 10;
    }
    return number;
}

uint8 uCompareStrings(char string1[], char string2[]) {
    uint8 i = 0;
    while(string1[i] && string2[i]) {
        if(string1[i] != string2[i]) {
            return 0;    
        }
        i++;
    }
    if(string1[i] || string2[i]) {
        return 0;    
    }
    return i;
}


CY_ISR(txIsr) { //FIFO EMPTY
    uint8 i = 0;
    for(; i < 4 && txBuffer[txCount]; i++) {
        //UART_PutChar(txBuffer[txCount++]);
    }  
 }


CY_ISR(rxIsr) { //BYTE RECEIVED
    uint8 byte = UART_GetChar();
    uint8 i;
    char str1[STRING_SIZE], str2[STRING_SIZE], str3[STRING_SIZE];
    switch (rxMode) {
        
        case RX_MODE_SETUP:
            if(rxCount == BUFFER_SIZE) {
                rxBuffer[BUFFER_SIZE - 1] = '\0';
                if(rxPrintAllowed) {
                    usbSendString(rxBuffer);
                }
                rxCount = 0;    
                rxBuffer[0] = '\0';
                break;
            } else if (byte) {
                rxBuffer[rxCount++] = byte;  
            } 
            if(byte == '\n') {
                rxBuffer[rxCount] = '\0'; //set char after newline to null
                if(rxPrintAllowed) {
                    usbSendString(rxBuffer);
                }
                if(!playerIpAddress) {
                     sscanf((char*)rxBuffer, "%s %s %s", str1, str2, str3);
                     if(uCompareStrings(str2, "Address:")) {
                        uartStoreIpAddress(PLAYER, str3);
                    }
                } else if(!enemyIpAddress) {
                    sscanf((char*)rxBuffer, "%s %s %s", str1, str2, str3);
                    if(uCompareStrings(str1, "Connected")) {
                         uartStoreIpAddress(ENEMY, str3);
                    }
                } 
                rxCount = 0;
                uartClearRxBuffer();
            }
            break;
        
        case RX_MODE_GAMEPLAY:
            if(byte == 0xaa && rxCount != 1) {
                rxBuffer[0] = 0x55;
                rxBuffer[1] = 0xaa;
                rxCount = 2;
            } else if(byte) {
                if(!enemyIdLength && byte == ' ') {
                    //sscanf((char*)&rxBuffer[ID_INDEX], "%s", str1);
                    for(i=ID_INDEX; rxBuffer[i] != ' ' && i < ID_INDEX + 8; i++) {
                        str1[i - ID_INDEX] = rxBuffer[i];  
                    }
                    str1[i] = '\0';
                    uartLoadId(ENEMY, str1);
                }
                if(rxCount < BUFFER_SIZE) {
                    rxBuffer[rxCount] = byte;
                    rxCount++;
                }
            }
            break;
            
        default:
            break;
    }
}


CY_ISR(timer500msIsr) {
    transmitAllowed = 1;
    Timer_500ms_ReadStatusRegister();
}

//EOF