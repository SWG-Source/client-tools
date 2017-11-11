// ============================================================================
//
// MustDeriveEdit.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstTemplateEditor.h"
#include "MustDeriveEdit.h"

// ============================================================================
//
// MustDeriveEdit
//
// ============================================================================

//-----------------------------------------------------------------------------
MustDeriveEdit::MustDeriveEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase)
 : DataTypeEdit(parentWidget, name, parentTableItemBase)
{
	m_mustDeriveLineEdit = new QLineEdit("@derived", this);
	m_mustDeriveLineEdit->setReadOnly(true);

	setFocusTarget(*m_mustDeriveLineEdit);
}

//-----------------------------------------------------------------------------
MustDeriveEdit::~MustDeriveEdit()
{
}

//-----------------------------------------------------------------------------
QString MustDeriveEdit::getText() const
{
	return m_mustDeriveLineEdit->text();
}

// ============================================================================
