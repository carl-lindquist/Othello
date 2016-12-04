/*
    Carl Lindquist
    Nov 15, 2016

    LED Panel control code for the 16x32 RGB display 
    from CE 121. Written for the PSOC 5LP.
*/

#include "ledDisplay.h"


#define HALF_OF_ROWS 8
#define ROW_COUNT 16
#define COLUMN_COUNT 32

uint8 colorArr[ROW_COUNT][COLUMN_COUNT] = {};

CY_ISR_PROTO(ledUpdateIsr);


//––––––––––––––––––––––––––––––  Public Functions  ––––––––––––––––––––––––––––––//

void ledDisplayStart(void) {
    CyGlobalIntEnable; // Enable global interrupts.
    LED_Timer_Isr_StartEx(ledUpdateIsr);
    LED_Display_Timer_Start();
}

uint8 ledDisplayWriteColor(uint8 color, uint8 row, uint8 column) {
    if (0 > row || row > 15 || 0 > column || column > 31) {
        return ROW_COLUMN_INDEX_ERROR;
    } else if ( 0 > color || color > 7) {
        return COLOR_VALUE_ERROR;
    }
    colorArr[row][column] = color;
    return 0;
}

void ledDisplayClearDisplay(void) {
    uint8 row, column;
    for(row=0; row < ROW_COUNT; row++) {
        for(column=0; column < COLUMN_COUNT; column++) {
            colorArr[row][column] = COLOR_OFF;
        }
    }
}

void ledDisplayTest(void) {
    uint8 row, column;
    for(row=0; row < ROW_COUNT; row++) {
        for(column=0; column < COLUMN_COUNT; column++) {
            if (row == 3) {
                colorArr[row][column] = COLOR_CYAN;
            } else if (column == 27) {
                colorArr[row][column] = COLOR_PURPLE;
            } else {
                colorArr[row][column] = (uint8)column;   
            }
            
        }
    }
    
}

//––––––––––––––––––––––––––––––  Private Functions  ––––––––––––––––––––––––––––––//

CY_ISR(ledUpdateIsr) {
    uint8 row, column;
    for(row=0; row < HALF_OF_ROWS; row++) {
        
        OE_Pin_Write(1); //disable display
        ABC_Control_Reg_Write(row); //change rows

        //Shift in 32 values
        for(column=0; column < COLUMN_COUNT; column++) {

            //Because display divides the display into two halves,
            //Shift the bits in parallel
            RGB_1_Control_Reg_Write(colorArr[row][column]);
            RGB_2_Control_Reg_Write(colorArr[row + HALF_OF_ROWS][column]); 

            //Shift in RGB bits
            Clk_Pin_Write(1);
            Clk_Pin_Write(0);
        }

        //Latch new row into display
        Lat_Pin_Write(1);
        Lat_Pin_Write(0);
        //enable display
        OE_Pin_Write(0); //enable display
        
        CyDelayUs(500); //Increase delay to increase brightness
    }
    LED_Display_Timer_ReadStatusRegister();
}

// EOF