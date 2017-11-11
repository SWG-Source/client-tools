/****h* EZ.LCD.SDK.Wrapper/EzLcd
 * NAME
 *   EZ_LCD.cpp
 * COPYRIGHT
 *   The Logitech EZ LCD SDK Wrapper, including all accompanying 
 *   documentation, is protected by intellectual property laws. All rights 
 *   not expressly granted by Logitech are reserved.
 * PURPOSE
 *   The EZ LCD SDK Wrapper is aimed at developers wanting to make use of the LCD 
 *   display on Logitech G-series keyboard. It comes with a very intuitive 
 *   and easy to use interface which enables one to easily display static 
 *   strings, scrolling strings, progress bars, and icons.
 *   See the following to get started:
 *       - readme.txt: Describes how to get started
 *       - Use one of the samples included to see how things work.
 * AUTHOR
 *   Christophe Juncker (christophe_juncker@logitech.com)
 * CREATION DATE
 *   06/13/2005
 * MODIFICATION HISTORY
 *   03/01/2006 - Vahid Afshar. Added the concept of pages to the API. A 
 *                client can now have multiple pages, each with its own 
 *                controls. A page can be shown, while another is modified.
 *              - Introduced the InitYourself() method.
 *******
 */

#include "StdAfx.h"

#include "EZ_LCD_Defines.h"
#include "EZ_LCD_Page.h"
#include "EZ_LCD.h"



/****f* EZ.LCD.Wrapper/CEzLcd::CEzLcd
 * NAME
 *  CEzLcd::CEzLcd() -- Basic constructor. The user must call the
 *      InitYourself(...) method after calling this constructor.
 * FUNCTION
 *  Object is created.
 * INPUTS
 ******
 */
CEzLcd::CEzLcd()
{
	m_iPageCount = 0;
}

/****f* EZ.LCD.Wrapper/CEzLcd::CEzLcd
 * NAME
 *  CEzLcd::CEzLcd(LPCTSTR pFriendlyName, INT iWidth, INT iHeight).
 * FUNCTION
 *  Does necessary initialization. If you are calling this constructor,
 *  then you should NOT call the InitYourself(...) method.
 * INPUTS
 *  pFriendlyName  - friendly name of the applet/game. This name will be 
 *                   displayed in the Logitech G-series LCD Manager.
 *  iWidth         - width in pixels of the LCD.
 *  iHeight        - height in pixels of the LCD.
 ******
 */
CEzLcd::CEzLcd(LPCTSTR pFriendlyName, INT iWidth, INT iHeight)
{
	m_iPageCount = 0;
    InitYourself(pFriendlyName, FALSE, FALSE, NULL, iWidth, iHeight);
}

CEzLcd::~CEzLcd()
{
    // delete all the screens
    LCD_PAGE_LIST::iterator it = m_LCDPageList.begin();
    while(it != m_LCDPageList.end())
    {
        CEzLcdPage *pPage = *it;
        LCDUIASSERT(NULL != pPage);
		pPage->Shutdown();

	    delete pPage;
        ++it;
    }

    m_output.Shutdown();
}

/****f* EZ.LCD.Wrapper/CEzLcd::InitYourself
 * NAME
 *  HRESULT CEzLcd::InitYourself(pFriendlyName, BOOL IsAutoStartable, 
 *      BOOL IsPersistent, lgLcdOnConfigureCB callbackFunction ,width , 
 *      height)
 * FUNCTION
 *  Does necessary initialization. This method SHOULD ONLY be called if
 *  the empty constructor is used: CEzLcd::CEzLcd()
 * INPUTS
 *  pFriendlyName   - friendly name of the applet/game. This name will be 
 *                    displayed in the Logitech G-series LCD Manager.
 *  IsAutoStartable - Determines if the applet is to be started 
 *                    automatically every time by the LCD manager software 
 *                    (Part of G15 software package)
 *  IsPersistent    - Determines if the applet's friendlyName will remain 
 *                    in the list of applets seen by the LCD manager 
 *                    software after the applet terminates
 *  pConfigContext  - Pointer to the lgLcdConfigContext structure used 
 *                    during callback into the applet
 *  width           - width in pixels of the LCD.
 *  height          - height in pixels of the LCD.
 * RETURN VALUE 
 *  The method returns the S_OK if it can connect to the LCD Manager 
 *  library, or returns E_FAIL if it can not.
 ******
 */
HRESULT CEzLcd::InitYourself(LPCTSTR pFriendlyName, BOOL bIsAutoStartable, BOOL bIsPersistent, 
                             lgLcdConfigureContext * pConfigContext, INT iWidth, INT iHeight)
{
    if (m_iPageCount != 0)
    {
        // Maybe the user is calling the old constructor and calling InitYourself as well.
        // Alert him of the problem. If the old constructor is called, then InitYourself should
        // not be called. InitYourself should be called, when the empty parameter constructor is
        // called, CEzLcd::CEzLcd()
        return E_FAIL; 
    }

    m_iLcdWidth = iWidth;
    m_iLcdHeight = iHeight;
    _tcscpy_s(m_chFriendlyName, 260 * sizeof(TCHAR), pFriendlyName);

    m_bInitNeeded = TRUE;
    m_bInitSucceeded = FALSE;

    for (INT ii = 0; ii < NUMBER_SOFT_BUTTONS; ii++)
    {
        m_bButtonIsPressed[ii] = FALSE;
        m_bButtonWasPressed[ii] = FALSE;
    }

    m_pConfigContext = pConfigContext;		// Keep the context structure pointer
	m_bIsPersistent = bIsPersistent;
	m_bIsAutoStartable = bIsAutoStartable;	

	// No active page to start with
	m_pActivePage = NULL;

	// Add the first page for backwards compatibility
	AddNewPage();
	ModifyControlsOnPage(0);

    // Will now connect to the real library and see it the lgLcdInit() succeeds
    lgLcdConnectContext     lgdConnectContext_;
    lgLcdConfigureContext   lgdConfigureContext_;

	if (m_pConfigContext != NULL)
	{
        lgdConfigureContext_.configCallback = m_pConfigContext->configCallback;
        lgdConfigureContext_.configContext = m_pConfigContext->configContext;
	}
	else
	{
        lgdConfigureContext_.configCallback = NULL;
        lgdConfigureContext_.configContext = NULL;
	}

    lgdConnectContext_.appFriendlyName = m_chFriendlyName;
    lgdConnectContext_.isPersistent = m_bIsPersistent;
    lgdConnectContext_.isAutostartable = m_bIsAutoStartable;
    lgdConnectContext_.onConfigure = lgdConfigureContext_;

    if (FAILED(m_output.Initialize(&lgdConnectContext_, FALSE)))
    {
        // This means the LCD SDK's lgLcdInit failed, and therefore
        // we will not be able to ever connect to the LCD, even if
        // a G-series keyboard is actually connected.
        LCDUITRACE(_T("ERROR: LCD SDK initialization failed\n"));
        m_bInitSucceeded = FALSE;
        return E_FAIL;
    }
	
	return S_OK;
}

/****f* EZ.LCD.Wrapper/CEzLcd::AddNewPage
 * NAME
 *  INT CEzLcd::AddNewPage(VOID) -- Call this method to create a new page 
 *      to be displayed on the LCD. The method returns the current number 
 *      of pages, after the page is added.
 * INPUTS
 *  NONE
 * RETURN VALUE 
 *  The method returns the current number of pages, after the page is 
 *  added.
 ******
 */
INT CEzLcd::AddNewPage(VOID)
{
	CEzLcdPage*	pPage = NULL;

	// Create a new page and add it in
	pPage = new CEzLcdPage(this, m_iLcdWidth, m_iLcdHeight);
	pPage->Initialize();
	pPage->SetExpiration(INFINITE);

    m_LCDPageList.push_back(pPage);

	m_iPageCount = m_LCDPageList.size();
	return m_iPageCount;
}

/****f* EZ.LCD.Wrapper/CEzLcd::RemovePage
 * NAME
 *  INT CEzLcd::RemovePage(INT iPageNumber) -- Call this method to remove 
 *      a page from the pages you've created to be displayed on the LCD. 
 *      The method returns the current number of pages, after the page is 
 *      deleted.
 * INPUTS
 *  iPageNumber - The number for the page that is to be removed.
 * RETURN VALUE 
 *  The method returns the current number of pages, after the page is 
 *  removed.
 ******
 */
INT CEzLcd::RemovePage(INT iPageNumber)
{
	// Do we have this page, if not return error
	if (iPageNumber >= m_iPageCount)
	{
		return -1;
	}

    // find the next active screen
    LCD_PAGE_LIST::iterator it = m_LCDPageList.begin();
    m_LCDPageList.erase(it + iPageNumber);

	--m_iPageCount;		// Decrement the number of pages
	if (m_iPageCount > 0)
	{
		m_pActivePage = *it;
	}
	else
	{
		m_pActivePage = NULL;
	}
	return m_iPageCount;
}

/****f* EZ.LCD.Wrapper/CEzLcd::GetPageCount
 * NAME
 *  INT CEzLcd::GetPageCount(VOID) returns the current number of pages.
 * INPUTS
 *  None.
 * RETURN VALUE 
 *  The method returns the current number of pages.
 ******
 */
INT CEzLcd::GetPageCount(VOID)
{
	return m_iPageCount;
}

/****f* EZ.LCD.Wrapper/CEzLcd::AddNumberOfPages
 * NAME
 *  INT CEzLcd::AddNumberOfPages(INT iNumberOfPages) - Adds iNumberOfPages 
 *      to the total of pages you've created.
 * INPUTS
 *  iNumberOfPages      - Count of pages to add in.
 * RETURN VALUE 
 *  The method returns the current number of pages, after the pages are 
 *  added.
 ******
 */
INT CEzLcd::AddNumberOfPages(INT iNumberOfPages)
{
    for (int iCount=0; iCount<iNumberOfPages; iCount++)
    {
        AddNewPage();
    }

	return GetPageCount();
}


/****f* EZ.LCD.Wrapper/CEzLcd::ModifyControlsOnPage
 * NAME
 *  INT CEzLcd::ModifyControlsOnPage(INT iPageNumber) - Call this method 
 *      in order to modify the controls on page. This method must be 
 *      called first prior to any modifications on that page.
 * INPUTS
 *  iPageNumber - The page number that the controls will be modified on.
 * RETURN VALUE 
 *  TRUE - If succeeded.
 *  FALSE - If encountered an error.
 ******
 */
BOOL CEzLcd::ModifyControlsOnPage(INT iPageNumber)
{
	if (iPageNumber >= m_iPageCount)
	{
		return FALSE;
	}

    LCD_PAGE_LIST::iterator it = m_LCDPageList.begin();
    m_pActivePage = *(it + iPageNumber);

	return TRUE;
}

/****f* EZ.LCD.Wrapper/CEzLcd::ShowPage
 * NAME
 *  INT CEzLcd::ShowPage(INT iPageNumber) Call this method in order to 
 *      make the page shown on the LCD, 
 * INPUTS
 *  iPageNumber - The page that will be shown among your pages on the LCD.
 * RETURN VALUE 
 *  TRUE - If succeeded.
 *  FALSE - If encountered an error.
 ******
 */
BOOL CEzLcd::ShowPage(INT iPageNumber)
{
	if (iPageNumber >= m_iPageCount)
	{
		return FALSE;
	}

    LCD_PAGE_LIST::iterator it = m_LCDPageList.begin();
    m_pActivePage = *(it + iPageNumber);

	m_output.UnlockScreen();
	m_output.LockScreen(m_pActivePage);

	return TRUE;
}

/****f* EZ.LCD.Wrapper/CEzLcd::AddText
 * NAME
 *  HANDLE CEzLcd::AddText(LGObjectType type, LGTextSize size, 
 *      INT iAlignment, INT iMaxLengthPixels) -- Add a text object to the 
 *      page you are working on.
 * INPUTS
 *  type             - specifies whether the text is static or 
 *                     scrolling. Possible types are: LG_SCROLLING_TEXT,
 *                     LG_STATIC_TEXT
 *  size             - size of the text. Choose between these three: 
 *                     LG_SMALL, LG_MEDIUM or LG_BIG.
 *  iAlignment       - alignment of the text. Values are: DT_LEFT, 
 *                     DT_CENTER, DT_RIGHT.
 *  iMaxLengthPixels - max length in pixels of the text. If the text is
 *                     longer and of type LG_STATIC_TEXT, it will be cut
 *                     off. If the text is longer and of type 
 *                     LG_SCROLLING_TEXT, it will scroll.
 * RETURN VALUE 
 *  Handle for this object.
 * SEE ALSO
 *  CEzLcd::AddIcon
 *  CEzLcd::AddProgressBar
 ******
 */
HANDLE CEzLcd::AddText(LGObjectType type, LGTextSize size, INT iAlignment, INT iMaxLengthPixels)
{
    return AddText(type, size, iAlignment, iMaxLengthPixels, 1);
}

/****f* EZ.LCD.Wrapper/CEzLcd::AddText
 * NAME
 *  HANDLE CEzLcd::AddText(LGObjectType type, LGTextSize size, 
 *      INT iAlignment, INT iMaxLengthPixels, INT iNumberOfLines) -- Add a 
 *      text object to the page you are working on.
 * INPUTS
 *  type             - specifies whether the text is static or 
 *                     scrolling. Possible types are: LG_SCROLLING_TEXT,
 *                     LG_STATIC_TEXT
 *  size             - size of the text. Choose between these three: 
 *                     LG_SMALL, LG_MEDIUM or LG_BIG.
 *  iAlignment       - alignment of the text. Values are: DT_LEFT, 
 *                     DT_CENTER, DT_RIGHT.
 *  iMaxLengthPixels - max length in pixels of the text. If the text is
 *                     longer and of type LG_STATIC_TEXT, it will be cut
 *                     off. If the text is longer and of type 
 *                     LG_SCROLLING_TEXT, it will scroll.
 *  iNumberOfLines   - number of lines the text can use. For static text 
 *                     only. If number bigger than 1 and statoc text is 
 *                     too long to fit on LCD, the text will be displayed 
 *                     on multiple lines as necessary.
 * RETURN VALUE 
 *  Handle for this object.
 * SEE ALSO
 *  CEzLcd::AddIcon
 *  CEzLcd::AddProgressBar
 ******
 */
HANDLE CEzLcd::AddText(LGObjectType type, LGTextSize size, INT iAlignment, INT iMaxLengthPixels, 
                       INT iNumberOfLines)
{
	if (m_pActivePage == NULL)
	{
	    return NULL;
	}

	return m_pActivePage->AddText(type, size, iAlignment, iMaxLengthPixels, iNumberOfLines);
}

/****f* EZ.LCD.Wrapper/CEzLcd::SetText
 * NAME
 *  HRESULT CEzLcd::SetText -- Sets the text in the control on the page 
 *      that you are working on.
 * INPUTS
 *  hHandle          - handle to the object.
 *  pText            - text string.
 * RETURN VALUE 
 *  E_FAIL if there was an error.
 *  S_OK if no error.
 ******
 */
HRESULT CEzLcd::SetText(HANDLE hHandle, LPCTSTR pText)
{
	if (m_pActivePage == NULL)
	{
	    return E_FAIL;
	}

	return m_pActivePage->SetText(hHandle, pText);
}

/****f* EZ.LCD.Wrapper/CEzLcd::AddIcon
 * NAME
 *  HANDLE CEzLcd::AddIcon -- Add an icon object to the page that 
 *      you are working on.
 * INPUTS
 *  hIcon            - icon to be displayed on the page. Should be 1 bpp
 *                     bitmap.
 *  iSizeX           - x-axis size of the bitmap.
 *  iSizeY           - y-axis size of the bitmap.
 * RETURN VALUE 
 *  Handle for this object.
 * SEE ALSO
 *  CEzLcd::AddText
 *  CEzLcd::AddProgressBar
 ******
 */
HANDLE CEzLcd::AddIcon(HICON hIcon, INT iSizeX, INT iSizeY)
{
	if (m_pActivePage == NULL)
	{
	    return NULL;
	}

	return m_pActivePage->AddIcon(hIcon, iSizeX, iSizeY);
}

/****f* EZ.LCD.Wrapper/CEzLcd::AddProgressBar
 * NAME
 *  HANDLE CEzLcd::AddProgressBar -- Add a progress bar object to the 
 *      page that you are working on.
 * INPUTS
 *  type            - type of the progress bar. Types are: LG_CURSOR, 
 *                    LG_FILLED, LG_DOT_CURSOR.
 * RETURN VALUE 
 *  Handle for this object.
 * SEE ALSO
 *  CEzLcd::AddText
 *  CEzLcd::AddIcon
 ******
 */
HANDLE CEzLcd::AddProgressBar(LGProgressBarType type)
{
	if (m_pActivePage == NULL)
	{
	    return NULL;
	}

	return m_pActivePage->AddProgressBar(type);
}

/****f* EZ.LCD.Wrapper/CEzLcd::SetProgressBarPosition
 * NAME
 *  HRESULT CEzLcd::SetProgressBarPosition -- Set position of the 
 *      progress bar's cursor.
 * INPUTS
 *  hHandle          - handle to the object.
 *  fPercentage      - percentage of progress (0 to 100).
 * RETURN VALUE 
 *  E_FAIL if there was an error or if handle does not correspond to a
 *  progress bar.
 *  S_OK if no error.
 ******
 */
HRESULT CEzLcd::SetProgressBarPosition(HANDLE hHandle, FLOAT fPercentage)
{
	if (m_pActivePage == NULL)
	{
	    return E_FAIL;
	}

	return m_pActivePage->SetProgressBarPosition(hHandle, fPercentage);
}

/****f* EZ.LCD.Wrapper/CEzLcd::SetProgressBarSize
 * NAME
 *  HRESULT CEzLcd::SetProgressBarSize -- Set size of progress bar.
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
HRESULT CEzLcd::SetProgressBarSize(HANDLE hHandle, INT iWidth, INT iHeight)
{
	if (m_pActivePage == NULL)
	{
	    return E_FAIL;
	}

	return m_pActivePage->SetProgressBarSize(hHandle, iWidth, iHeight);
}

/****f* EZ.LCD.Wrapper/CEzLcd::AddBitmap
 * NAME
 *  HANDLE CEzLcd::AddBitmap() -- Add a bitmap object to the page that
 *      you are working on.
 * NOTES
 *  A bitmap's size currently must always be 160x43 pixels.
 * RETURN VALUE 
 *  Handle for this object.
 * SEE ALSO
 *  CEzLcd::AddText
 *  CEzLcd::AddIcon
 ******
 */
HANDLE CEzLcd::AddBitmap()
{
	if (m_pActivePage == NULL)
	{
	    return NULL;
	}

	return m_pActivePage->AddBitmap();
}

/****f* EZ.LCD.Wrapper/CEzLcd::SetBitmap
 * NAME
 *  HRESULT CEzLcd::SetBitmap(HANDLE hHandle, HBITMAP hBitmap) -- Set the 
 *      bitmap.
 * INPUTS
 *  hHandle          - handle to the object.
 *  hBitmap          - 1bpp bitmap.
 * RETURN VALUE 
 *  E_FAIL if there was an error or if handle does not correspond to a
 *  bitmap.
 *  S_OK if no error.
 ******
 */
HRESULT CEzLcd::SetBitmap(HANDLE hHandle, HBITMAP hBitmap)
{
	if (m_pActivePage == NULL)
	{
	    return E_FAIL;
	}

	return m_pActivePage->SetBitmap(hHandle, hBitmap);
}

/****f* EZ.LCD.Wrapper/CEzLcd::SetOrigin
 * NAME
 *  HRESULT CEzLcd::SetOrigin -- Set the origin of an object. The origin 
 *      corresponds to the furthest pixel on the upper left corner of an 
 *      object.
 * INPUTS
 *  hHandle          - handle to the object.
 *  iXOrigin         - x-axis part of the origin.
 *  iYOrigin         - y-axis part of the origin.
 * RETURN VALUE 
 *  E_FAIL if there was an error.
 *  S_OK if no error.
 ******
 */
HRESULT CEzLcd::SetOrigin(HANDLE hHandle, INT iXOrigin, INT iYOrigin)
{
	if (m_pActivePage == NULL)
	{
	    return E_FAIL;
	}

	return m_pActivePage->SetOrigin(hHandle, iXOrigin, iYOrigin);
}

/****f* EZ.LCD.Wrapper/CEzLcd::SetVisible
 * NAME
 *  HRESULT CEzLcd::SetVisible -- set corresponding object to be 
 *      visible or invisible on the page that you are working on.
 * INPUTS
 *  hHandle          - handle to the object.
 *  bVisible         - set to FALSE to make object invisible, TRUE to 
 *                     make it visible (default).
 * RETURN VALUE 
 *  E_FAIL if there was an error.
 *  S_OK if no error.
 ******
 */
HRESULT CEzLcd::SetVisible(HANDLE hHandle, BOOL bVisible)
{
	if (m_pActivePage == NULL)
	{
	    return E_FAIL;
	}

	return m_pActivePage->SetVisible(hHandle, bVisible);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/****f* EZ.LCD.Wrapper/CEzLcd::ButtonTriggered
 * NAME
 *  BOOL CEzLcd::ButtonTriggered -- Check if a button was triggered.
 * INPUTS
 *  iButton      - name of the button to be checked. Possible names are:
 *                 LG_BUTTON_1, LG_BUTTON_2, LG_BUTTON_3, LG_BUTTON_4
 * RETURN VALUE 
 *  TRUE if the specific button was triggered
 *  FALSE otherwise
 * SEE ALSO
 *  CEzLcd::ButtonReleased
 *  CEzLcd::ButtonIsPressed
 ******
 */
BOOL CEzLcd::ButtonTriggered(INT iButton)
{
    if (m_bButtonIsPressed[iButton] && !m_bButtonWasPressed[iButton])
    {
        return TRUE;
    }

    return FALSE;
}

/****f* EZ.LCD.Wrapper/CEzLcd::ButtonReleased
 * NAME
 *  BOOL CEzLcd::ButtonReleased -- Check if a button was released.
 * INPUTS
 *  iButton      - name of the button to be checked. Possible names are:
 *                 LG_BUTTON_1, LG_BUTTON_2, LG_BUTTON_3, LG_BUTTON_4
 * RETURN VALUE 
 *  TRUE if the specific button was released
 *  FALSE otherwise
 * SEE ALSO
 *  CEzLcd::ButtonTriggered
 *  CEzLcd::ButtonIsPressed
 ******
 */
BOOL CEzLcd::ButtonReleased(INT iButton)
{
    if (!m_bButtonIsPressed[iButton] && m_bButtonWasPressed[iButton])
    {
        return TRUE;
    }
    return FALSE;
}

/****f* EZ.LCD.Wrapper/CEzLcd::ButtonIsPressed
 * NAME
 *  BOOL CEzLcd::ButtonIsPressed -- Check if a button is being pressed.
 * INPUTS
 *  iButton      - name of the button to be checked. Possible names are:
 *                 LG_BUTTON_1, LG_BUTTON_2, LG_BUTTON_3, LG_BUTTON_4
 * RETURN VALUE 
 *  TRUE if the specific button is being pressed
 *  FALSE otherwise
 * SEE ALSO
 *  CEzLcd::ButtonTriggered
 *  CEzLcd::ButtonReleased
 ******
 */
BOOL CEzLcd::ButtonIsPressed(INT iButton)
{
    return m_bButtonIsPressed[iButton];
}

/****f* EZ.LCD.Wrapper/CEzLcd::IsConnected
 * NAME
 *  BOOL CEzLcd::IsConnected -- Check if a Logitech G-series LCD is 
 *      connected.
 * RETURN VALUE 
 *  TRUE if an LCD is connected
 *  FALSE otherwise
 ******
 */
BOOL CEzLcd::IsConnected()
{
    return m_output.IsOpened();
}

/****f* EZ.LCD.Wrapper/CEzLcd::SetAsForeground
 * NAME
 *  HRESULT CEzLcd::SetAsForeground(BOOL bSetAsForeground) -- Become 
 *      foreground applet on LCD, or remove yourself as foreground applet.
 * INPUTS
 *  bSetAsForeground    - Determines whether to be foreground or not. 
 *                        Possible values are:
 *                          - TRUE implies foreground
 *                          - FALSE implies no longer foreground
 * RETURN VALUE 
 *  E_FAIL if there was an error.
 *  S_OK if no error.
 ******
 */
HRESULT CEzLcd::SetAsForeground(BOOL bSetAsForeground)
{
    m_output.SetAsForeground(bSetAsForeground);

    return S_OK;
}

/****f* EZ.LCD.Wrapper/CEzLcd::SetScreenPriority
 * NAME
 *  HRESULT CEzLcd::SetScreenPriority(DWORD priority) -- Set screen 
 *      priority.
 * INPUTS
 *  dwPriority    - priority of the screen. Possible values are:
 *                      - LGLCD_PRIORITY_IDLE_NO_SHOW
 *                      - LGLCD_PRIORITY_BACKGROUND
 *                      - LGLCD_PRIORITY_NORMAL
 *                      - LGLCD_PRIORITY_ALERT.
 *                      Default is LGLCD_PRIORITY_NORMAL.
 * RETURN VALUE 
 *  E_FAIL if there was an error.
 *  S_OK if no error.
 ******
 */
HRESULT CEzLcd::SetScreenPriority(DWORD dwPriority)
{
    m_output.SetScreenPriority(dwPriority);

    return S_OK;
}

/****f* EZ.LCD.Wrapper/CEzLcd::Update
 * NAME
 *  VOID CEzLcd::Update -- Update LCD display
 * FUNCTION
 *  Updates the display. Must be called every loop.
 ******
 */
VOID CEzLcd::Update()
{
    if (m_bInitNeeded)
    {
        // find the next active screen
        LCD_PAGE_LIST::iterator it = m_LCDPageList.begin();
        while(it != m_LCDPageList.end())
        {
			CEzLcdPage *pPage = *it;
            LCDUIASSERT(NULL != pPage);
			
			// Add the screen to list of screens that m_output manages
	        m_output.AddScreen(pPage);

            ++it;
        }

		// Make the active screen the one on the output
		m_output.UnlockScreen();
		m_output.LockScreen(m_pActivePage);

		m_bInitSucceeded = TRUE;

        m_bInitNeeded = FALSE;
    }

    // Only do stuff if initialization was successful. Otherwise 
    // IsConnected will simply return false.
    if (m_bInitSucceeded)
    {
        // Save copy of button state
        for (INT ii = 0; ii < NUMBER_SOFT_BUTTONS; ii++)
        {
            m_bButtonWasPressed[ii] = m_bButtonIsPressed[ii];
        }

        m_output.Update(GetTickCount());
        m_output.Draw();
    }
}


VOID CEzLcd::OnLCDButtonDown(INT nButton)
{
    switch(nButton)
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
}

VOID CEzLcd::OnLCDButtonUp(int nButton)
{
    switch(nButton)
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
}
