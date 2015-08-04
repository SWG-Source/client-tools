//======================================================================
//
// ComponentDescriptors.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ComponentDescriptors_H
#define INCLUDED_ComponentDescriptors_H

//======================================================================

#include "BaseComponentDescriptors.h"

class ShipChassis;
class ShipChassisWritable;
class ComponentDescriptorsDetails;
class ComponentDescriptorsList;

//-----------------------------------------------------------------------------

class ComponentDescriptors : public BaseComponentDescriptors
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	ComponentDescriptors(QWidget *parent, char const *name);
	~ComponentDescriptors();

	typedef stdvector<std::string>::fwd StringVector;
	typedef stdvector<int>::fwd IntVector;
	typedef stdvector<uint32>::fwd CrcVector;

	void getSelectedComponentsList(StringVector & result);

	void handleComponentTypeFilterChange(IntVector const & componentTypes);
	void handleChassisFilterChange(CrcVector const & chassisCrcs);

public slots:

signals:

protected:

private:

	ComponentDescriptorsDetails * m_details;
	ComponentDescriptorsList * m_list;
};

//======================================================================

#endif
