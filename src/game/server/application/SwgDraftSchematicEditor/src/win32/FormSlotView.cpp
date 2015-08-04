// ======================================================================
//
// FormSlotView.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgDraftSchematicEditor.h"
#include "FormSlotView.h"

#include "Configuration.h"
#include "DialogResource.h"
#include "RecentDirectory.h"

// ======================================================================

IMPLEMENT_DYNCREATE(FormSlotView, CFormView)

// ----------------------------------------------------------------------

FormSlotView::FormSlotView()
	: CFormView(FormSlotView::IDD)
{
	//{{AFX_DATA_INIT(FormSlotView)
	m_optional = FALSE;
	m_ingredientName = _T("");
	m_ingredientCount = 0;
	m_nameStringId = _T("");
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

FormSlotView::~FormSlotView()
{
}

// ----------------------------------------------------------------------

void FormSlotView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormSlotView)
	DDX_Control(pDX, IDC_COMBO_NAMESTRINGTABLE, m_nameStringTableComboBox);
	DDX_Control(pDX, IDC_EDIT_NAMESTRINGID, m_nameStringIdEditCtrl);
	DDX_Control(pDX, IDC_COMBO_INGREDIENTTYPE, m_ingredientTypeComboBox);
	DDX_Control(pDX, IDC_BUTTON_BROWSEINGREDIENTNAME, m_browseIngredientNameButton);
	DDX_Check(pDX, IDC_CHECK_OPTIONAL, m_optional);
	DDX_Text(pDX, IDC_EDIT_INGREDIENTNAME, m_ingredientName);
	DDX_Text(pDX, IDC_EDIT_INGREDIENTCOUNT, m_ingredientCount);
	DDX_Text(pDX, IDC_EDIT_NAMESTRINGID, m_nameStringId);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FormSlotView, CFormView)
	//{{AFX_MSG_MAP(FormSlotView)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEINGREDIENTNAME, OnButtonBrowseingredientname)
	ON_BN_CLICKED(IDC_CHECK_OPTIONAL, OnCheckOptional)
	ON_CBN_SELCHANGE(IDC_COMBO_INGREDIENTTYPE, OnSelchangeComboIngredienttype)
	ON_EN_CHANGE(IDC_EDIT_INGREDIENTCOUNT, OnChangeEditIngredientcount)
	ON_EN_CHANGE(IDC_EDIT_NAMESTRINGID, OnChangeEditNamestringid)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_CBN_SELCHANGE(IDC_COMBO_NAMESTRINGTABLE, OnSelchangeComboNamestringtable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

#ifdef _DEBUG
void FormSlotView::AssertValid() const
{
	CFormView::AssertValid();
}

void FormSlotView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

// ======================================================================

void FormSlotView::setItemData (SwgDraftSchematicEditorDoc::ItemData * itemData)
{
	m_itemData = itemData;

	assert (itemData->m_type == SwgDraftSchematicEditorDoc::ItemData::T_slot);
	DraftSchematic::Slot * const slot = itemData->m_slot;

	Configuration::populateStringTables (m_nameStringTableComboBox);
	m_nameStringTableComboBox.SelectString (0, slot->m_nameStringTable);
	m_nameStringId = slot->m_nameStringId;
	m_optional = slot->m_optional;
	Configuration::populateIngredientTypes (m_ingredientTypeComboBox);
	m_ingredientTypeComboBox.SelectString (0, slot->m_ingredientType);
	m_ingredientName = slot->m_ingredientName;
	m_ingredientCount = slot->m_ingredientCount;

	UpdateData (false);
}

// ----------------------------------------------------------------------

void FormSlotView::OnButtonBrowseingredientname() 
{
	DraftSchematic::Slot * const slot = m_itemData->m_slot;
	if (slot->m_ingredientType == "IT_template" || slot->m_ingredientType == "IT_templateGeneric")
	{
		CFileDialog dlg (true, "*.iff", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Object Template Files *.iff|*.iff||");
		dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ServerIngredientObjectTemplate");
		dlg.m_ofn.lpstrDefExt = "iff";
		dlg.m_ofn.lpstrTitle= "Select Ingredient Server Object Template";

		if (dlg.DoModal () == IDOK)
		{
			RecentDirectory::update ("ServerIngredientObjectTemplate", dlg.GetPathName ());
			
			CString serverObjectTemplate = dlg.GetPathName ();
			serverObjectTemplate.MakeLower ();
			serverObjectTemplate.Replace ('\\', '/');
			int const index = serverObjectTemplate.Find ("object/");
			if (index != -1)
				serverObjectTemplate = serverObjectTemplate.Right (serverObjectTemplate.GetLength () - index);

			slot->m_ingredientName = serverObjectTemplate;

			setItemData (m_itemData);
			GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));

			GetDocument ()->SetModifiedFlag (true);
		}
	}
	else
		if (slot->m_ingredientType == "IT_resourceType" || slot->m_ingredientType == "IT_resourceClass")
		{
			DialogResource dlg;
			if (dlg.DoModal () == IDOK)
			{
				slot->m_ingredientName = dlg.m_selection;

				setItemData (m_itemData);
				GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));

				GetDocument ()->SetModifiedFlag (true);
			}
		}
}

// ----------------------------------------------------------------------

void FormSlotView::OnCheckOptional() 
{
	UpdateData (true);

	DraftSchematic::Slot * const slot = m_itemData->m_slot;
	slot->m_optional = !slot->m_optional;
	setItemData (m_itemData);

	GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_slotChanged, 0);
	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormSlotView::OnSelchangeComboIngredienttype() 
{
	DraftSchematic::Slot * const slot = m_itemData->m_slot;
	CString result;
	m_ingredientTypeComboBox.GetLBText (m_ingredientTypeComboBox.GetCurSel (), result);
	slot->m_ingredientType = result;
	slot->m_ingredientName = "";
	setItemData (m_itemData);
	GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormSlotView::OnChangeEditIngredientcount() 
{
	UpdateData (true);

	DraftSchematic::Slot * const slot = m_itemData->m_slot;
	slot->m_ingredientCount = m_ingredientCount;
	setItemData (m_itemData);
	GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormSlotView::OnChangeEditNamestringid() 
{
	UpdateData (true);

	DraftSchematic::Slot * const slot = m_itemData->m_slot;
	CString result;
	m_nameStringIdEditCtrl.GetWindowText (result);
	slot->m_nameStringId = result;
	setItemData (m_itemData);
	GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));
	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormSlotView::OnEditCopy() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Copy ();
}

// ----------------------------------------------------------------------

void FormSlotView::OnEditCut() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Cut ();
}

// ----------------------------------------------------------------------

void FormSlotView::OnEditPaste() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Paste ();
}

// ----------------------------------------------------------------------

void FormSlotView::OnEditUndo() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Undo ();
}

// ----------------------------------------------------------------------

void FormSlotView::OnSelchangeComboNamestringtable() 
{
	DraftSchematic::Slot * const slot = m_itemData->m_slot;
	CString result;
	m_nameStringTableComboBox.GetLBText (m_nameStringTableComboBox.GetCurSel (), result);
	slot->m_nameStringTable = result;
	setItemData (m_itemData);

	GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));
	GetDocument ()->SetModifiedFlag (true);
}

// ======================================================================

