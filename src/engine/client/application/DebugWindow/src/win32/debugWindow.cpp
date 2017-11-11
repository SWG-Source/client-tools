// ======================================================================
//
// DebugWindow.cpp
//
// copyright 1999 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#define WIN32_LEAN_AND_MEAN
#pragma warning(disable: 4201 4514)
#include <windows.h>

// ======================================================================

template<class T>
inline const T& clamp(
	const T &minT,   // Minimum variable
	const T &t,      // Variable to clamp against min and max
	const T &maxT    // Minimum variable
	)
{
	return (t < minT) ? minT : ((t > maxT) ? maxT : t);
}

// ======================================================================

typedef void (*ChangedWindowCallback)(int x, int y, int width, int height);

#define DLLEXPORT __declspec(dllexport)
extern "C"
{
	DLLEXPORT bool install(int x, int y, int width, int height);
	DLLEXPORT void setChangedWindowCallback(ChangedWindowCallback);
	DLLEXPORT void remove(void);

	DLLEXPORT void setBehindWindow(HWND behindWindow);

	DLLEXPORT void showWindow();
	DLLEXPORT void hideWindow();

	DLLEXPORT void clearScreen(void);
	DLLEXPORT void clearToCursor(void);
	DLLEXPORT void gotoXY(int x, int y);
	DLLEXPORT void print(const char *string);
	DLLEXPORT void moveWindow(int x, int y);
}

static void home(void);
static void repaint(void);
static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ======================================================================

const char * const windowName = "debugWindow";
HWND               window;
HDC                hdc;
int                characterWidth;
int                characterHeight;
int                width;
int                height;
int                cursorX;
int                cursorY;
char              *text;
HBRUSH             blackBrush;
int                extraWidth;
int                extraHeight;
bool               closed = true;
ChangedWindowCallback changedWindowCallback;

// ======================================================================

bool install(int x, int y, int w, int h)
{
	WNDCLASSEX wclass;
	RECT       rect;
	ATOM       atom;
	BOOL       b;

	width = w;
	height = h;
	text = new char[width * height];

	blackBrush = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));

	// setup the window class
	memset(&wclass, 0, sizeof(wclass));
	wclass.cbSize        = sizeof(wclass);
	wclass.style         = CS_BYTEALIGNCLIENT;
	wclass.lpfnWndProc   = windowProc;
	wclass.hInstance     = NULL;
	wclass.hIcon         = NULL;
	wclass.hCursor       = NULL;
	wclass.hbrBackground = blackBrush;
	wclass.lpszClassName = windowName;
	wclass.hIconSm       = NULL;

	// register the window class
	atom = RegisterClassEx(&wclass);
	if (atom == 0)
		return false;

	// create the window
	window = CreateWindow(
		windowName,                                       // pointer to registered class name 
		windowName,                                       // pointer to window name 
		WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_SIZEBOX, // window style 
		x,                                                // horizontal position of window 
		y,                                                // vertical position of window 
		640,                                              // window width 
		480,                                              // window height 
		NULL,                                             // handle to parent or owner window 
    NULL,                                             // handle to menu or child-window identifier 
		NULL,                                             // handle to application instance 
		NULL);                                            // pointer to window-creation data 

	if (!window)
		return false;

	// get the DC to draw into
	hdc = GetDC(window);
	if (!hdc)
		return false;

	// set the stock fixed font
	SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));

	// set the font to be foreground white, background black and opaque
	SetTextColor(hdc, RGB(255, 255, 255));
	SetBkMode(hdc, OPAQUE);
	SetBkColor(hdc, RGB(0, 0, 0));

	// get the text metrics
	TEXTMETRIC textMetric;
	memset(&textMetric, 0, sizeof(textMetric));
	b = GetTextMetrics(hdc, &textMetric);
	if (!b)
		return false;

	characterWidth = textMetric.tmMaxCharWidth;
	characterHeight = textMetric.tmHeight + textMetric.tmExternalLeading;

	// make the window large enough to fit the text
	rect.left   = 0;
	rect.top    = 0;
	rect.right  = width * characterWidth;
	rect.bottom = height * characterHeight;
	b = AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_SIZEBOX, FALSE);
	if (!b)
		return false;

	extraWidth = (rect.right - rect.left) - (width * characterWidth);
	extraHeight = (rect.bottom - rect.top) - (height * characterHeight);

	b = SetWindowPos(window, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE | SWP_HIDEWINDOW);
	if (!b)
		return false;

	// clear the window
	memset(text, ' ', width * height);
	cursorX = 0;
	cursorY = 0;

	return true;
}

// ----------------------------------------------------------------------

void remove(void)
{
	// release the DC
	if (!closed)
	{
		static_cast<void>(ReleaseDC(window, hdc));
		DestroyWindow(window);
	}

	delete [] text;
}

// ----------------------------------------------------------------------

void showWindow()
{
	if (closed)
	{
		static_cast<void>(ShowWindow(window, SW_SHOW));
		closed = false;
		clearScreen();
	}
}

// ----------------------------------------------------------------------

void hideWindow()
{
	if (!closed)
	{
		static_cast<void>(ShowWindow(window, SW_HIDE));
		closed = true;
	}
}

// ----------------------------------------------------------------------

void setChangedWindowCallback(ChangedWindowCallback newChangedWindowCallback)
{
	changedWindowCallback = newChangedWindowCallback;
}

// ----------------------------------------------------------------------

static LRESULT CALLBACK windowProc(
	HWND   hwnd,   // Handle of window 
	UINT   uMsg,   // Message identifier 
	WPARAM wParam, // First message parameter 
	LPARAM lParam  // Second message parameter
	)
{
	switch (uMsg)
	{
		case WM_DESTROY:
			closed = true;
			return 0;

		case WM_SYSCOMMAND:
			if ((wParam & 0xFFF0) == SC_CLOSE)
			{
				hideWindow();
				return 0;
			}
			break;

		case WM_PAINT:
			{
				RECT        rect;
				PAINTSTRUCT paintStruct;
			
				if (GetUpdateRect(window, &rect, FALSE))
				{
					BeginPaint(window, &paintStruct);
					repaint();
					static_cast<void>(EndPaint(window, &paintStruct));
				}
			}
			return 0;

		case WM_SIZING:
			{
				// Force the window to size in full character increments
				RECT * r = reinterpret_cast<RECT *>(lParam);

				if (wParam == WMSZ_RIGHT || wParam == WMSZ_TOPRIGHT || wParam == WMSZ_BOTTOMRIGHT)
					r->right = r->left + ((((r->right - r->left) - extraWidth) / characterWidth) * characterWidth + extraWidth);
				else
					if (wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT)
						r->left = r->right - ((((r->right - r->left) - extraWidth) / characterWidth) * characterWidth + extraWidth);

				if (wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT)
					r->bottom = r->top + ((((r->bottom - r->top) - extraHeight) / characterHeight) * characterHeight + extraHeight);
				else
					if (wParam == WMSZ_TOP|| wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)
						r->top = r->bottom - ((((r->bottom - r->top) - extraHeight) / characterHeight) * characterHeight + extraHeight);

			}
			return 0;

		case WM_EXITSIZEMOVE:
			{
				RECT rect;
				GetWindowRect(hwnd, &rect);
				width = ((rect.right - rect.left) - extraWidth) / characterWidth;
				height = ((rect.bottom - rect.top) - extraHeight) / characterHeight;
				delete [] text;
				text = new char[width * height];
				clearScreen();

				if (changedWindowCallback)
					(*changedWindowCallback)(rect.left, rect.top, width, height);
			}
			break; 

		default:
			break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);	
}

// ----------------------------------------------------------------------

static void repaint(void)
{
	if (closed)
		return;

	for (int y = 0; y < height; ++y)
		static_cast<void>(TextOut(hdc, 0, y * characterHeight, text + (y * width), width));
}

// ----------------------------------------------------------------------

void setBehindWindow(HWND behindWindow)
{
	if (closed)
		return;

	static_cast<void>(SetWindowPos(window, behindWindow, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE));
	static_cast<void>(SetWindowPos(window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE));
}

// ----------------------------------------------------------------------

void clearScreen(void)
{
	if (closed)
		return;

	// clear the window
	RECT rect;
	rect.left   = 0;
	rect.top    = cursorY * characterHeight;
	rect.right  = width * characterWidth;
	rect.bottom = height * characterHeight;

	if (rect.top < rect.bottom)
		static_cast<void>(FillRect(hdc, &rect, blackBrush));

	memset(text, ' ', width * height);
	home();
}

// ----------------------------------------------------------------------

void clearToCursor(void)
{
	if (closed)
		return;

	clearScreen();
}

// ----------------------------------------------------------------------

void home(void)
{
	gotoXY(0, 0);
}

// ----------------------------------------------------------------------

void gotoXY(int x, int y)
{
	if (closed)
		return;

	cursorX = clamp(0, x, width-1);
	cursorY = clamp(0, y, height-1);
}

// ----------------------------------------------------------------------

bool newLine(void)
{
	cursorX = 0;

	if (cursorY == height-1)
	{
		// scroll the text up and clear the last line
		memmove(text, text + width, width * (height - 1));
		memset(text + (width * (height - 1)), ' ', width);

		return true;
	}
	
	++cursorY;
	return false;
}

// ----------------------------------------------------------------------

void print(const char *string)
{
	if (closed)
		return;

	int beginY = cursorY;
	bool doRepaint = false;

	for ( ; *string; ++string)
		if (*string == '\n')
			doRepaint = doRepaint || newLine();
		else
		{
			// handle normal character
			text[cursorY * width + cursorX] = *string;

			// advance the cursor
			if (cursorX == width-1)
				doRepaint = doRepaint || newLine();
			else
				++cursorX;
		}

	if (doRepaint)
		repaint();
	else
		// cursorX will be zero if the string is terminated with a newline or non-zero otherwise
		for (int i = beginY; i < cursorY + (cursorX ? 1 : 0); ++i)
			static_cast<void>(TextOut(hdc, 0, i * characterHeight, text + i * width, width));
}

// ======================================================================
