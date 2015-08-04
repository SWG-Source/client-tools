//======================================================================
//
// ChassisNewDialog.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ChassisNewDialog_H
#define INCLUDED_ChassisNewDialog_H

//======================================================================

#include "BaseChassisNewDialog.h"

class ChassisNewDialog : public BaseChassisNewDialog
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	ChassisNewDialog(QWidget *parent, char const *name, std::string const & cloneFromChassisName);
	~ChassisNewDialog();

public slots:

	void onNameTextChanged(const QString &);
	void onButtonCreateClicked();

signals:

protected:

private:

	ChassisNewDialog();

};
//======================================================================

#endif
