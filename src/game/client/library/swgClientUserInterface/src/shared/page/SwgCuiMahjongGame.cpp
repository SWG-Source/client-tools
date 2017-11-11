
#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "SwgCuiMahjongGame.h"

#include "SwgCuiMahjongDataTable.h"

#include "sharedRandom/Random.h"

#include <algorithm>


//----------------------------------------------------------------------

bool Tile::matches(Tile const & other) const
{
	return m_value == other.m_value;
}

//----------------------------------------------------------------------

bool Tile::blocks(Tile const & other) const
{
	return covers(other) || nextTo(other);
}

//----------------------------------------------------------------------

bool Tile::covers(Tile const & other) const
{
	return ((other.m_position.layer < m_position.layer) 
		&& (other.m_position.row >= m_position.row -1) && (other.m_position.row <= m_position.row + 1)
		&& (other.m_position.column >= m_position.column -1) && (other.m_position.column <= m_position.column + 1));
}

//----------------------------------------------------------------------

bool Tile::nextTo(Tile const & other) const
{
	return ((other.m_position.layer == m_position.layer)
		&& (other.m_position.row >= m_position.row -1) && (other.m_position.row <= m_position.row + 1) 
		&& (other.m_position.column >= m_position.column - 2) && (other.m_position.column <= m_position.column + 2));
}

//----------------------------------------------------------------------

void Tile::simpleFillDependencies(TileVector const & otherTiles)
{
	//todo: this can be waaaaaay faster

	m_dependencies.clear();
	for(TileVector::const_iterator i = otherTiles.begin(); i != otherTiles.end(); ++i)
	{
		Tile const & other = *i;
		if(other.getId() != getId())
		{
			if(other.m_position.layer == m_position.layer)
			{
				if((other.m_position.row >= m_position.row -1) && (other.m_position.row <= m_position.row + 1))
				{
					if(other.m_position.column == m_position.column - 2)
					{
						m_dependencies.leftOfMe.push_back(other.getId());
					}
					else if(other.m_position.column == m_position.column + 2)
					{
						m_dependencies.rightOfMe.push_back(other.getId());
					}
					else if(other.m_position.column >= m_position.column - 1 && other.m_position.column <= m_position.column + 1)
					{
						DEBUG_WARNING(true,("Tile position conflict between %d at (%d,%d,%d) and %d at (%d,%d,%d)",
							getId(), m_position.layer, m_position.row, m_position.column,
							other.getId(), other.m_position.layer, other.m_position.row, other.m_position.column));
					}
				}
			}
			else
			{
				if((other.m_position.row >= m_position.row -1) && (other.m_position.row <= m_position.row + 1)
					&& (other.m_position.column >= m_position.column -1) && (other.m_position.column <= m_position.column + 1))
				{
					if(other.m_position.layer + 1 == m_position.layer)
					{
						m_dependencies.underMe.push_back(other.getId());
					}
					else if (other.m_position.layer - 1 == m_position.layer)
					{
						m_dependencies.overMe.push_back(other.getId());
					}
				}
			}
		}
	}
}		

//----------------------------------------------------------------------

MahjongGame::MahjongGame(ScoreKeeper * scoreKeeper)
: m_tiles(), 
  m_moveHistory(), 
  m_numLayers(0), 
  m_numRows(0), 
  m_numColumns(0), 
  m_numTilesLeft(0),
  m_scoreKeeper(scoreKeeper)
{
}

//----------------------------------------------------------------------

MahjongGame::~MahjongGame()
{
	if(m_scoreKeeper != NULL)
	{
		delete m_scoreKeeper;
		m_scoreKeeper = NULL;
	}
}

//----------------------------------------------------------------------

bool MahjongGame::isTileIdValid(TileId id) const
{
	return (id >= 0 && id < static_cast<int>(m_tiles.size()));
}

//----------------------------------------------------------------------

Tile const & MahjongGame::getTile(TileId id) const
{
	FATAL(!isTileIdValid(id),("TileId %d out of range", id));
	return m_tiles[id];
}

//----------------------------------------------------------------------

bool MahjongGame::getTileIdsForValue(TileValue val, std::set<TileId> & outlist) const
{
	if(val < 0 || val > static_cast<TileValue>(m_valueTileLists.size()))
	{
		return false;
	}

	outlist = m_valueTileLists[val];
	return true;
}

//----------------------------------------------------------------------

void MahjongGame::setLayout(std::list<TilePosition> const & tiles)
{
	static const int tilesPerValue = 4;

	m_moveHistory.clear();
	m_tiles.clear();
	m_valueTileLists.clear();

	m_numLayers = 0;
	m_numRows = 0;
	m_numColumns = 0;

	TileId id = 0;
	for(std::list<TilePosition>::const_iterator pos = tiles.begin();  pos != tiles.end(); ++pos, ++id)
	{
		TileValue thisValue = id / tilesPerValue;
		m_tiles.push_back(Tile(id, *pos, thisValue));

		m_numLayers = std::max(m_numLayers, pos->layer + 1);
		m_numRows = std::max(m_numRows, pos->row + 1);
		m_numColumns = std::max(m_numColumns, pos->column + 1);
	}

	m_numTilesLeft = static_cast<int>(m_tiles.size());

	DEBUG_WARNING(m_numTilesLeft % 2 != 0, ("Layout has uneven number of tiles"));

	m_valueTileLists.resize((m_numTilesLeft / tilesPerValue) + 1);

	//TODO: this can be done in batch and much faster given the layout information
	for(TileVector::iterator i = m_tiles.begin(); i != m_tiles.end(); ++i)
	{
		Tile & thisTile = *i;
		thisTile.simpleFillDependencies(m_tiles);
		m_valueTileLists[thisTile.getValue()].insert(thisTile.getId());
	}
}

//----------------------------------------------------------------------

void MahjongGame::resetBoard()
{
	m_moveHistory.clear();
	m_numTilesLeft = static_cast<int>(m_tiles.size());

	for(TileVector::iterator i = m_tiles.begin(); i != m_tiles.end(); ++i)
	{
		(*i).setRemoved(false);
	}

	if(m_scoreKeeper)
	{
		m_scoreKeeper->gameStarted();
	}
}

//----------------------------------------------------------------------

void MahjongGame::randomizeTiles()
{
	//TODO: provide a way to undo across a shuffle?
	m_moveHistory.clear();

	std::vector<TileId> inPlayTileIds(m_tiles.size());
	std::vector<TileValue> inPlayTileValues(m_tiles.size());

	int inPlayCount = 0;
	for(TileVector::iterator tilesItr = m_tiles.begin(); tilesItr != m_tiles.end(); ++tilesItr)
	{
		if(!tilesItr->getIsRemoved())
		{
			inPlayTileIds[inPlayCount] = tilesItr->getId();
			inPlayTileValues[inPlayCount] = tilesItr->getValue();
			++inPlayCount;
		}
	}

	std::random_shuffle(inPlayTileValues.begin(), inPlayTileValues.begin() + inPlayCount);

	for(int i = 0; i < inPlayCount; ++i)
	{
		TileId const id = inPlayTileIds[i];
		TileValue const newVal = inPlayTileValues[i];
		TileValue const oldVal = m_tiles[id].getValue();

		m_tiles[id].setValue(newVal);

		//update the value to id list
		std::set<TileId>::iterator oldPos = m_valueTileLists[oldVal].find(id);
		if(oldPos != m_valueTileLists[oldVal].end())
		{
			m_valueTileLists[oldVal].erase(oldPos);
		}
		else
		{
			DEBUG_WARNING(true, ("Could not find old entry during shuffle value to id list fixup"));
		}

		m_valueTileLists[newVal].insert(id);
	}
}

//----------------------------------------------------------------------

bool MahjongGame::doTilesMatch(TileId tile1, TileId tile2) const
{
	if(!isTileIdValid(tile1) && !isTileIdValid(tile2))
	{
		return false;
	}

	return m_tiles[tile1].matches(m_tiles[tile2]);
}

//----------------------------------------------------------------------

bool MahjongGame::isTileBlocked(TileId id) const
{
	if(!isTileIdValid(id))
	{
		return false;
	}

	//check dependencies to see if there is an opening:
	//blocked = above || (left && right)

	Tile const & tile = m_tiles[id];
	TileDependencies const & dependencies = tile.getDependencies();

	for(std::list<TileId>::const_iterator above = dependencies.overMe.begin(); above != dependencies.overMe.end(); ++above)
	{
		if(!isTileRemoved(*above))
		{
			return true;
		}
	}

	bool blockedLeft = false;
	for(std::list<TileId>::const_iterator left = dependencies.leftOfMe.begin(); left != dependencies.leftOfMe.end(); ++left)
	{
		if(!isTileRemoved(*left))
		{
			blockedLeft = true;
			break;
		}
	}

	if(blockedLeft)
	{
		for(std::list<TileId>::const_iterator right = dependencies.rightOfMe.begin(); right != dependencies.rightOfMe.end(); ++right)
		{
			if(!isTileRemoved(*right))
			{
				return true;
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool MahjongGame::isTileRemoved(TileId id) const
{
	if(!isTileIdValid(id))
	{
		return false;
	}

	return m_tiles[id].getIsRemoved();
}

//----------------------------------------------------------------------

bool MahjongGame::isMoveAllowed(Move const & move) const
{
	if(!isTileIdValid(move.tile1) || !isTileIdValid(move.tile2))
	{
		return false;
	}

	if(!doTilesMatch(move.tile1, move.tile2) || move.tile1 == move.tile2)
	{
		return false;
	}

	if(isTileRemoved(move.tile1) || isTileRemoved(move.tile2))
	{
		return false;
	}

	if(isTileBlocked(move.tile1) || isTileBlocked(move.tile2))
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

bool MahjongGame::doMove(Move const & move)
{
	if(!isMoveAllowed(move))
	{
		return false;
	}

	m_tiles[move.tile1].setRemoved(true);
	m_tiles[move.tile2].setRemoved(true);

	m_numTilesLeft -= 2;

	m_moveHistory.push_back(move);

	if(m_scoreKeeper)
	{
		m_scoreKeeper->moveExecuted(move);
	}

	return true;
}

//----------------------------------------------------------------------

void MahjongGame::shuffle()
{
	randomizeTiles();

	if(m_scoreKeeper)
	{
		m_scoreKeeper->shufflePerformed();
	}
}

//----------------------------------------------------------------------

bool MahjongGame::canUndo()
{
	return !m_moveHistory.empty();
}

//----------------------------------------------------------------------

bool MahjongGame::undo(Move & outputMove)
{
	if(!canUndo())
	{
		return false;
	}

	outputMove = m_moveHistory.back();

	m_tiles[outputMove.tile1].setRemoved(false);
	m_tiles[outputMove.tile2].setRemoved(false);

	m_numTilesLeft += 2;

	m_moveHistory.pop_back();

	if(m_scoreKeeper)
	{
		m_scoreKeeper->undoPerformed(outputMove);
	}

	return true;
}

//----------------------------------------------------------------------

bool MahjongGame::isGameLost() const
{
	int numTiles = static_cast<int>(m_tiles.size());
	for(int i = 0; i < numTiles; ++i)
	{
		if(!m_tiles[i].getIsRemoved())
		{
			std::set<TileId> matching;
			getTileIdsForValue(m_tiles[i].getValue(), matching);

			for(std::set<TileId>::const_iterator other = matching.begin(); other != matching.end(); ++other)
			{
				if(isMoveAllowed(Move(i,*other)))
				{
					return false;
				}
			}
		}
	}
	return true;
}

//----------------------------------------------------------------------

int MahjongGame::getScore() const
{
	if(m_scoreKeeper != NULL)
	{
		return m_scoreKeeper->getScore();
	}
	return 0;
}



