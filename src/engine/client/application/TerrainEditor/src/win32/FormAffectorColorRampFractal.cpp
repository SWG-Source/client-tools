//
// FormAffectorColorRampFractal.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "FirstTerrainEditor.h"
#include "FormAffectorColorRampFractal.h"

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

IMPLEMENT_DYNCREATE(FormAffectorColorRampFractal, FormLayerItem)

//-------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable:4355)

FormAffectorColorRampFractal::FormAffectorColorRampFractal() : 
	FormLayerItem(FormAffectorColorRampFractal::IDD),
	m_affector (0),
	m_dib (0),
	m_dibSize (0),

	//-- widgets
	m_familyCtrl (),
	m_operation (),
	m_name (),
	m_rampWindow (),
	m_imageName ()
{
	//{{AFX_DATA_INIT(FormAffectorColorRampFractal)
	m_name = _T("");
	m_imageName = _T("");
	//}}AFX_DATA_INIT
}

#pragma warning(pop)

//-------------------------------------------------------------------

FormAffectorColorRampFractal::~FormAffectorColorRampFractal()
{
	m_affector = 0;

	if (m_dib)
		delete m_dib;
}

//-------------------------------------------------------------------

void FormAffectorColorRampFractal::DoDataExchange(CDataExchange* pDX)
{
	FormLayerItem::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAffectorColorRampFractal)
	DDX_Control(pDX, IDC_FAMILY, m_familyCtrl);
	DDX_Control(pDX, IDC_OPERATION, m_operation);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDX_Control(pDX, IDC_STATIC_WINDOW_COLOR, m_rampWindow);
	DDX_Text(pDX, IDC_EDIT_IMAGENAME, m_imageName);
	//}}AFX_DATA_MAP
}

//-------------------------------------------------------------------

//lint -save -e1924

BEGIN_MESSAGE_MAP(FormAffectorColorRampFractal, PropertyView)
	//{{AFX_MSG_MAP(FormAffectorColorRampFractal)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_OPERATION, OnSelchangeOperation)
	ON_CBN_SELCHANGE(IDC_FAMILY, OnSelchangeFamily)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//lint -restore

//-------------------------------------------------------------------

#ifdef _DEBUG
void FormAffectorColorRampFractal::AssertValid() const
{
	FormLayerItem::AssertValid();
}

void FormAffectorColorRampFractal::Dump(CDumpContext& dc) const
{
	FormLayerItem::Dump(dc);
}
#endif //_DEBUG

//-------------------------------------------------------------------

void FormAffectorColorRampFractal::OnDraw(CDC* pDC) 
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

void FormAffectorColorRampFractal::Initialize (PropertyView::ViewData* vd)
{
	NOT_NULL (vd);

	FormLayerItemViewData* flivd = dynamic_cast<FormLayerItemViewData*> (vd);
	NOT_NULL (flivd);

	m_affector = dynamic_cast<AffectorColorRampFractal*> (flivd->item->layerItem);
	NOT_NULL (m_affector);
}

//-------------------------------------------------------------------

void FormAffectorColorRampFractal::OnInitialUpdate() 
{
	PropertyView::OnInitialUpdate();

	NOT_NULL (m_affector);

	//-- update settings
	TerrainEditorDoc* const doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
	doc->BuildFractalFamilyDropList (m_familyCtrl, m_affector->getFamilyId ());

	IGNORE_RETURN (m_operation.SetCurSel (m_affector->getOperation ()));

	m_name = m_affector->getName ();

	m_imageName = m_affector->getImageName ().c_str ();

	Image* image = verifyImage (m_imageName, false);
	if (image)
	{
		createDib (image);

		delete image;
		image = 0;
	}

	IGNORE_RETURN (UpdateData (false));

	m_initialized = true;

	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorColorRampFractal::OnDestroy() 
{
	ApplyChanges ();

	PropertyView::OnDestroy();
}

//-------------------------------------------------------------------

bool FormAffectorColorRampFractal::HasChanged () const
{
	return true;
}

//-------------------------------------------------------------------

void FormAffectorColorRampFractal::ApplyChanges ()
{
	if (!m_initialized)
		return;

	//-- update the controls/data
	IGNORE_RETURN (UpdateData (true));

	if (HasChanged ())
	{
		NOT_NULL (m_affector);

		m_affector->setFamilyId (static_cast<int> (m_familyCtrl.GetItemData (m_familyCtrl.GetCurSel ())));
		m_affector->setOperation (static_cast<TerrainGeneratorOperation> (m_operation.GetCurSel ()));
		m_affector->setImage (static_cast<LPCSTR> (m_imageName));

		safe_cast<TerrainEditorApp*> (AfxGetApp ())->showFractalPreview ();
		GetParentFrame ()->ActivateFrame ();

		TerrainEditorDoc* doc = safe_cast<TerrainEditorDoc*> (GetDocument ());
		NOT_NULL (doc);
		
		const MultiFractal* multiFractal = doc->getTerrainGenerator ()->getFractalGroup ().getFamilyMultiFractal (m_affector->getFamilyId ());
		NOT_NULL (multiFractal);

		if (doc->getFractalPreviewFrame ())
			doc->getFractalPreviewFrame ()->updateBitmap (*multiFractal);

		//-- tell the document we've modified data
		GetDocument ()->SetModifiedFlag (true);
		GetDocument ()->UpdateAllViews (this);
	}
}

//-------------------------------------------------------------------

void FormAffectorColorRampFractal::createDib (const Image* image)
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

Image* FormAffectorColorRampFractal::verifyImage (const CString& name, bool verbose) const
{
	bool   valid = false;
	Image* image = ImageFormatList::loadImage (name);

	if (!image)
	{
		if (verbose)
		{
			CString tmp;
			tmp.Format ("%s is not a valid image!", static_cast<const char*> (name));
			IGNORE_RETURN (const_cast<FormAffectorColorRampFractal*> (this)->MessageBox (tmp));
		}
	}
	else
	{
		if (!(image->getPixelFormat () == Image::PF_rgb_888 || image->getPixelFormat () == Image::PF_bgr_888))
		{
			if (verbose)
			{
				CString tmp;
				tmp.Format ("%s is not a 24-bit tga!", static_cast<const char*> (name));
				IGNORE_RETURN (const_cast<FormAffectorColorRampFractal*> (this)->MessageBox (tmp));
			}
		}
		else
		{
			if (image->getHeight () != 1)
			{
				if (verbose)
				{
					CString tmp;
					tmp.Format ("%s is not a n x 1 image!", static_cast<const char*> (name));
					IGNORE_RETURN (const_cast<FormAffectorColorRampFractal*> (this)->MessageBox (tmp));
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

void FormAffectorColorRampFractal::OnButtonBrowse() 
{
	CFileDialog dlg (true, "*.tga", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR));
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Targa");
	dlg.m_ofn.lpstrTitle      = "Select Targa File";

	bool stillTrying = true;

	while (stillTrying)
	{
		if (dlg.DoModal () == IDOK)
		{
			IGNORE_RETURN (RecentDirectory::update ("Targa", dlg.GetPathName ()));

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

	IGNORE_RETURN (UpdateData (false));
	Invalidate (true);
}

//-------------------------------------------------------------------

void FormAffectorColorRampFractal::OnButtonRefresh() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorColorRampFractal::OnSelchangeOperation() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

void FormAffectorColorRampFractal::OnSelchangeFamily() 
{
	ApplyChanges ();
}

//-------------------------------------------------------------------

