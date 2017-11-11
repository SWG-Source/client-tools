//
// BitmapPreviewView.cpp
//
// copyright 2004, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "BitmapPreviewView.h"

#include "CDib.h"
#include "MainFrame.h"
#include "TerrainEditor.h"
#include "TerrainEditorDoc.h"
#include "sharedImage/Image.h"

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(BitmapPreviewView, CView)

//-------------------------------------------------------------------

BitmapPreviewView::BitmapPreviewView() :
	CView (),
	m_dib (0),
	m_dibSize (192)
{
}

//-------------------------------------------------------------------

BitmapPreviewView::~BitmapPreviewView()
{
	if (m_dib)
		delete m_dib;
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(BitmapPreviewView, CView)
	//{{AFX_MSG_MAP(BitmapPreviewView)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

void BitmapPreviewView::OnDraw(CDC* pDC)
{
	CRect rect;
	GetClientRect (&rect);


	NOT_NULL (m_dib);

	IGNORE_RETURN (m_dib->Draw (pDC, rect.TopLeft (), rect.Size ()));
}

//-------------------------------------------------------------------

#ifdef _DEBUG
void BitmapPreviewView::AssertValid() const
{
	CView::AssertValid();
}

void BitmapPreviewView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void BitmapPreviewView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	TerrainEditorDoc* terrainDocument = GetApp()->getDocument();
	int initialFamId = terrainDocument->getTerrainGenerator()->getBitmapGroup().getFamilyId(0);
	Image* image = terrainDocument->getTerrainGenerator()->getBitmapGroup().getFamilyBitmap(initialFamId);
	CreateDib(image);

}

//-------------------------------------------------------------------


void BitmapPreviewView::CreateDib(const Image* image)
{

	if (m_dib)
	{
		delete m_dib;
		m_dib = 0;
	}

	//-- Create a compatible DC
	CDC* pDC = GetDC();
	ASSERT (pDC);
	CDC dcCompat;
	IGNORE_RETURN (dcCompat.CreateCompatibleDC (pDC));
	IGNORE_RETURN (ReleaseDC(pDC));

	// Allocate memory for the bitmap
	if(image)
	{
		m_dib = new CDib (CSize (image->getWidth (), image->getHeight ()), 24);
	}
	else
	{
		m_dib = new CDib (CSize (64,64), 24);
		::MessageBox(NULL,"BitmapPreviewView::CreateDib - image null","error",MB_OK);
		return;
	}
	VERIFY (m_dib->CreateSection (&dcCompat));

	const uint8* data = image->lockReadOnly ();
	NOT_NULL (data);

	BYTE* dibits = m_dib->m_lpImage;
	NOT_NULL (dibits);

	int imageStride = image->getStride();
	int destStride = imageStride * 3;

	int y;
	const int imageHeight = image->getHeight();
	const int imageWidth = image->getWidth();
	for(y = 0; y < imageHeight; ++y)
	{
		int x;
		for (x = 0; x < imageWidth; ++x)
		{
			int destIndex = (y * destStride) + x*3;
			int sourceIndex = (imageHeight - y - 1) * imageStride + x ; // bottom up	
			dibits[destIndex] = data[sourceIndex]; 
			dibits[destIndex+1] = data[sourceIndex]; 
			dibits[destIndex+2] = data[sourceIndex]; 
			
		}
	}
	image->unlock();

}

//-------------------------------------------------------------------

BOOL BitmapPreviewView::OnEraseBkgnd(CDC* pDC) 
{
	UNREF (pDC);

	return true;
}

//-------------------------------------------------------------------

void BitmapPreviewView::updateBitmap (const Image& image, float low, float high, float gain)
{
	//-- update the bitmap
	__int64 start;
	IGNORE_RETURN (QueryPerformanceCounter (reinterpret_cast<LARGE_INTEGER*> (&start)));

	if(!m_dib)
	{
		CreateDib(&image);
	}
	else
	{

		// check to make sure the that image is the same size.  This preview pane is shared and different sized images could be used
		CSize dibSize = m_dib->GetDimensions();
		if(dibSize.cx != image.getWidth() || dibSize.cy != image.getHeight())
		{
			CreateDib(&image);
		}
	}

	BYTE*      dibits  = m_dib->m_lpImage;

	const uint8* data = image.lockReadOnly ();
	NOT_NULL (data);

	int imageStride = image.getStride();
	int destStride = imageStride * 3;
	int x;
	int y;
	const int imageHeight = image.getHeight();
	const int imageWidth = image.getWidth();

	for (y = 0; y < imageHeight; ++y)
	{
		for (x = 0; x < imageWidth; ++x)
		{
			
			int destIndex = (y * destStride) + x * 3;
			int sourceIndex = (imageHeight - y - 1) * imageStride + x; // bottom up

			real sample = ((real)data[sourceIndex])/255.0f + gain;
			
			if(sample > 1.0f)
				sample = 1.0f;
			else if(sample < 0.0f)
				sample = 0.0f;

			const bool clampedLow  = sample < low;
			const bool clampedHigh = sample > high;
			const real v = clamp (low, sample, high);


			const uint8 c = static_cast<uint8> (v * 255.0f);
			dibits[destIndex] = clampedLow ? static_cast<uint8> (0) : clampedHigh ? static_cast<uint8> (0) : c;  
			dibits[destIndex+1] = clampedLow ? static_cast<uint8> (0) : clampedHigh ? static_cast<uint8> (0) : c;  
			dibits[destIndex+2] = clampedLow ? static_cast<uint8> (0) : clampedHigh ? static_cast<uint8> (0) : c;  
		}
	}

	image.unlock();

	__int64 end;
	IGNORE_RETURN (QueryPerformanceCounter (reinterpret_cast<LARGE_INTEGER*> (&end)));

	__int64 frequency;
	IGNORE_RETURN (QueryPerformanceFrequency (reinterpret_cast<LARGE_INTEGER*> (&frequency)));

	const real last = (static_cast<float> (end) - static_cast<float> (start)) / static_cast<float> (frequency);
	UNREF (last);

	Invalidate ();
}

//-------------------------------------------------------------------

