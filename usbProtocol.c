/*
    Carl Lindquist
    Nov 15, 2016
    
    Control code to communicate over USBUART on the PSOC5 LP
*/


#include <project.h>
#include "usbProtocol.h"

#define MAX_USB_STRING_LENGTH 63


//––––––  Private Declarations  ––––––//

void usbStart(void);
uint8 usbGetByte(void);

//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void usbStart(void) {
    CyGlobalIntEnable; //Enable PSOC 5LP interrupts
    USBUART_Start(0u, USBUART_3V_OPERATION);
    while (!USBUART_GetConfiguration()){};
    USBUART_CDC_Init(); 
}

uint8 usbGetByte(void) {
    uint8 byte = '\0';
    if (USBUART_DataIsReady()) {
        USBUART_GetAll(&byte);
    }
    return byte;
}

void usbSendByte(uint8 byte) {
    while(!USBUART_CDCIsReady());
    USBUART_PutChar(byte);
}

void usbSendString(uint8 string[]) {
    uint8 length = 0;
    while(string[++length]);
    if(string[0] && length <= MAX_USB_STRING_LENGTH) {
        while(!USBUART_CDCIsReady());
        USBUART_PutData(string, length);
    }
}

//EOF