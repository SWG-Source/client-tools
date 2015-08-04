// ======================================================================
//
// Dialog.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Dialog_H
#define INCLUDED_Dialog_H

// ======================================================================

class QTextEdit;
class QPushButton;

#include <qdialog.h>

// ======================================================================

class TextEditDialog : public QDialog
{
public:
	TextEditDialog(QWidget *parent, QString name);

protected:
	void resizeEvent(QResizeEvent *event);

private:
	QTextEdit   *m_textEdit;
	QPushButton *m_okay;
	QPushButton *m_cancel;
};

// ======================================================================

#endif
