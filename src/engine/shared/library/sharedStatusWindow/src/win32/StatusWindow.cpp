// ======================================================================
//
// StatusWindow.cpp
// Portions copyright 1999 Bootprint Entertainment, Inc.
// Portions copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedStatusWindow/FirstSharedStatusWindow.h"
#include "sharedStatusWindow/StatusWindow.h"

#include "sharedFoundation/ExitChain.h"

#include <algorithm>
#include <deque>
#include <string>

// ======================================================================

const int MAX_HORIZONTAL_SCROLL = 1024;

const char *const StatusWindow::cms_windowClassName             = "Verant.StatusWindow.Class";
const char *const StatusWindow::cms_windowClassNameNoClose      = "Verant.StatusWindow.Class.NoClose";
const int         StatusWindow::cms_defaultScrollbackLineCount  = 512;


bool                StatusWindow::ms_installed;
HINSTANCE           StatusWindow::ms_ownerInstance;

// ======================================================================

void StatusWindow::install(HINSTANCE newOwnerInstance)
{
	DEBUG_FATAL(ms_installed, ("StatusWindow already ms_installed"));

	ms_ownerInstance = newOwnerInstance;

	WNDCLASSEX  wc;
	ATOM        classAtom;

	// register regular window class
	Zero(wc);
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = globalWindowProc;
	wc.hInstance     = ms_ownerInstance;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wc.lpszClassName = cms_windowClassName;

	classAtom = RegisterClassEx(&wc);
	FATAL(!classAtom, ("failed to register StatusWindow class"));

	// register no-close window class
	wc.style        |= CS_NOCLOSE;
	wc.lpszClassName = cms_windowClassNameNoClose;

	classAtom = RegisterClassEx(&wc);
	FATAL(!classAtom, ("failed to register StatusWindow class (no close)"));

	ms_installed = true;
	ExitChain::add(remove, "StatusWindow::remove");
}

// ----------------------------------------------------------------------

void StatusWindow::remove(void)
{
	DEBUG_FATAL(!ms_installed, ("StatusWindow not ms_installed"));
	ms_installed = false;

	BOOL result;

	// unregister the window class
	result = UnregisterClass(cms_windowClassName, ms_ownerInstance);

	// might fail if some status window windows are still open
	// -TRF- add StatusWindow users count so we can bug user if StatusWindows are still opened
	DEBUG_FATAL(!result, ("failed to unregister StatusWindow class"));

	// unregister the window class
	result = UnregisterClass(cms_windowClassNameNoClose, ms_ownerInstance);
}

// ----------------------------------------------------------------------

LRESULT CALLBACK StatusWindow::globalWindowProc(
	HWND   hwnd,
	UINT   uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	StatusWindow *statusWindow = reinterpret_cast<StatusWindow*>(GetWindowLong(hwnd, GWL_USERDATA));
	if (!statusWindow)
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	else
		return statusWindow->windowProc(hwnd, uMsg, wParam, lParam);
}

// ----------------------------------------------------------------------

StatusWindow::StatusWindow(bool closeSendsQuit, const char *title, bool disableCloseBox, bool makeTopMost, bool newJumpToNewOutput, int scrollbackLineCount) :
	m_hwndStatus(0),
	m_sendQuitOnClose(closeSendsQuit),
	m_jumpToNewOutput(newJumpToNewOutput),
	m_fontHeight(0),
	m_leftBorder(0),
	m_font(0),
	m_strings(new StringDeque()),
	m_currentLineNumber(0),
	m_pageSkipLineCount(0),
	m_firstVisibleCharacterIndex(0),
	m_scrollbackLineCount((scrollbackLineCount > 1) ? scrollbackLineCount : cms_defaultScrollbackLineCount),
	m_windowHeight(1024)
{
	DWORD exFlags = WS_EX_OVERLAPPEDWINDOW;
	if (makeTopMost)
		exFlags |= WS_EX_TOPMOST;

	const char *windowName      = title ? title : "Status Window";
	const char *const className = disableCloseBox ? cms_windowClassNameNoClose : cms_windowClassName;

	// create the status window
	m_hwndStatus = CreateWindowEx(
		exFlags,
		className,
		windowName,
		WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		ms_ownerInstance,
		0);
	FATAL(!m_hwndStatus, ("failed to create status window"));

	// save object (this) in window's user data area
	SetLastError(0);
	const LONG swlResult = SetWindowLong(m_hwndStatus, GWL_USERDATA, reinterpret_cast<LONG>(this));
	FATAL (!swlResult && GetLastError(), ("failed to set user data for StatusWindow"));


	// get m_font information

	TEXTMETRIC tm;
	BOOL       rc;

	HDC hdc = GetWindowDC(m_hwndStatus);
	DEBUG_FATAL(!hdc, ("failed to retrieve window dc"));

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));

	// -TRF- get this from configuration
	const int desiredFontPointSize = 8;

	lf.lfHeight        = -MulDiv(desiredFontPointSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	lf.lfWeight        = FW_NORMAL;
	lf.lfCharSet       = ANSI_CHARSET;
  lf.lfOutPrecision  = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality       = DEFAULT_QUALITY;
	strcpy(lf.lfFaceName, "Courier New");

	m_font = CreateFontIndirect(&lf);
	FATAL(!m_font, ("failed to create required m_font \"Courier New\""));

	HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, m_font));
	Zero(tm);
	rc = GetTextMetrics(hdc, &tm);
	DEBUG_FATAL(!rc, ("StatusWindow::handleCreate GetTextMetrics() failed"));

	m_fontHeight = tm.tmHeight + tm.tmExternalLeading;
	m_leftBorder = tm.tmMaxCharWidth;

	static_cast<void>(SelectObject(hdc, oldFont));

	ReleaseDC(m_hwndStatus, hdc);

	updateScrollBar();

	// display the status window
	ShowWindow(m_hwndStatus, SW_SHOWDEFAULT);

	// Update the window
	rc = UpdateWindow(m_hwndStatus);
	DEBUG_FATAL(!rc, ("StatusWindow::install UpdateWindow failed"));

	// add an initial line for a border
	appendLine("");
}

// ----------------------------------------------------------------------

StatusWindow::~StatusWindow(void)
{
	delete m_strings;
}

// ----------------------------------------------------------------------

LRESULT StatusWindow::handleDestroy(
	HWND   hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	UNREF(hwnd);
	UNREF(wParam);
	UNREF(lParam);

	DeleteObject(m_font);
	m_font = 0;

	if (m_sendQuitOnClose)
		PostQuitMessage(0);
	return 0;
}

// ----------------------------------------------------------------------

LRESULT StatusWindow::handleSize(
	HWND   hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	UNREF(hwnd);
	UNREF(wParam);

	WORD height = HIWORD(lParam);

	// this will round down, which is what we want, -1 for line of context
	DEBUG_FATAL(m_fontHeight <= 1, ("invalid m_fontHeight %d", m_fontHeight));
	m_pageSkipLineCount = static_cast<int>(height) / m_fontHeight - 1; //lint !e414 // possible division by 0 // huh? look at line above

	return 0;
}

// ----------------------------------------------------------------------

LRESULT StatusWindow::handlePaint(
	HWND   hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	UNREF(wParam);
	UNREF(lParam);

	PAINTSTRUCT ps;
	HDC         hdc;
	BOOL        rc;

	hdc = BeginPaint(hwnd, &ps);
	DEBUG_FATAL(!hdc, ("StatusWindow::handlePaint BeginPaint failed"));

	HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, m_font));

	// set the new text color
	COLORREF oldColor  = SetTextColor(hdc, RGB(127, 127, 127));
	int      oldBkMode = SetBkMode(hdc, TRANSPARENT);

	const StringDeque::iterator endIt = m_strings->end();
	StringDeque::iterator       it    = m_strings->begin() + m_currentLineNumber;

	for (int y = 0; (it != endIt) && (y < m_windowHeight); ++it, y += m_fontHeight)
	{
		//-- Skip printing lines that are shorter than the first output character.
		const std::string &text  = *it;
		const char *const  cText = text.c_str();

		const int length = static_cast<int>(text.length());
		if (m_firstVisibleCharacterIndex < length)
		{
			//-- Start printing from the first output character (used by horizontal scroll mechanism).
			rc = TextOut(hdc, m_leftBorder, y, cText + m_firstVisibleCharacterIndex, static_cast<int>(strlen(cText + m_firstVisibleCharacterIndex)));
			DEBUG_FATAL(!rc, ("StatusWindow::handlePaint TextOut failed"));
		}
	}

	// reset the text color
	static_cast<void>(SetTextColor(hdc, oldColor));
	static_cast<void>(SetBkMode(hdc, oldBkMode));

	static_cast<void>(SelectObject(hdc, oldFont));

	rc = EndPaint(hwnd, &ps);
	DEBUG_FATAL(!rc, ("StatusWindow::handlePaint EndPaint() failed"));
	return 0;
}

// ----------------------------------------------------------------------

LRESULT StatusWindow::handleHScroll(
	HWND   hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	UNREF(hwnd);
	UNREF(lParam);

	int   nScrollCode = static_cast<int>(LOWORD(wParam));
	short nPos = static_cast<short>(HIWORD(wParam));

	switch(nScrollCode)
	{
	case SB_BOTTOM:
		m_firstVisibleCharacterIndex = MAX_HORIZONTAL_SCROLL;
		break;
	case SB_LINEDOWN:
		m_firstVisibleCharacterIndex = std::min(m_firstVisibleCharacterIndex + 1, MAX_HORIZONTAL_SCROLL);
		break;
	case SB_LINEUP:
		m_firstVisibleCharacterIndex = std::max(m_firstVisibleCharacterIndex - 1, 0);
		break;
	case SB_PAGEDOWN:
		m_firstVisibleCharacterIndex = std::min(m_firstVisibleCharacterIndex + MAX_HORIZONTAL_SCROLL/10, MAX_HORIZONTAL_SCROLL);
		break;
	case SB_PAGEUP:
		m_firstVisibleCharacterIndex = std::max(m_firstVisibleCharacterIndex - MAX_HORIZONTAL_SCROLL/10, 0);
		break;
	case SB_THUMBPOSITION:
		m_firstVisibleCharacterIndex = nPos;
		break;
	case SB_THUMBTRACK:
		m_firstVisibleCharacterIndex = nPos;
		break;
	case SB_TOP:
		m_firstVisibleCharacterIndex = 0;
		break;
	case SB_ENDSCROLL:
		// nothing to do
		break;
	default:
		DEBUG_FATAL(true, ("unhandled scroll code %d", nScrollCode));
	}

	updateScrollBar();
	refreshWindow();

	return 0;
}

// ----------------------------------------------------------------------

LRESULT StatusWindow::handleVScroll(
	HWND   hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	UNREF(hwnd);
	UNREF(lParam);

	int   nScrollCode = static_cast<int>(LOWORD(wParam));
	short nPos = static_cast<short>(HIWORD(wParam));

	switch(nScrollCode)
	{
	case SB_BOTTOM:
		moveToLineNumber(static_cast<int>(m_strings->size()) - 1);
		break;
	case SB_LINEDOWN:
		moveToLineNumber(m_currentLineNumber + 1);
		break;
	case SB_LINEUP:
		moveToLineNumber(m_currentLineNumber - 1);
		break;
	case SB_PAGEDOWN:
		moveToLineNumber(m_currentLineNumber + m_pageSkipLineCount);
		break;
	case SB_PAGEUP:
		moveToLineNumber(m_currentLineNumber - m_pageSkipLineCount);
		break;
	case SB_THUMBPOSITION:
		moveToLineNumber(nPos);
		break;
	case SB_THUMBTRACK:
		moveToLineNumber(nPos);
		break;
	case SB_TOP:
		moveToLineNumber(0);
		break;
	case SB_ENDSCROLL:
		// nothing to do
		break;
	default:
		DEBUG_FATAL(true, ("unhandled scroll code %d", nScrollCode));
	}
	return 0;
}

// ----------------------------------------------------------------------

LRESULT StatusWindow::handleKeyDown(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	UNREF(hwnd);
	UNREF(lParam);

	int keyCode = static_cast<int>(wParam);
	switch (keyCode)
	{
		case VK_DOWN:
			moveToLineNumber(m_currentLineNumber + 1);
			return 0;
		case VK_UP:
			moveToLineNumber(m_currentLineNumber - 1);
			return 0;
		case VK_PRIOR:
			moveToLineNumber(m_currentLineNumber - m_pageSkipLineCount);
			return 0;
		case VK_NEXT:
			moveToLineNumber(m_currentLineNumber + m_pageSkipLineCount);
			return 0;
		default:
			// we didn't process it
			return -1;
	}
}

// ----------------------------------------------------------------------

LRESULT StatusWindow::handleAppendLineAsynchronous(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	UNREF (hwnd);
	UNREF (wParam);

	char *line = reinterpret_cast<char *>(lParam);
	appendLine(line);

	delete [] line;
	return 0;
}

// ----------------------------------------------------------------------

void StatusWindow::appendLine(const char *newLine)
{
	//-- Add line to deque.
	m_strings->push_back(std::string(newLine));

	//-- Remove first entry in deque if we exceeded the scrollback line count.
	if (static_cast<int>(m_strings->size()) > m_scrollbackLineCount)
		m_strings->pop_front();

	
	if (m_jumpToNewOutput)
	{
		//-- Move so last line is visible.
		moveToLineNumber((static_cast<int>(m_strings->size()) - 1) - m_pageSkipLineCount);
	}
	else
	{
		updateScrollBar();
		refreshWindow();
	}
}

// ----------------------------------------------------------------------

void StatusWindow::moveToLineNumber(int newLineNumber)
{
	//-- Clamp to range.
	const int lineCount = static_cast<int>(m_strings->size());

	if (newLineNumber >= (lineCount - 1))
		newLineNumber = lineCount - 1;
	if (newLineNumber < 0)
		newLineNumber = 0;

	//-- Save the adjusted line number.
	m_currentLineNumber = newLineNumber;

	//-- Update the window for changes.
	updateScrollBar();
	refreshWindow();
}

// ----------------------------------------------------------------------

void StatusWindow::updateScrollBar(void)
{
	const int lineCount = static_cast<int>(m_strings->size());

	BOOL rc;

	rc = SetScrollRange(m_hwndStatus, SB_VERT, 0, lineCount ? lineCount - 1 : 0, FALSE);
	DEBUG_FATAL(!rc, ("StatusWindow::updateScrollBar failed"));

	rc = SetScrollRange(m_hwndStatus, SB_HORZ, 0, MAX_HORIZONTAL_SCROLL, FALSE);
	DEBUG_FATAL(!rc, ("StatusWindow::updateScrollBar failed"));

	SetScrollPos(m_hwndStatus, SB_VERT, m_currentLineNumber, TRUE);
	SetScrollPos(m_hwndStatus, SB_HORZ, m_firstVisibleCharacterIndex, TRUE);
}

// ----------------------------------------------------------------------
/**
 * send line to the status window for printing.
 * 
 * The line will be appended to the status window.
 * 
 * This routine is thread-friendly.  Another thread can call this routine
 * and it will work as expected.
 * 
 * StatusWindow owns the string and will delete it as an array of characters.
 * 
 * The routine returns as soon as possible, possibly before the print
 * request is handled.
 */

void StatusWindow::sendLineAsynchronous (char *line)
{
	PostMessage (m_hwndStatus, SWM_AppendLineAsynchronous, 0, reinterpret_cast<LPARAM>(line));
}

// ----------------------------------------------------------------------

void StatusWindow::refreshWindow(void)
{
	const BOOL rc = InvalidateRect(m_hwndStatus, 0, TRUE);
	UNREF (rc);
	DEBUG_FATAL(!rc, ("StatusWindow::refreshWindow failed"));
}

// ----------------------------------------------------------------------

void StatusWindow::getWindowPlacement(WINDOWPLACEMENT *placement)
{
	DEBUG_FATAL(!m_hwndStatus, ("null m_hwndStatus"));
	const BOOL result = GetWindowPlacement(m_hwndStatus, placement);
	UNREF(result);
	DEBUG_FATAL(!result, ("GetWindowPlacement failed"));
}

// ----------------------------------------------------------------------

void StatusWindow::setWindowPlacement(const WINDOWPLACEMENT *placement)
{
	DEBUG_FATAL(!m_hwndStatus, ("null m_hwndStatus"));
	const BOOL result = SetWindowPlacement(m_hwndStatus, placement);
	UNREF(result);
	DEBUG_FATAL(!result, ("SetWindowPlacement failed"));
}

// ----------------------------------------------------------------------
/**
 * WindowProc for the window.
 * 
 * Override this function if you want to provide customization to the
 * handling of window messages.  Anything you don't handle should be
 * passed back to this function for handling.
 * 
 * This routine will NOT receive a WM_CREATE.  You must do an processing
 * for that routine within the constructor.  So long as the StatusWindow::StatusWindow()
 * constructor is called, the m_hwndStatus member will be correctly set to the window's
 * HWND.
 */

LRESULT StatusWindow::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DESTROY:
			return handleDestroy(hwnd, wParam, lParam);

		case WM_SIZE:
			return handleSize(hwnd, wParam, lParam);

		case WM_PAINT:
			return handlePaint(hwnd, wParam, lParam);

		case WM_CLOSE:
			DestroyWindow(hwnd);
			m_hwndStatus = 0;
			return 0;

		case WM_HSCROLL:
			return handleHScroll(hwnd, wParam, lParam);

		case WM_VSCROLL:
			return handleVScroll(hwnd, wParam, lParam);

		case WM_KEYDOWN:
			return handleKeyDown(hwnd, wParam, lParam);

		case SWM_AppendLineAsynchronous:
			return handleAppendLineAsynchronous (hwnd, wParam, lParam);

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

// ======================================================================
