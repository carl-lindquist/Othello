/*
    Carl Lindquist
    Nov 15, 2016
    UART control code for the ce 121 Final Project
    Protocol specified in lab manual

*/

#include "uartProtocol.h"
#include "usbProtocol.h"
#include <stdio.h>

#define PLAYER_ID_INDEX 2
#define SEQ_INDEX PLAYER_ID_LENGTH + 3
#define PASS_FLAG_INDEX SEQ_INDEX + 3
#define ROW_INDEX PASS_FLAG_INDEX + 1
#define COLUMN_INDEX ROW_INDEX + 2

CY_ISR_PROTO(txIsr);
CY_ISR_PROTO(rxIsr);


/*
(0x55) (0xaa) (1-8B ID) (0x20) (3B Move#)
*/

uint8 txPacketArr[19];
uint8 txCount;
uint8 rxCount;
void transferArray(uint8* source, uint8* dest, uint8 transferCount) {
    uint8 i;
    for(i=0; i < transferCount; i++) {
        dest[i] = source[i];
    }
}


void uartProtocolStart(void) {
    CyGlobalIntEnable 
    UART_Start();
    UART_Tx_Isr_StartEx(txIsr);
    UART_Rx_Isr_StartEx(rxIsr);
    txCount = 0;
    rxCount = 0;
    
    txPacketArr[0] = 0x55;
    txPacketArr[1] = 0xaa;
    
    txPacket.id[0] = 'C';
    txPacket.id[1] = 'R';
    txPacket.id[2] = 'A';
    txPacket.id[3] = 'L';
    
    txPacket.passFlag = '0';
}


void uartUpdateTxPacket(uint8 seq, uint8 passFlag, uint8 row, uint8 column) {
    transferArray(txPacket.id, &txPacketArr[PLAYER_ID_INDEX], PLAYER_ID_LENGTH);
    txPacketArr[PLAYER_ID_INDEX + PLAYER_ID_LENGTH] = 0x20;
    
    uint8 seqASCII[3];
    uint8 temp = seq;
    seqASCII[0] = (seq/100) + 0x30;
    seqASCII[1] = (seq - (seq/100)*100)/10 + 0x30;
    seqASCII[2] = seq - ((seq - (seq/100)*100)/10)*10 + 0x30;
    transferArray(seqASCII, &txPacketArr[SEQ_INDEX], 3);
    
    txPacketArr[PASS_FLAG_INDEX] = (passFlag + 0x30);
    
    txPacketArr[ROW_INDEX] = row/10 + 0x30;
    txPacketArr[ROW_INDEX + 1] = row - ((row/10)*10);
    
    txPacketArr[COLUMN_INDEX] = column/10 + 0x30;
    txPacketArr[COLUMN_INDEX + 1] = column - ((column/10)*10);   
}

void uartPrintPacket(uint8 packet[]) {
    char outString[PLAYER_ID_LENGTH + 11] = {};
    sprintf(outString, "%s", packet);
    LCD_PrintString(outString);
}

void uartPrintTxPacket(void) {
    uartPrintPacket(txPacketArr);    
}


CY_ISR(txIsr) { //FIFO EMPTY
    int i = 0;
    for(;i<4; i++) {
        UART_PutChar(txPacketArr[txCount++]);
    }
}

CY_ISR(rxIsr) { //BYTE RECEIVED
    rxPacketArr[rxCount++] = UART_GetChar();
    if(rxCount == PLAYER_ID_LENGTH + 11) {
        UART_Stop();
    }
}