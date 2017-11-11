//======================================================================
//
// AttachmentsChassisEditorList.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AttachmentsChassisEditorList_H
#define INCLUDED_AttachmentsChassisEditorList_H

//======================================================================

#include "BaseAttachmentsEditorList.h"

class ShipChassis;
class ShipComponentDescriptorWritable;
class ShipChassisWritable;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------------------

class AttachmentsChassisEditorList : public BaseAttachmentsEditorList
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	AttachmentsChassisEditorList(QWidget *parent, char const *name);
	~AttachmentsChassisEditorList();

	QTable * getTable();

	typedef stdvector<int>::fwd IntVector;
	typedef stdvector<uint32>::fwd CrcVector;
	typedef stdvector<std::string>::fwd StringVector;
	void getSelectedComponentsList(StringVector & result) const;

	//----------------------------------------------------------------------
	//-- callbacks
	//----------------------------------------------------------------------

	void onCallbackComponentListChanged(bool const & );
	void onCallbackComponentChanged(ShipComponentDescriptorWritable const & );
	void onCallbackAttachmentsChanged(std::pair<uint32, std::pair<uint32, int> > const &);

	void onCallbackChassisListChanged(bool const &);
	void onCallbackChassisChanged(ShipChassisWritable const &);

	void handleComponentTypeFilterChange(IntVector const & componentTypes);
	void handleChassisNameChange(std::string const & chassisName);

	void handleHideFlags(bool hideEmptyRows, bool hideEmptyCols, bool hideUninstallableRows, bool hideUninstallableCols);

public slots:

	void onTimerUpdate();

	void onLineEditNameFiltertextChanged(const QString & text);
	void onTableDoubleClicked(int row, int col, int button, const QPoint & mousePos);
	void onTableValueChanged (int row,int col);
	void onTableContextMenuRequested ( int row, int col, const QPoint & pos );
	void onTableSelectionChanged();

	void onTableContextCompatibility(int id);
	void onTableContextAttachmentsDelete(int id);

signals:

	void attachmentSelectionChanged(uint32, uint32, int, bool);

protected:

private:

	AttachmentsChassisEditorList();
	AttachmentsChassisEditorList(AttachmentsChassisEditorList const & rhs);
	AttachmentsChassisEditorList & operator=(AttachmentsChassisEditorList const & rhs);

private:

	ShipComponentDescriptorWritable * findComponentDescriptorWritableLastSelected() const;
	ShipComponentDescriptorWritable * findComponentDescriptorWritableForRow(int row) const;

	ShipChassisWritable * getShipChassisWritable() const;
	int getChassisSlotTypeForColumn(int col) const;
	int getChassisSlotTypeForColumnLastSelected() const;

	void resetTable();

	void setTableNeedsReset();

private:

	bool m_tableNeedsReset;
	int m_tableResetCountdown;

	std::string m_nameFilter;

	MessageDispatch::Callback * m_callback;

	IntVector m_filterComponentTypes;
	std::string m_chassisName;

	bool m_hideEmptyRows;
	bool m_hideEmptyCols;
	bool m_hideUninstallableRows;
	bool m_hideUninstallableCols;
};

//======================================================================

#endif
