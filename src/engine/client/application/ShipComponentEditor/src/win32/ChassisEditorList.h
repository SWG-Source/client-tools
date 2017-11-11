//======================================================================
//
// ChassisEditorList.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ChassisEditorList_H
#define INCLUDED_ChassisEditorList_H

//======================================================================

#include "BaseChassisEditorList.h"

class ShipChassis;
class ShipChassisWritable;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------------------

class ChassisEditorList : public BaseChassisEditorList
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	ChassisEditorList(QWidget *parent, char const *name);
	~ChassisEditorList();

	QTable * getTable();

	typedef stdvector<std::string>::fwd StringVector;
	void getSelectedChassisList(StringVector & result);

	//----------------------------------------------------------------------
	//-- callbacks
	//----------------------------------------------------------------------

	void onCallbackChassisListChanged(bool const & );
	void onCallbackChassisChanged(ShipChassisWritable const & );
	
public slots:

	void updateTimer();

	void onLineEditNameFiltertextChanged(const QString & text);
	void onTableDoubleClicked(int row, int col, int button, const QPoint & mousePos);
	void onTableValueChanged (int row,int col);
	void onTableContextMenuRequested ( int row, int col, const QPoint & pos );
	void onTableContextTargetable(int id);
	void onTableContextFlyby(int id);
	void onTableSelectionChanged();
	void onTableContextChassisNew(int id);
	void onTableContextChassisDelete(int id);
	void onTableContextChassisNewTemplate(int);

signals:

protected:

private:

	ChassisEditorList();
	ChassisEditorList(ChassisEditorList const & rhs);
	ChassisEditorList & operator=(ChassisEditorList const & rhs);

private:

	ShipChassisWritable * findShipChassisWritableLastSelected();
	ShipChassisWritable * findShipChassisWritableForRow(int row);
	int findChassisSlotTypeForColumn(int col) const;

	void resetTable();

	void setTableNeedsReset();

	bool m_tableNeedsReset;
	int m_tableResetCountdown;

	std::string m_nameFilter;

	MessageDispatch::Callback * m_callback;

};
//======================================================================

#endif
