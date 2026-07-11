/*
   Original file from DEv-C++ Project:
    	Name: Example\WinAnim
   		Author: Brook Miles
   		Description: Making an animation in windows
   
   Extension to progress by Peter Kuemmel
   
*/

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


int Files_To_Compile = 493;


static char g_szClassName[] = "Progress";
static HINSTANCE g_hInst = NULL;

const UINT idTimer1 = 1;
const UINT idTimer2 = 2;

UINT nTimerDelay1 = 30;
UINT nTimerDelay2 = 500;

int deltaValue = 1;
int deltaValueBuild = 10;

HBITMAP hbmLogo;
BITMAP bm;

int logoX, logoY;
int deltaX, deltaY;



char CountDir[1024];

bool stepMode = false;
//bool stepMode = true;
bool started = false;

float startProg;
float startCount;
float startTime;
float startTProg;

float lastBuild = 0;

UINT FastCountFiles( char* pcszDir, bool bRecurse );

void UpdatePos(HWND hwnd)
{
	RECT rc;
	GetClientRect(hwnd, &rc);

	logoX += deltaX;

	logoY = rc.bottom /2;

	if(logoX < 0)
	{
		logoX = 0;
		deltaX = deltaValue;
	}
	else if(logoX + bm.bmWidth > rc.right)
	{
		logoX = rc.right - bm.bmWidth;
		deltaX = -deltaValue;
	}
	if(logoY < 0)
	{
		logoY = 0;
		deltaY = deltaValue;
	}
	else if(logoY + bm.bmHeight > rc.bottom)
	{
		logoY = rc.bottom - bm.bmHeight;
		deltaY = -deltaValue;
	}
}


void DrawBuffered(HWND hwnd, HDC hdc)
{
	HDC buffer = CreateCompatibleDC( hdc );

	RECT rc;
	GetClientRect (hwnd, &rc);
	int w = rc.right - rc.left;
	int h = bm.bmHeight + 1 ;
	HBITMAP memBM = CreateCompatibleBitmap( hdc, w, h );
	SelectObject ( buffer, memBM );

	FillRect(buffer, &rc,(HBRUSH) (COLOR_BTNFACE+1));

	UpdatePos(hwnd);

	HDC hdcMemory = CreateCompatibleDC( hdc );
	SelectObject(hdcMemory, hbmLogo);
	BitBlt(buffer, logoX, 0, bm.bmWidth, bm.bmHeight, hdcMemory, 0, 0, SRCCOPY);

	BitBlt( hdc, rc.left, rc.bottom-h, w, h, buffer, 0, 0, SRCCOPY);

	DeleteDC(hdcMemory);
	DeleteDC(buffer);
	DeleteObject(memBM);
}

void DrawProgress( HWND hwnd, HDC hdc)
{
	int count= FastCountFiles( CountDir, 1 );

	float build = count - startCount;
	float now = GetTickCount();
	float left;

	if( !started && count > startCount+1 )
	{
		started = true;
		startTime = GetTickCount();
		if( stepMode )
		{
			deltaValue = deltaValueBuild;
			deltaX = deltaValueBuild;
			deltaY = deltaValueBuild;
		}
	}

	if( !started )
	{
		left = now - startTProg;
	}
	else
	{
		float perFile = ( now - startTime )/ build;
		left = ( Files_To_Compile - build) * perFile ;
	}

	left = left / 1000;
	int min = int(left / 60.0);
	int sec = int(fmod(left, 60.0));
	static int minold = 0;

	char text[1024];
	char minStr[65];
	char secStr[65];
	char buildStr[65];
	_itoa( min, minStr, 10 );
	_itoa( sec, secStr, 10 );
	_itoa( int(build), buildStr, 10 );

	strcpy(text,"Waiting for library build process ...");
    
	if( !started )
	{
		if( min < 1.0 && sec>20 )
		{
			strcpy(text," Waiting since ");
			strcat(text,secStr);
			strcat(text," seconds ...");
		}
		if( min >= 1.0 )
		{
			strcpy(text," Waiting since ");
			strcat(text,minStr);
			if( min == 1)
				strcat(text," minute for library build process. ");
			else
				strcat(text," minutes for library build process. ");
		}
	}
	else
	{
		if( build > 1)
		{
			strcpy(text,"  ");
			strcat(text,buildStr);
			strcat(text," files done.  ");
		}
		if( build > 12)
		{
			if( min == minold+1 ) 
				_itoa( minold, minStr, 10 ); // replace noise
			else
				minold = min;
				
				strcat(text," Time left:  ");
				strcat(text,minStr);
				strcat(text," min  ");
				
		}
		if( build > 150)
		{
			strcat(text,secStr);
			strcat(text," sec ");
		}

	}

	if( started && build >= Files_To_Compile )
	{
		min = int( floor((now - startTime)/60.0/1000.0 +.5) );
		_itoa( min, minStr, 10 );
		strcpy(text,"");
		strcat(text,"  Compiling done. ( ");
		strcat(text,minStr);
		strcat(text," minutes) ");
	}

	PAINTSTRUCT ps;
	BeginPaint (hwnd, &ps);
	RECT rc;
	GetClientRect (hwnd, &rc);

	rc.bottom = 40;

	DrawText (hdc, "                                                                                                                                               "
	          , -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	DrawText (hdc, text, -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	if( build-lastBuild > 0 && started && stepMode )
		DrawBuffered( hwnd, hdc );

	lastBuild = build;
	EndPaint (hwnd, &ps);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	switch(Message)
	{
	case WM_CREATE:
		hbmLogo = LoadBitmap(g_hInst, "QTLOGOBMP");
		if(!hbmLogo )
		{
			MessageBox(hwnd, "Load of resources failed.", "Error",
			           MB_OK | MB_ICONEXCLAMATION);
			return -1;
		}

		GetObject(hbmLogo, sizeof(bm), &bm);
		SetTimer(hwnd, idTimer1, nTimerDelay1, NULL);
		SetTimer(hwnd, idTimer2, nTimerDelay2, NULL);

		logoX = 0;
		logoY = 0;
		deltaX = deltaValue;
		deltaY = deltaValue;

		break;

	case WM_TIMER:
		if(hbmLogo )
		{
			HDC hdcWindow;
			hdcWindow = GetDC( hwnd );

			if( wParam == idTimer2 )
				DrawProgress(hwnd, hdcWindow);

			if( wParam == idTimer1 &&  ( !started || !stepMode ) )
				DrawBuffered( hwnd, hdcWindow );

			ReleaseDC(hwnd, hdcWindow);
		}
		break;

	case WM_PAINT:
		if(hbmLogo)
		{
			PAINTSTRUCT ps;
			HDC hdcWindow;
			hdcWindow = BeginPaint(hwnd, &ps);

			DrawProgress( hwnd, hdcWindow );
			DrawBuffered( hwnd, hdcWindow );

			EndPaint(hwnd, &ps);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		KillTimer(hwnd, idTimer1);
		KillTimer(hwnd, idTimer2);
		DeleteObject(hbmLogo);
		PostQuitMessage(0);
		break;

	case WM_SIZE:
		{
			HDC hdcWindow = GetDC(hwnd);
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			RECT rc;
			GetClientRect (hwnd, &rc);
			FillRect(hdcWindow, &rc, (HBRUSH)(COLOR_BTNFACE+1));
			EndPaint(hwnd, &ps);
			ReleaseDC(hwnd, hdcWindow);
		}
		break;

	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}


int WINAPI WinMain
(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX WndClass;
	HWND hwnd;
	MSG Msg;

	g_hInst = hInstance;
	memset (&WndClass, 0, sizeof(WNDCLASSEX));
	WndClass.cbSize        = sizeof(WNDCLASSEX);
	WndClass.lpszClassName = g_szClassName;
	WndClass.style         = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc   = WndProc;
	WndClass.cbClsExtra    = 0;
	WndClass.cbWndExtra    = 0;
	WndClass.hInstance     = g_hInst;
	WndClass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
	WndClass.lpszMenuName  = NULL;
	WndClass.lpszClassName = g_szClassName;
	WndClass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&WndClass))
	{
		MessageBox(0, "Window Registration Failed!", "Error!",
		           MB_ICONEXCLAMATION | MB_OK | MB_SYSTEMMODAL);
		return 0;
	}

	hwnd = CreateWindowEx(
	           WS_EX_CLIENTEDGE,
	           g_szClassName,
	           "Qt Libray Build Progress",
	           WS_OVERLAPPEDWINDOW,
	           CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
	           NULL, NULL, g_hInst, NULL);

	if(hwnd == NULL)
	{
		MessageBox(0, "Window Creation Failed!", "Error!",
		           MB_ICONEXCLAMATION | MB_OK | MB_SYSTEMMODAL);
		return 0;
	}

	char* qtdir = getenv( "QTDIR" );
	char* objdir= "\\src\\tmp\\obj\\";
	if( strlen(qtdir)+strlen(objdir) < 1023)
	{
		strcat(CountDir,qtdir);
		strcat(CountDir,objdir);
	}
	startCount = FastCountFiles( CountDir,1 );
	startTProg = GetTickCount();

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	HDC hdc = GetDC( hwnd );
	DrawProgress( hwnd, hdc);
	DeleteDC( hdc );

	while(GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}


UINT FastCountFiles( char* pcszDir, bool bRecurse )
{
	UINT uCount = 0;
	WIN32_FIND_DATA wfd;
	char szSpec[ MAX_PATH + MAX_PATH ];

	strcpy( szSpec, pcszDir );
	strcat( szSpec, ("\\*.*") );

	//_tcscpy( szSpec, pcszDir );
	//_tcscat( szSpec, _T("\\*.*") );


	HANDLE hFind = FindFirstFileA( szSpec, &wfd );
	if( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if( bRecurse )
				{
					if( wfd.cFileName[ 0 ] != '.'
					        && ( wfd.cFileName[ 1 ] != '\000' || ( wfd.cFileName[ 2 ] != '.' &&
					                                               wfd.cFileName[ 3 ] != '\000' ) ) )
					{

						//_tcscpy( szSpec, pcszDir );
						//_tcscat( szSpec, _T("\\") );
						//_tcscat( szSpec, wfd.cFileName );

						strcpy( szSpec, pcszDir );
						strcat( szSpec, ("\\") );
						strcat( szSpec, wfd.cFileName );
						uCount += FastCountFiles( szSpec, bRecurse );

					}
				}
			}
			else
			{
				uCount++;
			}
		}
		while( FindNextFileA( hFind, &wfd ) ); FindClose( hFind );
	}

	return uCount;

}
