// ======================================================================
//
// StatusWindow.h
// Portions copyright 1999 Bootprint Entertainment, Inc.
// Portions copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_StatusWindow_H
#define INCLUDED_StatusWindow_H

// ======================================================================
/**
 * A simple windows-based status window with scroll support.
 */

class StatusWindow
{
public:

	enum
	{
		SWM_AppendLineAsynchronous = WM_USER
	};

public:

	static void install(HINSTANCE newOwnerInstance);

	explicit StatusWindow(bool destroySendsQuit, const char *title = 0, bool disableCloseBox = false, bool makeTopMost = true, bool newJumpToNewOutput = true, int scrollbackLineCount = 0);
	~StatusWindow(void);

	void appendLine(const char *newLine);
	void sendLineAsynchronous (char *line);

	void refreshWindow(void);

	void getWindowPlacement(WINDOWPLACEMENT *placement);
	void setWindowPlacement(const WINDOWPLACEMENT *placement);

	HWND getWindowHandle(void);

	virtual LRESULT windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

	typedef stddeque<std::string>::fwd  StringDeque;

private:

	static void             remove(void);
	static LRESULT CALLBACK globalWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

	LRESULT  handleDestroy(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT  handleSize(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT  handlePaint(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT  handleHScroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT  handleVScroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT  handleKeyDown(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT  handleAppendLineAsynchronous(HWND hwnd, WPARAM wParam, LPARAM lParam);

	void     moveToLineNumber(int newLineNumber);
	void     updateScrollBar();

	// Disabled.
	StatusWindow();
	StatusWindow(const StatusWindow&);
	StatusWindow &operator =(const StatusWindow&);

private:

	static const char *const cms_windowClassName;
	static const char *const cms_windowClassNameNoClose;

	static const int         cms_defaultScrollbackLineCount;

	static bool              ms_installed;
	static HINSTANCE         ms_ownerInstance;

private:

	HWND         m_hwndStatus;
	bool         m_sendQuitOnClose;
	bool         m_jumpToNewOutput;
	int          m_fontHeight;
	int          m_leftBorder;
	HFONT        m_font;

	StringDeque *m_strings;

	int          m_currentLineNumber;
	int          m_pageSkipLineCount;
	int          m_firstVisibleCharacterIndex;

	int          m_scrollbackLineCount;
	int          m_windowHeight;

};

// ======================================================================

inline HWND StatusWindow::getWindowHandle(void)
{
	return m_hwndStatus;
}

// ======================================================================

#endif
