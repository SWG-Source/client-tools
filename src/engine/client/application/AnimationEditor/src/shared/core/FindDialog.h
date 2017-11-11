// ======================================================================
//
// FindDialog.h
// copyright (c) 2006 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FindDialog_H
#define INCLUDED_FindDialog_H

#include "BaseFindDialog.h"

class FindDialog : public BaseFindDialog
{
	Q_OBJECT; 
public:
	FindDialog(QWidget* parent, const char* name);
	~FindDialog();

private slots:
	virtual void onButtonFindNext();
signals:

private:
	//disabled
	FindDialog();
	FindDialog(const FindDialog& rhs);
	FindDialog & operator=(const FindDialog& rhs);
};


#endif

