//======================================================================
//
// AttachmentsChassisEditorDetails.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AttachmentsChassisEditorDetails_H
#define INCLUDED_AttachmentsChassisEditorDetails_H

//======================================================================

#include "BaseAttachmentsChassisEditorDetails.h"

class ShipChassis;
class ShipChassisWritable;
class AttachmentsChassisEditor;
class QListBoxItem;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------------------

class AttachmentsChassisEditorDetails : public BaseAttachmentsChassisEditorDetails
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	AttachmentsChassisEditorDetails(QWidget *parent, char const *name, AttachmentsChassisEditor & editor);
	~AttachmentsChassisEditorDetails();

	QLineEdit * getLineEditNameFilter();

	//----------------------------------------------------------------------
	//-- callbacks
	//----------------------------------------------------------------------

	void onCallbackChassisListChanged(bool const & );

	void setupState();

public slots:

	void onLineEditNameFiltertextChanged(QString const & text);
	void onListComponentTypesHighlighted(const QString &);

	void onListComponentTypesSelectionChanged();
	void onComboChassisHighlighted(const QString &);

	void onCheckHideGenericToggled(bool on);

signals:

protected:

private:

	AttachmentsChassisEditorDetails();
	AttachmentsChassisEditorDetails(AttachmentsChassisEditorDetails const & rhs);
	AttachmentsChassisEditorDetails & operator=(AttachmentsChassisEditorDetails const & rhs);

private:

	void resetListComponentTypes();
	void resetComboChassisTypes();

	AttachmentsChassisEditor * m_editor;

	MessageDispatch::Callback * m_callback;
};
//======================================================================

#endif
