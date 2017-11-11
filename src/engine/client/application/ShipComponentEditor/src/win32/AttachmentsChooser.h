//======================================================================
//
// AttachmentsChooser.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AttachmentsChooser_H
#define INCLUDED_AttachmentsChooser_H

//======================================================================

#include "BaseAttachmentsChooser.h"

class ShipChassis;
class ShipChassisWritable;
class ShipComponentDescriptorWritable;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------------------

class AttachmentsChooser : public BaseAttachmentsChooser
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	AttachmentsChooser(QWidget *parent, char const *name);
	~AttachmentsChooser();

public slots:

	void setEditingAttachments(uint32 chassisCrc, uint32 componentCrc, int chassisSlotType, bool confirmed);

	void onButtonApplyClicked();
	void onButtonRevertClicked();
	void onButtonAddClicked();
	void onButtonDeleteClicked();

	void onButtonSlotAddClicked();
	void onButtonSlotCompatClicked();
	void onButtonComponentCompatClicked();

	void onTableSelectionChanged();
	void onTableValueChanged(int row, int col);

	void onTableDoubleClicked(int, int, int, const QPoint &);

	void onTableContextMenuRequested(int row, int col, const QPoint & pos);
	void onTableContextTemplateSelect(int id);
	void onTableContextTemplateEdit(int id);
	void onTableContextCdfEdit(int id);

	void onTimerUpdate();

	//-- callbacks

	void onCallbackComponentListChanged(ShipComponentDescriptorWritable const & payload);
	void onCallbackComponentChanged(bool const & payload);
	void onCallbackAttachmentsChanged(std::pair<uint32, std::pair<uint32, int> > const & payload);
	void onCallbackChassisListChanged(bool const & payload);
	void onCallbackChassisChanged(ShipChassisWritable const & payload);
	void onCallbackTemplateListChanged(bool const & payload);

signals:

protected:

private:

	AttachmentsChooser();
	AttachmentsChooser(AttachmentsChooser const & rhs);
	AttachmentsChooser & operator=(AttachmentsChooser const & rhs);

private:

	void resetTable();
	ShipChassisWritable * getShipChassisWritable();
	ShipComponentDescriptorWritable * getShipComponentDescriptorWritable();
	void updateButtonsOnModificationChanged();
	void setModified(bool modified);
	void setTableNeedsReset();
	void handleSelectTemplateForRow(int row);
	void handleEditTemplateForRow(int row) const;
	void handleEditCdfForRow(int row) const;
	int getLastSelectedRow() const;
	int getLastSelectedCol() const;

private:

	class State;

	uint32 m_chassisCrc;
	uint32 m_componentCrc;
	int m_chassisSlotType;

	State * m_state;

	bool m_tableNeedsReset;
	int m_tableResetCountdown;

	MessageDispatch::Callback * m_callback;

};

//======================================================================

#endif
