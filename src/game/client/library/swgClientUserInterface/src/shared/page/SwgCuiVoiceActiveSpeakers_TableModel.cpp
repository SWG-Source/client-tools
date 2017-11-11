//======================================================================
//
// SwgCuiResourceExtractionHopperTableModel.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiVoiceActiveSpeakers_TableModel.h"

#include "sharedFoundation/CrcString.h"
#include "sharedObject/CachedNetworkId.h"

#include "clientGame/ClientObject.h"
#include "clientGame/ResourceIconManager.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientGraphics/Texture.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "swgClientUserInterface/SwgCuiSurvey.h"

#include "UITable.h"

//======================================================================


namespace SwgCuiVoiceActiveSpeakersTableModelNamespace
{
	namespace ColumnInfo
	{
		enum Id
		{
			I_none,
			I_state,
			I_name,
			I_channel,
			I_count
		};
		
		struct ColumnData
		{
			StringId name;
			StringId tooltip;
			UITableTypes::CellType cellType;

			ColumnData ()
			: name (),
			  tooltip (),
			  cellType (UITableTypes::CT_none)
			{
			}

			ColumnData (const StringId & _name, UITableTypes::CellType _cellType)
			: name (_name),
			  tooltip (_name),
			  cellType (_cellType)
			{
			}

			ColumnData (const StringId & _name, const StringId & _tooltip, UITableTypes::CellType _cellType)
				: name (_name),
				tooltip (_tooltip),
				cellType (_cellType)
			{
			}
		};
		
		ColumnData data [I_count];

		//-- install the data
		void install ()
		{
			static bool installed = false;
			
			if (installed)
				return;
			
			installed = true;
			
			//@TODO: actual string IDs
			const static std::string voiceTable("ui_voice");

			data [I_none] = ColumnData (StringId ("NONE"), UITableTypes::CT_none);
			data [I_state] = ColumnData (StringId(voiceTable,"activespeakers_state"), UITableTypes::CT_image);
			data [I_name]  = ColumnData (StringId(voiceTable, "activespeakers_name"), StringId(voiceTable, "activespeakers_name_tooltip"), UITableTypes::CT_text);
			data [I_channel] = ColumnData (StringId(voiceTable,"activespeakers_channel"), StringId(voiceTable,"activespeakers_channel_tooltip"), UITableTypes::CT_text);
		}

		namespace ActiveSpeakers
		{
			const int numColumns = I_count-1;
			const Id ids [numColumns] =
			{
				I_state,
				I_name,
				I_channel
			};
		}

		Id findColumnId (SwgCuiVoiceActiveSpeakers::TableModel::Type type, int col)
		{
			if(col < 0)
			{
				DEBUG_WARNING(true,("SwgCuiVoiceActiveSpeakers::TableModel::findColumnId got bad column index %d", col));
				return I_none;
			}

			switch (type)
			{
			case SwgCuiVoiceActiveSpeakers::TableModel::T_ActiveSpeaker:
				{
					if(col >= ColumnInfo::ActiveSpeakers::numColumns)
					{
						DEBUG_WARNING(true,("SwgCuiVoiceActiveSpeakers::TableModel::findColumnId got bad column index %d", col));
						return I_none;
					}
					return ColumnInfo::ActiveSpeakers::ids [col];
				}
				break;
			default:
				return I_none;
			}
		}
	}
}

using namespace SwgCuiVoiceActiveSpeakersTableModelNamespace;

//----------------------------------------------------------------------

SwgCuiVoiceActiveSpeakers::TableModel::TableModel (SwgCuiVoiceActiveSpeakers & ownerMediator)
: UITableModel(),
  m_type (T_ActiveSpeaker),
  m_ownerMediator (ownerMediator)
{
	ColumnInfo::install ();
}

//----------------------------------------------------------------------

SwgCuiVoiceActiveSpeakers::TableModel::~TableModel ()
{
}

//----------------------------------------------------------------------

bool SwgCuiVoiceActiveSpeakers::TableModel::GetValueAtText(int visualRow, int col, UIString & value) const
{
	const int row = GetLogicalDataRowIndex (visualRow);

	CuiVoiceChatManager::ActiveSpeaker data;
	
	if(!m_ownerMediator.getParticipantAtPosition(row,data))
	{
		DEBUG_WARNING(true, ("Could not find participant data at row %d", row));
		return false;
	}

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	switch (id)
	{
	case ColumnInfo::I_name:
		{
			Unicode::String name = Unicode::narrowToWide(data.displayName);
			value = name;
		}
		break;
	case ColumnInfo::I_channel:
		{
			std::string displayName;
			if(!CuiVoiceChatManager::getChannelDisplayName(data.channelName,displayName))
			{
				displayName = data.channelName;
			}		
			
			Unicode::String channel = Unicode::narrowToWide(displayName);
			value = channel;
		}
		break;
	case ColumnInfo::I_none:
	case ColumnInfo::I_state:
	case ColumnInfo::I_count:
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiVoiceActiveSpeakers::TableModel::GetValueAtImage (int visualRow, int col, UIImageStyle *& value) const
{
	const int row = GetLogicalDataRowIndex (visualRow);

	CuiVoiceChatManager::ActiveSpeaker data;

	if(!m_ownerMediator.getParticipantAtPosition(row,data))
	{
		DEBUG_WARNING(true, ("Could not find participant data at row %d", row));
		return false;
	}

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	SwgCuiVoiceActiveSpeakers::SpeakingState state;
	if(data.muted)
	{
		state = SwgCuiVoiceActiveSpeakers::SS_muted;
	}
	else if(data.speaking)
	{
		state = SwgCuiVoiceActiveSpeakers::SS_speaking;
	}
	else
	{
		state = SwgCuiVoiceActiveSpeakers::SS_notSpeaking;
	}

	switch (id)
	{
	case ColumnInfo::I_state:
		{
			return m_ownerMediator.getImageForState(state, value);
		}
		break;
	case ColumnInfo::I_none:
	case ColumnInfo::I_name:
	case ColumnInfo::I_channel:
	case ColumnInfo::I_count:
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiVoiceActiveSpeakers::TableModel::GetTooltipAt (int visualRow, int col, UIString & tooltip) const
{
	const int row = GetLogicalDataRowIndex (visualRow);

	CuiVoiceChatManager::ActiveSpeaker data;

	if(!m_ownerMediator.getParticipantAtPosition(row,data))
	{
		DEBUG_WARNING(true, ("Could not find participant data at row %d", row));
		return false;
	}

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	SwgCuiVoiceActiveSpeakers::SpeakingState state;
	if(data.muted)
	{
		state = SwgCuiVoiceActiveSpeakers::SS_muted;
	}
	else if(data.speaking)
	{
		state = SwgCuiVoiceActiveSpeakers::SS_speaking;
	}
	else
	{
		state = SwgCuiVoiceActiveSpeakers::SS_notSpeaking;
	}

	switch (id)
	{
	case ColumnInfo::I_state:
		{
			return m_ownerMediator.getTooltipForState(state, tooltip);
		}
		break;
	case ColumnInfo::I_none:
	case ColumnInfo::I_name:
	case ColumnInfo::I_channel:
	case ColumnInfo::I_count:
	default:
		return false;
	}

}

//----------------------------------------------------------------------

bool SwgCuiVoiceActiveSpeakers::TableModel::GetLocalizedColumnName (int col, UIString & str) const
{
	if (col < 0 || col >= GetColumnCount ())
		return false;

	using namespace ColumnInfo;

	switch (m_type)
	{
	case T_ActiveSpeaker:
		str = data [ActiveSpeakers::ids [col]].name.localize ();
		break;
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool SwgCuiVoiceActiveSpeakers::TableModel::GetLocalizedColumnTooltip (int col, UIString & str) const
{
	if (col < 0 || col >= GetColumnCount ())
		return false;

	using namespace ColumnInfo;

	switch (m_type)
	{
	case T_ActiveSpeaker:
		str = data [ActiveSpeakers::ids [col]].tooltip.localize ();
		break;
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

UITableTypes::CellType SwgCuiVoiceActiveSpeakers::TableModel::FindColumnCellType     (int col) const
{
	if (col < 0 || col >= FindColumnCount ())
		return UITableTypes::CT_none;

	using namespace ColumnInfo;

	switch (m_type)
	{
	case T_ActiveSpeaker:
		return data [ActiveSpeakers::ids [col]].cellType;
	default:
		return UITableTypes::CT_none;
	}
}

//----------------------------------------------------------------------

int SwgCuiVoiceActiveSpeakers::TableModel::FindColumnCount () const
{
	using namespace ColumnInfo;

	switch (m_type)
	{
	case T_ActiveSpeaker:
		return ActiveSpeakers::numColumns;
	default:
		return 0;
	}
}

//----------------------------------------------------------------------

int SwgCuiVoiceActiveSpeakers::TableModel::FindRowCount () const
{
	return static_cast<int>(m_ownerMediator.getNumParticipants());
}

//----------------------------------------------------------------------

int SwgCuiVoiceActiveSpeakers::TableModel::getLogicalRowFromVisualRow(int visualRow) const
{
	return GetLogicalDataRowIndex (visualRow);
}


//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::TableModel::setType (Type type)
{
	m_type = type;
	fireColumnsChanged ();
	fireDataChanged ();
}

//----------------------------------------------------------------------

void SwgCuiVoiceActiveSpeakers::TableModel::updateTableColumnSizes (UITable & table) const
{
	//-- copy vector
	UITable::ColumnSizeInfoVector csiv = table.GetColumnSizeInfo ();
	const int columnCount = FindColumnCount ();
	csiv.resize (static_cast<unsigned int>(columnCount));

	int numToDivide = columnCount;

	long widthToDivide = table.GetTotalColumnAvailableWidth ();

	for (int i = 0; i < columnCount; ++i)
	{
		const ColumnInfo::Id id = ColumnInfo::findColumnId (T_ActiveSpeaker, i);
		UITable::ColumnSizeInfo & csi = csiv [static_cast<unsigned int>(i)];

		switch (id)
		{
		case ColumnInfo::I_state:
			csi.constant = true;
			csi.width    = 30;
			widthToDivide -= csi.width;
			--numToDivide;
			break;
		case ColumnInfo::I_name:
		case ColumnInfo::I_channel:
		case ColumnInfo::I_none:
		case ColumnInfo::I_count:
		default:
			break;
		}
		
		if (numToDivide)
		{
			const float prop = 1.0f / static_cast<float>(numToDivide);
			
			for (UITable::ColumnSizeInfoVector::iterator it = csiv.begin (); it != csiv.end (); ++it)
			{
				UITable::ColumnSizeInfo & csi2 = *it;
				csi2.proportion = prop;
			}
		}
	}

	table.SetColumnSizeInfo (csiv);
}

//======================================================================

