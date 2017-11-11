
#ifndef EZLCD_PAGE_H_INCLUDED_
#define EZLCD_PAGE_H_INCLUDED_


#include "LCDManager.h"
#include "LCDOutput.h"


#include "EZ_LCD_Defines.h"

class CEzLcd;


class CEzLcdPage : public CLCDManager
{
public:
    CEzLcdPage();
    CEzLcdPage(CEzLcd * pContainer, INT width, INT height);
    ~CEzLcdPage();

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

    VOID Update();

    virtual void OnLCDButtonDown(int iButton);
    virtual void OnLCDButtonUp(int iButton);

protected:
    CLCDBase* GetObject(HANDLE hHandle);
    VOID Init(INT iWidth, INT iHeight);

protected:
    CEzLcd *    m_pContainer;
    INT         m_iLcdWidth;
    INT         m_iLcdHeight;
    BOOL        m_bButtonIsPressed[NUMBER_SOFT_BUTTONS];
    BOOL        m_bButtonWasPressed[NUMBER_SOFT_BUTTONS];

};

typedef vector <CEzLcdPage*> LCD_PAGE_LIST;
typedef LCD_PAGE_LIST::iterator LCD_PAGE_LIST_ITER;


#endif		// EZLCD_PAGE_H_INCLUDED_
