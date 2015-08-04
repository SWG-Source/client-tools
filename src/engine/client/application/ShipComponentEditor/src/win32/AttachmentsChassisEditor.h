//======================================================================
//
// AttachmentsChassisEditor.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AttachmentsChassisEditor_H
#define INCLUDED_AttachmentsChassisEditor_H

//======================================================================

class ShipChassis;
class ShipChassisWritable;
class AttachmentsChassisEditorDetails;
class AttachmentsChassisEditorList;
class AttachmentsChooser;

//-----------------------------------------------------------------------------

class AttachmentsChassisEditor : public QWidget
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	AttachmentsChassisEditor(QWidget *parent, char const *name);
	~AttachmentsChassisEditor();

	typedef stdvector<std::string>::fwd StringVector;
	typedef stdvector<int>::fwd IntVector;
	typedef stdvector<uint32>::fwd CrcVector;

	void getSelectedComponentsList(StringVector & result);

	void handleComponentTypeFilterChange(IntVector const & componentTypes);
	void handleChassisNameChange(std::string const & chassisName);

	void handleHideFlags(bool hideEmptyRows, bool hideEmptyCols, bool hideUninstallableRows, bool hideUninstallableCols);

public slots:

signals:

protected:

private:

	 AttachmentsChassisEditor();
	 AttachmentsChassisEditor(AttachmentsChassisEditor const & rhs);
	 AttachmentsChassisEditor & operator=(AttachmentsChassisEditor const & rhs);

private:

	AttachmentsChassisEditorDetails * m_details;
	AttachmentsChassisEditorList * m_list;
	AttachmentsChooser * m_chooser;
};

//======================================================================

#endif
