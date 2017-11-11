// ======================================================================
//
// FormAttributeView.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "FirstSwgDraftSchematicEditor.h"
#include "FormAttributeView.h"

#include "Configuration.h"

// ======================================================================

IMPLEMENT_DYNCREATE(FormAttributeView, CFormView)

// ----------------------------------------------------------------------

FormAttributeView::FormAttributeView()
	: CFormView(FormAttributeView::IDD)
{
	//{{AFX_DATA_INIT(FormAttributeView)
	m_valueType = -1;
	m_maximumValue = _T("");
	m_minimumValue = _T("");
	m_nameStringId = _T("");
	m_nameStringTable = _T("");
	m_experimentStringId = _T("");
	m_experimentStringTable = _T("");
	//}}AFX_DATA_INIT
}

// ----------------------------------------------------------------------

FormAttributeView::~FormAttributeView()
{
}

// ----------------------------------------------------------------------

void FormAttributeView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FormAttributeView)
	DDX_Control(pDX, IDC_EDIT_EXPERIMENTSTRINGTABLE, m_experimentStringTableEditCtrl);
	DDX_Control(pDX, IDC_EDIT_EXPERIMENTSTRINGID, m_experimentStringIdEditCtrl);
	DDX_Control(pDX, IDC_EDIT_NAMESTRINGTABLE, m_nameStringTableEditCtrl);
	DDX_Control(pDX, IDC_EDIT_NAMESTRINGID, m_nameStringIdEditCtrl);
	DDX_Control(pDX, IDC_EDIT_MINIMUM, m_minimumEditCtrl);
	DDX_Control(pDX, IDC_EDIT_MAXIMUM, m_maximumEditCtrl);
	DDX_Control(pDX, IDC_COMBO_MINIMUM, m_minimumComboBox);
	DDX_Control(pDX, IDC_COMBO_MAXIMUM, m_maximumComboBox);
	DDX_Radio(pDX, IDC_RADIO_VALUETYPE, m_valueType);
	DDX_Text(pDX, IDC_EDIT_MAXIMUM, m_maximumValue);
	DDX_Text(pDX, IDC_EDIT_MINIMUM, m_minimumValue);
	DDX_Text(pDX, IDC_EDIT_NAMESTRINGID, m_nameStringId);
	DDX_Text(pDX, IDC_EDIT_NAMESTRINGTABLE, m_nameStringTable);
	DDX_Text(pDX, IDC_EDIT_EXPERIMENTSTRINGID, m_experimentStringId);
	DDX_Text(pDX, IDC_EDIT_EXPERIMENTSTRINGTABLE, m_experimentStringTable);
	//}}AFX_DATA_MAP
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(FormAttributeView, CFormView)
	//{{AFX_MSG_MAP(FormAttributeView)
	ON_BN_CLICKED(IDC_RADIO_VALUETYPE, OnRadioValuetype)
	ON_BN_CLICKED(IDC_RADIO_VALUETYPEARMORRATING, OnRadioValuetypearmorrating)
	ON_BN_CLICKED(IDC_RADIO_VALUETYPEDAMAGETYPE, OnRadioValuetypedamagetype)
	ON_CBN_SELCHANGE(IDC_COMBO_MAXIMUM, OnSelchangeComboMaximum)
	ON_CBN_SELCHANGE(IDC_COMBO_MINIMUM, OnSelchangeComboMinimum)
	ON_EN_CHANGE(IDC_EDIT_EXPERIMENTSTRINGID, OnChangeEditExperimentstringid)
	ON_EN_CHANGE(IDC_EDIT_MAXIMUM, OnChangeEditMaximum)
	ON_EN_CHANGE(IDC_EDIT_MINIMUM, OnChangeEditMinimum)
	ON_EN_CHANGE(IDC_EDIT_NAMESTRINGID, OnChangeEditNamestringid)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

#ifdef _DEBUG
void FormAttributeView::AssertValid() const
{
	CFormView::AssertValid();
}

void FormAttributeView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

void FormAttributeView::setItemData (SwgDraftSchematicEditorDoc::ItemData * itemData)
{
	m_itemData = itemData;

	assert (itemData->m_type == SwgDraftSchematicEditorDoc::ItemData::T_attribute);
	DraftSchematic::Attribute * const attribute = itemData->m_attribute;

	m_nameStringTable = attribute->m_nameStringTable;
	m_nameStringId = attribute->m_nameStringId;
	m_experimentStringTable = attribute->m_experimentStringTable;
	m_experimentStringId = attribute->m_experimentStringId;
	m_valueType = attribute->m_valueType;
	
	switch (m_valueType)
	{
	case DraftSchematic::Attribute::VT_integer:
		{
			m_maximumEditCtrl.ShowWindow (SW_SHOW);
			m_minimumEditCtrl.ShowWindow (SW_SHOW);
			m_maximumComboBox.ShowWindow (SW_HIDE);
			m_minimumComboBox.ShowWindow (SW_HIDE);

			m_maximumValue = attribute->m_maximumValue;
			m_minimumValue = attribute->m_minimumValue;
		}
		break;

	case DraftSchematic::Attribute::VT_armorRating:
		{
			m_maximumEditCtrl.ShowWindow (SW_HIDE);
			m_minimumEditCtrl.ShowWindow (SW_HIDE);
			m_maximumComboBox.ShowWindow (SW_SHOW);
			m_minimumComboBox.ShowWindow (SW_SHOW);

			Configuration::populateArmorRatings (m_maximumComboBox);
			m_maximumComboBox.SelectString (0, attribute->m_maximumValue);
			Configuration::populateArmorRatings (m_minimumComboBox);
			m_minimumComboBox.SelectString (0, attribute->m_minimumValue);
		}
		break;

	case DraftSchematic::Attribute::VT_damageType:
		{
			m_maximumEditCtrl.ShowWindow (SW_HIDE);
			m_minimumEditCtrl.ShowWindow (SW_HIDE);
			m_maximumComboBox.ShowWindow (SW_SHOW);
			m_minimumComboBox.ShowWindow (SW_SHOW);

			Configuration::populateDamageTypes (m_maximumComboBox);
			m_maximumComboBox.SelectString (0, attribute->m_maximumValue);
			Configuration::populateDamageTypes (m_minimumComboBox);
			m_minimumComboBox.SelectString (0, attribute->m_minimumValue);
		}
		break;
	}

	if (attribute->m_nameStringTable == "crafting" && attribute->m_nameStringId == "xp")
	{
		m_nameStringIdEditCtrl.SetReadOnly (true);
	}
	else
	{
		m_nameStringIdEditCtrl.SetReadOnly (false);
	}

	if (attribute->m_nameStringTable == "crafting" && attribute->m_nameStringId == "complexity")
	{
		m_nameStringIdEditCtrl.SetReadOnly (true);

		GetDlgItem (IDC_RADIO_VALUETYPE)->EnableWindow (false);
		GetDlgItem (IDC_RADIO_VALUETYPEARMORRATING)->EnableWindow (false);
		GetDlgItem (IDC_RADIO_VALUETYPEDAMAGETYPE)->EnableWindow (false);

		m_maximumEditCtrl.SetReadOnly (true);
		m_minimumEditCtrl.SetReadOnly (true);
	}
	else
	{
		m_nameStringIdEditCtrl.SetReadOnly (false);

		GetDlgItem (IDC_RADIO_VALUETYPE)->EnableWindow (true);
		GetDlgItem (IDC_RADIO_VALUETYPEARMORRATING)->EnableWindow (true);
		GetDlgItem (IDC_RADIO_VALUETYPEDAMAGETYPE)->EnableWindow (true);

		m_maximumEditCtrl.SetReadOnly (false);
		m_minimumEditCtrl.SetReadOnly (false);
	}

	UpdateData (false);
}

// ----------------------------------------------------------------------

void FormAttributeView::OnRadioValuetype() 
{
	DraftSchematic::Attribute * const attribute = m_itemData->m_attribute;
	attribute->m_valueType = DraftSchematic::Attribute::VT_integer;
	attribute->m_maximumValue = Configuration::getDefaultInteger ();
	attribute->m_minimumValue = Configuration::getDefaultInteger ();
	setItemData (m_itemData);
	GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormAttributeView::OnRadioValuetypearmorrating() 
{
	DraftSchematic::Attribute * const attribute = m_itemData->m_attribute;
	attribute->m_valueType = DraftSchematic::Attribute::VT_armorRating;
	attribute->m_maximumValue = Configuration::getDefaultArmorRating ();
	attribute->m_minimumValue = Configuration::getDefaultArmorRating ();
	setItemData (m_itemData);
	GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormAttributeView::OnRadioValuetypedamagetype() 
{
	DraftSchematic::Attribute * const attribute = m_itemData->m_attribute;
	attribute->m_valueType = DraftSchematic::Attribute::VT_damageType;
	attribute->m_maximumValue = Configuration::getDefaultDamageType ();
	attribute->m_minimumValue = Configuration::getDefaultDamageType ();
	setItemData (m_itemData);
	GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormAttributeView::OnSelchangeComboMaximum() 
{
	DraftSchematic::Attribute * const attribute = m_itemData->m_attribute;
	CString result;
	m_maximumComboBox.GetLBText (m_maximumComboBox.GetCurSel (), result);
	attribute->m_maximumValue = result;
	setItemData (m_itemData);
	GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormAttributeView::OnSelchangeComboMinimum() 
{
	DraftSchematic::Attribute * const attribute = m_itemData->m_attribute;
	CString result;
	m_minimumComboBox.GetLBText (m_minimumComboBox.GetCurSel (), result);
	attribute->m_minimumValue = result;
	setItemData (m_itemData);
	GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormAttributeView::OnChangeEditExperimentstringid() 
{
	UpdateData (true);

	DraftSchematic::Attribute * const attribute = m_itemData->m_attribute;
	CString result;
	m_experimentStringIdEditCtrl.GetWindowText (result);
	attribute->m_experimentStringTable = result.IsEmpty () ? "" : "crafting";
	attribute->m_experimentStringId = result;
	setItemData (m_itemData);

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormAttributeView::OnChangeEditMaximum() 
{
	UpdateData (true);

	DraftSchematic::Attribute * const attribute = m_itemData->m_attribute;
	attribute->m_maximumValue = m_maximumValue;
	setItemData (m_itemData);
	GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormAttributeView::OnChangeEditMinimum() 
{
	UpdateData (true);

	DraftSchematic::Attribute * const attribute = m_itemData->m_attribute;
	attribute->m_minimumValue = m_minimumValue;
	setItemData (m_itemData);
	GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));

	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormAttributeView::OnChangeEditNamestringid() 
{
	UpdateData (true);

	DraftSchematic::Attribute * const attribute = m_itemData->m_attribute;
	CString result;
	m_nameStringIdEditCtrl.GetWindowText (result);
	attribute->m_nameStringId = result;
	setItemData (m_itemData);
	GetDocument ()->UpdateAllViews (this, SwgDraftSchematicEditorDoc::H_nameChanged, reinterpret_cast<CObject *> (m_itemData->m_treeItem));
	GetDocument ()->SetModifiedFlag (true);
}

// ----------------------------------------------------------------------

void FormAttributeView::OnEditCopy() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Copy ();
}

// ----------------------------------------------------------------------

void FormAttributeView::OnEditCut() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Cut ();
}

// ----------------------------------------------------------------------

void FormAttributeView::OnEditPaste() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Paste ();
}

// ----------------------------------------------------------------------

void FormAttributeView::OnEditUndo() 
{
	if (dynamic_cast<CEdit *> (GetActiveWindow ()->GetFocus ()))
		safe_cast<CEdit *> (GetActiveWindow ()->GetFocus ())->Undo ();
}

// ======================================================================

