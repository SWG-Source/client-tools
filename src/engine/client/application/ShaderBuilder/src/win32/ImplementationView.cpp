// ImplementationView.cpp : implementation file
//

#include "FirstShaderBuilder.h"
#include "ShaderBuilder.h"
#include "ImplementationView.h"

#include "Node.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImplementationView

IMPLEMENT_DYNCREATE(CImplementationView, CFormView)

CImplementationView::CImplementationView()
	: CFormView(CImplementationView::IDD),
	m_implementationNode(NULL)
{
	//{{AFX_DATA_INIT(CImplementationView)
	//}}AFX_DATA_INIT
}

CImplementationView::~CImplementationView()
{
	m_implementationNode = NULL;
}

void CImplementationView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImplementationView)
	DDX_Control(pDX, IDC_CHECK2, m_castsShadows);
	DDX_Control(pDX, IDC_CHECK1, m_collidable);
	DDX_Control(pDX, IDC_EDIT2, m_optionTags);
	DDX_Control(pDX, IDC_EDIT3, m_shaderCapabilityCompatibility);
	DDX_Control(pDX, IDC_EDIT1, m_phaseTag);
	DDX_Control(pDX, IDC_NAME, m_name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImplementationView, CFormView)
	//{{AFX_MSG_MAP(CImplementationView)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	ON_EN_CHANGE(IDC_EDIT2, OnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT3, OnChangeEdit3)
	ON_BN_CLICKED(IDC_CHECK1, OnCheckCollidable)
	ON_BN_CLICKED(IDC_CHECK2, OnCheckCastsShadows)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImplementationView diagnostics

#ifdef _DEBUG
void CImplementationView::AssertValid() const
{
	CFormView::AssertValid();
}

void CImplementationView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

void CImplementationView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
	m_phaseTag.SetLimitText(4);
}

/////////////////////////////////////////////////////////////////////////////
// CImplementationView message handlers

void CImplementationView::SetNode(CImplementationNode &implementationNode)
{
	m_implementationNode = &implementationNode;

	// set the file name correctly
	if (!implementationNode.GetFileName().IsEmpty())
	{
		CString newText = "Name:\t";
		newText += implementationNode.GetFileName();
		m_name.SetWindowText(newText);
	}
	else
		m_name.SetWindowText("Local implementation");

	if (implementationNode.IsLocked(CNode::Either))	
		EnableWindow(FALSE);
	else
		EnableWindow(TRUE);

	m_phaseTag.SetWindowText(m_implementationNode->m_phaseTag);

	{
		CString shaderCapabilityCompatibility;
		const int numberOfShaderCapabilityCompatibilityEntries = m_implementationNode->m_shaderCapabilityCompatibility.size();
		for (int i = 0; i < numberOfShaderCapabilityCompatibilityEntries; ++i)
		{
			shaderCapabilityCompatibility += m_implementationNode->m_shaderCapabilityCompatibility[i] + char(0x0d) + char(0x0a);
		}
		m_shaderCapabilityCompatibility.SetWindowText(shaderCapabilityCompatibility);
	}

	{
		CString optionTags;
		const int numberOfOptionTags = m_implementationNode->m_optionTags.size();
		for (int i = 0; i < numberOfOptionTags; ++i)
		{
			optionTags += m_implementationNode->m_optionTags[i] + char(0x0d) + char(0x0a);
		}
		m_optionTags.SetWindowText(optionTags);
	}

#define SET(a) a.SetCheck(implementationNode.a)

	SET(m_collidable);
	SET(m_castsShadows);

#undef SET
}

void CImplementationView::OnChangeEdit1() 
{
	m_phaseTag.GetWindowText(m_implementationNode->m_phaseTag);
}

void CImplementationView::OnChangeEdit2() 
{
	bool bad = false;

	CString a;
	m_optionTags.GetWindowText(a);

	const int numberOfLines = m_optionTags.GetLineCount();
	for (int i = 0; i < numberOfLines; ++i)
	{
		if (m_optionTags.LineLength(m_optionTags.LineIndex(i)) > 4)
		{
			bad = true;
			m_optionTags.Undo();
		}
	}

	if (!bad)
	{
		m_implementationNode->m_optionTags.clear();
		for (int i = 0; i < numberOfLines; ++i)
		{
			if (m_optionTags.LineLength(i) > 0)
			{
				char buffer[8];
				memset(buffer, 0, sizeof(buffer));
				m_optionTags.GetLine(i, buffer, 5);
				if (isprint(buffer[0]))
				{
					CString s = buffer;
					m_implementationNode->m_optionTags.push_back(s);
				}
			}
		}
	}
}

void CImplementationView::OnChangeEdit3() 
{
	CString a;
	m_shaderCapabilityCompatibility.GetWindowText(a);
	m_implementationNode->m_shaderCapabilityCompatibility.clear();
	const int numberOfLines = m_shaderCapabilityCompatibility.GetLineCount();
	for (int i = 0; i < numberOfLines; ++i)
	{
		if (m_shaderCapabilityCompatibility.LineLength(i) > 0)
		{
			char buffer[16];
			memset(buffer, 0, sizeof(buffer));
			m_shaderCapabilityCompatibility.GetLine(i, buffer, sizeof(buffer)-1);
			if (isprint(buffer[0]))
			{
				CString s = buffer;
				m_implementationNode->m_shaderCapabilityCompatibility.push_back(s);
			}
		}
	}
}

void CImplementationView::OnCheckCollidable() 
{
	m_implementationNode->m_collidable = m_collidable.GetCheck();
}

void CImplementationView::OnCheckCastsShadows() 
{
	m_implementationNode->m_castsShadows = m_castsShadows.GetCheck();
}
