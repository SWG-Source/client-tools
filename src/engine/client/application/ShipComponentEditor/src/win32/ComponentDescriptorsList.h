//======================================================================
//
// ComponentDescriptorsList.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ComponentDescriptorsList_H
#define INCLUDED_ComponentDescriptorsList_H

//======================================================================

#include "BaseComponentDescriptorsList.h"

class ShipChassis;
class ShipComponentDescriptorWritable;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------------------

class ComponentDescriptorsList : public BaseComponentDescriptorsList
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	ComponentDescriptorsList(QWidget *parent, char const *name);
	~ComponentDescriptorsList();

	QTable * getTable();

	typedef stdvector<int>::fwd IntVector;
	typedef stdvector<uint32>::fwd CrcVector;
	typedef stdvector<std::string>::fwd StringVector;
	void getSelectedComponentsList(StringVector & result);

	//----------------------------------------------------------------------
	//-- callbacks
	//----------------------------------------------------------------------

	void onCallbackComponentListChanged(bool const & );
	void onCallbackComponentChanged(ShipComponentDescriptorWritable const & );

	void handleComponentTypeFilterChange(IntVector const & componentTypes);
	void handleChassisFilterChange(CrcVector const & chassisCrcs);

public slots:

	void onTimerUpdate();

	void onLineEditNameFiltertextChanged(const QString & text);
	void onTableDoubleClicked(int row, int col, int button, const QPoint & mousePos);
	void onTableValueChanged (int row,int col);
	void onTableContextMenuRequested ( int row, int col, const QPoint & pos );
	void onTableSelectionChanged();
	void onTableContextComponentDelete(int id);
	void onTableContextTemplate(int id);
	void onTableContextTemplateFix(int);
	void onTableContextComponentNew(int);

signals:

protected:

private:

	ComponentDescriptorsList();
	ComponentDescriptorsList(ComponentDescriptorsList const & rhs);
	ComponentDescriptorsList & operator=(ComponentDescriptorsList const & rhs);

private:

	ShipComponentDescriptorWritable * findComponentDescriptorWritableLastSelected();
	ShipComponentDescriptorWritable * findComponentDescriptorWritableForRow(int row);

	void resetTable();

	void setTableNeedsReset();

	bool m_tableNeedsReset;
	int m_tableResetCountdown;

	std::string m_nameFilter;

	MessageDispatch::Callback * m_callback;

	IntVector m_filterComponentTypes;
	CrcVector m_filterChassis;

	bool m_shouldAdjustColumns;
};

//======================================================================

#endif
