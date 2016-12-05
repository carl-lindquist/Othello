/* 
    Carl Lindquist
    Nov 15, 2016
    
    Interface to communicate over USBUART on the PSOC5 LP

*/

#ifndef USB_PROTOCOL_H
#define USB_PROTOCOL_H
    
#include <project.h>
    

/*
[desc]  Initializes USB, blocks until USB is ready.
		COM port must be connected for this function to finish
*/
void usbStart(void);
    

/*
[desc]  Returns the byte sent from the host PC over USBUART.

[ret]	Returns one byte of data if available, '\0' otherwise.
*/
uint8 usbGetByte(void);


/*
[desc]  Sends a byte to the host PC over USBUART.

[byte] Byte to be sent
*/
void usbSendByte(uint8 byte);


/*
[desc]  Sends a string to the host PC over USBUART. String not sent if
		length is greater than 63

[string] String to be sent. Length should be less than 64.
*/
void usbSendString(uint8 string[]);
    
#endif //USB_PROTOCOL_H
