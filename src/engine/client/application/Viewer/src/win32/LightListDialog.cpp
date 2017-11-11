// LightListDialog.cpp : implementation file
//

#include "FirstViewer.h"
#include "viewer.h"

#include "sharedFile/Iff.h"
#include "LightListDialog.h"
#include "LightDialog.h"

/////////////////////////////////////////////////////////////////////////////
// LightListDialog dialog


LightListDialog::LightListDialog(CWnd* pParent, const LightManager& newLightManager) : 
	CDialog(LightListDialog::IDD, pParent),
	lightManager (newLightManager)
{
	//{{AFX_DATA_INIT(LightListDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void LightListDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LightListDialog)
	DDX_Control(pDX, IDC_LIGHT_LIST, m_lightList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(LightListDialog, CDialog)
	//{{AFX_MSG_MAP(LightListDialog)
	ON_BN_CLICKED(IDC_ADD_LIGHT, OnAddLight)
	ON_BN_CLICKED(IDC_EDIT_LIGHT, OnEditLight)
	ON_LBN_DBLCLK(IDC_LIGHT_LIST, OnDblclkLightList)
	ON_BN_CLICKED(IDC_LOAD_LIGHTS, OnLoadLights)
	ON_BN_CLICKED(IDC_REMOVE_LIGHT, OnRemoveLight)
	ON_BN_CLICKED(IDC_SAVE_LIGHTS, OnSaveLights)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LightListDialog message handlers

void LightListDialog::OnAddLight() 
{
	// TODO: Add your control notification handler code here
	LightDialog dlg (this, LightDialog::M_add, lightManager, 0);
	if (dlg.DoModal () == IDOK)
	{
		LightManager::LightData data = dlg.getLightData ();

		lightManager.addLight (data);
		m_lightList.AddString (data.name);
	}
}

void LightListDialog::OnEditLight() 
{
	//-- is a light selected?
	if (m_lightList.GetCurSel () != LB_ERR)
	{
		//-- get the text from the box
		CString selection;
		m_lightList.GetText (m_lightList.GetCurSel (), selection);

		//-- find light data in light data list
		int i;
		for (i = 0; i < lightManager.getNumberOfLights (); i++)
			if (strcmp (lightManager.getLight (i).name, selection) == 0)
				break;

		//-- found light, now edit it
		if (i < lightManager.getNumberOfLights ())
		{
			//--
			LightDialog dlg (this, LightDialog::M_edit, lightManager, i);
			dlg.setLightData (lightManager.getLight (i));

			if (dlg.DoModal () == IDOK)
			{
				lightManager.setLight (i, dlg.getLightData ());
				m_lightList.DeleteString (m_lightList.GetCurSel ());
				m_lightList.AddString (lightManager.getLight (i).name);
			}
		}
	}
}

void LightListDialog::OnDblclkLightList() 
{
	// TODO: Add your control notification handler code here
	OnEditLight ();
}

void LightListDialog::OnLoadLights() 
{
	CFileDialog dlg (true, 0, 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER));
	if (dlg.DoModal () == IDOK)
	{
		m_lightList.ResetContent ();

		{
			FatalSetThrowExceptions (true);
			try
			{
				lightManager.load (dlg.GetPathName ());
			}
			catch (FatalException& e)
			{
				MessageBox(e.getMessage(), "Error loading lights file", MB_ICONSTOP);
			}
			FatalSetThrowExceptions (false);
		}

		int i;
		for (i = 0; i < lightManager.getNumberOfLights (); i++)
			m_lightList.AddString (lightManager.getLight (i).name);
	}
}

void LightListDialog::OnRemoveLight() 
{
	// TODO: Add your control notification handler code here
	//-- is a light selected?
	if (m_lightList.GetCurSel () != LB_ERR)
	{
		//-- get the text from the box
		CString selection;
		m_lightList.GetText (m_lightList.GetCurSel (), selection);

		//-- find light data in light data list
		int i;
		for (i = 0; i < lightManager.getNumberOfLights (); i++)
			if (strcmp (lightManager.getLight (i).name, selection) == 0)
				break;

		//-- found light, now edit it
		if (i < lightManager.getNumberOfLights ())
		{
			CString text = "Are you sure you wish to delete ";
			text += selection;
			text += "?";

			if (MessageBox (text, "Confirm", MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				lightManager.removeLight (i);
				m_lightList.DeleteString (m_lightList.GetCurSel ());
			}
		}
	}
}

void LightListDialog::OnSaveLights() 
{
	CFileDialog dlg (false, 0, 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER));
	if (dlg.DoModal () == IDOK)
	{
		FatalSetThrowExceptions (true);
		try
		{
			lightManager.save (dlg.GetPathName ());
		}
		catch (FatalException& e)
		{
			MessageBox(e.getMessage(), "Error loading lights.iff", MB_ICONSTOP);
		}
		FatalSetThrowExceptions (false);
	}
}

BOOL LightListDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	int i;
	for (i = 0; i < lightManager.getNumberOfLights (); i++)
		m_lightList.AddString (lightManager.getLight (i).name);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
