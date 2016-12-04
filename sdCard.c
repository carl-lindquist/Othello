/* 
    Carl Lindquist
    Nov 29, 2016
    
    Control code for logging moves from the Reversi game 
    on the PSOC 5LP to an SD Card
*/
    
#include "sdCard.h"
#include <FS.h>
#include <stdio.h>
#include "usbProtocol.h"
    
uint8 initSuccessful;
    
FS_FILE* openFile;
    

//––––––  Private Declarations  ––––––//
uint8 sdAppendString(char string[]);
    

//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

uint8 sdStart(void) {
    FS_Init();
    initSuccessful = 1;
    
    openFile = FS_FOpen("REPORT.TXT", "w");
    if(!openFile) {
        usbSendString((uint8*)"---File did not open\r");
        initSuccessful = 0;
        return 0;
    }
    FS_Write(openFile, "Reversi Game Report:\r", 21);
    sdClose();
    
    return initSuccessful;
}

uint8 sdWriteMove(char id[], uint8 pFlag, uint8 row, uint8 column) {
    if(!initSuccessful) {
        return 0;    
    }
    char string[20] = {};
    sprintf(string, "%s %d %d %d\r\n", id, pFlag, row, column);
    if(!sdAppendString(string)) {
        return 0;
    }
    return 1;
}

uint8 sdClose(void) {
    if(!FS_FClose(openFile)) {
        return 1;    
    }
    return 0;
}


//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

uint8 sdAppendString(char string[]) {
    if(!initSuccessful) {
        return 0;    
    }
    openFile = FS_FOpen("REPORT.TXT", "a");
    uint16 length = 0;
    for(; string[length]; length++);
    if(length && FS_Write(openFile, string, length)) {
        sdClose();
        return 1;    
    } else {
        sdClose();
        return 0;    
    }
}



//#define SD_CARD_TEST
#ifdef SD_CARD_TEST
        
        if(sdStart()) {
            usbSendString((uint8*)"Start Successful\r");    
        } else {
            usbSendString((uint8*)"Start Failed\r");   
        }
        if(sdWriteMove((char*)playerId, 1, 3, 6)) {
            usbSendString((uint8*)"Write Successful\r");    
        } else {
            usbSendString((uint8*)"Write Failed\r");   
        }
        sdWriteMove((char*)enemyId, 0, 9, 4);
        
//        if(sdClose()) {
//            usbSendString((uint8*)"Close Successful\r");    
//        } else {
//            usbSendString((uint8*)"Close Failed\r");   
//        }
        usbSendString((uint8*)"END\r\r");
        for(;;);
#endif //SD_CARD_TEST

//EOF
