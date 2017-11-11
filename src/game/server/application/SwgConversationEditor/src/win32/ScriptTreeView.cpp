// ======================================================================
//
// ScriptTreeView.cpp
// asommers 2003-09-23
//
// copyright2003, sony online entertainment
// 
// ======================================================================

#include "FirstSwgConversationEditor.h"
#include "ScriptTreeView.h"

#include "Conversation.h"
#include "RecentDirectory.h"
#include "ScriptFrame.h"
#include "ScriptGroup.h"
#include "sharedUtility/FileName.h"
#include "SwgConversationEditor.h"
#include "SwgConversationEditorDoc.h"
#include "WarningFrame.h"

#define HIDE_DEFAULT 1

// ======================================================================
// ScriptTreeViewNamespace
// ======================================================================

namespace ScriptTreeViewNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int getUniqueFamilyId (ScriptGroup const * const scriptGroup)
	{
		NOT_NULL (scriptGroup);

		int familyId = 1;
		while (scriptGroup->hasFamily (familyId))
			++familyId;

		return familyId;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool familyExists (ScriptGroup const * const scriptGroup, char const * const familyName)
	{
		NOT_NULL (scriptGroup);

		int const numberOfFamilies = scriptGroup->getNumberOfFamilies ();
		for (int i = 0; i < numberOfFamilies; ++i)
		{
			int const familyId = scriptGroup->getFamilyId (i);

			if (strcmp (familyName, scriptGroup->getFamilyName (familyId)) == 0)
				return true;
		}

		return false;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	CString const createUniqueScriptName (ScriptGroup const * const scriptGroup, CString const & rootName, bool const duplicate)
	{
		NOT_NULL (scriptGroup);

		CString familyName;
		int familyIndex = 1;
		do
		{
			if (duplicate)
				familyName.Format ("%s_%i", rootName, familyIndex++);
			else
				familyName.Format ("%s%04i", rootName, familyIndex++);
		}
		while (familyExists (scriptGroup, familyName));

		return familyName;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace ScriptTreeViewNamespace;

// ======================================================================

// ======================================================================

IMPLEMENT_DYNCREATE(ScriptTreeView, CTreeView)

ScriptTreeView::ScriptTreeView() :
	m_imageListSet (false),
	m_imageList (),
	m_rootLibrary (0),
 	m_rootCondition (0),
	m_rootAction (0),
	m_rootTokenTO (0),
	m_rootTokenDI (0),
	m_rootTokenDF (0),
	m_rootLabel (0),
	m_rootTrigger (0)
{
}

ScriptTreeView::~ScriptTreeView()
{
}

// ----------------------------------------------------------------------

void ScriptTreeView::selectItem (HTREEITEM treeItem)
{
	IGNORE_RETURN (GetTreeCtrl ().SelectItem (treeItem));
	IGNORE_RETURN (GetTreeCtrl ().EnsureVisible (treeItem));
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isCondition (HTREEITEM const treeItem) const
{
	return GetTreeCtrl ().GetParentItem (treeItem) == m_rootCondition;
}

// ----------------------------------------------------------------------

HTREEITEM ScriptTreeView::getSelectedCondition () const
{
	//-- am i pointing to anything?
	HTREEITEM const treeItem = GetTreeCtrl ().GetSelectedItem ();

	if (treeItem)
		return isCondition (treeItem) ? treeItem : 0;
	else
		return 0;
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isConditionSelected () const
{
	return getSelectedCondition () != 0;
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isAction (HTREEITEM const treeItem) const
{
	return GetTreeCtrl ().GetParentItem (treeItem) == m_rootAction;
}

// ----------------------------------------------------------------------

HTREEITEM ScriptTreeView::getSelectedAction () const
{
	//-- am i pointing to anything?
	HTREEITEM const treeItem = GetTreeCtrl ().GetSelectedItem ();

	if (treeItem)
		return isAction (treeItem) ? treeItem : 0;
	else
		return 0;
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isActionSelected () const
{
	return getSelectedAction () != 0;
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isTokenTO (HTREEITEM const treeItem) const
{
	return GetTreeCtrl ().GetParentItem (treeItem) == m_rootTokenTO;
}

// ----------------------------------------------------------------------

HTREEITEM ScriptTreeView::getSelectedTokenTO () const
{
	//-- am i pointing to anything?
	HTREEITEM const treeItem = GetTreeCtrl ().GetSelectedItem ();

	if (treeItem)
		return isTokenTO (treeItem) ? treeItem : 0;
	else
		return 0;
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isTokenTOSelected () const
{
	return getSelectedTokenTO () != 0;
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isTokenDI (HTREEITEM const treeItem) const
{
	return GetTreeCtrl ().GetParentItem (treeItem) == m_rootTokenDI;
}

// ----------------------------------------------------------------------

HTREEITEM ScriptTreeView::getSelectedTokenDI () const
{
	//-- am i pointing to anything?
	HTREEITEM const treeItem = GetTreeCtrl ().GetSelectedItem ();

	if (treeItem)
		return isTokenDI (treeItem) ? treeItem : 0;
	else
		return 0;
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isTokenDISelected () const
{
	return getSelectedTokenDI () != 0;
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isTokenDF (HTREEITEM const treeItem) const
{
	return GetTreeCtrl ().GetParentItem (treeItem) == m_rootTokenDF;
}

// ----------------------------------------------------------------------

HTREEITEM ScriptTreeView::getSelectedTokenDF () const
{
	//-- am i pointing to anything?
	HTREEITEM const treeItem = GetTreeCtrl ().GetSelectedItem ();

	if (treeItem)
		return isTokenDF (treeItem) ? treeItem : 0;
	else
		return 0;
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isTokenDFSelected () const
{
	return getSelectedTokenDF () != 0;
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isLibrary (HTREEITEM const treeItem) const
{
	return GetTreeCtrl ().GetParentItem (treeItem) == m_rootLibrary;
}

// ----------------------------------------------------------------------

HTREEITEM ScriptTreeView::getSelectedLibrary () const
{
	//-- am i pointing to anything?
	HTREEITEM const treeItem = GetTreeCtrl ().GetSelectedItem ();

	if (treeItem)
		return isLibrary (treeItem) ? treeItem : 0;
	else
		return 0;
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isLibrarySelected () const
{
	return getSelectedLibrary () != 0;
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isLabel (HTREEITEM const treeItem) const
{
	return GetTreeCtrl ().GetParentItem (treeItem) == m_rootLabel;
}

// ----------------------------------------------------------------------

HTREEITEM ScriptTreeView::getSelectedLabel () const
{
	//-- am i pointing to anything?
	HTREEITEM const treeItem = GetTreeCtrl ().GetSelectedItem ();

	if (treeItem)
		return isLabel (treeItem) ? treeItem : 0;
	else
		return 0;
}

// ----------------------------------------------------------------------

bool ScriptTreeView::isLabelSelected () const
{
	return getSelectedLabel () != 0;
}

// ======================================================================

BEGIN_MESSAGE_MAP(ScriptTreeView, CTreeView)
	//{{AFX_MSG_MAP(ScriptTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID__SCRIPT_ADDACTION, OnScriptAddaction)
	ON_COMMAND(ID__SCRIPT_ADDCONDITION, OnScriptAddcondition)
	ON_COMMAND(ID__SCRIPT_DELETE, OnScriptDelete)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	ON_COMMAND(ID__SCRIPT_RENAME, OnScriptRename)
	ON_UPDATE_COMMAND_UI(ID__SCRIPT_RENAME, OnUpdateScriptRename)
	ON_COMMAND(ID__SCRIPT_ADDLIBRARY, OnScriptAddlibrary)
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	ON_COMMAND(ID_BUTTON_DELETEUNUSED, OnButtonDeleteunused)
	ON_COMMAND(ID_BUTTON_DELETE, OnButtonDelete)
	ON_COMMAND(ID__SCRIPT_DUPLICATE, OnScriptDuplicate)
	ON_UPDATE_COMMAND_UI(ID__SCRIPT_DUPLICATE, OnUpdateScriptDuplicate)
	ON_COMMAND(ID__SCRIPT_ADDLABEL, OnScriptAddlabel)
	ON_COMMAND(ID__SCRIPT_ADDTOKENTO, OnScriptAddtokento)
	ON_COMMAND(ID__SCRIPT_ADDTOKENDI, OnScriptAddtokendi)
	ON_COMMAND(ID__SCRIPT_ADDTOKENDF, OnScriptAddtokendf)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ======================================================================

void ScriptTreeView::OnDraw(CDC* /*pDC*/)
{
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void ScriptTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void ScriptTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void ScriptTreeView::OnInitialUpdate() 
{
	CTreeView::OnInitialUpdate();

	//-- set the tree's image list
	if (!m_imageListSet)
	{
		IGNORE_RETURN (m_imageList.Create (IDB_BITMAP_SCRIPTTREEVIEW, 16, 1, RGB (255,255,255)));
		IGNORE_RETURN (GetTreeCtrl ().SetImageList (&m_imageList, TVSIL_NORMAL));

		m_imageListSet = true;
	}

	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	{
		m_rootCondition = GetTreeCtrl ().InsertItem ("Conditions", TVIT_root, TVIT_root);

		ScriptGroup const * const conditionGroup = document->getConditionGroup ();
#if HIDE_DEFAULT
		for (int i = 1; i < conditionGroup->getNumberOfFamilies (); ++i)
#else
		for (int i = 0; i < conditionGroup->getNumberOfFamilies (); ++i)
#endif
		{
			int const familyId = conditionGroup->getFamilyId (i);
			HTREEITEM conditionItem = GetTreeCtrl ().InsertItem (conditionGroup->getFamilyName (familyId), TVIT_condition, TVIT_conditionSelected, m_rootCondition, TVI_SORT);
			GetTreeCtrl ().SetItemData (conditionItem, static_cast<DWORD> (familyId));
		}

		GetTreeCtrl ().Expand (m_rootCondition, TVE_EXPAND);
	}	

	{
		m_rootAction = GetTreeCtrl ().InsertItem ("Actions", TVIT_root, TVIT_root);

		ScriptGroup const * const actionGroup = document->getActionGroup ();
#if HIDE_DEFAULT
		for (int i = 1; i < actionGroup->getNumberOfFamilies (); ++i)
#else
		for (int i = 0; i < actionGroup->getNumberOfFamilies (); ++i)
#endif
		{
			int const familyId = actionGroup->getFamilyId (i);
			HTREEITEM actionItem = GetTreeCtrl ().InsertItem (actionGroup->getFamilyName (familyId), TVIT_action, TVIT_actionSelected, m_rootAction, TVI_SORT);
			GetTreeCtrl ().SetItemData (actionItem, static_cast<DWORD> (familyId));
		}

		GetTreeCtrl ().Expand (m_rootAction, TVE_EXPAND);
	}

	{
		m_rootTokenTO = GetTreeCtrl ().InsertItem ("%TO Tokens", TVIT_root, TVIT_root);

		ScriptGroup const * const tokenTOGroup = document->getTokenTOGroup ();
#if HIDE_DEFAULT
		for (int i = 1; i < tokenTOGroup->getNumberOfFamilies (); ++i)
#else
		for (int i = 0; i < tokenTOGroup->getNumberOfFamilies (); ++i)
#endif
		{
			int const familyId = tokenTOGroup->getFamilyId (i);
			HTREEITEM tokenTOItem = GetTreeCtrl ().InsertItem (tokenTOGroup->getFamilyName (familyId), TVIT_tokenTO, TVIT_tokenTOSelected, m_rootTokenTO, TVI_SORT);
			GetTreeCtrl ().SetItemData (tokenTOItem, static_cast<DWORD> (familyId));
		}

		GetTreeCtrl ().Expand (m_rootTokenTO, TVE_EXPAND);
	}	

	{
		m_rootTokenDI = GetTreeCtrl ().InsertItem ("%DI Tokens", TVIT_root, TVIT_root);

		ScriptGroup const * const tokenDIGroup = document->getTokenDIGroup ();
#if HIDE_DEFAULT
		for (int i = 1; i < tokenDIGroup->getNumberOfFamilies (); ++i)
#else
		for (int i = 0; i < tokenDIGroup->getNumberOfFamilies (); ++i)
#endif
		{
			int const familyId = tokenDIGroup->getFamilyId (i);
			HTREEITEM tokenDIItem = GetTreeCtrl ().InsertItem (tokenDIGroup->getFamilyName (familyId), TVIT_tokenDI, TVIT_tokenDISelected, m_rootTokenDI, TVI_SORT);
			GetTreeCtrl ().SetItemData (tokenDIItem, static_cast<DWORD> (familyId));
		}

		GetTreeCtrl ().Expand (m_rootTokenDI, TVE_EXPAND);
	}	

	{
		m_rootTokenDF = GetTreeCtrl ().InsertItem ("%DF Tokens", TVIT_root, TVIT_root);

		ScriptGroup const * const tokenDFGroup = document->getTokenDFGroup ();
#if HIDE_DEFAULT
		for (int i = 1; i < tokenDFGroup->getNumberOfFamilies (); ++i)
#else
		for (int i = 0; i < tokenDFGroup->getNumberOfFamilies (); ++i)
#endif
		{
			int const familyId = tokenDFGroup->getFamilyId (i);
			HTREEITEM tokenDFItem = GetTreeCtrl ().InsertItem (tokenDFGroup->getFamilyName (familyId), TVIT_tokenDF, TVIT_tokenDFSelected, m_rootTokenDF, TVI_SORT);
			GetTreeCtrl ().SetItemData (tokenDFItem, static_cast<DWORD> (familyId));
		}

		GetTreeCtrl ().Expand (m_rootTokenDF, TVE_EXPAND);
	}	

	{
		m_rootLibrary = GetTreeCtrl ().InsertItem ("Libraries", TVIT_root, TVIT_root);

		Conversation::LibrarySet const & librarySet = document->getConversation ()->getLibrarySet ();
		Conversation::LibrarySet::iterator end = librarySet.end ();
		for (Conversation::LibrarySet::iterator iterator = librarySet.begin (); iterator != end; ++iterator)
			GetTreeCtrl ().InsertItem (iterator->c_str (), TVIT_library, TVIT_library, m_rootLibrary, TVI_SORT);

		GetTreeCtrl ().Expand (m_rootLibrary, TVE_EXPAND);
	}

	{
		m_rootLabel = GetTreeCtrl ().InsertItem ("Labels", TVIT_root, TVIT_root);

		ScriptGroup const * const labelGroup = document->getLabelGroup ();
#if HIDE_DEFAULT
		for (int i = 2; i <labelGroup->getNumberOfFamilies (); ++i)
#else
		for (int i = 0; i < labelGroup->getNumberOfFamilies (); ++i)
#endif
		{
			int const familyId = labelGroup->getFamilyId (i);
			HTREEITEM labelItem = GetTreeCtrl ().InsertItem (labelGroup->getFamilyName (familyId), TVIT_label, TVIT_label, m_rootLabel, TVI_SORT);
			GetTreeCtrl ().SetItemData (labelItem, static_cast<DWORD> (familyId));
		}

		GetTreeCtrl ().Expand (m_rootLabel, TVE_EXPAND);
	}

	{
		m_rootTrigger = GetTreeCtrl ().InsertItem ("Triggers", TVIT_root, TVIT_root);
	}

	GetTreeCtrl ().EnsureVisible (m_rootCondition);
}

// ----------------------------------------------------------------------

BOOL ScriptTreeView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= (TVS_SHOWSELALWAYS | TVS_DISABLEDRAGDROP | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_EDITLABELS);
	
	return CTreeView::PreCreateWindow(cs);
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnSelchanged(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
	HTREEITEM selection = GetTreeCtrl ().GetSelectedItem ();

	//-- if it's a child tell the frame about it to route the message to ShaderView
	if (GetTreeCtrl ().GetParentItem (selection) == m_rootCondition)
		safe_cast<ScriptFrame *> (GetParentFrame ())->editCondition (GetTreeCtrl ().GetItemData (selection));
	else
		if (GetTreeCtrl ().GetParentItem (selection) == m_rootAction)
			safe_cast<ScriptFrame *> (GetParentFrame ())->editAction (GetTreeCtrl ().GetItemData (selection));
		else
			if (GetTreeCtrl ().GetParentItem (selection) == m_rootTokenTO)
				safe_cast<ScriptFrame *> (GetParentFrame ())->editTokenTO (GetTreeCtrl ().GetItemData (selection));
			else
				if (GetTreeCtrl ().GetParentItem (selection) == m_rootTokenDI)
					safe_cast<ScriptFrame *> (GetParentFrame ())->editTokenDI (GetTreeCtrl ().GetItemData (selection));
				else
					if (GetTreeCtrl ().GetParentItem (selection) == m_rootTokenDF)
						safe_cast<ScriptFrame *> (GetParentFrame ())->editTokenDF (GetTreeCtrl ().GetItemData (selection));
					else
						if (selection == m_rootTrigger)
							safe_cast<ScriptFrame *> (GetParentFrame ())->editTrigger ();
						else
							safe_cast<ScriptFrame *> (GetParentFrame ())->editNothing ();

	*pResult = 0;
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CTreeView::OnRButtonDown(nFlags, point);

	UINT flags = 0;
	HTREEITEM treeItem = GetTreeCtrl ().HitTest (point, &flags);
	if (treeItem)
	{
		selectItem (treeItem);

		CPoint pt = point;
		ClientToScreen (&pt);

		CMenu menu;
		IGNORE_RETURN (menu.LoadMenu (IDR_MENU_TREEVIEW));

		CMenu* rootMenu = menu.GetSubMenu (0);
		CMenu* subMenu = rootMenu->GetSubMenu (2);
		IGNORE_RETURN (subMenu->TrackPopupMenu (TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this));
	}
}

// ----------------------------------------------------------------------

void ScriptTreeView::addFamily (HTREEITEM rootItem, ScriptGroup * const scriptGroup, CString const & rootName, TreeViewIconType const icon, SwgConversationEditorDoc::CreateEmptyScriptFunction createEmptyScriptFunction)
{
	NOT_NULL (scriptGroup);

	//-- create new family
	int const familyId = getUniqueFamilyId (scriptGroup);

	CString const newFamilyName = createUniqueScriptName (scriptGroup, rootName, false);
	scriptGroup->addFamily (familyId, newFamilyName);

	if (createEmptyScriptFunction)
		scriptGroup->setFamilyText (familyId, createEmptyScriptFunction (newFamilyName));

	//-- insert into tree
	HTREEITEM const treeItem = GetTreeCtrl ().InsertItem (newFamilyName, icon, icon + 1, rootItem, TVI_SORT);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (treeItem, static_cast<DWORD> (familyId)));

	selectItem (treeItem);
	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_scriptChanged);
	GetDocument ()->SetModifiedFlag ();

	IGNORE_RETURN (GetTreeCtrl ().EditLabel (treeItem));

	Invalidate ();
}

//-------------------------------------------------------------------

void ScriptTreeView::duplicateFamily (HTREEITEM rootItem, ScriptGroup * const scriptGroup, CString const & rootName, TreeViewIconType const icon, CString const & text)
{
	NOT_NULL (scriptGroup);

	//-- create new family
	int const familyId = getUniqueFamilyId (scriptGroup);

	CString const newFamilyName = createUniqueScriptName (scriptGroup, rootName, true);
	scriptGroup->addFamily (familyId, newFamilyName);
	scriptGroup->setFamilyText (familyId, text);

	//-- insert into tree
	HTREEITEM const treeItem = GetTreeCtrl ().InsertItem (newFamilyName, icon, icon + 1, rootItem, TVI_SORT);
	IGNORE_RETURN (GetTreeCtrl ().SetItemData (treeItem, static_cast<DWORD> (familyId)));

	selectItem (treeItem);
	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_scriptChanged);
	GetDocument ()->SetModifiedFlag ();

	IGNORE_RETURN (GetTreeCtrl ().EditLabel (treeItem));

	Invalidate ();
}

//-------------------------------------------------------------------

void ScriptTreeView::OnScriptAddaction() 
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	addFamily (m_rootAction, document->getActionGroup (), "action", TVIT_action, SwgConversationEditorDoc::createEmptyActionScript);
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnScriptAddcondition() 
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	addFamily (m_rootCondition, document->getConditionGroup (), "condition", TVIT_condition, SwgConversationEditorDoc::createEmptyConditionScript);
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnScriptAddlabel() 
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	addFamily (m_rootLabel, document->getLabelGroup (), "label", TVIT_label, 0);
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnScriptDelete() 
{
	OnButtonDelete ();
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnButtonDelete() 
{
	HTREEITEM treeItem = GetTreeCtrl ().GetSelectedItem ();
	if (treeItem)
	{
		HTREEITEM parentTreeItem = GetTreeCtrl ().GetParentItem (treeItem);
		if (parentTreeItem)
		{
			CString const name = GetTreeCtrl ().GetItemText (treeItem);
			if ((parentTreeItem == m_rootAction && name == "_defaultAction") || 
				(parentTreeItem == m_rootCondition && name == "_defaultCondition" ) || 
				(parentTreeItem == m_rootLibrary && (name == "ai_lib" || name == "chat")))
			{
				MessageBox ("You cannot delete any default items.");
			}
			else
				if ((parentTreeItem == m_rootLibrary) || (parentTreeItem != m_rootLibrary && GetTreeCtrl ().GetItemData (treeItem) != 0))
				{
					SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
					NOT_NULL (document);

					if (isConditionSelected ())
					{
						int const conditionFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
						SwgConversationEditorDoc::ConversationItemList conversationItemList;
						if (document->findConditions (conditionFamilyId, conversationItemList))
						{
							safe_cast<SwgConversationEditorApp *> (AfxGetApp ())->onViewWarning (document);
							document->getWarningFrame ()->clear ();

							CString const message = "Condition " + name + " is currently in use.";

							uint const numberOfConversationItems = conversationItemList.size ();
							for (uint i = 0; i < numberOfConversationItems; ++i)
								document->addWarning (message, conversationItemList [i]);

							MessageBox (message);
						}
						else
						{
							CString message;
							message.Format ("Are you sure you want to delete %s?", name);

							if (MessageBox (message, 0, MB_YESNO) == IDYES)
								deleteCondition (treeItem);
						}
					}
					else
					{
						if (isActionSelected ())
						{
							int const actionFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
							SwgConversationEditorDoc::ConversationItemList conversationItemList;
							if (document->findActions (actionFamilyId, conversationItemList))
							{
								safe_cast<SwgConversationEditorApp *> (AfxGetApp ())->onViewWarning (document);
								document->getWarningFrame ()->clear ();

								CString const message = "Action " + name + " is currently in use.  See Output window for details.";

								uint const numberOfConversationItems = conversationItemList.size ();
								for (uint i = 0; i < numberOfConversationItems; ++i)
									document->addWarning (message, conversationItemList [i]);

								MessageBox (message);
							}
							else
							{
								CString message;
								message.Format ("Are you sure you want to delete %s?", name);

								if (MessageBox (message, 0, MB_YESNO) == IDYES)
									deleteAction (treeItem);
							}
						}
						else
							if (isActionSelected ())
							{
								int const actionFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
								SwgConversationEditorDoc::ConversationItemList conversationItemList;
								if (document->findActions (actionFamilyId, conversationItemList))
								{
									safe_cast<SwgConversationEditorApp *> (AfxGetApp ())->onViewWarning (document);
									document->getWarningFrame ()->clear ();

									CString const message = "Action " + name + " is currently in use.  See Output window for details.";

									uint const numberOfConversationItems = conversationItemList.size ();
									for (uint i = 0; i < numberOfConversationItems; ++i)
										document->addWarning (message, conversationItemList [i]);

									MessageBox (message);
								}
								else
								{
									CString message;
									message.Format ("Are you sure you want to delete %s?", name);

									if (MessageBox (message, 0, MB_YESNO) == IDYES)
										deleteAction (treeItem);
								}
							}
							else
								if (isLibrarySelected ())
								{
									deleteLibrary (treeItem);
								}
								else
								{
									if (isLabelSelected ())
									{
										int const labelFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
										SwgConversationEditorDoc::ConversationItemList conversationItemList;
										if (document->findLabels (labelFamilyId, conversationItemList))
										{
											safe_cast<SwgConversationEditorApp *> (AfxGetApp ())->onViewWarning (document);
											document->getWarningFrame ()->clear ();

											CString const message = "Label " + name + " is currently in use.  See Output window for details.";

											uint const numberOfConversationItems = conversationItemList.size ();
											for (uint i = 0; i < numberOfConversationItems; ++i)
												document->addWarning (message, conversationItemList [i]);

											MessageBox (message);
										}
										else
										{
											CString message;
											message.Format ("Are you sure you want to delete %s?", name);

											if (MessageBox (message, 0, MB_YESNO) == IDYES)
												deleteLabel (treeItem);
										}
									}
								}
					}
				}
		}
	}
}

// ----------------------------------------------------------------------

void ScriptTreeView::deleteCondition (HTREEITEM treeItem)
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	int const conditionFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));

	HTREEITEM siblingItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);
	if (!siblingItem)
		siblingItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);

	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (treeItem));
	selectItem (siblingItem);

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_scriptChanged);
	GetDocument ()->SetModifiedFlag ();

	if (conditionFamilyId != 0)
		document->getConditionGroup ()->removeFamily (conditionFamilyId);

	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptTreeView::deleteAction (HTREEITEM treeItem)
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	int const actionFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));

	HTREEITEM siblingItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);
	if (!siblingItem)
		siblingItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);

	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (treeItem));
	selectItem (siblingItem);

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_scriptChanged);
	GetDocument ()->SetModifiedFlag ();

	if (actionFamilyId != 0)
		document->getActionGroup ()->removeFamily (actionFamilyId);

	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptTreeView::deleteTokenTO (HTREEITEM treeItem)
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	int const tokenTOFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));

	HTREEITEM siblingItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);
	if (!siblingItem)
		siblingItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);

	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (treeItem));
	selectItem (siblingItem);

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_scriptChanged);
	GetDocument ()->SetModifiedFlag ();

	if (tokenTOFamilyId != 0)
		document->getTokenTOGroup ()->removeFamily (tokenTOFamilyId);

	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptTreeView::deleteTokenDI (HTREEITEM treeItem)
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	int const tokenDIFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));

	HTREEITEM siblingItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);
	if (!siblingItem)
		siblingItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);

	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (treeItem));
	selectItem (siblingItem);

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_scriptChanged);
	GetDocument ()->SetModifiedFlag ();

	if (tokenDIFamilyId != 0)
		document->getTokenDIGroup ()->removeFamily (tokenDIFamilyId);

	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptTreeView::deleteTokenDF (HTREEITEM treeItem)
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	int const tokenDFFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));

	HTREEITEM siblingItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);
	if (!siblingItem)
		siblingItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);

	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (treeItem));
	selectItem (siblingItem);

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_scriptChanged);
	GetDocument ()->SetModifiedFlag ();

	if (tokenDFFamilyId != 0)
		document->getTokenDFGroup ()->removeFamily (tokenDFFamilyId);

	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptTreeView::deleteLabel (HTREEITEM treeItem)
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	int const labelFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));

	HTREEITEM siblingItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);
	if (!siblingItem)
		siblingItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);

	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (treeItem));
	selectItem (siblingItem);

	GetDocument ()->UpdateAllViews (this, SwgConversationEditorDoc::H_labelChanged);
	GetDocument ()->SetModifiedFlag ();

	if (labelFamilyId != 0)
		document->getLabelGroup ()->removeFamily (labelFamilyId);

	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptTreeView::deleteLibrary (HTREEITEM treeItem)
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	Conversation::LibrarySet & librarySet = document->getLibrarySet ();

	std::string libraryName (GetTreeCtrl ().GetItemText (treeItem));
	Conversation::LibrarySet::iterator iterator = librarySet.find (libraryName);
	if (iterator != librarySet.end ())
		document->getLibrarySet ().erase (iterator);

	HTREEITEM siblingItem = GetTreeCtrl ().GetPrevSiblingItem (treeItem);
	if (!siblingItem)
		siblingItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);

	IGNORE_RETURN (GetTreeCtrl ().DeleteItem (treeItem));
	selectItem (siblingItem);

	GetDocument ()->SetModifiedFlag ();
	Invalidate ();
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnBeginlabeledit(NMHDR * const pNMHDR, LRESULT * const pResult) 
{
	//-- get the tree view display info
	NMTVDISPINFO const * const pTVDispInfo = reinterpret_cast<NMTVDISPINFO const *> (pNMHDR);

	//-- disallow editing by default
	*pResult = 1;

	HTREEITEM const treeItem = pTVDispInfo->item.hItem;

	if ((isCondition (treeItem) || isAction (treeItem) || isLabel (treeItem) || isTokenTO (treeItem) || isTokenDI (treeItem) || isTokenDF (treeItem)) && GetTreeCtrl ().GetItemData (treeItem) != 0)
	{
		//-- limit to 100 characters
		GetTreeCtrl ().GetEditControl()->LimitText (100);

		//-- allow edit
		*pResult = 0;
		return;
	}
}

// ----------------------------------------------------------------------

bool isValidName (char const * name)
{
	while (*name != 0)
	{
		if (!(isalpha (*name) || isdigit (*name) || *name == '_'))
			return false;

		++name;
	}

	return true;
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnEndlabeledit(NMHDR * const pNMHDR, LRESULT * const pResult) 
{
	//-- get the tree view display info
	NMTVDISPINFO * const pTVDispInfo = reinterpret_cast<NMTVDISPINFO *> (pNMHDR);

	HTREEITEM treeItem = pTVDispInfo->item.hItem;

	//-- get the new text string
	char const * const newName = pTVDispInfo->item.pszText;

	if (!newName || istrlen (newName) == 0)
	{
		*pResult = 0;
		return;
	}

	if (isdigit (*newName))
	{
		MessageBox ("Names cannot start with a number.  Please select another name.", "Bad Name");
		*pResult = 0;
		return;
	}

	if (*newName == '_')
	{
		MessageBox ("Names starting with underscores are reserved.  Please select another name.", "Bad Name");
		*pResult = 0;
		return;
	}

	if (!isValidName (newName))
	{
		MessageBox ("Names can only contain alpha-numberic characters with first character not a digit.  Please select another name.", "Bad Name");
		*pResult = 0;
		return;
	}

	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	ScriptGroup * scriptGroup = 0;
	if (isCondition (treeItem))
		scriptGroup = document->getConditionGroup ();
	else
		if (isAction (treeItem))
			scriptGroup = document->getActionGroup ();
		else
			if (isTokenTO (treeItem))
				scriptGroup = document->getTokenTOGroup ();
			else
				if (isTokenDI (treeItem))
					scriptGroup = document->getTokenDIGroup ();
				else
					if (isTokenDF (treeItem))
						scriptGroup = document->getTokenDFGroup ();
					else
						if (isLabel (treeItem))
							scriptGroup = document->getLabelGroup ();

	if (scriptGroup && !familyExists (scriptGroup, newName))
	{
		//-- change name
		int const familyId = GetTreeCtrl ().GetItemData (treeItem);
		scriptGroup->setFamilyName (familyId, newName);

		//-- tell tree about new name
		GetTreeCtrl ().SetItem (&pTVDispInfo->item);

		selectItem (treeItem);

		document->UpdateAllViews (this, SwgConversationEditorDoc::H_scriptChanged);

		*pResult = 1;
	}
	else 
	{
		MessageBox ("A script with this name already exists.  Please select another name.", "Bad Name");
		*pResult = 0;
	}
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnScriptRename() 
{
	if (isActionSelected () || isConditionSelected () || isLabelSelected () || isTokenTOSelected () || isTokenDISelected () || isTokenDFSelected ())
		GetTreeCtrl ().EditLabel (GetTreeCtrl ().GetSelectedItem ());
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnUpdateScriptRename(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isActionSelected () || isConditionSelected () || isLabelSelected () || isTokenTOSelected () || isTokenDISelected () || isTokenDFSelected ());
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnScriptAddlibrary() 
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	Conversation::LibrarySet & librarySet = document->getLibrarySet ();

	CFileDialog dlg (true, "*.scriptlib", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "ScriptLib Files *.scriptlib|*.scriptlib||");
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("Script");
	dlg.m_ofn.lpstrDefExt = "scriptlib";
	dlg.m_ofn.lpstrTitle = "Add Script Library";
	if (dlg.DoModal () == IDOK)
	{
		FileName fileName (dlg.GetPathName ());
		fileName.stripPathAndExt ();

		if (librarySet.insert (std::string (fileName)).second)
		{
			GetTreeCtrl ().InsertItem (fileName, TVIT_library, TVIT_library, m_rootLibrary, TVI_SORT);
			GetTreeCtrl ().Expand (m_rootLibrary, TVE_EXPAND);

			GetDocument ()->SetModifiedFlag ();

			Invalidate ();

			IGNORE_RETURN (RecentDirectory::update ("Script", dlg.GetPathName ()));
		}
	}
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnKeydown (NMHDR * const pNMHDR, LRESULT * const pResult) 
{
	TV_KEYDOWN const * const pTVKeyDown = reinterpret_cast<TV_KEYDOWN const *> (pNMHDR);

	switch (pTVKeyDown->wVKey)
	{
	case VK_DELETE:
		OnButtonDelete ();
		break;
	}

	*pResult = 0;
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnButtonDeleteunused() 
{
	if (MessageBox ("Are you sure you want to delete all unused scripts?", 0, MB_YESNO) == IDYES)
	{
		SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
		NOT_NULL (document);

		int numberOfScriptsDeleted = 0;

		{
			HTREEITEM treeItem = GetTreeCtrl ().GetChildItem (m_rootCondition);
			while (treeItem)
			{
				int const conditionFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
				if (!document->conditionInUse (conditionFamilyId))
				{
					HTREEITEM deleteTreeItem = treeItem;
					treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);
					deleteCondition (deleteTreeItem);

					++numberOfScriptsDeleted;
				}
				else
					treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);
			}
		}

		{
			HTREEITEM treeItem = GetTreeCtrl ().GetChildItem (m_rootAction);
			while (treeItem)
			{
				int const actionFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
				if (!document->actionInUse (actionFamilyId))
				{
					HTREEITEM deleteTreeItem = treeItem;
					treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);
					deleteAction (deleteTreeItem);

					++numberOfScriptsDeleted;
				}
				else
					treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);
			}
		}

		{
			HTREEITEM treeItem = GetTreeCtrl ().GetChildItem (m_rootTokenTO);
			while (treeItem)
			{
				int const tokenTOFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
				if (!document->tokenTOInUse (tokenTOFamilyId))
				{
					HTREEITEM deleteTreeItem = treeItem;
					treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);
					deleteTokenTO (deleteTreeItem);

					++numberOfScriptsDeleted;
				}
				else
					treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);
			}
		}

		{
			HTREEITEM treeItem = GetTreeCtrl ().GetChildItem (m_rootTokenDI);
			while (treeItem)
			{
				int const tokenDIFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
				if (!document->tokenDIInUse (tokenDIFamilyId))
				{
					HTREEITEM deleteTreeItem = treeItem;
					treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);
					deleteTokenDI (deleteTreeItem);

					++numberOfScriptsDeleted;
				}
				else
					treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);
			}
		}

		{
			HTREEITEM treeItem = GetTreeCtrl ().GetChildItem (m_rootTokenDF);
			while (treeItem)
			{
				int const tokenDFFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
				if (!document->tokenDFInUse (tokenDFFamilyId))
				{
					HTREEITEM deleteTreeItem = treeItem;
					treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);
					deleteTokenDF (deleteTreeItem);

					++numberOfScriptsDeleted;
				}
				else
					treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);
			}
		}

		{
			HTREEITEM treeItem = GetTreeCtrl ().GetChildItem (m_rootLabel);
			while (treeItem)
			{
				int const labelFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
				if (!document->labelInUseAsLabelOrLink (labelFamilyId))
				{
					HTREEITEM deleteTreeItem = treeItem;
					treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);
					deleteLabel (deleteTreeItem);

					++numberOfScriptsDeleted;
				}
				else
					treeItem = GetTreeCtrl ().GetNextSiblingItem (treeItem);
			}
		}

		CString message;
		message.Format ("%i script%s deleted.", numberOfScriptsDeleted, numberOfScriptsDeleted == 1 ? "" : "s");
		MessageBox (message);
	}
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnScriptDuplicate() 
{
	if (isActionSelected ())
	{
		SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
		NOT_NULL (document);

		HTREEITEM treeItem = getSelectedAction ();
		int const actionFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
		if (actionFamilyId)
		{
			ScriptGroup * const actionGroup = document->getActionGroup ();
			std::string const name = actionGroup->getFamilyName (actionFamilyId);
			std::string const text = actionGroup->getFamilyText (actionFamilyId);
			duplicateFamily (m_rootAction, document->getActionGroup (), name.c_str (), TVIT_action, text.c_str ());
		}
	}
	else
		if (isConditionSelected ())
		{
			SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
			NOT_NULL (document);

			HTREEITEM treeItem = getSelectedCondition ();
			int const conditionFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
			if (conditionFamilyId)
			{
				ScriptGroup * const conditionGroup = document->getConditionGroup ();
				std::string const name = conditionGroup->getFamilyName (conditionFamilyId);
				std::string const text = conditionGroup->getFamilyText (conditionFamilyId);
				duplicateFamily (m_rootCondition, document->getConditionGroup (), name.c_str (), TVIT_condition, text.c_str ());
			}
		}
		else
			if (isTokenTOSelected ())
			{
				SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
				NOT_NULL (document);

				HTREEITEM treeItem = getSelectedTokenTO ();
				int const tokenTOFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
				if (tokenTOFamilyId)
				{
					ScriptGroup * const tokenTOGroup = document->getTokenTOGroup ();
					std::string const name = tokenTOGroup->getFamilyName (tokenTOFamilyId);
					std::string const text = tokenTOGroup->getFamilyText (tokenTOFamilyId);
					duplicateFamily (m_rootTokenTO, document->getTokenTOGroup (), name.c_str (), TVIT_tokenTO, text.c_str ());
				}
			}
			else
				if (isTokenDISelected ())
				{
					SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
					NOT_NULL (document);

					HTREEITEM treeItem = getSelectedTokenDI ();
					int const tokenDIFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
					if (tokenDIFamilyId)
					{
						ScriptGroup * const tokenDIGroup = document->getTokenDIGroup ();
						std::string const name = tokenDIGroup->getFamilyName (tokenDIFamilyId);
						std::string const text = tokenDIGroup->getFamilyText (tokenDIFamilyId);
						duplicateFamily (m_rootTokenDI, document->getTokenDIGroup (), name.c_str (), TVIT_tokenDI, text.c_str ());
					}
				}
				else
					if (isTokenDFSelected ())
					{
						SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
						NOT_NULL (document);

						HTREEITEM treeItem = getSelectedTokenDF ();
						int const tokenDFFamilyId = static_cast<int> (GetTreeCtrl ().GetItemData (treeItem));
						if (tokenDFFamilyId)
						{
							ScriptGroup * const tokenDFGroup = document->getTokenDFGroup ();
							std::string const name = tokenDFGroup->getFamilyName (tokenDFFamilyId);
							std::string const text = tokenDFGroup->getFamilyText (tokenDFFamilyId);
							duplicateFamily (m_rootTokenDF, document->getTokenDFGroup (), name.c_str (), TVIT_tokenDF, text.c_str ());
						}
					}
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnUpdateScriptDuplicate(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (isActionSelected () || isConditionSelected () || isTokenTOSelected () || isTokenDISelected () || isTokenDFSelected ());
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnScriptAddtokento() 
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	addFamily (m_rootTokenTO, document->getTokenTOGroup (), "tokenTO", TVIT_tokenTO, SwgConversationEditorDoc::createEmptyTokenTOScript);
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnScriptAddtokendi() 
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	addFamily (m_rootTokenDI, document->getTokenDIGroup (), "tokenDI", TVIT_tokenDI, SwgConversationEditorDoc::createEmptyTokenDIScript);
}

// ----------------------------------------------------------------------

void ScriptTreeView::OnScriptAddtokendf() 
{
	SwgConversationEditorDoc * const document = safe_cast<SwgConversationEditorDoc *> (GetDocument ());
	NOT_NULL (document);

	addFamily (m_rootTokenDF, document->getTokenDFGroup (), "tokenDF", TVIT_tokenDF, SwgConversationEditorDoc::createEmptyTokenDFScript);
}

// ======================================================================

