/*
    Carl Lindquist
    Nov 15, 2016
    UART control header for the ce 121 Final Project
    Protocol specified in lab manual

*/

#include <project.h>

#define PLAYER_ID_LENGTH 4

typedef struct {
    uint8 id[PLAYER_ID_LENGTH];
    uint8 seq[3];
    uint8 passFlag;
    uint8 row[2];
    uint8 column[2];
    
} packet;

packet txPacket;
packet rxPacket;

uint8 rxPacketArr[19];

void uartProtocolStart(void);

void uartUpdateTxPacket(uint8 seq, uint8 passFlag, uint8 row, uint8 column);

void uartPrintPacket(uint8 packet[]);

void uartPrintTxPacket(void);