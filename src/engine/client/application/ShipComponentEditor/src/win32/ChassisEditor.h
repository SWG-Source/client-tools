//======================================================================
//
// ChassisEditor.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ChassisEditor_H
#define INCLUDED_ChassisEditor_H

//======================================================================

#include "BaseChassisEditor.h"

class ShipChassis;
class ShipChassisWritable;
class ChassisEditorDetails;
class ChassisEditorList;

//-----------------------------------------------------------------------------

class ChassisEditor : public BaseChassisEditor
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	ChassisEditor(QWidget *parent, char const *name);
	~ChassisEditor();

	typedef stdvector<std::string>::fwd StringVector;

	void getSelectedChassisList(StringVector & result);

public slots:

signals:

protected:

private:

	ChassisEditorDetails * m_details;
	ChassisEditorList * m_list;
};

//======================================================================

#endif
