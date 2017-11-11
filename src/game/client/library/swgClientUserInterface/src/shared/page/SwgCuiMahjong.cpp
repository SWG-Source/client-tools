

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "SwgCuiMahjong.h"

#include "SwgCuiMahjongGame.h"
#include "SwgCuiMahjongDataTable.h"

#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Timer.h"
#include "sharedUtility/ValueDictionary.h"
#include "sharedUtility/ValueTypeString.h"

#include "StringId.h"

#include "UIButton.h"
#include "UIButtonStyle.h"
#include "UIComboBox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIList.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIText.h"

#include <algorithm>


std::string const SwgCuiMahjong::cms_gameName("Mahjong");
std::string const SwgCuiMahjong::keyLayout("layout");


namespace SwgCuiMahjongNamespace
{
	struct RenderOrderSorter
	{
		TileVector const & m_tileList;
		RenderOrderSorter(TileVector const & l)
			:m_tileList(l)
		{
		}

		bool operator()(int i, int j)
		{
			TilePosition const & a = m_tileList[i].getPosition();
			TilePosition const & b = m_tileList[j].getPosition();

			bool const afirst = false;
			bool const bfirst = true;

			if(a.layer < b.layer)
			{
				return afirst;
			}
			else if (a.layer > b.layer)
			{
				return bfirst;
			}
			else //a.layer == b.layer
			{
				if(((a.row <= b.row + 1) && (a.column < b.column))
					|| ((a.column <= b.column + 1) && (a.row < b.row)))
				{
					return afirst;
				}
				else
				{
					return bfirst;
				}
			}

			return bfirst;
		}

	private:
		RenderOrderSorter const & operator=(RenderOrderSorter const &);
	};

	//----------------------------------------------------------------------

	class SwgMahjongScoreKeeper : public MahjongGame::ScoreKeeper
	{
	public:
		SwgMahjongScoreKeeper();
		virtual void setOwner(MahjongGame *) {}
		virtual void gameStarted();
		virtual void moveExecuted(MahjongGame::Move const &);
		virtual void undoPerformed(MahjongGame::Move const &);
		virtual void shufflePerformed();
		virtual int getScore();

		void update(float deltaTimeSeconds);

		int getChain() {return m_chainCount;}
		float getChainTimerRemainingRatio() {return m_chainTimer.getRemainingRatio();}

		static const int pointsForMove = 2;
		static const int pointsForShuffle = -4;
		static const int pointsForUndo = -1;
		static const int maxChainCount = 8;
		static const float chainTime;

	private:
		int m_totalPoints;
		std::list<int> m_pointsForMoves;
		int m_numUndos;
		int m_numShuffles;
		int m_chainCount;
		Timer m_chainTimer;
	};

	float const SwgMahjongScoreKeeper::chainTime = 3.0f;


	static MahjongGame ms_mahjongBoard(new SwgMahjongScoreKeeper());
	static SwgCuiMahjongDataTable::TilesetImages ms_tileset;

	static const UILowerString tileIdProperty = UILowerString("tileId");
	static const UILowerString palColorProperty = UILowerString("PalColor");
	static const UILowerString palSelectedColorProperty = UILowerString("SelectedPalColor");
	static const UILowerString palMatchingColorProperty = UILowerString("HighlightPalColor");

	static std::string const minigameStringTable("ui_minigame");
	static StringId const gameOverText(minigameStringTable, "mahjong_gameover_generic");
	static StringId const victoryText(minigameStringTable, "mahjong_gameover_victory");
	static StringId const lossText(minigameStringTable, "mahjong_gameover_loss");


	static char const * const highlightIconName = "front.highlight";
	static char const * const backgroundIconName = "background";
	static char const * const tileIconName = "front.icon";
	static char const * const frontPageName = "front";

	//----------------------------------------------------------------------

	bool playSoundHelper(std::string const & tileset, SwgCuiMahjongDataTable::MahjongTilesetSound soundType)
	{
		std::string name;
		if(SwgCuiMahjongDataTable::getTilesetSound(tileset, soundType, name) && !name.empty())
		{
			UIManager::gUIManager().PlaySound( name.c_str () );
			return true;
		}
		return false;
	}

	//----------------------------------------------------------------------

	UIImageStyle * getTileImageForValue(TileValue value)
	{
		if(value >= 0 && value < static_cast<int>(ms_tileset.tileImages.size()))
		{
			UIImageStyle * imageStyle = safe_cast<UIImageStyle *>(UIManager::gUIManager().GetRootPage()->GetObjectFromPath (ms_tileset.tileImages[value].c_str(), TUIImageStyle));
			return imageStyle;
		}

		return NULL;
	}

	//----------------------------------------------------------------------

	UIImageStyle * getTileBackgroundImage()
	{

		UIImageStyle * imageStyle = safe_cast<UIImageStyle *>(UIManager::gUIManager().GetRootPage()->GetObjectFromPath (ms_tileset.baseTile.c_str(), TUIImageStyle));
		return imageStyle;
	}

	//----------------------------------------------------------------------

	UIImageStyle * getTileHighlightImage()
	{
		UIImageStyle * imageStyle = safe_cast<UIImageStyle *>(UIManager::gUIManager().GetRootPage()->GetObjectFromPath (ms_tileset.highlight.c_str(), TUIImageStyle));
		return imageStyle;
	}

	//----------------------------------------------------------------------

	bool setChildIconStyle(UIPage * page, char const * const childName, UIImageStyle * style)
	{
		UIBaseObject * iconObject = page->GetChild(childName);
		if(iconObject && iconObject->IsA(TUIImage))
		{
			if(style)
			{
				UIImage * iconImage = static_cast<UIImage*>(iconObject);
				iconImage->SetStyle(style);
				return true;
			}
		}
		return false;
	}

	//----------------------------------------------------------------------

	SwgMahjongScoreKeeper::SwgMahjongScoreKeeper() 
	: 	m_totalPoints(0),
		m_pointsForMoves(),
		m_numUndos(0),
		m_numShuffles(0),
		m_chainCount(1),
		m_chainTimer(chainTime)
	{}

	//----------------------------------------------------------------------

	void SwgMahjongScoreKeeper::gameStarted()
	{
		m_totalPoints = 0;
		m_pointsForMoves.clear();
		m_numUndos = 0;
		m_numShuffles = 0;
		m_chainCount = 1;
		m_chainTimer.reset();
	}

	//----------------------------------------------------------------------

	void SwgMahjongScoreKeeper::moveExecuted(MahjongGame::Move const &)
	{
		m_chainTimer.reset();

		int pointsEarned = pointsForMove * (1 << m_chainCount);

		m_pointsForMoves.push_back(pointsEarned);
		m_totalPoints += pointsEarned;

		++m_chainCount;
		m_chainCount = std::min(m_chainCount, maxChainCount);
	}

	//----------------------------------------------------------------------

	void SwgMahjongScoreKeeper::undoPerformed(MahjongGame::Move const &)
	{
		m_chainCount = 1;

		if(!m_pointsForMoves.empty())
		{
			int pointsFromMove = m_pointsForMoves.back();
			m_pointsForMoves.pop_back();
			m_totalPoints -= pointsFromMove;
		}

		++m_numUndos;
		m_totalPoints += pointsForUndo;
		m_totalPoints = std::max(0, m_totalPoints);
	}

	//----------------------------------------------------------------------

	void SwgMahjongScoreKeeper::shufflePerformed()
	{
		m_chainCount = 1;
		++m_numShuffles;
		m_totalPoints += pointsForShuffle;
		m_totalPoints = std::max(0, m_totalPoints);
	}

	//----------------------------------------------------------------------

	int SwgMahjongScoreKeeper::getScore()
	{
		return m_totalPoints;
	}

	//----------------------------------------------------------------------

	void SwgMahjongScoreKeeper::update(float deltaTimeSeconds)
	{
		if( m_chainCount > 1 && m_chainTimer.updateZero(deltaTimeSeconds))
		{
			m_chainCount = 1;
		}
	}

}


using namespace SwgCuiMahjongNamespace;

SwgCuiMahjong::SwgCuiMahjong (UIPage & page) :
CuiMinigameBase ("SwgCuiMahjong", page),
UIEventCallback (),
m_mahjongPage(NULL),
m_closeButton(NULL),
m_newGameButton(NULL),
m_undoButton(NULL),
m_shuffleButton(NULL),
m_helpButton(NULL),
m_tilesLeftText(NULL),
m_scoreText(NULL),
m_chainText(NULL),
m_chainTimerBar(NULL),
m_newGameLayoutCombo(NULL),
m_newGameTilesetCombo(NULL),
m_newGameOkButton(NULL),
m_newGamePage(NULL),
m_gameOverPage(NULL),
m_gameOverOkButton(NULL),
m_newGameCancelButton(NULL),
m_gameOverUndoButton(NULL),
m_gameOverShuffleButton(NULL),
m_gameOverText(NULL),
m_helpPage(NULL),
m_helpOkButton(NULL),
m_exitPage(NULL),
m_exitYesButton(NULL),
m_exitNoButton(NULL),
m_sampleTilePage(NULL),
m_selectedTileId(-1),
m_highlightTileValue(-1),
m_waitingOnDialogue(false),
m_tilesetName(),
m_layoutName(),
m_tableOid(),
m_tileIdToTileMap(),
m_currentState(GS_notReady)
{
	setState (MS_closeable);
	setState (MS_closeDeactivates);

	getCodeDataObject(TUIPage, m_mahjongPage, "mahjongpage");

	getCodeDataObject(TUIButton, m_closeButton, "closebutton");
	getCodeDataObject(TUIButton, m_newGameButton, "newgamebutton");
	getCodeDataObject(TUIButton, m_undoButton, "undobutton");
	getCodeDataObject(TUIButton, m_shuffleButton, "shufflebutton");
	getCodeDataObject(TUIButton, m_helpButton, "helpbutton");

	getCodeDataObject(TUIText, m_tilesLeftText, "tilesleft");
	getCodeDataObject(TUIText, m_scoreText, "score");
	getCodeDataObject(TUIText, m_chainText, "chaintext");
	getCodeDataObject(TUIPage, m_chainTimerBar, "chainzipbar");

	{
		getCodeDataObject(TUIComboBox, m_newGameLayoutCombo, "newgamelayoutcombo");
		UIDataSource * const dataSource = m_newGameLayoutCombo->GetDataSource();
		dataSource->Clear();
	}

	{
		getCodeDataObject(TUIComboBox, m_newGameTilesetCombo, "newgametilesetcombo");
		UIDataSource * const dataSource = m_newGameTilesetCombo->GetDataSource();
		dataSource->Clear();
	}

	getCodeDataObject(TUIButton, m_newGameOkButton, "newgameokbutton");
	getCodeDataObject(TUIButton, m_newGameCancelButton, "newgamecancelbutton");
	getCodeDataObject(TUIPage, m_newGamePage, "newgamepage");
	m_newGamePage->SetVisible(false);

	getCodeDataObject(TUIText, m_gameOverText, "gameovertext");
	getCodeDataObject(TUIButton, m_gameOverOkButton, "gameoverokbutton");
	getCodeDataObject(TUIButton, m_gameOverUndoButton, "gameoverundobutton");
	getCodeDataObject(TUIButton, m_gameOverShuffleButton, "gameovershufflebutton");
	getCodeDataObject(TUIPage, m_gameOverPage, "gameoverpage");
	m_gameOverPage->SetVisible(false);

	getCodeDataObject(TUIPage, m_helpPage, "helppage");
	getCodeDataObject(TUIButton, m_helpOkButton, "helpokbutton");
	m_helpPage->SetVisible(false);

	getCodeDataObject(TUIPage, m_exitPage, "exitpage");
	getCodeDataObject(TUIButton, m_exitYesButton, "exityesbutton");
	getCodeDataObject(TUIButton, m_exitNoButton, "exitnobutton");
	m_exitPage->SetVisible(false);

	getCodeDataObject(TUIPage, m_sampleTilePage, "sampletile");
	m_sampleTilePage->SetVisible(false);

}

//----------------------------------------------------------------------

SwgCuiMahjong::~SwgCuiMahjong()
{
	clearTiles();

	m_mahjongPage = NULL;
	m_closeButton = NULL;
	m_newGameButton = NULL;
	m_undoButton = NULL;
	m_shuffleButton = NULL;
	m_helpButton = NULL;
	m_sampleTilePage = NULL;
	m_tilesLeftText = NULL;
	m_scoreText = NULL;
	m_chainText = NULL;
	m_chainTimerBar = NULL;
	m_newGameLayoutCombo = NULL;
	m_newGameTilesetCombo = NULL;
	m_newGameOkButton = NULL;
	m_newGameCancelButton = NULL;
	m_newGamePage = NULL;
	m_gameOverPage = NULL;
	m_gameOverOkButton = NULL;
	m_gameOverUndoButton = NULL;
	m_gameOverShuffleButton = NULL;
	m_gameOverText = NULL;
	m_helpPage = NULL;
	m_helpOkButton = NULL;
	m_exitPage = NULL;
	m_exitYesButton = NULL;
	m_exitNoButton = NULL;
}

//----------------------------------------------------------------------

void SwgCuiMahjong::performActivate   ()
{
	CuiManager::requestPointer(true);
	hideAllDialogueWindows();
	setIsUpdating(true);
	populateTiles();

	if(m_currentState == GS_inProgress)
	{
		checkGameOver(false);
	}
	else if (m_currentState != GS_ready)
	{
		showNewGameWindow();
	}
}

//----------------------------------------------------------------------

void SwgCuiMahjong::performDeactivate ()
{
	CuiManager::requestPointer(false);
	clearTiles();
	setIsUpdating(false);
}

//----------------------------------------------------------------------

bool SwgCuiMahjong::close ()
{
	playSoundHelper(m_tilesetName, SwgCuiMahjongDataTable::MTS_gameClosed);
	submitScore();
	return CuiMediator::close();
}

//----------------------------------------------------------------------

void SwgCuiMahjong::clearTiles()
{
	m_selectedTileId = -1;
	m_tileIdToTileMap.clear();

	UIBaseObject::UIObjectList tileObjects;
	m_mahjongPage->GetChildren(tileObjects);

	for(UIBaseObject::UIObjectList::iterator i = tileObjects.begin(); i != tileObjects.end(); ++i)
	{
		unregisterMediatorObject(*(*i));
		m_mahjongPage->RemoveChild(*i);
		(*i)->Detach(0);
	}
}

//----------------------------------------------------------------------

void SwgCuiMahjong::populateTiles()
{
	FormattedString<100> fs;

	updateTilesLeftCount();
	updateScoreText();

	TileVector const & tileSet = ms_mahjongBoard.getTiles();

	UISize pageSize = m_mahjongPage->GetSize();

	int const numRows = ms_mahjongBoard.getNumRows();
	int const numCols = ms_mahjongBoard.getNumColumns();
	int const numTiles = static_cast<int>(tileSet.size());

	UIWidget * iconWidget = m_sampleTilePage;

	UISize unscaledTileParentOffset(0,0);

	{
		UIBaseObject * frontObject = m_sampleTilePage->GetChild(frontPageName);
		if(frontObject && frontObject->IsA(TUIWidget))	
		{
			iconWidget = static_cast<UIImage*>(frontObject);
			unscaledTileParentOffset = iconWidget->GetLocation();
		}
	}

	UISize const unscaledTileSize = iconWidget->GetSize();
	UISize const unscaledTileParentSize = m_sampleTilePage->GetSize();

	//calculate the largest tile we can use to fit this layout on the screen
	UISize tileSize = unscaledTileSize;

	{
		UISize tileMaxSize = iconWidget->GetMaximumSize();
		UISize tileMinSize = iconWidget->GetMinimumSize();

		int maxHeight = clamp(tileMinSize.y, (pageSize.y * 2) / std::max(1,(numRows + 2)), tileMaxSize.y);
		int widthFromHeight = maxHeight * tileSize.x / tileSize.y;

		int maxWidth = clamp(tileMinSize.x, (pageSize.x * 2) / std::max(1,(numCols + 2)), tileMaxSize.x);
		int heightFromWidth = maxWidth * tileSize.y / tileSize.x;

		if(maxHeight < heightFromWidth)
		{
			tileSize.y = maxHeight;
			tileSize.x = widthFromHeight;
		}
		else
		{
			tileSize.y = heightFromWidth;
			tileSize.x = maxWidth;
		}
	}

	UISize const scaledParentSize = (unscaledTileParentSize * tileSize.y) / unscaledTileSize.y;
	UISize const scaledTileParentOffset = ((unscaledTileParentOffset* tileSize.y) / unscaledTileSize.y);
	UISize const scaledTileLayerOffset = scaledParentSize - (scaledTileParentOffset + tileSize);

	int boardMidpointRow = (numRows + 1) / 2;
	int boardMidpointCol = (numCols + 1) / 2;

	UIPoint boardOffset = (pageSize / 2) - UIPoint((boardMidpointCol * tileSize.x) / 2, (boardMidpointRow * tileSize.y) / 2);

	//figure out the order we need to add the tiles to the ui for them to draw properly
	std::vector<int> addOrder;
	addOrder.resize(numTiles);
	for(int i = 0; i < numTiles; ++i)
	{
		addOrder[i] = i;
	}

	{
		RenderOrderSorter sorter(tileSet);
		std::sort(addOrder.begin(), addOrder.end(), sorter);
	}

	//add all the tiles
	for(int i = 0; i < numTiles; ++i)
	{
		Tile const & tileData = tileSet[addOrder[i]];
		UIBaseObject * dupe = m_sampleTilePage->DuplicateObject();

		if(dupe && dupe->IsA(TUIPage))
		{
			UIPage * tilePage = static_cast<UIPage*>(dupe);
			tilePage->SetSize(scaledParentSize);
			tilePage->Pack();

			setChildIconStyle(tilePage, tileIconName, getTileImageForValue(tileData.getValue()));
			setChildIconStyle(tilePage, backgroundIconName, getTileBackgroundImage());
			setChildIconStyle(tilePage, highlightIconName, getTileHighlightImage());

			{
				std::string tileText(fs.sprintf("tile %d\n[%d]", tileData.getId(), tileData.getValue()));
				tilePage->SetName(tileText);
			}

			{
				TilePosition const & tileDataPos = tileData.getPosition();
				int x = ((tileDataPos.column + 1) * tileSize.x) / 2;
				int y = ((tileDataPos.row + 1) * tileSize.y) / 2;

				UIPoint layerOffset = scaledTileLayerOffset * static_cast<UIScalar>(tileDataPos.layer);

				UIPoint tilePageLocation(x, y);
				tilePageLocation -= layerOffset;
				tilePageLocation -= scaledTileParentOffset;
				tilePageLocation += boardOffset;

				tilePage->SetLocation(tilePageLocation, true);
			}

			tilePage->SetPropertyInteger(tileIdProperty, tileData.getId());

			tilePage->SetEnabled(true);
			tilePage->SetVisible(!tileData.getIsRemoved());

			tilePage->Attach(0);
			m_mahjongPage->AddChild(tilePage);

			registerMediatorObject(*tilePage, true);

			m_tileIdToTileMap[tileData.getId()] = tilePage;
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiMahjong::refreshTileButtonState(TileId id)
{
	if(!ms_mahjongBoard.isTileIdValid(id))
	{
		return;
	}

	Tile const & tile = ms_mahjongBoard.getTile(id);

	bool const isTileSelected = (id == m_selectedTileId);
	bool const highlightOn = isTileSelected || (tile.getValue() == m_highlightTileValue);
	bool const tileVisible = (!ms_mahjongBoard.isTileRemoved(id));

	UIString palSelected(Unicode::narrowToWide("contrast1"));
	m_sampleTilePage->GetProperty(palSelectedColorProperty, palSelected);

	UIString palMatching(Unicode::narrowToWide("contrast3a"));
	m_sampleTilePage->GetProperty(palMatchingColorProperty, palMatching);

	std::map<int,UIPage*>::const_iterator itr = m_tileIdToTileMap.find(id);
	if(itr != m_tileIdToTileMap.end())
	{
		UIPage * const & page = (*itr).second;
		page->SetVisible(tileVisible);

		UIBaseObject * highlight = page->GetChild(highlightIconName);
		if(highlight && highlight->IsA(TUIWidget))
		{
			UIWidget* highlightWidget = static_cast<UIWidget*>(highlight);
			highlightWidget->SetVisible(highlightOn);

			if(highlightOn)
			{
				highlightWidget->SetProperty(palColorProperty, isTileSelected ? palSelected : palMatching);
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiMahjong::refreshTilesByValue(int tileValue)
{
	std::set<TileId> matchingTiles;
	if(ms_mahjongBoard.getTileIdsForValue(tileValue, matchingTiles))
	{
		for(std::set<TileId>::const_iterator i = matchingTiles.begin(); i != matchingTiles.end(); ++i)
		{
			TileId const & id = *i;
			refreshTileButtonState(id);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiMahjong::updateTilesLeftCount()
{
	if(m_tilesLeftText)
	{
		m_tilesLeftText->SetText(Unicode::intToWide(ms_mahjongBoard.getNumTilesLeft()));
	}
}

//----------------------------------------------------------------------

void SwgCuiMahjong::updateScoreText()
{
	if(m_scoreText)
	{
		m_scoreText->SetText(Unicode::intToWide(ms_mahjongBoard.getScore()));
	}
}

//----------------------------------------------------------------------

void SwgCuiMahjong::update(float deltaTimeSecs)
{
	SwgMahjongScoreKeeper * scoreKeeper = static_cast<SwgMahjongScoreKeeper*>(ms_mahjongBoard.getScoreKeeper());

	if(scoreKeeper)
	{
		scoreKeeper->update(deltaTimeSecs);

		int chainMultipiler = scoreKeeper->getChain();
		float chainTimerRatio = scoreKeeper->getChainTimerRemainingRatio();

		m_chainText->SetText(Unicode::intToWide(chainMultipiler));

		m_chainTimerBar->SetVisible(chainMultipiler > 1);

		UISize barMaxSize = m_chainTimerBar->GetMaximumSize();
		UISize barMinSize = m_chainTimerBar->GetMinimumSize();
		UISize barSize = (barMaxSize - barMinSize) * chainTimerRatio + barMinSize;
		
		m_chainTimerBar->SetSize(barSize);
	}
}

//----------------------------------------------------------------------

bool SwgCuiMahjong::OnMessage(UIWidget *context, const UIMessage & msg )
{
	UNREF(msg);

	if(waitingForDialogue())
	{
		return true;
	}

	if(context->GetParent() == m_mahjongPage)
	{
		if(msg.Type == UIMessage::LeftMouseUp)
		{
			TileId id = -1;

			if(context && context->GetPropertyInteger(tileIdProperty, id))
			{
				handleTileClicked(id);
			}
		}

		else if(msg.Type == UIMessage::RightMouseDown)
		{
			TileId id = -1;

			if(context && context->GetPropertyInteger(tileIdProperty, id))
			{
				setHighlightTileValueById(id);
			}
		}
	}
	else if (context == m_mahjongPage)
	{
		if(msg.Type == UIMessage::MiddleMouseUp)
		{
			setSelectedTile(-1);
		}
		else if(msg.Type == UIMessage::RightMouseUp)
		{
			setHighlightTileValueById(-1);
		}
	}

	return true;
}

//----------------------------------------------------------------------

void SwgCuiMahjong::OnButtonPressed(UIWidget *context)
{
	if(context == m_closeButton)
	{
		showExitWindow();
	}

	else if(waitingForDialogue())
	{
		if (context == m_newGameOkButton)
		{
			std::string selectedTileset;
			m_newGameTilesetCombo->GetSelectedIndexName(selectedTileset);

			std::string selectedLayout;
			m_newGameLayoutCombo->GetSelectedIndexName(selectedLayout);

			if(doNewGame(selectedLayout, selectedTileset))
			{
				hideNewGameWindow();
			}
		}

		else if (context == m_newGameCancelButton)
		{
			hideNewGameWindow();
			checkGameOver(false);
		}

		else if (context == m_gameOverOkButton)
		{
			hideGameOverWindow();
			showNewGameWindow();
		}

		else if (context == m_gameOverUndoButton)
		{
			hideGameOverWindow();
			doUndo();
		}

		else if (context == m_gameOverShuffleButton)
		{
			hideGameOverWindow();
			doShuffle();
		}

		else if (context == m_helpOkButton)
		{
			hideHelpWindow();
		}

		else if (context == m_exitYesButton)
		{
			close();
		}

		else if (context == m_exitNoButton)
		{
			hideExitWindow();

			if(m_currentState == GS_inProgress)
			{
				checkGameOver(false);
			}
			else if (m_currentState != GS_ready)
			{
				showNewGameWindow();
			}
		}
	}

	else if(context == m_newGameButton)
	{
		showNewGameWindow();
	}

	else if(context == m_shuffleButton)
	{
		doShuffle();
	}

	else if (context == m_undoButton)
	{
		doUndo();
	}

	else if (context == m_helpButton)
	{
		showHelpWindow();
	}

}

//----------------------------------------------------------------------

void SwgCuiMahjong::handleTileClicked(int id)
{
	if(ms_mahjongBoard.isTileIdValid(id))
	{
		MahjongGame::Move move(id, m_selectedTileId);

		if(ms_mahjongBoard.isTileIdValid(m_selectedTileId) && ms_mahjongBoard.doMove(move))
		{
			handleMoveExecuted(move.tile1, move.tile2);
		}
		else
		{
			setSelectedTile(id);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiMahjong::setSelectedTile(int id)
{
	TileId const oldSelectedId = m_selectedTileId;

	if(!ms_mahjongBoard.isTileBlocked(id))
	{
		m_selectedTileId = id;
	}
	else
	{
		if(ms_mahjongBoard.isTileIdValid(id))
		{
			playSoundHelper(m_tilesetName, SwgCuiMahjongDataTable::MTS_tileCannotSelect);
		}
		m_selectedTileId = -1;
	}

	if(oldSelectedId != m_selectedTileId)
	{
		if( m_selectedTileId != -1)
		{
			playSoundHelper(m_tilesetName, SwgCuiMahjongDataTable::MTS_tileSelected);
		}

		refreshTileButtonState(oldSelectedId);
		refreshTileButtonState(m_selectedTileId);
	}
}

//----------------------------------------------------------------------

void SwgCuiMahjong::setHighlightTileValueById(int id)
{
	TileValue oldHighlightValue = m_highlightTileValue;

	if(ms_mahjongBoard.isTileIdValid(id))
	{
		m_highlightTileValue = ms_mahjongBoard.getTile(id).getValue();
	}
	else
	{
		m_highlightTileValue = -1;
	}

	if(oldHighlightValue != m_highlightTileValue)
	{
		refreshTilesByValue(oldHighlightValue);
		refreshTilesByValue(m_highlightTileValue);
	}
}

//----------------------------------------------------------------------

bool SwgCuiMahjong::checkGameOver(bool playSound)
{
	if(ms_mahjongBoard.isGameWon())
	{
		if(playSound)
		{
			playSoundHelper(m_tilesetName, SwgCuiMahjongDataTable::MTS_gameWon);
		}

		showGameOverWindow();

		return true;
	}
	else if(ms_mahjongBoard.isGameLost())
	{
		if(playSound)
		{
			playSoundHelper(m_tilesetName, SwgCuiMahjongDataTable::MTS_gameLost);
		}		
		
		showGameOverWindow();

		return true;
	}
	return false;
}

//----------------------------------------------------------------------

void SwgCuiMahjong::handleMoveExecuted(int tile1, int tile2)
{
	m_currentState = GS_inProgress;

	setSelectedTile(-1);

	refreshTileButtonState(tile1);
	refreshTileButtonState(tile2);
	updateTilesLeftCount();
	updateScoreText();

	if(!checkGameOver())
	{
		playSoundHelper(m_tilesetName, SwgCuiMahjongDataTable::MTS_tileRemoved);
	}
}

//----------------------------------------------------------------------

void SwgCuiMahjong::doShuffle()
{
	if(!ms_mahjongBoard.isGameWon())
	{
		clearTiles();
		ms_mahjongBoard.shuffle();
		populateTiles();
		playSoundHelper(m_tilesetName, SwgCuiMahjongDataTable::MTS_shuffle);

		checkGameOver();
	}
}

//----------------------------------------------------------------------

void SwgCuiMahjong::doUndo()
{
	setSelectedTile(-1);
	MahjongGame::Move undoneMove;
	if(ms_mahjongBoard.undo(undoneMove))
	{
		refreshTileButtonState(undoneMove.tile1);
		refreshTileButtonState(undoneMove.tile2);
		updateTilesLeftCount();
		updateScoreText();
		playSoundHelper(m_tilesetName, SwgCuiMahjongDataTable::MTS_undo);
	}
	else
	{
		UIManager::gUIManager().PlaySoundGenericNegative();
	}
}

//----------------------------------------------------------------------

bool SwgCuiMahjong::doNewGame(std::string const & layoutName, std::string const & tilesetName)
{
	submitScore();

	m_tilesetName = tilesetName;
	m_layoutName = layoutName;

	if(!SwgCuiMahjongDataTable::getTilesetImages(m_tilesetName, ms_tileset))
	{
		return false;
	}

	std::list<TilePosition> layoutTiles;
	if(!SwgCuiMahjongDataTable::getLayoutTiles(m_layoutName, layoutTiles))
	{
		return false;
	}

	m_currentState = GS_ready;

	clearTiles();
	ms_mahjongBoard.setLayout(layoutTiles);
	ms_mahjongBoard.resetBoard();
	ms_mahjongBoard.randomizeTiles();
	populateTiles();

	playSoundHelper(m_tilesetName, SwgCuiMahjongDataTable::MTS_gameStart);
	return true;
}

//----------------------------------------------------------------------

void SwgCuiMahjong::showNewGameWindow()
{
	hideAllDialogueWindows();

	{
		UIDataSource * const layoutDataSource = m_newGameLayoutCombo->GetDataSource();
		layoutDataSource->Clear();

		std::list<std::string> layoutNames;
		SwgCuiMahjongDataTable::getLayoutNames(layoutNames);
		for(std::list<std::string>::const_iterator i = layoutNames.begin(); i != layoutNames.end(); ++i)
		{
			UIData * const data = new UIData();
			data->SetName(*i);
			data->SetPropertyNarrow (UIList::DataProperties::TEXT, *i);
			layoutDataSource->AddChild(data);
		}
	}

	{
		UIDataSource * const tilesetDataSource = m_newGameTilesetCombo->GetDataSource();
		tilesetDataSource->Clear();

		std::list<std::string> tilesetNames;
		SwgCuiMahjongDataTable::getTilesetNames(tilesetNames);
		for(std::list<std::string>::const_iterator i = tilesetNames.begin(); i != tilesetNames.end(); ++i)
		{
			UIData * const data = new UIData();
			data->SetName(*i);
			data->SetPropertyNarrow (UIList::DataProperties::TEXT, *i);
			tilesetDataSource->AddChild(data);
		}
	}
	
	m_newGameLayoutCombo->SetSelectedIndex(0);
	m_newGameTilesetCombo->SetSelectedIndex(0);

	m_newGameCancelButton->SetVisible(m_currentState != GS_notReady);

	m_waitingOnDialogue = true;
	m_newGamePage->SetVisible(true);
}

//----------------------------------------------------------------------

void SwgCuiMahjong::hideNewGameWindow()
{
	m_waitingOnDialogue = false;
	m_newGamePage->SetVisible(false);
}

//----------------------------------------------------------------------

void SwgCuiMahjong::showGameOverWindow()
{
	hideAllDialogueWindows();

	UIString text;
	bool shuffleVisible = false;
	bool undoVisible = ms_mahjongBoard.canUndo();

	if(ms_mahjongBoard.isGameWon())
	{
		CuiStringVariablesManager::process (victoryText, Unicode::intToWide(ms_mahjongBoard.getScore()), Unicode::String (), Unicode::String (), text);
		undoVisible = false;
	}
	else if(ms_mahjongBoard.isGameLost())
	{
		CuiStringVariablesManager::process (lossText, Unicode::intToWide(ms_mahjongBoard.getScore()), Unicode::String (), Unicode::String (), text);
		shuffleVisible = true;
	}
	else
	{
		CuiStringVariablesManager::process (gameOverText, Unicode::intToWide(ms_mahjongBoard.getScore()), Unicode::String (), Unicode::String (), text);
	}

	m_gameOverText->SetText(text);

	m_waitingOnDialogue = true;
	m_gameOverPage->SetVisible(true);
	m_gameOverUndoButton->SetVisible(undoVisible);
	m_gameOverShuffleButton->SetVisible(shuffleVisible);
}

//----------------------------------------------------------------------

void SwgCuiMahjong::hideGameOverWindow()
{
	m_waitingOnDialogue = false;
	m_gameOverPage->SetVisible(false);
}

//----------------------------------------------------------------------

void SwgCuiMahjong::showHelpWindow()
{
	hideAllDialogueWindows();

	m_waitingOnDialogue = true;
	m_helpPage->SetVisible(true);
}

//----------------------------------------------------------------------

void SwgCuiMahjong::hideHelpWindow()
{
	m_waitingOnDialogue = false;
	m_helpPage->SetVisible(false);
}

//----------------------------------------------------------------------

void SwgCuiMahjong::showExitWindow()
{
	hideAllDialogueWindows();
	m_waitingOnDialogue = true;
	m_exitPage->SetVisible(true);
}

//----------------------------------------------------------------------

void SwgCuiMahjong::hideExitWindow()
{
	m_waitingOnDialogue = false;
	m_exitPage->SetVisible(false);
}

//----------------------------------------------------------------------

void SwgCuiMahjong::hideAllDialogueWindows()
{
	hideNewGameWindow();
	hideGameOverWindow();
	hideHelpWindow();
	hideExitWindow();
	m_waitingOnDialogue = false;
}

//----------------------------------------------------------------------

bool SwgCuiMahjong::waitingForDialogue()
{
	return m_waitingOnDialogue;
}

//----------------------------------------------------------------------

void SwgCuiMahjong::clearBoard()
{
	clearTiles();
	ms_mahjongBoard.setLayout(std::list<TilePosition>());
	ms_mahjongBoard.resetBoard();
	populateTiles();

	m_currentState = GS_notReady;
}

//----------------------------------------------------------------------

void SwgCuiMahjong::submitScore()
{
	if(m_currentState != GS_notReady)
	{
		int score = ms_mahjongBoard.getScore();

		if(score > 0)
		{
			ValueDictionary data;
			CuiMinigameManager::addStandardScoreDataElements(data, cms_gameName, m_tableOid, score);

			data.insert(keyLayout, ValueTypeString(m_layoutName));

			CuiMinigameManager::submitMinigameResult(data);
		}

		m_currentState = GS_notReady;
	}
}

//----------------------------------------------------------------------

bool SwgCuiMahjong::readData(ValueDictionary const & data)
{
	submitScore();

	clearBoard();

	if(!CuiMinigameManager::getNetworkIdFromData(data, CuiMinigameManager::keyTableOid, m_tableOid))
	{
		m_tableOid = NetworkId::cms_invalid;
	}

	showNewGameWindow();

	return true;
}

//----------------------------------------------------------------------
