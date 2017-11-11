//======================================================================
//
// SwgCuiVoiceActiveSpeakers_TableModel.h
// copyright (c) 2008 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiVoiceActiveSpeakers_TableModel_H
#define INCLUDED_SwgCuiVoiceActiveSpeakers_TableModel_H

//======================================================================

#include "swgClientUserInterface/SwgCuiVoiceActiveSpeakers.h"

#include "UITableModel.h"

class UITable;

//----------------------------------------------------------------------

class SwgCuiVoiceActiveSpeakers::TableModel : public UITableModel
{
public:

	TableModel (SwgCuiVoiceActiveSpeakers & containerMediator);
	~TableModel ();

	enum Type
	{
		T_ActiveSpeaker,
	};

	virtual bool           GetValueAtText (int visualRow, int col, UIString & value) const;
	virtual bool           GetValueAtImage (int visualRow, int col, UIImageStyle *& value)  const;
	virtual bool           GetTooltipAt (int row, int col, UIString & tooltip) const;

	UIBaseObject          *Clone() const { return 0; }

	bool                   GetLocalizedColumnName    (int col, UIString & str) const;
	bool                   GetLocalizedColumnTooltip (int col, UIString & str) const;

	UITableTypes::CellType FindColumnCellType     (int col) const;
	int                    FindColumnCount        () const;
	int                    FindRowCount           () const;
	int getLogicalRowFromVisualRow(int visualRow) const;

	void                           setType (Type type);
	Type                           getType () const;

	void                   updateTableColumnSizes (UITable & table) const;

	bool findDataForRowVisual(int row, CuiVoiceChatManager::ActiveSpeaker & data);

private:
	TableModel (const TableModel &);
	TableModel & operator= (const TableModel &);


	Type                       m_type;
	SwgCuiVoiceActiveSpeakers &             m_ownerMediator;
};

//----------------------------------------------------------------------

inline SwgCuiVoiceActiveSpeakers::TableModel::Type SwgCuiVoiceActiveSpeakers::TableModel::getType () const
{
	return m_type;
}

#endif

//======================================================================

