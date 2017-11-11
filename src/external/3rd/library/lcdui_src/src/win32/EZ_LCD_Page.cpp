/****h* EZ.LCD.SDK.Wrapper/EZ_LCD_Page.cpp
 * NAME
 *   EZ_LCD_Page.cpp
 * COPYRIGHT
 *   The Logitech EZ LCD SDK Wrapper, including all accompanying 
 *   documentation, is protected by intellectual property laws. All rights 
 *   not expressly granted by Logitech are reserved.
 * PURPOSE
 *   Part of the SDK package.
 * AUTHOR
 *   Christophe Juncker (christophe_juncker@logitech.com)
 *   Vahid Afshar (Vahid_Afshar@logitech.com)
 * CREATION DATE
 *   06/13/2005
 * MODIFICATION HISTORY
 *   03/01/2006 - Added the concept of pages.
 *   
 *******
 */

#include "StdAfx.h"

#include "LCDText.h"
#include "LCDStreamingText.h"
#include "LCDIcon.h"
#include "LCDProgressBar.h"
#include "LCDBitmap.h"
#include "EZ_LCD.h"
#include "EZ_LCD_Page.h"

// text box height for various font sizes
CONST INT LG_SMALL_FONT_TEXT_BOX_HEIGHT = 12;
CONST INT LG_MEDIUM_FONT_TEXT_BOX_HEIGHT = 13;
CONST INT LG_BIG_FONT_TEXT_BOX_HEIGHT = 20;

// corresponding font size
CONST INT LG_SMALL_FONT_SIZE = 7;
CONST INT LG_MEDIUM_FONT_SIZE = 8;
CONST INT LG_BIG_FONT_SIZE = 12;

// logical origin Y value for various font sizes
CONST INT LG_SMALL_FONT_LOGICAL_ORIGIN_Y = -3;
CONST INT LG_MEDIUM_FONT_LOGICAL_ORIGIN_Y = -2;
CONST INT LG_BIG_FONT_LOGICAL_ORIGIN_Y = -4;

// Scrolling text parameters
CONST INT LG_SCROLLING_SPEED = 7;
CONST INT LG_SCROLLING_STEP = 7;
CONST INT LG_SCROLLING_DELAY_MS = 2000;
CONST TCHAR LG_SCROLLING_GAP_TEXT[] = _T("       ");

// Progress bar parameters
CONST INT LG_PROGRESS_BAR_RANGE_MIN = 0;
CONST INT LG_PROGRESS_BAR_RANGE_MAX = 100;
CONST INT LG_PROGRESS_BAR_INITIAL_HEIGHT = 5;

// Font
CONST LPCTSTR LG_FONT = _T("Microsoft Sans Serif");

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::CEzLcdPage
 * NAME
 *  CEzLcdPage::CEzLcdPage(LPCTSTR friendlyName, INT width, INT height) -- Basic constructor
 * FUNCTION
 *  Does necessary initialization.
 * INPUTS
 *  friendlyName  - friendly name of the applet/game. This name will be 
 *                  displayed in the Logitech G-series LCD Manager.
 *  width         - width in pixels of the LCD.
 *  height        - height in pixels of the LCD.
 ******
 */
CEzLcdPage::CEzLcdPage()
{
	m_pContainer = NULL;
}

CEzLcdPage::CEzLcdPage(CEzLcd * pContainer, INT iWidth, INT iHeight)
{
	m_pContainer = pContainer;
	Init(iWidth, iHeight);
}

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::CEzLcdPage
 * NAME
 *  CEzLcdPage::CEzLcdPage(LPCTSTR friendlyName, INT width, INT height, lgLcdOnConfigureCB callbackFunction, PVOID configContext) -- Constructor including configure button callback function
 * FUNCTION
 *  Does necessary initialization.
 * INPUTS
 *  friendlyName  - friendly name of the applet/game. This name will be 
 *                  displayed in the Logitech G-series LCD Manager.
 *  width         - width in pixels of the LCD.
 *  height        - height in pixels of the LCD.
 ******
 */

CEzLcdPage::~CEzLcdPage()
{
    LCD_OBJECT_LIST::iterator it_ = m_Objects.begin();
    while(it_ != m_Objects.end())
    {
        CLCDBase *pObject_ = *it_;
        LCDUIASSERT(NULL != pObject_);
        delete pObject_;

        ++it_;
    }
}

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::AddText
 * NAME
 *  HANDLE CEzLcdPage::AddText(LGObjectType type, LGTextSize size, INT alignment, INT maxLengthPixels) -- Add a text object to the LCD
 * INPUTS
 *  type            - specifies whether the text is static or 
 *                    scrolling. Possible types are: LG_SCROLLING_TEXT,
 *                    LG_STATIC_TEXT
 *  size            - size of the text. Choose between these three: 
 *                    LG_SMALL, LG_MEDIUM or LG_BIG.
 *  alignment       - alignment of the text. Values are: DT_LEFT, 
 *                    DT_CENTER, DT_RIGHT.
 *  maxLengthPixels - max length in pixels of the text. If the text is
 *                    longer and of type LG_STATIC_TEXT, it will be cut
 *                    off. If the text is longer and of type 
 *                    LG_SCROLLING_TEXT, it will scroll.
 * RETURN VALUE 
 *  Handle for this object.
 * SEE ALSO
 *  CEzLcdPage::AddIcon
 *  CEzLcdPage::AddProgressBar
 ******
 */
HANDLE CEzLcdPage::AddText(LGObjectType type, LGTextSize size, INT iAlignment, INT iMaxLengthPixels)
{
    return AddText(type, size, iAlignment, iMaxLengthPixels, 1);
}

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::AddText
 * NAME
 *  HANDLE CEzLcdPage::AddText(LGObjectType type, LGTextSize size, INT alignment, INT maxLengthPixels, INT numberOfLines) -- Add a text object to the LCD
 * INPUTS
 *  type            - specifies whether the text is static or 
 *                    scrolling. Possible types are: LG_SCROLLING_TEXT,
 *                    LG_STATIC_TEXT
 *  size            - size of the text. Choose between these three: 
 *                    LG_SMALL, LG_MEDIUM or LG_BIG.
 *  alignment       - alignment of the text. Values are: DT_LEFT, 
 *                    DT_CENTER, DT_RIGHT.
 *  maxLengthPixels - max length in pixels of the text. If the text is
 *                    longer and of type LG_STATIC_TEXT, it will be cut
 *                    off. If the text is longer and of type 
 *                    LG_SCROLLING_TEXT, it will scroll.
 *  numberOfLines   - number of lines the text can use. For static text 
 *                    only. If number bigger than 1 and statoc text is too
 *                    long to fit on LCD, the text will be displayed on 
 *                    multiple lines as necessary.
 * RETURN VALUE 
 *  Handle for this object.
 * SEE ALSO
 *  CEzLcdPage::AddIcon
 *  CEzLcdPage::AddProgressBar
 ******
 */
HANDLE CEzLcdPage::AddText(LGObjectType type, LGTextSize size, INT iAlignment, INT iMaxLengthPixels, INT iNumberOfLines)
{
    LCDUIASSERT(LG_SCROLLING_TEXT == type || LG_STATIC_TEXT == type);
    CLCDText* pStaticText_;
    CLCDStreamingText* pStreamingText_;

    INT iBoxHeight_ = LG_MEDIUM_FONT_TEXT_BOX_HEIGHT;
    INT iFontSize_ = LG_MEDIUM_FONT_SIZE;
    INT iLocalOriginY_ = LG_MEDIUM_FONT_LOGICAL_ORIGIN_Y;

    switch (type)
    {
    case LG_SCROLLING_TEXT:
        pStreamingText_ = new CLCDStreamingText();
        LCDUIASSERT(NULL != pStreamingText_);
        pStreamingText_->Initialize();
        pStreamingText_->SetOrigin(0, 0);
        pStreamingText_->SetFontFaceName(LG_FONT);
        pStreamingText_->SetAlignment(iAlignment);
        pStreamingText_->SetText(_T(" "));
        pStreamingText_->SetGapText(LG_SCROLLING_GAP_TEXT);
        pStreamingText_->SetSpeed(LG_SCROLLING_SPEED);
        pStreamingText_->SetScrollingStep(LG_SCROLLING_STEP);
        pStreamingText_->SetStartDelay(LG_SCROLLING_DELAY_MS);

        if (LG_SMALL == size)
        {
            iBoxHeight_ = LG_SMALL_FONT_TEXT_BOX_HEIGHT;
            iFontSize_ = LG_SMALL_FONT_SIZE;
            iLocalOriginY_ = LG_SMALL_FONT_LOGICAL_ORIGIN_Y;
        }
        else if (LG_MEDIUM == size)
        {
            iBoxHeight_ = LG_MEDIUM_FONT_TEXT_BOX_HEIGHT;
            iFontSize_ = LG_MEDIUM_FONT_SIZE;
            iLocalOriginY_ = LG_MEDIUM_FONT_LOGICAL_ORIGIN_Y;
        }
        else if (LG_BIG == size)
        {
            pStreamingText_->SetFontWeight(FW_BOLD);
            iBoxHeight_ = LG_BIG_FONT_TEXT_BOX_HEIGHT;
            iFontSize_ = LG_BIG_FONT_SIZE;
            iLocalOriginY_ = LG_BIG_FONT_LOGICAL_ORIGIN_Y;
        }

        pStreamingText_->SetSize(iMaxLengthPixels, iBoxHeight_);		
        pStreamingText_->SetFontPointSize(iFontSize_);
        pStreamingText_->SetLogicalOrigin(0, iLocalOriginY_);
        pStreamingText_->SetObjectType(LG_SCROLLING_TEXT);

        AddObject(pStreamingText_);

        return pStreamingText_;
        break;
    case LG_STATIC_TEXT:
        pStaticText_ = new CLCDText();
        LCDUIASSERT(NULL != pStaticText_);
        pStaticText_->Initialize();
        pStaticText_->SetOrigin(0, 0);
        pStaticText_->SetFontFaceName(LG_FONT);
        pStaticText_->SetAlignment(iAlignment);
        pStaticText_->SetBackgroundMode(OPAQUE);
        pStaticText_->SetText(_T(" "));

        if (LG_SMALL == size)
        {
            iBoxHeight_ = iNumberOfLines * LG_SMALL_FONT_TEXT_BOX_HEIGHT;
            iFontSize_ = LG_SMALL_FONT_SIZE;
            iLocalOriginY_ = LG_SMALL_FONT_LOGICAL_ORIGIN_Y;
        }
        else if (LG_MEDIUM == size)
        {
            iBoxHeight_ = iNumberOfLines * LG_MEDIUM_FONT_TEXT_BOX_HEIGHT;
            iFontSize_ = LG_MEDIUM_FONT_SIZE;
            iLocalOriginY_ = LG_MEDIUM_FONT_LOGICAL_ORIGIN_Y;
        }
        else if (LG_BIG == size)
        {
            pStaticText_->SetFontWeight(FW_BOLD);
            iBoxHeight_ = iNumberOfLines * LG_BIG_FONT_TEXT_BOX_HEIGHT;
            iFontSize_ = LG_BIG_FONT_SIZE;
            iLocalOriginY_ = LG_BIG_FONT_LOGICAL_ORIGIN_Y;
        }

        pStaticText_->SetSize(iMaxLengthPixels, iBoxHeight_);
        pStaticText_->SetFontPointSize(iFontSize_);
        pStaticText_->SetLogicalOrigin(0, iLocalOriginY_);
        pStaticText_->SetObjectType(LG_STATIC_TEXT);

        if (1 < iNumberOfLines)
            pStaticText_->SetWordWrap(TRUE);

        AddObject(pStaticText_);

        return pStaticText_;
        break;
    default:
        LCDUITRACE(_T("ERROR: trying to add text object with undefined type\n"));
    }

    return NULL;
}

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::SetText
 * NAME
 *  HRESULT CEzLcdPage::SetText -- Set text
 * INPUTS
 *  hHandle          - handle to the object.
 *  pText            - text string.
 * RETURN VALUE 
 *  E_FAIL if there was an error.
 *  S_OK if no error.
 ******
 */
HRESULT CEzLcdPage::SetText(HANDLE hHandle, LPCTSTR pText)
{
    CLCDBase* pMyObject_ = GetObject(hHandle);

    if (NULL != pMyObject_)
    {
        LCDUIASSERT(LG_STATIC_TEXT == pMyObject_->GetObjectType() || LG_SCROLLING_TEXT == pMyObject_->GetObjectType());
        if (LG_STATIC_TEXT == pMyObject_->GetObjectType())
        {
            CLCDText* pStaticText_ = static_cast<CLCDText*>(pMyObject_);
            LCDUIASSERT(NULL != pStaticText_);
            pStaticText_->SetText(pText);
            return S_OK;
        }
        else if (LG_SCROLLING_TEXT == pMyObject_->GetObjectType())
        {
            CLCDStreamingText* pStreamingText_ = static_cast<CLCDStreamingText*>(pMyObject_);
            LCDUIASSERT(NULL != pStreamingText_);
            pStreamingText_->SetText(pText);
            return S_OK;
        }
    }

    return E_FAIL;
}

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::AddIcon
 * NAME
 *  HANDLE CEzLcdPage::AddIcon -- Add an icon object to the LCD
 * INPUTS
 *  hIcon            - icon to be displayed on the LCD. Should be 1 bpp
 *                     bitmap.
 *  iWidth           - x-axis size of the bitmap.
 *  iHeight          - y-axis size of the bitmap.
 * RETURN VALUE 
 *  Handle for this object.
 * SEE ALSO
 *  CEzLcdPage::AddText
 *  CEzLcdPage::AddProgressBar
 ******
 */
HANDLE CEzLcdPage::AddIcon(HICON hIcon, INT iWidth, INT iHeight)
{
    CLCDIcon* hIcon_ = new CLCDIcon();
    LCDUIASSERT(NULL != hIcon_);
    hIcon_->Initialize();
    hIcon_->SetOrigin(0, 0);
    hIcon_->SetSize(iWidth, iHeight);
    hIcon_->SetIcon(hIcon, iWidth, iHeight);
    hIcon_->SetObjectType(LG_ICON);

    AddObject(hIcon_);

    return hIcon_;
}

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::AddProgressBar
 * NAME
 *  HANDLE CEzLcdPage::AddProgressBar -- Add a progress bar object to the LCD.
 * INPUTS
 *  type            - type of the progress bar. Types are: LG_CURSOR, 
 *                    LG_FILLED.
 * RETURN VALUE 
 *  Handle for this object.
 * SEE ALSO
 *  CEzLcdPage::AddText
 *  CEzLcdPage::AddIcon
 ******
 */
HANDLE CEzLcdPage::AddProgressBar(LGProgressBarType type)
{
    LCDUIASSERT(LG_FILLED == type || LG_CURSOR == type || LG_DOT_CURSOR == type);
    CLCDProgressBar *pProgressBar_ = new CLCDProgressBar();
    LCDUIASSERT(NULL != pProgressBar_);
    pProgressBar_->Initialize();
    pProgressBar_->SetOrigin(0, 0);
    pProgressBar_->SetSize(m_iLcdWidth, LG_PROGRESS_BAR_INITIAL_HEIGHT);
    pProgressBar_->SetRange(LG_PROGRESS_BAR_RANGE_MIN, LG_PROGRESS_BAR_RANGE_MAX );
    pProgressBar_->SetPos(static_cast<FLOAT>(LG_PROGRESS_BAR_RANGE_MIN));
    pProgressBar_->SetObjectType(LG_PROGRESS_BAR);

	// Map the progress style into what the UI classes understand
	CLCDProgressBar::ePROGRESS_STYLE eStyle = CLCDProgressBar::STYLE_FILLED;
	switch (type)
	{
		case LG_FILLED:
			eStyle = CLCDProgressBar::STYLE_FILLED;
			break;
		case LG_CURSOR:
			eStyle = CLCDProgressBar::STYLE_CURSOR;
			break;
		case LG_DOT_CURSOR:
			eStyle = CLCDProgressBar::STYLE_DASHED_CURSOR;
			break;
	}

	pProgressBar_->SetProgressStyle(eStyle);

    AddObject(pProgressBar_);

    return pProgressBar_;
}

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::SetProgressBarPosition
 * NAME
 *  HRESULT CEzLcdPage::SetProgressBarPosition -- Set position of the 
 *  progress bar's cursor
 * INPUTS
 *  hHandle          - handle to the object.
 *  fPercentage      - percentage of progress (0 to 100).
 * RETURN VALUE 
 *  E_FAIL if there was an error or if handle does not correspond to a
 *  progress bar.
 *  S_OK if no error.
 ******
 */
HRESULT CEzLcdPage::SetProgressBarPosition(HANDLE hHandle, FLOAT fPercentage)
{
    CLCDBase* pMyObject_ = GetObject(hHandle);

    if (NULL != pMyObject_)
    {
        LCDUIASSERT(LG_PROGRESS_BAR == pMyObject_->GetObjectType());
        // only allow this function for progress bars
        if (LG_PROGRESS_BAR == pMyObject_->GetObjectType())
        {
            CLCDProgressBar *progressBar_ = static_cast<CLCDProgressBar*>(pMyObject_);
            LCDUIASSERT(NULL != progressBar_);
            progressBar_->SetPos(fPercentage);
            return S_OK;
        }
    }

    return E_FAIL;
}

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::SetProgressBarSize
 * NAME
 *  HRESULT CEzLcdPage::SetProgressBarSize -- Set size of progress bar
 * INPUTS
 *  hHandle          - handle to the object.
 *  iWidth           - x-axis part of the size
 *  iHeight          - y-axis part of the size (a good default value is 5).
 * RETURN VALUE 
 *  E_FAIL if there was an error or if handle does not correspond to a
 *  progress bar.
 *  S_OK if no error.
 ******
 */
HRESULT CEzLcdPage::SetProgressBarSize(HANDLE hHandle, INT iWidth, INT iHeight)
{
    CLCDBase* ppMyObject_ = GetObject(hHandle);
    LCDUIASSERT(NULL != ppMyObject_);

    if (NULL != ppMyObject_)
    {
        LCDUIASSERT(LG_PROGRESS_BAR == ppMyObject_->GetObjectType());
        // only allow this function for progress bars
        if (LG_PROGRESS_BAR == ppMyObject_->GetObjectType())
        {
            ppMyObject_->SetSize(iWidth, iHeight);
            return S_OK;
        }
    }

    return E_FAIL;
}

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::AddBitmap
 * NAME
 *  HANDLE CEzLcdPage::AddBitmap() -- Add a bitmap object to the LCD.
 * COMMENTS
 *  A bitmap's size currently must always be 160x43 pixels.
 * RETURN VALUE 
 *  Handle for this object.
 * SEE ALSO
 *  CEzLcdPage::AddText
 *  CEzLcdPage::AddIcon
 ******
 */
HANDLE CEzLcdPage::AddBitmap(void)
{
    CLCDBitmap *hBitmap_ = new CLCDBitmap();
    LCDUIASSERT(NULL != hBitmap_);
    hBitmap_->Initialize();
    hBitmap_->SetOrigin(0, 0);

    AddObject(hBitmap_);

    return hBitmap_;
}

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::SetBitmap
 * NAME
 *  HRESULT CEzLcdPage::SetBitmap(HANDLE handle, HBITMAP bitmap) -- Set the bitmap.
 * INPUTS
 *  handle          - handle to the object.
 *  bitmap          - 160x43 1bpp bitmap.
 * RETURN VALUE 
 *  E_FAIL if there was an error or if handle does not correspond to a
 *  bitmap.
 *  S_OK if no error.
 ******
 */
HRESULT CEzLcdPage::SetBitmap(HANDLE hHandle, HBITMAP hBitmap)
{
    if (NULL == hBitmap)
        return E_FAIL;

    CLCDBase* pMyObject_ = GetObject(hHandle);

    if (NULL != pMyObject_)
    {
        CLCDBitmap* hBitmap_ = static_cast<CLCDBitmap*>(pMyObject_);
        LCDUIASSERT(hBitmap_);
        hBitmap_->SetBitmap(hBitmap);
        return S_OK;
    }

    return S_OK;
}

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::SetOrigin
 * NAME
 *  HRESULT CEzLcdPage::SetOrigin -- Set the origin of an object. The 
 *                  origin corresponds to the furthest pixel on the 
 *                  upper left corner of an object.
 * INPUTS
 *  hHandle          - handle to the object.
 *  iOriginX         - x-axis part of the origin.
 *  iOriginY         - y-axis part of the origin.
 * RETURN VALUE 
 *  E_FAIL if there was an error.
 *  S_OK if no error.
 ******
 */
HRESULT CEzLcdPage::SetOrigin(HANDLE hHandle, INT iOriginX, INT iOriginY)
{
    CLCDBase* pMyObject_ = GetObject(hHandle);
    LCDUIASSERT(NULL != pMyObject_);
    LCDUIASSERT(NULL != pMyObject_);

    if (NULL != pMyObject_ && NULL != pMyObject_)
    {
        pMyObject_->SetOrigin(iOriginX, iOriginY);
        return S_OK;
    }

    return E_FAIL;
}

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::SetVisible
 * NAME
 *  HRESULT CEzLcdPage::SetVisible -- set corresponding object to be 
 *  visible or invisible.
 * INPUTS
 *  hHandle          - handle to the object.
 *  bVisible         - set to FALSE to make object invisible, TRUE to 
 *                     make it visible (default).
 * RETURN VALUE 
 *  E_FAIL if there was an error.
 *  S_OK if no error.
 ******
 */
HRESULT CEzLcdPage::SetVisible(HANDLE hHandle, BOOL bVisible)
{
    CLCDBase* pMyObject_ = GetObject(hHandle);
    LCDUIASSERT(NULL != pMyObject_);
    LCDUIASSERT(NULL != pMyObject_);

    if (NULL != pMyObject_ && NULL != pMyObject_)
    {        
        pMyObject_->Show(bVisible);
        return S_OK;
    }

    return E_FAIL;
}

/****f* EZ.LCD.SDK.Wrapper/CEzLcdPage::Update
 * NAME
 *  VOID CEzLcdPage::Update -- Update LCD display
 * FUNCTION
 *  Updates the display. Must be called every loop.
 ******
 */
VOID CEzLcdPage::Update()
{
// Save copy of button state
    for (INT ii = 0; ii < NUMBER_SOFT_BUTTONS; ii++)
    {
        m_bButtonWasPressed[ii] = m_bButtonIsPressed[ii];
    }
}

CLCDBase* CEzLcdPage::GetObject(HANDLE hHandle)
{
    LCD_OBJECT_LIST::iterator it_ = m_Objects.begin();
    while(it_ != m_Objects.end())
    {
        CLCDBase *pObject_ = *it_;
        LCDUIASSERT(NULL != pObject_);

        if (pObject_ == hHandle)
        {
            return pObject_;
        }
        ++it_;
    }

    return NULL;
}

VOID CEzLcdPage::Init(INT iWidth, INT iHeight)
{
    m_iLcdWidth = iWidth;
    m_iLcdHeight = iHeight;

    for (INT ii = 0; ii < NUMBER_SOFT_BUTTONS; ii++)
    {
        m_bButtonIsPressed[ii] = FALSE;
        m_bButtonWasPressed[ii] = FALSE;
    }
}

void CEzLcdPage::OnLCDButtonDown(int iButton)
{
    switch(iButton)
    {
    case LGLCDBUTTON_BUTTON0:
        m_bButtonIsPressed[LG_BUTTON_1] = TRUE;
        break;
    case LGLCDBUTTON_BUTTON1:
        m_bButtonIsPressed[LG_BUTTON_2] = TRUE;
        break;
    case LGLCDBUTTON_BUTTON2:
        m_bButtonIsPressed[LG_BUTTON_3] = TRUE;
        break;
    case LGLCDBUTTON_BUTTON3:
        m_bButtonIsPressed[LG_BUTTON_4] = TRUE;
        break;
    default:
        LCDUITRACE(_T("ERROR: unknown button was pressed\n"));
        break;
    }

	m_pContainer->OnLCDButtonDown(iButton);
}

void CEzLcdPage::OnLCDButtonUp(int iButton)
{
    switch(iButton)
    {
    case LGLCDBUTTON_BUTTON0:
        m_bButtonIsPressed[LG_BUTTON_1] = FALSE;
        break;
    case LGLCDBUTTON_BUTTON1:
        m_bButtonIsPressed[LG_BUTTON_2] = FALSE;
        break;
    case LGLCDBUTTON_BUTTON2:
        m_bButtonIsPressed[LG_BUTTON_3] = FALSE;
        break;
    case LGLCDBUTTON_BUTTON3:
        m_bButtonIsPressed[LG_BUTTON_4] = FALSE;
        break;
    default:
        LCDUITRACE(_T("ERROR: unknown button was pressed\n"));
        break;
    }

	m_pContainer->OnLCDButtonUp(iButton);
}
