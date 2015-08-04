// ============================================================================
//
// BaseDialogEdit.h
//
// An edit field which allows the user to press a button to bring up a custom
// dialog. On closing the dialog (and pressing 'OK'), the text description for
// this field is filled in.
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_BaseDialogEdit_H
#define INCLUDED_BaseDialogEdit_H

#include "DataTypeEdit.h"

class TableItemBase;

//-----------------------------------------------------------------------------
class BaseDialogEdit : public DataTypeEdit
{
	Q_OBJECT

public:

	BaseDialogEdit(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase);

	virtual QString getText() const;
	virtual void    init();

protected:

	void             setInputDialog(QDialog &dialog);
	void             setText(QString const &text);
	void             setTextLineEditReadOnly(bool const readOnly);

	virtual QString  getTextFromDialog() const = 0;
	QPushButton *    getOpenDialogButton();                         // Provided for the rare case a child wants to hijack the dialog control
	virtual void     mouseDoubleClickEvent(QMouseEvent *mouseEvent);
	QLineEdit const *getLineEdit() const;

	QDialog *m_inputDialog;

private slots:

	void getUserDialogInput();

private:

	QLineEdit *  m_textLineEdit;
	QPushButton *m_openDialogPushButton;

private:

	// Disabled

	BaseDialogEdit &operator =(BaseDialogEdit const &);
};

// ============================================================================

#endif // INCLUDED_BaseDialogEdit_H
