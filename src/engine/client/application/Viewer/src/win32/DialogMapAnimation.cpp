// ======================================================================
//
// DialogMapAnimation.cpp
// Portions Copyright 1999, Bootprint Entertainment
// Portions Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "FirstViewer.h"
#include "viewer.h"
#include "DialogMapAnimation.h"

#include "sharedFoundation/ConstCharCrcLowerString.h"

// ======================================================================

namespace
{
	const ConstCharCrcLowerString cs_trackNameLookup[] =
	{
		ConstCharCrcLowerString("loop"),
		ConstCharCrcLowerString("action"),
		ConstCharCrcLowerString("add")
	};
}

// ======================================================================

BEGIN_MESSAGE_MAP(DialogMapAnimation, CDialog)
	//{{AFX_MSG_MAP(DialogMapAnimation)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_SOUND_BROWSE, OnButtonSoundBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

DialogMapAnimation::DialogMapAnimation(CWnd* pParent /*=NULL*/)
	: CDialog(DialogMapAnimation::IDD, pParent)
{
	//{{AFX_DATA_INIT(DialogMapAnimation)
	animationFileName = _T("");
	blendTime = 0.0f;
	key = _T("");
	timeScale = 0.0f;
	playModeIndex = -1;
	isLooping = FALSE;
	isInterruptible = FALSE;
	m_weight = 0.0f;
	m_soundFilename = _T("");
	m_trackNameIndex = 0;
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

const CrcLowerString &DialogMapAnimation::getTrackName() const
{
	if (m_trackNameIndex >= 0)
		return cs_trackNameLookup[m_trackNameIndex];
	else
		return CrcLowerString::empty;
}

// ----------------------------------------------------------------------

void DialogMapAnimation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogMapAnimation)
	DDX_Control(pDX, IDC_EDIT_ANIMATION_FILENAME, animationFileNameEdit);
	DDX_Text(pDX, IDC_EDIT_ANIMATION_FILENAME, animationFileName);
	DDX_Text(pDX, IDC_EDIT_BLEND_TIME, blendTime);
	DDV_MinMaxFloat(pDX, blendTime, 0.f, 100.f);
	DDX_Text(pDX, IDC_EDIT_KEY, key);
	DDV_MaxChars(pDX, key, 1);
	DDX_Text(pDX, IDC_EDIT_TIME_SCALE, timeScale);
	DDV_MinMaxFloat(pDX, timeScale, 0.f, 100.f);
	DDX_Radio(pDX, IDC_RADIO_PLAYMODE_QUEUED, playModeIndex);
	DDX_Check(pDX, IDC_CHECK_LOOPING, isLooping);
	DDX_Check(pDX, IDC_CHECK_INTERRUPTIBLE, isInterruptible);
	DDX_Text(pDX, IDC_EDIT_WEIGHT, m_weight);
	DDV_MinMaxFloat(pDX, m_weight, 0.f, 1.f);
	DDX_Text(pDX, IDC_EDIT_SOUND_FILENAME, m_soundFilename);
	DDX_CBIndex(pDX, IDC_TRACKNAME_COMBO, m_trackNameIndex);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

void DialogMapAnimation::OnButtonBrowse() 
{
	// bring up open-file dialog box
	char buffer[MAX_PATH];
	animationFileNameEdit.GetWindowText(buffer, MAX_PATH-1);

	//-- do the file dialog
	CFileDialog dlg(TRUE, 0, buffer, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER), "Skeletal Animation Files (*.ans)|*.ans|Mesh Animation Files (*.man)|*.man||");
	if (dlg.DoModal() == IDOK)
	{
		// save animation file name
		animationFileNameEdit.SetWindowText(dlg.GetPathName());
	}
}

// ----------------------------------------------------------------------

void DialogMapAnimation::OnOK ()
{
	UpdateData (true);

	if (animationFileName.GetLength () == 0)
	{
		MessageBox ("No file selected");
	}
	else
		CDialog::OnOK ();
}

//-------------------------------------------------------------------

void DialogMapAnimation::OnButtonSoundBrowse() 
{
	CFileDialog dlg(TRUE, 0, m_soundFilename, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER), "Sound Files (*.wav)|*.wav||");

	if (dlg.DoModal() == IDOK)
	{
		m_soundFilename = dlg.GetPathName ();
		UpdateData (false);
	}
}

// ======================================================================
