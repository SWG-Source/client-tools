//======================================================================
//
// ChassisEditorDetails.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ChassisEditorDetails_H
#define INCLUDED_ChassisEditorDetails_H

//======================================================================

#include "BaseChassisEditorDetails.h"

class ShipChassis;
class ShipChassisWritable;

class ChassisEditor;
class QListBoxItem;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------------------

class ChassisEditorDetails : public BaseChassisEditorDetails
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	ChassisEditorDetails(QWidget *parent, char const *name, ChassisEditor & editor);
	~ChassisEditorDetails();

	QLineEdit * getLineEditNameFilter();

	//----------------------------------------------------------------------
	//-- callbacks
	//----------------------------------------------------------------------

	void onCallbackChassisListChanged(bool const & );
	void onCallbackTemplateListChanged(bool const &);

public slots:

	void onLineEditNameFiltertextChanged(const QString & text);
	void onTableSelectionChanged();
	void onListContextMenuRequested(QListBoxItem * item, const QPoint & pos);
	void onListBoxContext(int);
	void onButtonReloadTemplatesClicked();
	void onButtonRegenerateDbClicked();

signals:

protected:

private:

	void resetTemplateList();
	void resetTemplateOrphanList();

	ChassisEditor * m_chassisEditor;

	MessageDispatch::Callback * m_callback;
};
//======================================================================

#endif
