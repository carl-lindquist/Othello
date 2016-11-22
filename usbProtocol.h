#ifndef _USB_PROTOCOL_H_
#define _USB_PROTOCOL_H_
    
#include <project.h>
    
//Initializes USB, blocks until USB is ready
//Note that COM port must be connected for this function to finish
void usbStart(void);
    
//Returns 1 byte of data from the USBUART if a byte was received since last call
//Return NULL otherwise
uint8 usbGetByte(void);
    
#endif

//EOF