/*
    Carl Lindquist
    Nov 15, 2016
    UART control header for the ce 121 Final Project
    Protocol specified in lab manual

    USAGE:
    	Call uartProtocolStart()
        Advertise and connect to a remote player
        Change to RX_MODE_GAMEPLAY
        
		Continuously poll uartParseRxPacket()
            Execute move in rxPacket struct
        uartSendTxPacket() when appropraite
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
} packet;

packet rxPacket;

//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

/*
[desc]  Initializes UART comms. Sets rxMode to RX_MODE_SETUP.
*/
void uartProtocolStart(void);


/*
[desc]  Loads an ID for either player or enemy. Both calls are required for
        successful packet parsing and sending.

[playerOrEnemy] One of two macros deciding where to load the id.
[id] String holding the PlayerId to be loaded.
*/
void uartLoadId(uint8 playerOrEnemy, char id[]);

/*
[desc]  Stores an Ip Address for either player or enemy. Both calls are required 
        to start reversi correctly, as the Ip Addresses decide which player goes
        first.

[playerOrEnemy] One of two macros deciding where to store the Ip Address.
[id] String holding the "dotted decimal" Ip Address to be loaded.
*/
void uartStoreIpAddress(uint8 playerOrEnemy, char dottedIpAddress[]);


/*
[desc]  Sends the advertise command over UART for the WIFI card to broadcast.
        User is required to call this function prior to sending any txPackets.

[id] String holding the PlayerId to be broadcast.
*/
void uartSendAdvertise(char id[]);


/*
[desc]  Sends the connect command over UART for the WIFI card
        to connect two players. 

[id] String holding the "dotted decimal" Ip Address to connect to.
*/
void uartSendConnect(char ipAddress[]);


/*
[desc]  Sends the disconnect command over UART for the WIFI card
        to disconnect two players. 
*/
void uartSendDisconnect(void);


/*
[desc]  Sends a txPacket formatted to lab manual specification
        to the WIFI card. Two players must already be connected
        prior to calling this function.

[id] String holding the "dotted decimal" Ip Address to connect to.
*/
void uartSendTxPacket(uint8 seq, uint8 passFlag, uint8 row, uint8 column);


/*
[desc]  Retransmits the last packet loaded by uartSendTxPacket().
*/
void uartTransmitPacketAgain(void);


/*
[desc]  Changes the rxMode between RX_MODE_SETUP and RX_MODE_GAMEPLAY.
*/
void uartSetRxMode(uint8 mode);


/*
[desc]  Loads data received by UART into the rxPacket struct. 
        This function should be called constantly to catch any incoming 
        enemy packets when in RX_MODE_GAMEPLAY.

[ret]   1 if a packet was successfully parsed, 0 otherwise
*/
uint8 uartParseRxPacket(void);

/*
[desc]  Empties the RxBuffer. This function should be called after a move
        has been successfully executed.
*/
void uartClearRxBuffer(void);

/*
[desc]  Prints the latest RxPacket received to the USBUART
*/
void uartPrintRxPacket(void);

#endif //UART_PROTOCOL_H
