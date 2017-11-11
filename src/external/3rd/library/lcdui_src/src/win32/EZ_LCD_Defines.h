#ifndef EZLCD_H_DEFINES_INCLUDED_
#define EZLCD_H_DEFINES_INCLUDED_


#define NUMBER_SOFT_BUTTONS		4
#define LCD_DEFAULT_WIDTH		160
#define LCD_DEFAULT_HEIGHT		43

typedef enum
{
    LG_BUTTON_1, LG_BUTTON_2, LG_BUTTON_3, LG_BUTTON_4
} LGSoftButton;

typedef enum
{
    LG_SMALL, LG_MEDIUM, LG_BIG
} LGTextSize;

typedef enum
{
    LG_CURSOR, LG_FILLED, LG_DOT_CURSOR
} LGProgressBarType;


#endif		// EZLCD_H_DEFINES_INCLUDED_





