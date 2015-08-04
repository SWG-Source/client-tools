// ======================================================================
//
// FormPropertyView.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgDraftSchematicEditor.h"
#include "FormPropertyView.h"

#include "Configuration.h"
#include "RecentDirectory.h"
#include "SwgDraftSchematicEditorDoc.h"

// ======================================================================

IMPLEMENT_DYNCREATE(FormPropertyView, CFormView)

// ----------------------------------------------------------------------

FormPropertyView::FormPropertyView()
	: CFormView(FormPropertyView::IDD)
{
	//{{AFX_DATA_INIT(FormPropertyView)
	m_manufactureScript = _T("");
	m_serverCraftedObjectTemplate = _T("");
	m_sharedCraftedObjectTemplate = _T("");
	m_overrideCraftingType = FALSE;
	m_overrideXpType = FALSE;
	m_overrideDestroyIngredients = FALSE;
	m_overrideDestroyIngredientsValue = FALSE;
	m_complexity = 0;
	m_itemsPerContainer = 0;
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

FormPropertyView::~FormPropertyView()
{
}

// ----------------------------------------------------------------------

void FormPropertyView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormPropertyView)
	DDX_Control(pDX, IDC_CHECK_OVERRIDEDESTROYINGREDIENTVALUE, m_overrideDestroyIngredientsButton);
	DDX_Control(pDX, IDC_COMBO_OVERRIDECRAFTINGTYPEVALUE, m_overrideCraftingTypeComboBox);
	DDX_Control(pDX, IDC_COMBO_OVERRIDEXPTYPEVALUE, m_overrideXpTypeComboBox);
	DDX_Control(pDX, IDC_COMBO_SERVERBASEDRAFTSCHEMATIC, m_serverBaseDraftSchematicCtrl);
	DDX_Control(pDX, IDC_BUTTON_BROWSESHAREDCRAFTEDOBJECTTEMPLATE, m_browseSharedCraftedObjectTemplate);
	DDX_Control(pDX, IDC_BUTTON_BROWSESERVERCRAFTEDOBJECTTEMPLATE, m_browseServerCraftedObjectTemplate);
	DDX_Control(pDX, IDC_BUTTON_BROWSEMANUFACTURESCRIPT, m_browseManufactureScriptButton);
	DDX_Text(pDX, IDC_EDIT_MANUFACTURESCRIPT, m_manufactureScript);
	DDX_Text(pDX, IDC_EDIT_SERVERCRAFTEDOBJECTTEMPLATE, m_serverCraftedObjectTemplate);
	DDX_Text(pDX, IDC_EDIT_SHAREDCRAFTEDOBJECTTEMPLATE, m_sharedCraftedObjectTemplate);
	DDX_Check(pDX, IDC_CHECK_OVERRIDECRAFTINGTYPE, m_overrideCraftingType);
	DDX_Check(pDX, IDC_CHECK_OVERRIDEXPTYPE, m_overrideXpType);
	DDX_Check(pDX, IDC_CHECK_OVERRIDEDESTROYINGREDIENT, m_overrideDestroyIngredients);
	DDX_Check(pDX, IDC_CHECK_OVERRIDEDESTROYINGREDIENTVALUE, m_overrideDestroyIngredientsValue);
	DDX_Text(pDX, IDC_EDIT_COMPLEXITY, m_complexity);
	DDX_Text(pDX, IDC_EDIT_ITEMSPERCONTAINER, m_itemsPerContainer);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FormPropertyView, CFormView)
	//{{AFX_MSG_MAP(FormPropertyView)
	ON_BN_CLICKED(IDC_CHECK_OVERRIDECRAFTINGTYPE, OnCheckOverridecraftingtype)
	ON_BN_CLICKED(IDC_CHECK_OVERRIDEDESTROYINGREDIENT, OnCheckOverridedestroyingredient)
	ON_BN_CLICKED(IDC_CHECK_OVERRIDEXPTYPE, OnCheckOverridexptype)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEMANUFACTURESCRIPT, OnButtonBrowsemanufacturescript)
	ON_BN_CLICKED(IDC_BUTTON_BROWSESERVERCRAFTEDOBJECTTEMPLATE, OnButtonBrowseservercraftedobjecttemplate)
	ON_BN_CLICKED(IDC_BUTTON_BROWSESHAREDCRAFTEDOBJECTTEMPLATE, OnButtonBrowsesharedcraftedobjecttemplate)
	ON_BN_CLICKED(IDC_CHECK_OVERRIDEDESTROYINGREDIENTVALUE, OnCheckOverridedestroyingredientvalue)
	ON_CBN_SELCHANGE(IDC_COMBO_OVERRIDECRAFTINGTYPEVALUE, OnSelchangeComboOverridecraftingtypevalue)
	ON_CBN_SELCHANGE(IDC_COMBO_OVERRIDEXPTYPEVALUE, OnSelchangeComboOverridexptypevalue)
	ON_CBN_SELCHANGE(IDC_COMBO_SERVERBASEDRAFTSCHEMATIC, OnSelchangeComboServerbasedraftschematic)
	ON_BN_CLICKED(IDC_BUTTON_GENERATESHAREDCRAFTEDOBJECTTEMPLATENAME, OnButtonGeneratesharedcraftedobjecttemplatename)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_EN_CHANGE(IDC_EDIT_COMPLEXITY, OnChangeEditComplexity)
	ON_EN_CHANGE(IDC_EDIT_ITEMSPERCONTAINER, OnChangeEditItemspercontainer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

#ifdef _DEBUG
void FormPropertyView::AssertValid() const
{
	CFormView::AssertValid();
}

void FormPropertyView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void FormPropertyView::update ()
{
	SwgDraftSchematicEditorDoc const * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic const & draftSchematic = document->getDraftSchematic ();

	m_overrideCraftingType = draftSchematic.getOverrideCraftingType () ? TRUE : FALSE;
	Configuration::populateCraftingTypes (m_overrideCraftingTypeComboBox);
	m_overrideCraftingTypeComboBox.EnableWindow (m_overrideCraftingType);
	m_overrideCraftingTypeComboBox.SelectString (0, draftSchematic.getOverrideCraftingTypeValue ());

	m_overrideXpType = draftSchematic.getOverrideXpType () ? TRUE : FALSE;
	Configuration::populateXpTypes (m_overrideXpTypeComboBox);
	m_overrideXpTypeComboBox.EnableWindow (m_overrideXpType);
	m_overrideXpTypeComboBox.SelectString (0, draftSchematic.getOverrideXpTypeValue ());

	m_overrideDestroyIngredients = draftSchematic.getOverrideDestroyIngredients () ? TRUE : FALSE;
	m_overrideDestroyIngredientsButton.EnableWindow (m_overrideDestroyIngredients);
	m_overrideDestroyIngredientsValue = draftSchematic.getOverrideDestroyIngredientsValue ();

	Configuration::populateBaseServerDraftSchematicObjectTemplates (m_serverBaseDraftSchematicCtrl);
	m_serverBaseDraftSchematicCtrl.SelectString (0, draftSchematic.getBaseServerObjectTemplate ());

	m_complexity = draftSchematic.getComplexity ();
	m_itemsPerContainer = draftSchematic.getItemsPerContainer ();
	m_manufactureScript = draftSchematic.getManufactureScript ();
	m_serverCraftedObjectTemplate = draftSchematic.getCraftedServerObjectTemplate ();
	m_sharedCraftedObjectTemplate = draftSchematic.getCraftedSharedObjectTemplate ();

	UpdateData (false);
}

// ----------------------------------------------------------------------

void FormPropertyView::OnCheckOverridecraftingtype() 
{
	SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic & draftSchematic = document->getDraftSchematic ();
	draftSchematic.setOverrideCraftingType (!draftSchematic.getOverrideCraftingType ());
	update ();

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormPropertyView::OnCheckOverridedestroyingredient() 
{
	SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic & draftSchematic = document->getDraftSchematic ();
	draftSchematic.setOverrideDestroyIngredients (!draftSchematic.getOverrideDestroyIngredients ());
	update ();

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormPropertyView::OnCheckOverridexptype() 
{
	SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic & draftSchematic = document->getDraftSchematic ();
	draftSchematic.setOverrideXpType (!draftSchematic.getOverrideXpType ());
	update ();

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormPropertyView::OnButtonBrowsemanufacturescript() 
{
	CFileDialog dlg (true, "*.script", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Script Files *.script|*.script||");
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ManufactureScript");
	dlg.m_ofn.lpstrDefExt = "script";
	dlg.m_ofn.lpstrTitle= "Select Script";

	if (dlg.DoModal () == IDOK)
	{
		RecentDirectory::update ("ManufactureScript", dlg.GetPathName ());
		
		SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
		DraftSchematic & draftSchematic = document->getDraftSchematic ();
		CString manufactureScript = dlg.GetPathName ();
		manufactureScript.MakeLower ();
		manufactureScript.Replace ('\\', '/');
		int const index = manufactureScript.Find ("script/");
		if (index != -1)
		{
			manufactureScript = manufactureScript.Right (manufactureScript.GetLength () - index - 7);
			manufactureScript.Replace (".script", "");
			manufactureScript.Replace ('/', '.');
		}

		draftSchematic.setManufactureScript (manufactureScript);
		update ();

		GetDocument ()->SetModifiedFlag (true);
	}
}

// ----------------------------------------------------------------------

void FormPropertyView::OnButtonBrowseservercraftedobjecttemplate() 
{
	CFileDialog dlg (true, "*.iff", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Object Template Files *.iff|*.iff||");
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("ServerCraftedObjectTemplate");
	dlg.m_ofn.lpstrDefExt = "iff";
	dlg.m_ofn.lpstrTitle= "Select Crafted Server Object Template";

	if (dlg.DoModal () == IDOK)
	{
		RecentDirectory::update ("ServerCraftedObjectTemplate", dlg.GetPathName ());
		
		SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
		DraftSchematic & draftSchematic = document->getDraftSchematic ();
		CString craftedServerObjectTemplate = dlg.GetPathName ();
		craftedServerObjectTemplate.MakeLower ();
		craftedServerObjectTemplate.Replace ('\\', '/');
		int const index = craftedServerObjectTemplate.Find ("object/");
		if (index != -1)
			craftedServerObjectTemplate = craftedServerObjectTemplate.Right (craftedServerObjectTemplate.GetLength () - index);

		draftSchematic.setCraftedServerObjectTemplate (craftedServerObjectTemplate);
		update ();

		GetDocument ()->SetModifiedFlag (true);
	}
}

// ----------------------------------------------------------------------

void FormPropertyView::OnButtonBrowsesharedcraftedobjecttemplate() 
{
	CFileDialog dlg (true, "*.iff", 0, (OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR), "Object Template Files *.iff|*.iff||");
	dlg.m_ofn.lpstrInitialDir = RecentDirectory::find ("SharedCraftedObjectTemplate");
	dlg.m_ofn.lpstrDefExt = "iff";
	dlg.m_ofn.lpstrTitle= "Select Crafted Shared Object Template";

	if (dlg.DoModal () == IDOK)
	{
		RecentDirectory::update ("SharedCraftedObjectTemplate", dlg.GetPathName ());
		
		SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
		DraftSchematic & draftSchematic = document->getDraftSchematic ();
		CString craftedSharedObjectTemplate = dlg.GetPathName ();
		craftedSharedObjectTemplate.MakeLower ();
		craftedSharedObjectTemplate.Replace ('\\', '/');
		int const index = craftedSharedObjectTemplate.Find ("object/");
		if (index != -1)
			craftedSharedObjectTemplate = craftedSharedObjectTemplate.Right (craftedSharedObjectTemplate.GetLength () - index);

		draftSchematic.setCraftedSharedObjectTemplate (craftedSharedObjectTemplate);
		update ();

		GetDocument ()->SetModifiedFlag (true);
	}
}

// ----------------------------------------------------------------------

void FormPropertyView::OnButtonGeneratesharedcraftedobjecttemplatename() 
{
	SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic & draftSchematic = document->getDraftSchematic ();
	CString craftedSharedObjectTemplate = draftSchematic.getCraftedServerObjectTemplate ();
	if (!craftedSharedObjectTemplate.IsEmpty ())
	{
		int const index = craftedSharedObjectTemplate.ReverseFind ('/');
		craftedSharedObjectTemplate.Insert (index + 1, "shared_");
		draftSchematic.setCraftedSharedObjectTemplate (craftedSharedObjectTemplate);
		update ();

		GetDocument ()->SetModifiedFlag (true);
	}
}

// ----------------------------------------------------------------------

void FormPropertyView::OnCheckOverridedestroyingredientvalue() 
{
	SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic & draftSchematic = document->getDraftSchematic ();
	draftSchematic.setOverrideDestroyIngredientsValue (!draftSchematic.getOverrideDestroyIngredientsValue ());
	update ();

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormPropertyView::OnSelchangeComboOverridecraftingtypevalue() 
{
	SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic & draftSchematic = document->getDraftSchematic ();
	CString result;
	m_overrideCraftingTypeComboBox.GetLBText (m_overrideCraftingTypeComboBox.GetCurSel (), result);
	draftSchematic.setOverrideCraftingTypeValue (result);
	update ();

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormPropertyView::OnSelchangeComboOverridexptypevalue() 
{
	SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic & draftSchematic = document->getDraftSchematic ();
	CString result;
	m_overrideXpTypeComboBox.GetLBText (m_overrideXpTypeComboBox.GetCurSel (), result);
	draftSchematic.setOverrideXpTypeValue (result);
	update ();

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormPropertyView::OnSelchangeComboServerbasedraftschematic() 
{
	SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic & draftSchematic = document->getDraftSchematic ();
	CString result;
	m_serverBaseDraftSchematicCtrl.GetLBText (m_serverBaseDraftSchematicCtrl.GetCurSel (), result);
	draftSchematic.setBaseServerObjectTemplate (result);
	update ();

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormPropertyView::OnEditCopy() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Copy ();
}

// ----------------------------------------------------------------------

void FormPropertyView::OnEditCut() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Cut ();
}

// ----------------------------------------------------------------------

void FormPropertyView::OnEditPaste() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Paste ();
}

// ----------------------------------------------------------------------

void FormPropertyView::OnEditUndo() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Undo ();
}

// ----------------------------------------------------------------------

void FormPropertyView::OnChangeEditComplexity() 
{
	UpdateData (true);

	SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic & draftSchematic = document->getDraftSchematic ();
	draftSchematic.setComplexity (m_complexity);	

	for (int i = 0; i < draftSchematic.getNumberOfAttributes (); ++i)
	{
		DraftSchematic::Attribute * const attribute = draftSchematic.getAttribute (i);
		if (attribute->m_nameStringTable == "crafting" && attribute->m_nameStringId == "complexity")
		{
			attribute->m_minimumValue.Format ("%i", m_complexity);
			attribute->m_maximumValue.Format ("%i", m_complexity);
		}
	}

	update ();

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormPropertyView::OnChangeEditItemspercontainer() 
{
	UpdateData (true);

	SwgDraftSchematicEditorDoc * const document = safe_cast<SwgDraftSchematicEditorDoc *> (GetDocument ());
	DraftSchematic & draftSchematic = document->getDraftSchematic ();
	draftSchematic.setItemsPerContainer (m_itemsPerContainer);	
	update ();

	GetDocument ()->SetModifiedFlag (true);
}

// ======================================================================

