//======================================================================
//
// SwgCuiMissionBrowser_TableModel.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiMissionBrowser_TableModel.h"

#include "UITable.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"
#include "sharedUtility/Location.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponse.h"
#include "sharedObject/NetworkIdManager.h"
#include "clientUserInterface/CuiStringIdsMission.h"
#include "clientGame/ClientMissionObject.h"
#include "clientGame/Game.h"
#include "clientGame/ClientWaypointObject.h"
#include <vector>

//======================================================================

namespace
{
	namespace ColumnInfo
	{
		enum Id
		{
			I_creator,
			I_title,
			I_payment,
			I_start,
			I_none,
			I_count
		};
		
		struct ColumnData
		{
			StringId            name;
			UITableTypes::CellType cellType;

			ColumnData () :
				name (),
				cellType (UITableTypes::CT_none)
			{
			}

			ColumnData (const StringId & _name, UITableTypes::CellType _cellType) :
				name (_name),
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
			
			data [I_start]      = ColumnData (CuiStringIdsMission::table_start,       UITableTypes::CT_text);
			data [I_payment]    = ColumnData (CuiStringIdsMission::table_payment,     UITableTypes::CT_text);
			data [I_creator]    = ColumnData (CuiStringIdsMission::table_creator,     UITableTypes::CT_text);
			data [I_title]      = ColumnData (CuiStringIdsMission::table_title,       UITableTypes::CT_text);
		}


		namespace Destroy
		{
			const int numColumns = 4;
			const Id ids [numColumns] =
			{
				I_creator,
				I_title,
				I_payment,
				I_start
			};
		}
		
		namespace Deliver
		{
			const int numColumns = 4;
			const Id ids [numColumns] =
			{
				I_creator,
				I_title,
				I_payment,
				I_start
			};
		}
		
		namespace Bounty
		{
			const int numColumns = 3;
			const Id ids [numColumns] =
			{
				I_creator,
				I_title,
				I_payment
			};
		}

		namespace Dancer
		{
			const int numColumns = 4;
			const Id ids [numColumns] =
			{
				I_creator,
				I_title,
				I_payment,
				I_start
			};
		}

		namespace Musician
		{
			const int numColumns = 4;
			const Id ids [numColumns] =
			{
				I_creator,
				I_title,
				I_payment,
				I_start
			};
		}

		namespace Crafting
		{
			const int numColumns = 4;
			const Id ids [numColumns] =
			{
				I_creator,
				I_title,
				I_payment,
				I_start
			};
		}

		namespace Survey
		{
			const int numColumns = 4;
			const Id ids [numColumns] =
			{
				I_creator,
				I_title,
				I_payment,
				I_start
			};
		}

		namespace Hunting
		{
			const int numColumns = 4;
			const Id ids [numColumns] =
			{
				I_creator,
				I_title,
				I_payment,
				I_start
			};
		}

		namespace Assassin
		{
			const int numColumns = 4;
			const Id ids [numColumns] =
			{
				I_creator,
				I_title,
				I_payment,
				I_start
			};
		}

		namespace Recon
		{
			const int numColumns = 4;
			const Id ids [numColumns] =
			{
				I_creator,
				I_title,
				I_payment,
				I_start
			};
		}

		Id findColumnId (SwgCuiMissionBrowser::TableModel::Type type, int col)
		{
			DEBUG_FATAL (col < 0, ("bad col"));

			switch (type)
			{
			case SwgCuiMissionBrowser::TableModel::T_Destroy:
				DEBUG_FATAL (col >= ColumnInfo::Destroy::numColumns, ("bad col"));	
				return ColumnInfo::Destroy::ids [col];

			case SwgCuiMissionBrowser::TableModel::T_Deliver:
				DEBUG_FATAL (col >= ColumnInfo::Deliver::numColumns, ("bad col"));	
				return ColumnInfo::Deliver::ids [col];

			case SwgCuiMissionBrowser::TableModel::T_Bounty:
				DEBUG_FATAL (col >= ColumnInfo::Bounty::numColumns,  ("bad col"));	
				return ColumnInfo::Bounty::ids [col];

			case SwgCuiMissionBrowser::TableModel::T_Dancer:
				DEBUG_FATAL (col >= ColumnInfo::Dancer::numColumns,  ("bad col"));	
				return ColumnInfo::Dancer::ids [col];

			case SwgCuiMissionBrowser::TableModel::T_Musician:
				DEBUG_FATAL (col >= ColumnInfo::Musician::numColumns,  ("bad col"));	
				return ColumnInfo::Musician::ids [col];

			case SwgCuiMissionBrowser::TableModel::T_Crafting:
				DEBUG_FATAL (col >= ColumnInfo::Crafting::numColumns,  ("bad col"));	
				return ColumnInfo::Crafting::ids [col];

			case SwgCuiMissionBrowser::TableModel::T_Survey:
				DEBUG_FATAL (col >= ColumnInfo::Survey::numColumns,  ("bad col"));	
				return ColumnInfo::Survey::ids [col];

			case SwgCuiMissionBrowser::TableModel::T_Hunting:
				DEBUG_FATAL (col >= ColumnInfo::Hunting::numColumns,  ("bad col"));	
				return ColumnInfo::Hunting::ids [col];
			
			case SwgCuiMissionBrowser::TableModel::T_Assassin:
				DEBUG_FATAL (col >= ColumnInfo::Assassin::numColumns,  ("bad col"));	
				return ColumnInfo::Assassin::ids [col];

			case SwgCuiMissionBrowser::TableModel::T_Recon:
				DEBUG_FATAL (col >= ColumnInfo::Recon::numColumns,  ("bad col"));	
				return ColumnInfo::Recon::ids [col];

			case SwgCuiMissionBrowser::TableModel::T_Count:
			default:
				return I_none;
			}			
		}		
	}
}

//----------------------------------------------------------------------

SwgCuiMissionBrowser::TableModel::TableModel () :
UITableModel (),
m_type (T_Destroy),
m_data (new DataVector [static_cast<int>(T_Count)])
{
	ColumnInfo::install ();
	fireDataChanged();
	fireColumnsChanged();
	//start with the browser sorting on the name
	sortOnColumn (0, SD_down);
}

//----------------------------------------------------------------------

SwgCuiMissionBrowser::TableModel::~TableModel ()
{
	delete[] m_data;
	m_data = 0;
}

//----------------------------------------------------------------------

const SwgCuiMissionBrowser::TableModel::DataElement * SwgCuiMissionBrowser::TableModel::findDataElement (int visualRow) const
{
	if (visualRow < 0)
		return NULL;

	const long row = GetLogicalDataRowIndex (visualRow);
	const int size = static_cast<int>(m_data [m_type].size ());
	UNREF (size);
	if(row < 0 || size <= row)
	{
		DEBUG_WARNING (true, ("bad row"));
		return NULL;
	}
	return m_data [m_type][static_cast<unsigned int>(row)];
}

//----------------------------------------------------------------------

bool SwgCuiMissionBrowser::TableModel::GetValueAtText   (int row, int col, UIString     & value)  const
{
	const DataElement * const elem = findDataElement (row);
	if(!elem)
		return false;

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	char buffer[256];

	// @todo we must do some sort of caching and localizing here!!!
	switch (id)
	{
	case ColumnInfo::I_start:
		{
			const Location & location = elem->getStartLocation();
			StringId sid("planet_n", location.getSceneId());
			//if no info, display unknown planet
			if(strlen(location.getSceneId()) == 0)
			{
				value = CuiStringIdsMission::unknown_planet.localize();
			}
			//if the location's on another planet, just list the planet
			else if(location.getSceneIdCrc() != Location::getCrcBySceneName(Game::getSceneId().c_str()))
			{				
				value = sid.localize();
			}
			//else list the direction/distance
			else
			{
				const Object* const player = Game::getPlayer();
				if(player)
				{
					const Vector& playerLoc = player->getPosition_w();
					const Vector& waypointLoc = location.getCoordinates();
					float distance = abs(playerLoc.magnitudeBetween(waypointLoc));
					int iDistance = static_cast<int>(distance);
					_itoa(iDistance, buffer, 10);
					value = Unicode::narrowToWide(buffer);
					
					Vector delta = waypointLoc - playerLoc;
					real theta = delta.theta();
					
					std::string cardinal;
					
					if(theta > -PI/8 && theta < PI/8)
						cardinal = "N";
					else if(theta > PI/8 && theta < 3*PI/8)
						cardinal = "NE";
					else if(theta > 3*PI/8 && theta < 5*PI/8)
						cardinal = "E";
					else if(theta > 5*PI/8 && theta < 7*PI/8)
						cardinal = "SE";
					else if(theta < -PI/8 && theta > -3*PI/8)
						cardinal = "NW";
					else if(theta < -3*PI/8 && theta > -5*PI/8)
						cardinal = "W";
					else if(theta < -5*PI/8 && theta > -7*PI/8)
						cardinal = "SW";
					else
						cardinal = "S";
					value += Unicode::narrowToWide("m to the ");
					value += Unicode::narrowToWide(cardinal);
				}
			}
		}
		
		break;

	case ColumnInfo::I_creator:
		{
			value = elem->getMissionCreator();
			if(value.empty())
				value = Unicode::narrowToWide("Dynamic Mission");
		}
		break;
	case ColumnInfo::I_title:
		value = elem->getTitle().localize();
		break;

	case ColumnInfo::I_payment:
		{
			int const valueInt = elem->getReward();

			if(valueInt == 0)
			{
				value = CuiStringIdsMission::unknown_reward.localize();
			}
			else
			{
				_itoa(valueInt, buffer, 10);
				value = Unicode::narrowToWide(buffer);
			}
		}
		break;

	case ColumnInfo::I_none:
	case ColumnInfo::I_count:
	default:
		return false;
	}
	
	return true;
}

//----------------------------------------------------------------------

bool SwgCuiMissionBrowser::TableModel::GetValueAtBool   (int row, int col, bool &)          const
{
	if (GetColumnCellType (col) != UITableTypes::CT_bool)
		return false;

	DEBUG_FATAL (row < 0, ("bad row"));

	const DataElement * const elem = findDataElement (row);
	if(!elem)
		return false;
	
	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	switch (id)
	{
	case ColumnInfo::I_none:
	case ColumnInfo::I_start:
	case ColumnInfo::I_payment:
	case ColumnInfo::I_creator:
	case ColumnInfo::I_title:
	case ColumnInfo::I_count:
	default:
		return false;
	}
}

//----------------------------------------------------------------------

bool SwgCuiMissionBrowser::TableModel::GetValueAtInteger(int row, int col, int & value)           const
{
	DEBUG_FATAL (row < 0, ("bad row"));
	UNREF(value);

	const DataElement * const elem = findDataElement (row);
	if(!elem)
		return false;

	const ColumnInfo::Id id = ColumnInfo::findColumnId (m_type, col);

	// @todo we must do some sort of caching and localizing here!!!
	switch (id)
	{
	case ColumnInfo::I_payment:
	case ColumnInfo::I_none:
	case ColumnInfo::I_start:
	case ColumnInfo::I_creator:
	case ColumnInfo::I_title:
	case ColumnInfo::I_count:
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::TableModel::getMissionDataId (int row, NetworkId & id) const
{
	if (row < 0 || row >= GetRowCount ())
		id = NetworkId::cms_invalid;
	else
	{		
		const DataElement * const elem = findDataElement (row);
		if(!elem)
			id = NetworkId::cms_invalid;
		else
			id = elem->getNetworkId();
	}
}

//----------------------------------------------------------------------

bool SwgCuiMissionBrowser::TableModel::GetLocalizedColumnName (int col, UIString & str) const
{
	if (col < 0 || col >= GetColumnCount ())
		return false;

	switch (m_type)
	{
	case T_Destroy:
		str = ColumnInfo::data [ColumnInfo::Destroy::ids [col]].name.localize ();
		break;
	case T_Deliver:
		str = ColumnInfo::data [ColumnInfo::Deliver::ids [col]].name.localize ();
		break;
	case T_Bounty:
		str = ColumnInfo::data [ColumnInfo::Bounty::ids [col]].name.localize ();
		break;
	case T_Dancer:
		str = ColumnInfo::data [ColumnInfo::Dancer::ids [col]].name.localize ();
		break;
	case T_Musician:
		str = ColumnInfo::data [ColumnInfo::Musician::ids [col]].name.localize ();
		break;
	case T_Crafting:
		str = ColumnInfo::data [ColumnInfo::Crafting::ids [col]].name.localize ();
		break;
	case T_Survey:
		str = ColumnInfo::data [ColumnInfo::Survey::ids [col]].name.localize ();
		break;
	case T_Hunting:
		str = ColumnInfo::data [ColumnInfo::Hunting::ids [col]].name.localize ();
		break;
	case T_Assassin:
		str = ColumnInfo::data [ColumnInfo::Assassin::ids [col]].name.localize ();
		break;
	case T_Recon:
		str = ColumnInfo::data [ColumnInfo::Recon::ids [col]].name.localize ();
		break;
	case T_Count:
	default:
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

UITableTypes::CellType SwgCuiMissionBrowser::TableModel::FindColumnCellType (int col) const
{
	if (col < 0 || col >= FindColumnCount ())
		return UITableTypes::CT_none;

	switch (m_type)
	{
	case T_Destroy:
		return ColumnInfo::data [ColumnInfo::Destroy::ids [col]].cellType;
	case T_Deliver:
		return ColumnInfo::data [ColumnInfo::Deliver::ids [col]].cellType;
	case T_Bounty:
		return ColumnInfo::data [ColumnInfo::Bounty::ids [col]].cellType;
	case T_Dancer:
		return ColumnInfo::data [ColumnInfo::Dancer::ids [col]].cellType;
	case T_Musician:
		return ColumnInfo::data [ColumnInfo::Musician::ids [col]].cellType;
	case T_Crafting:
		return ColumnInfo::data [ColumnInfo::Crafting::ids [col]].cellType;
	case T_Survey:
		return ColumnInfo::data [ColumnInfo::Survey::ids [col]].cellType;
	case T_Hunting:
		return ColumnInfo::data [ColumnInfo::Hunting::ids [col]].cellType;
	case T_Assassin:
		return ColumnInfo::data [ColumnInfo::Assassin::ids [col]].cellType;
	case T_Recon:
		return ColumnInfo::data [ColumnInfo::Recon::ids [col]].cellType;
	case T_Count:
	default:
		return UITableTypes::CT_none;
	}
}

//----------------------------------------------------------------------

int  SwgCuiMissionBrowser::TableModel::FindColumnCount () const
{
	switch (m_type)
	{
	case T_Destroy:
		return ColumnInfo::Destroy::numColumns;
	case T_Deliver:
		return ColumnInfo::Deliver::numColumns;
	case T_Bounty:
		return ColumnInfo::Bounty::numColumns;
	case T_Dancer:
		return ColumnInfo::Dancer::numColumns;
	case T_Musician:
		return ColumnInfo::Musician::numColumns;
	case T_Crafting:
		return ColumnInfo::Crafting::numColumns;
	case T_Survey:
		return ColumnInfo::Survey::numColumns;
	case T_Hunting:
		return ColumnInfo::Hunting::numColumns;
	case T_Assassin:
		return ColumnInfo::Assassin::numColumns;
	case T_Recon:
		return ColumnInfo::Recon::numColumns;
	case T_Count:
	default:
		return 0;
	}
}

//----------------------------------------------------------------------

int  SwgCuiMissionBrowser::TableModel::FindRowCount () const
{
	return static_cast<int>(m_data [m_type].size ());
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::TableModel::setType (Type type)
{
	m_type = type;

	fireDataChanged();
	fireColumnsChanged ();
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::TableModel::setData (Type type, const std::vector<ConstWatcher<ClientMissionObject> > & dv)
{
	DEBUG_FATAL (static_cast<int>(type) >= static_cast<int>(T_Count), ("bad type"));

	m_data [type] = dv;

	if (type == m_type)
	{
		fireDataChanged();
		fireColumnsChanged ();
	}
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::TableModel::clearAll ()
{
	for(int i = 0; i < T_Count; ++i)
		m_data [i].clear ();
	
	fireDataChanged();
	fireColumnsChanged ();
}

//----------------------------------------------------------------------

void SwgCuiMissionBrowser::TableModel::updateTableColumnSizes (UITable & table) const
{
	//-- copy vector
	UITable::ColumnSizeInfoVector csiv = table.GetColumnSizeInfo ();
	const int columnCount = FindColumnCount ();
	csiv.resize (static_cast<unsigned int>(columnCount));

	//divy up the total amount to the columns
	int numToDivide = columnCount;

	for (int i = 0; i < columnCount; ++i)
	{
		const ColumnInfo::Id id = ColumnInfo::Id(i);
		UITable::ColumnSizeInfo & csi = csiv [static_cast<unsigned int>(i)];

		switch (id)
		{
		case ColumnInfo::I_payment:
			csi.constant = true;
			//set the icon column to a specific size (to help the icon size)
			csi.width    = 60;
			--numToDivide;
			break;
		case ColumnInfo::I_none:
		case ColumnInfo::I_start:
		case ColumnInfo::I_creator:
		case ColumnInfo::I_title:
		case ColumnInfo::I_count:
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
