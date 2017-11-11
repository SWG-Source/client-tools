// ============================================================================
//
// EnumEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "EnumEdit.h"

#include "sharedTemplateDefinition/TemplateData.h"

// ============================================================================
//
// EnumEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
EnumEdit::EnumEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, const TemplateData::EnumList &enumList)
 : DataTypeEdit(parentWidget, name, parentTableItemBase)
 , m_enumComboBox(NULL)
{
	m_enumComboBox = new QComboBox(this);

	// Iterate through the names and add them to our editor

	TemplateData::EnumList::const_iterator enumIter;

	for (enumIter = enumList.begin(); enumIter != enumList.end(); ++enumIter)
	{
		m_enumComboBox->insertItem((*enumIter).name.c_str());
	}

	setValue(0);

	setFocusTarget(*m_enumComboBox);
}

//-----------------------------------------------------------------------------
EnumEdit::~EnumEdit()
{
}

//-----------------------------------------------------------------------------
void EnumEdit::setValue(int index)
{
	m_enumComboBox->setCurrentItem(index);
}

//-----------------------------------------------------------------------------
QString EnumEdit::getText() const
{
	QString result;

	// Current value

	result += m_enumComboBox->currentText();

	// Weighted percent

	if (isWeighted())
	{
		result += ":" + QString::number(getWeight());
	}

	return result;
}

// ============================================================================
