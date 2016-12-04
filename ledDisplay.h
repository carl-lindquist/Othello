/*
    Carl Lindquist
    Nov 20, 2016
    
    Interface for the 16x32 RGB display from ce121.
    Relies on hardware in the PSOC 5LP
*/

    
#ifndef LED_DISPLAY_H
#define LED_DISPLAY_H
    
#include <project.h>
    
    
#define COLOR_OFF 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_PURPLE 5
#define COLOR_CYAN 6
#define COLOR_WHITE 7

#define ROW_COLUMN_INDEX_ERROR -1
#define COLOR_VALUE_ERROR -2
    
    
//	Starts LEDInterrupt and LEDInterrupt Timer
void ledDisplayStart(void);


//	Writes a color value into the display, and will appear on the 
//	next display interrupt. Color and error defines start on line 16
//	errors: ret -1 row > 15, column > 31
//	        ret -2 color < 0, color > 7
//	EX: ledDisplayWriteColor(COLOR_RED, 0, 0);
uint8 ledDisplayWriteColor(uint8 color, uint8 row, uint8 column);


//	Clears the entire display
void ledDisplayClearDisplay(void);


//	Displays a test pattern on the display
void ledDisplayTest(void);


#endif //LED_DISPLAY_H
