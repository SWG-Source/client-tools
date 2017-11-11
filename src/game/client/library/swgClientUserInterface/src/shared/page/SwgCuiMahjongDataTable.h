// ======================================================================
//
// SwgCuiMahjongDataTable.h
// Copyright 2008 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#ifndef INCLUDED_SwgCuiMahjongDataTable_H
#define INCLUDED_SwgCuiMahjongDataTable_H


#include "SwgCuiMahjongGame.h"
#include <list>

class SwgCuiMahjongDataTable
{
public:
	static void install();
	static void remove();

	static void SwgCuiMahjongDataTable::getLayoutNames(std::list<std::string> & names);
	static bool SwgCuiMahjongDataTable::getLayoutTiles(std::string const & layoutName, std::list<TilePosition> & tiles);

	struct TilesetImages
	{
		std::string baseTile;
		std::string highlight;
		std::vector<std::string> tileImages;
	};

	static void getTilesetNames(std::list<std::string> & names);
	static bool getTilesetImages(std::string const & tilesetName, TilesetImages & images);

	enum MahjongTilesetSound
	{
		MTS_tileSelected,
		MTS_tileRemoved,
		MTS_tileCannotSelect,
		MTS_gameStart,
		MTS_gameWon,
		MTS_gameLost,
		MTS_shuffle,
		MTS_undo,
		MTS_gameClosed,
	};

	static bool getTilesetSound(std::string const & tilesetName, MahjongTilesetSound soundType, std::string & soundName);
};

#endif