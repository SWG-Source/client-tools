//
// FractalPreviewView.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FractalPreviewView.h"

#include "CDib.h"
#include "MainFrame.h"
#include "sharedFractal/MultiFractal.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FractalPreviewView, CView)

//-------------------------------------------------------------------

FractalPreviewView::FractalPreviewView() :
	CView (),
	m_dib (0),
	m_dibSize (192)
{
}

//-------------------------------------------------------------------

FractalPreviewView::~FractalPreviewView()
{
	if (m_dib)
		delete m_dib;
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FractalPreviewView, CView)
	//{{AFX_MSG_MAP(FractalPreviewView)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void FractalPreviewView::OnDraw(CDC* pDC)
{
	CRect rect;
	GetClientRect (&rect);

	NOT_NULL (m_dib);

	IGNORE_RETURN (m_dib->Draw (pDC, rect.TopLeft (), rect.Size ()));
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void FractalPreviewView::AssertValid() const
{
	CView::AssertValid();
}

void FractalPreviewView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FractalPreviewView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	//-- create the bitmap
	{
		//-- Create a compatible DC
		CDC* pDC = GetDC();
		ASSERT (pDC);
		CDC dcCompat;
		IGNORE_RETURN (dcCompat.CreateCompatibleDC (pDC));
		IGNORE_RETURN (ReleaseDC(pDC));

		// Allocate memory for the bitmap
		m_dib = new CDib (CSize (m_dibSize, m_dibSize), 24);
		VERIFY (m_dib->CreateSection (&dcCompat));
	}
}

//-------------------------------------------------------------------

BOOL FractalPreviewView::OnEraseBkgnd(CDC* pDC) 
{
	UNREF (pDC);

	return true;
}

//-------------------------------------------------------------------

void FractalPreviewView::updateBitmap (const MultiFractal& multiFractal, float low, float high)
{
	//-- update the bitmap
	__int64 start;
	IGNORE_RETURN (QueryPerformanceCounter (reinterpret_cast<LARGE_INTEGER*> (&start)));

	//-- fill it
	NOT_NULL (m_dib);
	BYTE*      dibits  = m_dib->m_lpImage;
	const int  dibsize = m_dibSize;
	const real zoom    = RECIP (GetMainFrame ()->getZoomLevel ());

	int x;
	int y;
	for (y = 0; y < dibsize; y++)
		for (x = 0; x < dibsize; x++)
		{
			const real dx =  static_cast<real> (x) - (static_cast<real> (dibsize) * 0.5f);
			const real dy =  static_cast<real> (y) - (static_cast<real> (dibsize) * 0.5f);

			const real sample = multiFractal.getValue (zoom * dx, zoom * dy);
			const bool clampedLow  = sample < low;
			const bool clampedHigh = sample > high;
			const real v = clamp (low, sample, high);

			const uint8 c = static_cast<uint8> (v * 255.0f);
			*(dibits++) = clampedLow ? static_cast<uint8> (0) : clampedHigh ? static_cast<uint8> (0) : c;  //-- b
			*(dibits++) = clampedLow ? static_cast<uint8> (0) : clampedHigh ? static_cast<uint8> (0) : c;  //-- g
			*(dibits++) = clampedLow ? static_cast<uint8> (0) : clampedHigh ? static_cast<uint8> (0) : c;  //-- r
		}

	__int64 end;
	IGNORE_RETURN (QueryPerformanceCounter (reinterpret_cast<LARGE_INTEGER*> (&end)));

	__int64 frequency;
	IGNORE_RETURN (QueryPerformanceFrequency (reinterpret_cast<LARGE_INTEGER*> (&frequency)));

	const real last = (static_cast<float> (end) - static_cast<float> (start)) / static_cast<float> (frequency);
	UNREF (last);

	Invalidate ();
}

//-------------------------------------------------------------------

