//======================================================================
//
// ComponentDescriptorsDetails.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ComponentDescriptorsDetails_H
#define INCLUDED_ComponentDescriptorsDetails_H

//======================================================================

#include "BaseComponentDescriptorsDetails.h"

class ShipChassis;
class ShipChassisWritable;

class ComponentDescriptors;
class QListBoxItem;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------------------

class ComponentDescriptorsDetails : public BaseComponentDescriptorsDetails
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	ComponentDescriptorsDetails(QWidget *parent, char const *name, ComponentDescriptors & descriptors);
	~ComponentDescriptorsDetails();

	QLineEdit * getLineEditNameFilter();

	//----------------------------------------------------------------------
	//-- callbacks
	//----------------------------------------------------------------------

	void onCallbackChassisListChanged(bool const & );

public slots:

	void onLineEditNameFiltertextChanged(QString const & text);
	void onListComponentTypesHighlighted(const QString &);
	void onListChassisTypesHighlighted(const QString &);

	void onListComponentTypesSelectionChanged();
	void onListChassisSelectionChanged();

signals:

protected:

private:

	void resetListComponentTypes();
	void resetListChassisTypes();

	ComponentDescriptors * m_componentDescriptors;

	MessageDispatch::Callback * m_callback;
};
//======================================================================

#endif
