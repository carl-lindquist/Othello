/*
    Carl Lindquist
    Nov 15, 2016
    UART control header for the ce 121 Final Project
    Protocol specified in lab manual

    USAGE:
    	Start by calling uartProtocolStart().
		Continuously poll uartValidateNewestPacket(), after
			a successful return uartUpdateTxPacket() and
			uartParseRxPacket() can be called.
*/
            
#ifndef UART_PROTOCOL_H
#define UART_PROTOCOL_H

    
#include <project.h>

    
#define MAX_ID_LENGTH 8
 
#define PLAYER 1
#define ENEMY 2

    
char playerId[MAX_ID_LENGTH + 1];
uint64 playerIpAddress;

char enemyId[MAX_ID_LENGTH + 1];
uint64 enemyIpAddress;


enum rxmodes {
    RX_MODE_SETUP,
    RX_MODE_GAMEPLAY,
};

uint8 rxPrintAllowed;

typedef struct {
    uint8 id[9];
    uint8 seq;
    uint8 passFlag;
    uint8 row;
    uint8 column;
    uint8 packetReady;
} packet;

// packet txPacket;
packet rxPacket;


//Initializes txPacket and UART comms, does not init enemyIndices
void uartProtocolStart(void);

//Load Id into player or enemy
void uartLoadId(uint8 playerOrEnemy, char id[]);

//Store ipAddress
void uartStoreIpAddress(uint8 playerOrEnemy, char dottedIpAddress[]);

//Sends the advertise command over UART, given a player id to braodcast
void uartSendAdvertise(char id[]);

//Sends the connect command over UART, given an ipAddress to connect to
void uartSendConnect(char ipAddress[]);

//Sends the disconnect command
void uartSendDisconnect(void);

//Sends a string over UART. Length < 129
void uartSendString(uint8 string[]);

//Loads data passed into txBuffer to be sent by UART
void uartSendTxPacket(uint8 seq, uint8 passFlag, uint8 row, uint8 column);

//Retransmits whatever was last sent by UART
void uartTransmitPacketAgain(void);

//Checks if a valid rxPacket is ready to be parsed
uint8 uartValidPacketReceived(void);

//Modifies Rx mode according to enum above
void uartSetRxMode(uint8 mode);

//Loads data received by UART into the rxPacket struct
//Returns 1 for success, 0 for parsing failure
uint8 uartParseRxPacket(void);


void uartClearRxBuffer(void);


//Loads arbitrary data into the rxPacket buffer to test parsing
void uartTestRxPacket(void);


void uartPrintRxPacket(void);

#endif