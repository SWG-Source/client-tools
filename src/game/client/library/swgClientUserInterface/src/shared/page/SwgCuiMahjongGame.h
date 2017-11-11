
#ifndef INCLUDED_SwgCuiMahjongGame_H
#define INCLUDED_SwgCuiMahjongGame_H



#include <list>
#include <vector>
#include <set>

class Tile;
class MahjongGame;
typedef std::list<Tile> TileList;
typedef std::vector<Tile> TileVector;

typedef int TileId;
typedef int TileValue;

//----------------------------------------------------------------------

struct TileDependencies
{
	std::list<TileId> underMe;
	std::list<TileId> leftOfMe;
	std::list<TileId> rightOfMe;
	std::list<TileId> overMe;

	inline void clear()
	{
		underMe.clear();
		leftOfMe.clear();
		rightOfMe.clear();
		overMe.clear();
	}
};

//----------------------------------------------------------------------

struct TilePosition
{
	TilePosition() : layer(-1), row(-1), column(-1) {}
	TilePosition(int l, int r, int c) : layer(l), row(r), column(c) {}
	TilePosition(TilePosition const & p) : layer(p.layer), row(p.row), column(p.column) {}
	int layer;
	int row;
	int column;
};

//----------------------------------------------------------------------

class Tile
{
public:
	Tile() : m_id(-1), m_position(), m_value(0), m_removed(false) {}

	Tile(TileId id, TilePosition const& position, TileValue value)
		: m_id(id), m_position(position), m_value(value), m_removed(false)
	{
	}

	bool matches(Tile const & other) const;
	bool blocks(Tile const & other) const;
	bool covers(Tile const & other) const;
	bool nextTo(Tile const & other) const;

	inline TileId getId() const {return m_id;}
	inline TileValue getValue() const {return m_value;}
	inline TilePosition const & getPosition() const {return m_position;}
	inline bool getIsRemoved() const {return m_removed;}
	inline void setRemoved(bool removed) {m_removed = removed;}

	inline TileDependencies const & getDependencies() const {return m_dependencies;}
	void simpleFillDependencies(TileVector const & otherTiles);
	inline void setValue(TileValue value) {m_value = value;}


private:
	TileId m_id;
	TilePosition m_position;
	TileValue m_value;
	TileDependencies m_dependencies;

	bool m_removed;
};

//----------------------------------------------------------------------

class MahjongGame
{
public:

	struct Move
	{
		Move() : tile1(-1), tile2(-1) {}
		Move(TileId t1, TileId t2) : tile1(t1), tile2(t2){}
		TileId tile1;
		TileId tile2;
	};

	class ScoreKeeper
	{
	public:
		virtual void setOwner(MahjongGame * owner) = 0;
		virtual void gameStarted() = 0;
		virtual void moveExecuted(MahjongGame::Move const & move) = 0;
		virtual void undoPerformed(MahjongGame::Move const & move) = 0;
		virtual void shufflePerformed() = 0;
		virtual int getScore() = 0;
	};

	MahjongGame(ScoreKeeper * scoreKeeper);
	~MahjongGame();

	bool isTileIdValid(TileId id) const;

	bool doTilesMatch(TileId tile1, TileId tile2) const;
	bool isTileBlocked(TileId tile) const;
	bool isTileRemoved(TileId tile) const;
	bool isMoveAllowed(Move const & move) const;
	bool doMove(Move const & move);
	void shuffle();

	bool canUndo();
	bool undo(Move & outputMove);

	//Hide tiles?
	Tile const & getTile(TileId id) const;
	TileVector const & getTiles() const {return m_tiles;}

	bool getTileIdsForValue(TileValue val, std::set<TileId> & outlist) const;

	void setLayout(std::list<TilePosition> const & tiles);
	void resetBoard();
	void randomizeTiles();

	inline int getNumLayers() const {return m_numLayers;}
	inline int getNumRows() const {return m_numRows;}
	inline int getNumColumns() const {return m_numColumns;}
	inline int getNumTilesLeft() const {return m_numTilesLeft;}

	bool isGameWon() const {return m_numTilesLeft == 0;}
	bool isGameLost() const;

	int getScore() const;

	ScoreKeeper * getScoreKeeper() {return m_scoreKeeper;}

protected:

	TileVector m_tiles;

	std::vector<std::set<TileId> > m_valueTileLists;

	std::list<Move> m_moveHistory;

	int m_numLayers;
	int m_numRows;
	int m_numColumns;
	int m_numTilesLeft;

	ScoreKeeper * m_scoreKeeper;

};



#endif
