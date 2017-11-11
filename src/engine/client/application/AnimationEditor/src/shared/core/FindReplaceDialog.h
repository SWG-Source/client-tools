// ======================================================================
//
// FindReplaceDialog.h
// copyright (c) 2006 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FindReplaceDialog_H
#define INCLUDED_FindReplaceDialog_H

#include "BaseFindReplaceDialog.h"

class FindReplaceDialog : public BaseFindReplaceDialog
{
	Q_OBJECT; 
public:
	FindReplaceDialog(QWidget* parent, const char* name);
	~FindReplaceDialog();

private slots:

	virtual void onButtonFindNext();
	virtual void onButtonReplace();
	virtual void onButtonReplaceNext();
	virtual void onButtonReplaceAll();

signals:

private:
	//disabled
	FindReplaceDialog();
	FindReplaceDialog(const FindReplaceDialog& rhs);
	FindReplaceDialog & operator=(const FindReplaceDialog& rhs);
};


#endif