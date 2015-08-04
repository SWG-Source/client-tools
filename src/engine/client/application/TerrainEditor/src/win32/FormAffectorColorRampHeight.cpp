//
// FormAffectorColorRampHeight.cpp
// asommers
//
// copyright 2000, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorColorRampHeight.h"

#include "sharedTerrain/AffectorColor.h"
#include "CDib.h"
#include "sharedUtility/FileName.h"
#include "FractalPreviewFrame.h"
#include "sharedImage/Image.h"
#include "sharedImage/ImageFormatList.h"
#include "MainFrame.h"
#include "RecentDirectory.h"
#include "TerrainEditor.h"
#include "sharedFile/TreeFile.h"

#include <string>

//-------------------------------------------------------------------

IMPLEMENT_DYNCREATE(FormAffectorColorRampHeight, FormLayerItem)

//-------------------------------------------------------------------

FormAffectorColorRampHeight::FormAffectorColorRampHeight() : 
	FormLayerItem(FormAffectorColorRampHeight::IDD),
	affector (0),
	m_dib (0),
	m_dibSize (0),

	//-- widgets
	m_highHeight (true),
	m_lowHeight (true),
	m_name (),
	m_operation (0)
{
	//{{AFX_DATA_INIT(FormAffectorColorRampHeight)
	m_imageName = _T("");
	m_name = _T("");
	m_operation = -1;
	//}}AFX_DATA_INIT
}

//-------------------------------------------------------------------
	
FormAffectorColorRampHeight::~FormAffectorColorRampHeight()
{
	affector = 0;

	if (m_dib)
		delete m_dib;
}

//-------------------------------------------------------------------

void FormAffectorColorRampHeight::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorColorRampHeight)
	DDX_Control(pDX, IDC_HIGHHEIGHT, m_highHeight);
	DDX_Control(pDX, IDC_LOWHEIGHT, m_lowHeight);
	DDX_Control(pDX, IDC_STATIC_WINDOW_COLOR, m_rampWindow);
	DDX_Text(pDX, IDC_EDIT_IMAGENAME, m_imageName);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_CBIndex(pDX, IDC_OPERATION, m_operation);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FormAffectorColorRampHeight, PropertyView)
	//{{AFX_MSG_MAP(FormAffectorColorRampHeight)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
	ON_CBN_EDITCHANGE(IDC_OPERATION, OnEditchangeOperation)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorColorRampHeight::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorColorRampHeight::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorColorRampHeight::OnDraw(CDC* pDC) 
{
	PropertyView::OnDraw (pDC);

	if (m_dib)
	{
		CRect rect;
		m_rampWindow.GetWindowRect (&rect);
		ScreenToClient (&rect);

		NOT_NULL (m_dib);

		IGNORE_RETURN (m_dib->Draw (pDC, rect.TopLeft (), rect.Size ()));
	}
}

//-------------------------------------------------------------------

void FormAffectorColorRampHeight::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	affector = dynamic_cast<AffectorColorRampHeight*> (flivd->item->layerItem);
	NOT_NULL (affector);
}

//-------------------------------------------------------------------

void FormAffectorColorRampHeight::OnInitialUpdate() 
{
	FormLayerItem::OnInitialUpdate();

	NOT_NULL (affector);

	m_name       = affector->getName ();
	m_operation  = static_cast<int> (affector->getOperation ());
	m_lowHeight  = affector->getLowHeight ();
	m_highHeight = affector->getHighHeight ();
	m_imageName  = affector->getImageName ().c_str ();

	Image* image = verifyImage (m_imageName, false);
	if (image)
	{
		createDib (image);

		delete image;
		image = 0;
	}

	IGNORE_RETURN (UpdateData(false));

	m_initialized = true;
}

//-------------------------------------------------------------------

void FormAffectorColorRampHeight::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorColorRampHeight::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormAffectorColorRampHeight::ApplyChanges (void)
{
	if (!m_initialized)
		return;

	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (affector);

		affector->setOperation (static_cast<TerrainGeneratorOperation> (m_operation));
		affector->setLowHeight (m_lowHeight);
		affector->setHighHeight (m_highHeight);
		affector->setImage (static_cast<LPCSTR> (m_imageName));

		GetDocument ()->UpdateAllViews (this);
		GetDocument ()->SetModifiedFlag ();
	}
}

//-------------------------------------------------------------------

void FormAffectorColorRampHeight::createDib (const Image* image)
{
	if (m_dib)
		delete m_dib;

	//-- Create a compatible DC
	CDC* pDC = GetDC();
	ASSERT (pDC);
	CDC dcCompat;
	IGNORE_RETURN (dcCompat.CreateCompatibleDC (pDC));
	IGNORE_RETURN (ReleaseDC(pDC));

	// Allocate memory for the bitmap
	m_dib = new CDib (CSize (image->getWidth (), image->getHeight ()), 24);
	VERIFY (m_dib->CreateSection (&dcCompat));

	const uint8* data = image->lockReadOnly ();
	NOT_NULL (data);

	BYTE* dibits = m_dib->m_lpImage;
	NOT_NULL (dibits);

	int i;
	for (i = 0; i < image->getWidth (); ++i)
	{
		if (image->getPixelFormat () == Image::PF_bgr_888)
		{
			*dibits++ = *data++;  // b
			*dibits++ = *data++;  // g
			*dibits++ = *data++;  // r
		}
		else
		{
			*dibits++ = *(data + 2);  // b
			*dibits++ = *(data + 1);  // g
			*dibits++ = *(data + 0);  // r

			data += 3;
		}
	}
}

//-------------------------------------------------------------------

Image* FormAffectorColorRampHeight::verifyImage (const CString& name, bool verbose) const
{
	bool   valid = false;
	Image* image = ImageFormatList::loadImage (name);

	if (!image)
	{
		if (verbose)
		{
			CString tmp;
			tmp.Format ("%s is not a valid image!", name);
			const_cast<FormAffectorColorRampHeight*> (this)->MessageBox (tmp);
		}
	}
	else
	{
		if (!(image->getPixelFormat () == Image::PF_rgb_888 || image->getPixelFormat () == Image::PF_bgr_888))
		{
			if (verbose)
			{
				CString tmp;
				tmp.Format ("%s is not a 24-bit tga!", name);
				const_cast<FormAffectorColorRampHeight*> (this)->MessageBox (tmp);
			}
		}
		else
		{
			if (image->getHeight () != 1)
			{
				if (verbose)
				{
					CString tmp;
					tmp.Format ("%s is not a n x 1 image!", name);
					const_cast<FormAffectorColorRampHeight*> (this)->MessageBox (tmp);
				}
			}
			else
				valid = true;
		}
	}

	if (!valid)
	{
		delete image;
		image = 0;
	}

	return image;
}

//-------------------------------------------------------------------

void FormAffectorColorRampHeight::OnButtonBrowse() 
{
	CFileDialog dlg (true, "*.tga", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Targa");
	dlg.m_ofn.lpstrTitle      = "Select Targa File";

	bool stillTrying = true;

	while (stillTrying)
	{
		if (dlg.DoModal () == IDOK)
		{
			RecentDirectory::update ("Targa", dlg.GetPathName ());

			//-- get new name
			const CString name = FileName (FileName::P_terrain, dlg.GetFileName (), "");

			if (TreeFile::exists (name))
			{
				Image* image = verifyImage (name);
				if (image)
				{
					stillTrying = false;
					m_imageName = name;
					createDib (image);

					delete image;
					image = 0;
				}
			}
		}
		else
			stillTrying = false;
	}

	UpdateData (false);
	Invalidate (true);
}

//-------------------------------------------------------------------

void FormAffectorColorRampHeight::OnButtonRefresh() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorColorRampHeight::OnEditchangeOperation() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------
