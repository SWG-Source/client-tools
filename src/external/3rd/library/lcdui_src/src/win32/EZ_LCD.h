#ifndef EZLCD_H_INCLUDED_
#define EZLCD_H_INCLUDED_

#include "LCDManager.h"
#include "LCDOutput.h"


#include "EZ_LCD_Defines.h"
#include "EZ_LCD_Page.h"

#define DT_LEFT                     0x00000000

class CEzLcd
{
public:
    CEzLcd();
    ~CEzLcd();
    CEzLcd(LPCTSTR pFriendlyName, 
        INT iWidth = LCD_DEFAULT_WIDTH, 
        INT iHeight = LCD_DEFAULT_HEIGHT);

    HRESULT InitYourself(LPCTSTR pFriendlyName, 
                    BOOL bIsAutoStartable = FALSE,
                    BOOL bIsPersistent = FALSE,
                    lgLcdConfigureContext * pConfigContext = NULL, 
                    INT iWidth = LCD_DEFAULT_WIDTH, 
                    INT	 iHeight = LCD_DEFAULT_HEIGHT);


    // Adds one page
    INT AddNewPage(VOID);
    // Removes a page
    INT RemovePage(INT iPageNumber);
    // Returns the total number of pages
    INT GetPageCount(VOID);
    // Adds iNumberOfPages to the total of pages you've created
    INT AddNumberOfPages(INT iNumberOfPages);
    // Call this method prior to adjusting any control on a page
    BOOL ModifyControlsOnPage(INT iPageNumber);
    // Call this method to show a certain page on the LCD
    BOOL ShowPage(INT iPageNumber);

    // The methods below are used to add or modify a control on a certain page. Method
    // ModifyControlsOnPage(INT pageNumber) must be called prior to using any of the following
    // methods.
    HANDLE AddText(LGObjectType type, LGTextSize size, INT iAlignment, INT iMaxLengthPixels);
    HANDLE AddText(LGObjectType type, LGTextSize size, INT iAlignment, INT iMaxLengthPixels, INT iNumberOfLines);
    HRESULT SetText(HANDLE hHandle, LPCTSTR pText);

    HANDLE AddIcon(HICON hIcon, INT iSizeX, INT iSizeY);

    HANDLE AddProgressBar(LGProgressBarType type);
    HRESULT SetProgressBarPosition(HANDLE hHandle, FLOAT fPercentage);
    HRESULT SetProgressBarSize(HANDLE hHandle, INT iWidth, INT iHeight);

    HANDLE AddBitmap();
    HRESULT SetBitmap(HANDLE hHandle, HBITMAP hBitmap);

    HRESULT SetOrigin(HANDLE hHandle, INT iOriginX, INT iOriginY);
    HRESULT SetVisible(HANDLE hHandle, BOOL bVisible);

    BOOL IsConnected();
    HRESULT SetAsForeground(BOOL bSetAsForeground);
    HRESULT SetScreenPriority(DWORD dwPriority);

    BOOL ButtonTriggered(INT iButton);
    BOOL ButtonReleased(INT iButton);
    BOOL ButtonIsPressed(INT iButton);

    VOID Update();

    virtual void OnLCDButtonDown(INT iButton);
    virtual void OnLCDButtonUp(INT iButton);

protected:
    INT                     m_iLcdWidth;
    INT                     m_iLcdHeight;
    TCHAR                   m_chFriendlyName[MAX_PATH];
    CLCDOutput              m_output;
    CEzLcdPage  *           m_pActivePage;
    LCD_PAGE_LIST           m_LCDPageList;
    INT                     m_iPageCount;		// How many pages are there
    INT                     m_iCurrentPageNumberShown;
    BOOL                    m_bInitNeeded;
    BOOL                    m_bInitSucceeded;
    BOOL                    m_bButtonIsPressed[NUMBER_SOFT_BUTTONS];
    BOOL                    m_bButtonWasPressed[NUMBER_SOFT_BUTTONS];
    lgLcdConfigureContext * m_pConfigContext;
    BOOL                    m_bIsPersistent;
    BOOL                    m_bIsAutoStartable;	
};


#endif
