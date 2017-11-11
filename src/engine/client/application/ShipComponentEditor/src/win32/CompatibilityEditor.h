//======================================================================
//
// CompatibilityEditor.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CompatibilityEditor_H
#define INCLUDED_CompatibilityEditor_H

//======================================================================

#include "BaseCompatibilityEditor.h"

class ShipChassisSlot;
class ShipComponentDescriptor;

//-----------------------------------------------------------------------------

class CompatibilityEditor : public BaseCompatibilityEditor
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	enum Mode
	{
		M_multi,
		M_single
	};

	typedef stdvector<std::string>::fwd StringVector;

	CompatibilityEditor(QWidget *parent, char const *name, ShipChassisSlot const & chassisSlot, ShipComponentDescriptor const * shipComponentDescriptor);
	CompatibilityEditor(QWidget *parent, char const *name, ShipComponentDescriptor const & shipComponentDescriptor, ShipChassisSlot const * chassisSlot);
	CompatibilityEditor(QWidget *parent, char const *name, Mode mode, StringVector const & initalSelection);
	~CompatibilityEditor();

	bool showAndTell(std::string & finalSelection);

public slots:

	void onPushButtonTypeNewClicked();
	void onPushButtonTypeDeleteClicked();
	void onPushButtonTypeRenameClicked();

	void onPushButtonOkClicked();
	void onPushButtonSelectClicked();
	void onPushButtonDeselectClicked();

	void onLineEditNewTextChanged(const QString & text);
	void onLineEditRenameTextChanged(const QString & text);

	void onListCompatAvailableSelectionChanged();
	void onListCompatChosenSelectionChanged();

signals:

protected:

private:

	CompatibilityEditor();

private:

	void setupSignals();
	void populateLists(StringVector const & initalSelection, StringVector const & appropriateAvailableCompats);

	void handleSelectionAdd();
	void handleSelectionRemove();

	void handleRenameCompatibiliy(std::string const & oldName, std::string const & newName);

private:

	Mode m_mode;
	std::string m_finalSelection;
	bool m_ok;
};

//======================================================================

#endif
