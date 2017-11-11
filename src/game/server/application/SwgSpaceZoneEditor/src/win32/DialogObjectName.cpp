// DialogObjectName.cpp : implementation file
//

#include "FirstSwgSpaceZoneEditor.h"
#include "SwgSpaceZoneEditor.h"
#include "DialogObjectName.h"


// DialogObjectName dialog

IMPLEMENT_DYNAMIC(DialogObjectName, CDialog)

DialogObjectName::DialogObjectName(CWnd* pParent /*=NULL*/)
	: CDialog(DialogObjectName::IDD, pParent)
{

}

DialogObjectName::~DialogObjectName()
{
}

void DialogObjectName::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DialogObjectName, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_NAMEOK, &DialogObjectName::OnBnClickedButtonNameok)
	ON_BN_CLICKED(IDC_BUTTON_NAMECANCEL, &DialogObjectName::OnBnClickedButtonNamecancel)
END_MESSAGE_MAP()


// DialogObjectName message handlers

void DialogObjectName::OnBnClickedButtonNameok()
{
	GetDlgItemText(IDC_OBJECTNAME, mNameText);

	if(mNameText.IsEmpty())
	{
		CDialog::OnCancel();
		return;
	}

	CDialog::OnOK();
}

void DialogObjectName::OnBnClickedButtonNamecancel()
{
	CDialog::OnCancel();
}