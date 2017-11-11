// ======================================================================
//
// Dialog.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "Dialog.h"

#include <qtextedit.h>
#include <qpushbutton.h>

// ======================================================================

TextEditDialog::TextEditDialog(QWidget *parent, QString name)
: QDialog(parent, name)
{
	setSizeGripEnabled(true);
	
	m_textEdit = new QTextEdit(this);

	m_okay = new QPushButton("Okay", this);
	m_okay->setDefault(true);

	m_cancel = new QPushButton("Cancel", this);

	QObject::connect(m_okay, SIGNAL(clicked()), this, SLOT(accept()));
	QObject::connect(m_cancel, SIGNAL(clicked()), this, SLOT(reject()));

	setMinimumSize(m_okay->width() * 3, m_okay->height() * 4);
	resize(600, 600);
}

// ----------------------------------------------------------------------

void TextEditDialog::resizeEvent(QResizeEvent *event)
{
	QDialog::resizeEvent(event);

	const int buttonSpacing = (width() - m_okay->width() - m_cancel->width()) / 3;
	const int buttonY = height() - ((m_okay->height() * 3) / 2);

	m_okay->move(buttonSpacing, buttonY);
	m_cancel->move(buttonSpacing * 2 + m_okay->width(), buttonY);
	
	m_textEdit->move(4,4);
	m_textEdit->resize(width() - 8, buttonY - m_okay->height() / 2);
}

// ======================================================================
