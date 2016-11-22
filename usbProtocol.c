/* ========================================
 *
 * Carl Lindquist
 * USB Communication Protocol
 *
 * ========================================
*/


#include <project.h>
#include "usbProtocol.h"

void usbStart(void) {
    CyGlobalIntEnable; /* Enable global interrupts. */
    USBUART_Start(0u, USBUART_5V_OPERATION);
    while (!USBUART_GetConfiguration()){};
    USBUART_CDC_Init(); 
}

uint8 usbGetByte(void) {
    uint8 byte = 0;
    if (USBUART_DataIsReady()) {
        USBUART_GetAll(&byte);
        while (!USBUART_CDCIsReady()); //Wait until USB is ready to send
        //USBUART_PutData(&byte, 1); //ECHO byte to host
        return byte;
    }
    return '\0';
}


#ifdef ECHO_CODE
#define BUFFER_SIZE 64
#define BUFFER_ZERO 0
#define BUFFER_ONE 1

uint8 currentBuffer = 0;

uint8 RxBuffer0[BUFFER_SIZE] = {};
uint8 RxBuffer1[BUFFER_SIZE] = {};

void transferToCurrentBuffer(uint8 src[], uint16 count, uint8 index);

void switchBuffers(void);


int main()
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    uint16 bytesReceivedCount = 0;
    uint16 bufferCount = 0;

    
    
    USBUART_Start(0u, USBUART_3V_OPERATION);
    while (!USBUART_GetConfiguration()){};
    USBUART_CDC_Init();
    

    uint8 tempBuffer[BUFFER_SIZE] = {};

    uint8 bytesBeforeOverflow = 0;

    for(;;) {
        if (USBUART_DataIsReady()) {
            /* Read received data and re-enable OUT endpoint. */  
            bytesReceivedCount = USBUART_GetAll(tempBuffer);
            if (bytesReceivedCount + bufferCount <= BUFFER_SIZE) {
                transferToCurrentBuffer(tempBuffer, bytesReceivedCount, bufferCount);
                bufferCount += bytesReceivedCount;
                if (bufferCount == BUFFER_SIZE) {
                    
                    //  send data back
                    if (bytesReceivedCount) {
                        /* Wait until component is ready to send data to host. */
                        while (!USBUART_CDCIsReady());

                        /* Send data back to host. */
                        if (currentBuffer == BUFFER_ZERO) {
                            USBUART_PutData(RxBuffer0, BUFFER_SIZE);
                        } else {
                            USBUART_PutData(RxBuffer1, BUFFER_SIZE);
                        }
                        
                        /* Wait until component is ready to send data to PC. */
                        while (!USBUART_CDCIsReady());
                        /* Send zero-length packet to PC. */
                        USBUART_PutData(NULL, 0u);
                    }
                    
                    switchBuffers(); 
                    bufferCount = 0;
                }
            } else {
                bytesBeforeOverflow = BUFFER_SIZE - bufferCount;
                transferToCurrentBuffer(tempBuffer, bytesBeforeOverflow, bufferCount);
                //send back this newly filled buffer
                if (bytesReceivedCount) {
                    /* Wait until component is ready to send data to host. */
                    while (!USBUART_CDCIsReady());

                    /* Send data back to host. */
                    if (currentBuffer == BUFFER_ZERO) {
                        USBUART_PutData(RxBuffer0, BUFFER_SIZE);
                    } else {
                        USBUART_PutData(RxBuffer1, BUFFER_SIZE);
                    }


                    /* Wait until component is ready to send data to PC. */
                    while (!USBUART_CDCIsReady()); 
                    /* Send zero-length packet to PC. */
                    USBUART_PutData(NULL, 0u);
                    
                    switchBuffers();
                    bufferCount = bytesReceivedCount - bytesBeforeOverflow;
                    //store remaining bytes into new buffer
                    transferToCurrentBuffer(&tempBuffer[bytesBeforeOverflow], bufferCount, 0);
                }
            }
        }
    }
    
    
    //just in case, sit and spin
    for(;;); 
}

//relies on global destination arrays 
void transferToCurrentBuffer(uint8 src[], uint16 count, uint8 index) {
    int i, j;
    for(i=index, j=0; i < index + count; i++, j++) {
        if (currentBuffer == BUFFER_ZERO) {
            RxBuffer0[i] = src[j];
        } else {
            RxBuffer1[i] = src[j];
        }
    }
}


void switchBuffers(void) {
    if (currentBuffer == BUFFER_ZERO) {
        currentBuffer = BUFFER_ONE;
    } else {
        currentBuffer = BUFFER_ZERO;
    }
}


#endif //ECHO_CODE

//EOF
