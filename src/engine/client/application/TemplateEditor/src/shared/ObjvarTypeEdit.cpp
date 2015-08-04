// ============================================================================
//
// ObjvarTypeEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "ObjvarTypeEdit.h"

// ============================================================================
//
// ObjvarTypeEdit
//
// ============================================================================

/*
const int ObjvarTypeEdit::ms_integerChoiceIndex = 0;
const int ObjvarTypeEdit::ms_floatChoiceIndex = 1;
const int ObjvarTypeEdit::ms_stringChoiceIndex = 2;
const int ObjvarTypeEdit::ms_objvarListChoiceIndex = 3;

//-----------------------------------------------------------------------------

ObjvarTypeEdit::ObjvarTypeEdit(TableItemBase *parent, const char *name)
 : DataTypeEdit(parent, name)
{
	m_objvarTypeComboBox = new QComboBox(this);

	m_objvarTypeComboBox->insertItem("Integer", ms_integerChoiceIndex);
	m_objvarTypeComboBox->insertItem("Float", ms_floatChoiceIndex);
	m_objvarTypeComboBox->insertItem("String", ms_stringChoiceIndex);
	m_objvarTypeComboBox->insertItem("Objvar List", ms_objvarListChoiceIndex);

	setValue(ms_integerChoiceIndex);

	// All DataTypeEdits should call update text immediately as part of initialization
	updateText();

	setFocusTarget(*m_objvarTypeComboBox);
}

//-----------------------------------------------------------------------------
ObjvarTypeEdit::~ObjvarTypeEdit()
{
	delete m_objvarTypeComboBox;
}

//-----------------------------------------------------------------------------
void ObjvarTypeEdit::setValue(int index)
{
	m_objvarTypeComboBox->setCurrentItem(index);
	updateText();
}

//-----------------------------------------------------------------------------
int ObjvarTypeEdit::currentItem() const
{
	return m_objvarTypeComboBox->currentItem();
}

//-----------------------------------------------------------------------------
void ObjvarTypeEdit::updateText()
{
	setText(m_objvarTypeComboBox->currentText());
}
*/

// ============================================================================
