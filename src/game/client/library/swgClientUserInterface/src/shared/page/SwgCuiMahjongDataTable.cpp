

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "SwgCuiMahjongDataTable.h"

#include "SwgCuiMahjongGame.h"

#include "sharedFoundation/Exitchain.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"


#include <map>



namespace SwgCuiMahjongDataTableNamespace
{
	bool m_installed = false;

	std::string const ms_columnLayoutsName("NAME");
	std::string const ms_columnLayoutsFile("FILE");

	std::string const ms_columnTilesetsSelectedSound("SELECTED_SOUND");
	std::string const ms_columnTilesetsCannotSelectSound("CANNOT_SELECT_SOUND");
	std::string const ms_columnTilesetsRemovedSound("REMOVED_SOUND");
	std::string const ms_columnTilesetsStartSound("START_SOUND");
	std::string const ms_columnTilesetsVictorySound("VICTORY_SOUND");
	std::string const ms_columnTilesetsLostSound("LOST_SOUND");
	std::string const ms_columnTilesetsShuffleSound("SHUFFLE_SOUND");
	std::string const ms_columnTilesetsUndoSound("UNDO_SOUND");
	std::string const ms_columnTilesetsGameClosedSound("GAME_CLOSED_SOUND");
	std::string const ms_columnTilesetsBaseTile("BASE_TILE");
	std::string const ms_columnTilesetsHighlight("HIGHLIGHT");


	std::string const ms_columnLayoutLayer("LAYER");
	std::string const ms_columnLayoutRow("ROW");
	std::string const ms_columnLayoutColumn("COLUMN");

	std::string const ms_columnTilesetImage("IMAGE");

	std::string const ms_layoutsTableName = "datatables/minigame/mahjong/layouts.iff";
	std::string const ms_tilesetsTableName = "datatables/minigame/mahjong/tilesets.iff";


	//----------------------------------------------------------------------

	struct LayoutEntry 
	{
		std::string name;
		std::string filename;
		std::list<TilePosition> tiles;
	};

	//----------------------------------------------------------------------

	struct TilesetEntry
	{
		std::string name;
		std::string filename;
		SwgCuiMahjongDataTable::TilesetImages images;
		std::string tileSelectedSound;
		std::string tileCannotSelectSound;
		std::string tileRemovedSound;
		std::string gameStartSound;
		std::string gameWonSound;
		std::string gameLostSound;
		std::string shuffleSound;
		std::string undoSound;
		std::string gameClosedSound;
	};


	//----------------------------------------------------------------------

	std::map<std::string, LayoutEntry> ms_layouts;

	std::map<std::string, TilesetEntry> ms_tilesets;

	//----------------------------------------------------------------------

	std::string toLower(const std::string & source)
	{
		std::string lower;
		size_t i;
		for(i = 0; i < source.length(); ++i)
		{
			char a = static_cast<char>(tolower(source[i]));
			lower += a;
		}
		return lower;
	}

	//----------------------------------------------------------------------

	bool readLayoutTable(DataTable* layoutDatatable, std::list<TilePosition> & tiles)
	{
		if(layoutDatatable)
		{
			tiles.clear();

			unsigned int const numRows = static_cast<unsigned int>(layoutDatatable->getNumRows());
			int const layerColumn = layoutDatatable->findColumnNumber(ms_columnLayoutLayer);
			int const rowColumn = layoutDatatable->findColumnNumber(ms_columnLayoutRow);
			int const columnColumn = layoutDatatable->findColumnNumber(ms_columnLayoutColumn);

			for(uint32 i = 0; i < numRows; ++i)
			{
				int const layer = layoutDatatable->getIntValue(layerColumn, i);
				int const row = layoutDatatable->getIntValue(rowColumn, i);
				int const column = layoutDatatable->getIntValue(columnColumn, i);

				tiles.push_back(TilePosition(layer,row,column));
			}
			return true;
		}
		return false;
	}

	//----------------------------------------------------------------------

	bool readTilesetTable(DataTable* tilesetDatatable, std::vector<std::string> & images)
	{
		if(tilesetDatatable)
		{
			images.clear();

			unsigned int const numRows = static_cast<unsigned int>(tilesetDatatable->getNumRows());
			int const imageColumn = tilesetDatatable->findColumnNumber(ms_columnTilesetImage);

			images.resize(numRows);
			for(uint32 i = 0; i < numRows; ++i)
			{
				images[i] = tilesetDatatable->getStringValue(imageColumn, i);
			}
			return true;
		}
		return false;
	}
};

using namespace SwgCuiMahjongDataTableNamespace;

//======================================================================

void SwgCuiMahjongDataTable::install()
{
	if(m_installed)
		return;
	m_installed = true;

	{
		DataTable const * const  allLayoutsDatatable = DataTableManager::getTable(ms_layoutsTableName, true);

		if(allLayoutsDatatable)
		{
			unsigned int const numRows = static_cast<unsigned int>(allLayoutsDatatable->getNumRows());
			int const nameColumn = allLayoutsDatatable->findColumnNumber(ms_columnLayoutsName);
			int const fileColumn = allLayoutsDatatable->findColumnNumber(ms_columnLayoutsFile);

			unsigned int i;
			for(i = 0; i < numRows; ++i)
			{
				LayoutEntry layout;

				layout.name = allLayoutsDatatable->getStringValue(nameColumn, i);
				layout.filename = allLayoutsDatatable->getStringValue(fileColumn, i);

				DataTable * layoutDatatable = DataTableManager::getTable(layout.filename, true);

				if(readLayoutTable(layoutDatatable, layout.tiles))
				{
					ms_layouts.insert(std::make_pair(toLower(layout.name), layout));
				}

				DataTableManager::close(layout.filename);
			}

			DataTableManager::close(ms_layoutsTableName);
		}
	}

	{
		DataTable const * const  tilesetsDatatable = DataTableManager::getTable(ms_tilesetsTableName, true);

		if(tilesetsDatatable)
		{
			unsigned int const numRows = static_cast<unsigned int>(tilesetsDatatable->getNumRows());
			int const nameColumn = tilesetsDatatable->findColumnNumber(ms_columnLayoutsName);
			int const fileColumn = tilesetsDatatable->findColumnNumber(ms_columnLayoutsFile);
			int const selectedColumn = tilesetsDatatable->findColumnNumber(ms_columnTilesetsSelectedSound);
			int const cannotSelectColumn = tilesetsDatatable->findColumnNumber(ms_columnTilesetsCannotSelectSound);
			int const removedColumn = tilesetsDatatable->findColumnNumber(ms_columnTilesetsRemovedSound);
			int const startColumn = tilesetsDatatable->findColumnNumber(ms_columnTilesetsStartSound);
			int const victoryColumn = tilesetsDatatable->findColumnNumber(ms_columnTilesetsVictorySound);
			int const lostColumn = tilesetsDatatable->findColumnNumber(ms_columnTilesetsLostSound);
			int const shuffleColumn = tilesetsDatatable->findColumnNumber(ms_columnTilesetsShuffleSound);
			int const undoColumn = tilesetsDatatable->findColumnNumber(ms_columnTilesetsUndoSound);
			int const gameClosedColumn = tilesetsDatatable->findColumnNumber(ms_columnTilesetsGameClosedSound);
			int const baseTileColumn = tilesetsDatatable->findColumnNumber(ms_columnTilesetsBaseTile);
			int const highlightColumn = tilesetsDatatable->findColumnNumber(ms_columnTilesetsHighlight);


			unsigned int i;
			for(i = 0; i < numRows; ++i)
			{
				TilesetEntry tileset;

				tileset.name = tilesetsDatatable->getStringValue(nameColumn, i);
				tileset.filename = tilesetsDatatable->getStringValue(fileColumn, i);
				tileset.tileSelectedSound = tilesetsDatatable->getStringValue(selectedColumn, i);
				tileset.tileCannotSelectSound = tilesetsDatatable->getStringValue(cannotSelectColumn, i);
				tileset.tileRemovedSound = tilesetsDatatable->getStringValue(removedColumn, i);
				tileset.gameStartSound = tilesetsDatatable->getStringValue(startColumn, i);
				tileset.gameWonSound = tilesetsDatatable->getStringValue(victoryColumn, i);
				tileset.gameLostSound = tilesetsDatatable->getStringValue(lostColumn, i);
				tileset.shuffleSound = tilesetsDatatable->getStringValue(shuffleColumn, i);
				tileset.undoSound = tilesetsDatatable->getStringValue(undoColumn, i);
				tileset.gameClosedSound = tilesetsDatatable->getStringValue(gameClosedColumn, i);

				tileset.images.baseTile = tilesetsDatatable->getStringValue(baseTileColumn, i);
				tileset.images.highlight = tilesetsDatatable->getStringValue(highlightColumn, i);


				DataTable * tilesetDatatable = DataTableManager::getTable(tileset.filename, true);

				if(readTilesetTable(tilesetDatatable, tileset.images.tileImages))
				{
					ms_tilesets.insert(std::make_pair(toLower(tileset.name), tileset));
				}

				DataTableManager::close(tileset.filename);
			}

			DataTableManager::close(ms_tilesetsTableName);
		}
	}

	ExitChain::add(SwgCuiMahjongDataTable::remove, "SwgCuiMahjongDataTable::remove", 0, false);
}

//======================================================================

void SwgCuiMahjongDataTable::remove()
{

}

//----------------------------------------------------------------------

void SwgCuiMahjongDataTable::getLayoutNames(std::list<std::string> & names)
{
	names.clear();
	for(std::map<std::string,LayoutEntry>::const_iterator i = ms_layouts.begin(); i != ms_layouts.end(); ++i)
	{
		names.push_back(i->second.name);
	}
}

//----------------------------------------------------------------------

bool SwgCuiMahjongDataTable::getLayoutTiles(std::string const & layoutName, std::list<TilePosition> & tiles)
{
	std::map<std::string,LayoutEntry>::const_iterator itr = ms_layouts.find(toLower(layoutName));
	if(itr != ms_layouts.end())
	{
		tiles = (itr->second).tiles;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

void SwgCuiMahjongDataTable::getTilesetNames(std::list<std::string> & names)
{
	names.clear();
	for(std::map<std::string,TilesetEntry>::const_iterator i = ms_tilesets.begin(); i != ms_tilesets.end(); ++i)
	{
		names.push_back(i->second.name);
	}
}

//----------------------------------------------------------------------

bool SwgCuiMahjongDataTable::getTilesetImages(std::string const & tilesetName, SwgCuiMahjongDataTable::TilesetImages & images)
{
	std::map<std::string,TilesetEntry>::const_iterator itr = ms_tilesets.find(toLower(tilesetName));
	if(itr != ms_tilesets.end())
	{
		images = (itr->second).images;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

bool SwgCuiMahjongDataTable::getTilesetSound(std::string const & tilesetName, MahjongTilesetSound soundType, std::string & soundName)
{
	std::map<std::string,TilesetEntry>::const_iterator itr = ms_tilesets.find(toLower(tilesetName));
	if(itr != ms_tilesets.end())
	{
		TilesetEntry const & entry = itr->second;
		switch(soundType)
		{
		case MTS_tileSelected:
			soundName = entry.tileSelectedSound;
			break;
		case MTS_tileCannotSelect:
			soundName = entry.tileCannotSelectSound;
			break;
		case MTS_tileRemoved:
			soundName = entry.tileRemovedSound;
			break;
		case MTS_gameStart:
			soundName = entry.gameStartSound;
			break;
		case MTS_gameWon:
			soundName = entry.gameWonSound;
			break;
		case MTS_gameLost:
			soundName = entry.gameLostSound;
			break;
		case MTS_shuffle:
			soundName = entry.shuffleSound;
			break;
		case MTS_undo:
			soundName = entry.undoSound;
			break;
		case MTS_gameClosed:
			soundName = entry.gameClosedSound;
			break;
		default:
			DEBUG_WARNING(true,("getTilesetSound called with invalid sound type %d", soundType));
			return false;
		}
		return true;
	}
	return false;
}
