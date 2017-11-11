// ======================================================================
//
// ConversationBranchView.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "ConversationBranchView.h"

#include "Configuration.h"
#include "Conversation.h"
#include "ConversationFrame.h"
#include "SwgConversationEditor.h"

// ======================================================================

IMPLEMENT_DYNCREATE(ConversationBranchView, CFormView)

// ======================================================================

ConversationBranchView::ConversationBranchView() : 
	CFormView(ConversationBranchView::IDD),
	m_itemData (0)
{
	//{{AFX_DATA_INIT(ConversationBranchView)
	m_negateCondition = FALSE;
	m_debug = FALSE;
	m_useProsePackage = FALSE;
	m_groupEcho = FALSE;
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

ConversationBranchView::~ConversationBranchView()
{
}

// ----------------------------------------------------------------------

void ConversationBranchView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ConversationBranchView)
	DDX_Control(pDX, IDC_COMBO_TOKENTO, m_tokenTOCtrl);
	DDX_Control(pDX, IDC_COMBO_TOKENDI, m_tokenDICtrl);
	DDX_Control(pDX, IDC_COMBO_TOKENDF, m_tokenDFCtrl);
	DDX_Control(pDX, IDC_COMBO_PLAYERANIMATION, m_playerAnimationCtrl);
	DDX_Control(pDX, IDC_COMBO_NPCANIMATION, m_npcAnimationCtrl);
	DDX_Control(pDX, IDC_CHECK_DEBUG, m_debugCtrl);
	DDX_Control(pDX, IDC_COMBO_LINK, m_linkCtrl);
	DDX_Control(pDX, IDC_COMBO_LABEL, m_labelCtrl);
	DDX_Control(pDX, IDC_COMBO_CONDITION, m_conditionCtrl);
	DDX_Control(pDX, IDC_COMBO_ACTION, m_actionCtrl);
	DDX_Control(pDX, IDC_EDIT_NOTES, m_notesCtrl);
	DDX_Control(pDX, IDC_EDIT_TEXT, m_textCtrl);
	DDX_Check(pDX, IDC_CHECK_NEGATECONDITION, m_negateCondition);
	DDX_Check(pDX, IDC_CHECK_DEBUG, m_debug);
	DDX_Check(pDX, IDC_CHECK_USEPROSEPACKAGE, m_useProsePackage);
	DDX_Check(pDX, IDC_CHECK_GROUPECHO_BRANCH, m_groupEcho);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ConversationBranchView, CFormView)
	//{{AFX_MSG_MAP(ConversationBranchView)
	ON_EN_CHANGE(IDC_EDIT_NOTES, OnChangeEditNotes)
	ON_EN_CHANGE(IDC_EDIT_TEXT, OnChangeEditText)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_CBN_SELCHANGE(IDC_COMBO_ACTION, OnSelchangeComboAction)
	ON_CBN_SELCHANGE(IDC_COMBO_CONDITION, OnSelchangeComboCondition)
	ON_BN_CLICKED(IDC_CHECK_NEGATECONDITION, OnCheckNegatecondition)
	ON_BN_CLICKED(IDC_CHECK_DEBUG, OnCheckDebug)
	ON_BN_CLICKED(IDC_CHECK_GROUPECHO_BRANCH, OnCheckGroupEcho)
	ON_CBN_SELCHANGE(IDC_COMBO_LABEL, OnSelchangeComboLabel)
	ON_CBN_SELCHANGE(IDC_COMBO_LINK, OnSelchangeComboLink)
	ON_CBN_SELCHANGE(IDC_COMBO_NPCANIMATION, OnSelchangeComboNpcanimation)
	ON_CBN_SELCHANGE(IDC_COMBO_PLAYERANIMATION, OnSelchangeComboPlayeranimation)
	ON_BN_CLICKED(IDC_CHECK_USEPROSEPACKAGE, OnCheckUseprosepackage)
	ON_CBN_SELCHANGE(IDC_COMBO_TOKENDF, OnSelchangeComboTokendf)
	ON_CBN_SELCHANGE(IDC_COMBO_TOKENDI, OnSelchangeComboTokendi)
	ON_CBN_SELCHANGE(IDC_COMBO_TOKENTO, OnSelchangeComboTokento)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

#ifdef _DEBUG
void ConversationBranchView::AssertValid() const
{
	CFormView::AssertValid();
}

void ConversationBranchView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void ConversationBranchView::setItemData (SwgConversationEditorDoc::ItemData * const itemData) 
{
	m_itemData = itemData;

	m_textCtrl.SetWindowText (itemData->m_conversationItem->getText ().c_str ());
	m_notesCtrl.SetWindowText (itemData->m_conversationItem->getNotes ().c_str ());
	m_negateCondition = itemData->m_conversationItem->getNegateCondition ();
	m_debug = itemData->m_conversationItem->getDebug ();
	m_debugCtrl.EnableWindow (itemData->m_conversationItem->getParent () && !itemData->m_conversationItem->getParent ()->getDebug ());
	m_groupEcho = itemData->m_conversationItem->getGroupEcho ();

	SwgConversationEditorDoc const * const document = safe_cast<SwgConversationEditorDoc const *> (GetDocument ());
	m_conditionCtrl.ResetContent ();
	document->buildConditionGroupComboBox (m_conditionCtrl, itemData->m_conversationItem->getConditionFamilyId ());
	m_actionCtrl.ResetContent ();
	document->buildActionGroupComboBox (m_actionCtrl, itemData->m_conversationItem->getActionFamilyId ());
	m_tokenTOCtrl.ResetContent ();
	document->buildTokenTOGroupComboBox (m_tokenTOCtrl, itemData->m_conversationItem->getTokenTOFamilyId ());
	m_tokenDICtrl.ResetContent ();
	document->buildTokenDIGroupComboBox (m_tokenDICtrl, itemData->m_conversationItem->getTokenDIFamilyId ());
	m_tokenDFCtrl.ResetContent ();
	document->buildTokenDFGroupComboBox (m_tokenDFCtrl, itemData->m_conversationItem->getTokenDFFamilyId ());
	m_labelCtrl.ResetContent ();
	document->buildLabelGroupComboBox (m_labelCtrl, itemData->m_conversationItem->getLabelFamilyId (), true);
	m_labelCtrl.EnableWindow (itemData->m_conversationItem->getLinkFamilyId () == 0);
	m_linkCtrl.ResetContent ();
	document->buildLabelGroupComboBox (m_linkCtrl, itemData->m_conversationItem->getLinkFamilyId (), false);
	m_linkCtrl.EnableWindow (itemData->m_conversationItem->getLabelFamilyId () == 0 && !itemData->m_conversationItem->hasChildren ());

	m_npcAnimationCtrl.ResetContent();
	Configuration::populateAnimationActions(m_npcAnimationCtrl);
	m_npcAnimationCtrl.SelectString(0, itemData->m_conversationItem->getNpcAnimation().c_str());

	m_playerAnimationCtrl.ResetContent();
	Configuration::populateAnimationActions(m_playerAnimationCtrl);
	m_playerAnimationCtrl.SelectString(0, itemData->m_conversationItem->getPlayerAnimation().c_str());

	m_useProsePackage = itemData->m_conversationItem->getUseProsePackage ();
	m_tokenTOCtrl.EnableWindow (m_useProsePackage);
	m_tokenDICtrl.EnableWindow (m_useProsePackage);
	m_tokenDFCtrl.EnableWindow (m_useProsePackage);

	UpdateData (false);
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnChangeEditNotes() 
{
	UpdateData (true);

	CString text;
	m_notesCtrl.GetWindowText (text);
	m_itemData->m_conversationItem->setNotes (std::string (text));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnChangeEditText() 
{
	UpdateData (true);

	CString text;
	m_textCtrl.GetWindowText (text);
	m_itemData->m_conversationItem->setText (std::string (text));

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_textChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));
	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnEditCopy() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Copy ();
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnEditCut() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Cut ();
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnEditPaste() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Paste ();
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnEditUndo() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Undo ();
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnSelchangeComboAction() 
{
	UpdateData (true);

	m_itemData->m_conversationItem->setActionFamilyId (static_cast<int> (m_actionCtrl.GetItemData (m_actionCtrl.GetCurSel ())));

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_actionChanged);
	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnSelchangeComboCondition() 
{
	UpdateData (true);

	m_itemData->m_conversationItem->setConditionFamilyId (static_cast<int> (m_conditionCtrl.GetItemData (m_conditionCtrl.GetCurSel ())));

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_conditionChanged);
	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnSelchangeComboLabel() 
{
	UpdateData (true);

	int const familyId = static_cast<int> (m_labelCtrl.GetItemData (m_labelCtrl.GetCurSel ()));
	m_itemData->m_conversationItem->setLabelFamilyId (familyId);

	if (familyId != 0)
		m_itemData->m_conversationItem->setLinkFamilyId (0);

	m_linkCtrl.EnableWindow (familyId == 0);

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_labelChanged);
	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnSelchangeComboLink() 
{
	UpdateData (true);

	int const familyId = static_cast<int> (m_linkCtrl.GetItemData (m_linkCtrl.GetCurSel ()));
	m_itemData->m_conversationItem->setLinkFamilyId (familyId);

	if (familyId != 0)
		m_itemData->m_conversationItem->setLabelFamilyId (0);

	m_labelCtrl.EnableWindow (familyId == 0);

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_linkChanged);
	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnUpdate(CView * const pSender, LPARAM const lHint, CObject * const /*pHint*/) 
{
	if (this != pSender)
	{
		switch (lHint)
		{
		case SwgConversationEditorDoc::H_scriptChanged:
		case SwgConversationEditorDoc::H_conversationItemChanged:
		case SwgConversationEditorDoc::H_labelChanged:
		case SwgConversationEditorDoc::H_linkChanged:
		case SwgConversationEditorDoc::H_groupEchoChanged:
			safe_cast<ConversationFrame *> (GetParentFrame ())->updateView ();
			break;

		default:
			break;
		}
	}
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnCheckNegatecondition() 
{
	UpdateData (true);

	m_itemData->m_conversationItem->setNegateCondition (!m_itemData->m_conversationItem->getNegateCondition ());
	
	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_negateConditionChanged);
	GetDocument ()->SetModifiedFlag ();
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnCheckDebug() 
{
	UpdateData (true);

	m_itemData->m_conversationItem->setDebug (!m_itemData->m_conversationItem->getDebug ());
	
	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_debugChanged);
	GetDocument ()->SetModifiedFlag ();
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnCheckGroupEcho()
{
	UpdateData (true);

	m_itemData->m_conversationItem->setGroupEcho (!m_itemData->m_conversationItem->getGroupEcho ());

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_groupEchoChanged);
	GetDocument ()->SetModifiedFlag ();
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnSelchangeComboNpcanimation() 
{
	UpdateData (true);

	CString npcAnimation;
	m_npcAnimationCtrl.GetWindowText(npcAnimation);
	m_itemData->m_conversationItem->setNpcAnimation (std::string (npcAnimation));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnSelchangeComboPlayeranimation() 
{
	UpdateData (true);

	CString playerAnimation;
	m_playerAnimationCtrl.GetWindowText(playerAnimation);
	m_itemData->m_conversationItem->setPlayerAnimation (std::string (playerAnimation));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnCheckUseprosepackage() 
{
	UpdateData (true);

	bool const useProsePackage = !m_itemData->m_conversationItem->getUseProsePackage ();
	m_itemData->m_conversationItem->setUseProsePackage (useProsePackage);

	m_tokenTOCtrl.EnableWindow (useProsePackage);
	m_tokenDICtrl.EnableWindow (useProsePackage);
	m_tokenDFCtrl.EnableWindow (useProsePackage);

	GetDocument ()->SetModifiedFlag ();
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnSelchangeComboTokento() 
{
	UpdateData (true);

	m_itemData->m_conversationItem->setTokenTOFamilyId (static_cast<int> (m_tokenTOCtrl.GetItemData (m_tokenTOCtrl.GetCurSel ())));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnSelchangeComboTokendi() 
{
	UpdateData (true);

	m_itemData->m_conversationItem->setTokenDIFamilyId (static_cast<int> (m_tokenDICtrl.GetItemData (m_tokenDICtrl.GetCurSel ())));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void ConversationBranchView::OnSelchangeComboTokendf() 
{
	UpdateData (true);

	m_itemData->m_conversationItem->setTokenDFFamilyId (static_cast<int> (m_tokenDFCtrl.GetItemData (m_tokenDFCtrl.GetCurSel ())));

	GetDocument ()->SetModifiedFlag (true);
}

// ======================================================================

