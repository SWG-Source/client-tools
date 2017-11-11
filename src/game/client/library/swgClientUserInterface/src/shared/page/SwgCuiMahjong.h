
#ifndef INCLUDED_SwgCuiMahjong_H
#define INCLUDED_SwgCuiMahjong_H


#include "clientUserInterface/CuiMinigameManager.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "UIEventCallback.h"

#include <map>

class UIPage;
class UIWidget;
class UIButton;
class UIText;
class UIComboBox;

class SwgCuiMahjong : public CuiMinigameBase, public UIEventCallback
{
public:
	static std::string const cms_gameName;
	static std::string const keyLayout;

	explicit SwgCuiMahjong (UIPage & page);

	virtual void performActivate   ();
	virtual void performDeactivate ();
	virtual bool close ();

	virtual void update(float deltaTimeSecs);

	virtual bool OnMessage(UIWidget *context, const UIMessage & msg );
	virtual void OnButtonPressed(UIWidget *context);

	virtual bool readData(ValueDictionary const & data);

private:
	virtual ~SwgCuiMahjong();
	SwgCuiMahjong();
	SwgCuiMahjong(SwgCuiMahjong const & rhs);
	SwgCuiMahjong& operator= (SwgCuiMahjong const & rhs);

	UIPage * m_mahjongPage;
	UIButton * m_closeButton;
	UIButton * m_newGameButton;
	UIButton * m_undoButton;
	UIButton * m_shuffleButton;
	UIButton * m_helpButton;
	UIText * m_tilesLeftText;
	UIText * m_scoreText;
	UIText * m_chainText;
	UIPage * m_chainTimerBar;

	UIComboBox * m_newGameLayoutCombo;
	UIComboBox * m_newGameTilesetCombo;
	UIButton * m_newGameOkButton;
	UIPage * m_newGamePage;

	UIPage * m_gameOverPage;
	UIButton * m_gameOverOkButton;
	UIButton * m_newGameCancelButton;
	UIButton * m_gameOverUndoButton;
	UIButton * m_gameOverShuffleButton;
	UIText * m_gameOverText;

	UIPage * m_helpPage;
	UIButton * m_helpOkButton;

	UIPage * m_exitPage;
	UIButton * m_exitYesButton;
	UIButton * m_exitNoButton;

	UIPage * m_sampleTilePage;

	int m_selectedTileId;
	int m_highlightTileValue;
	bool m_waitingOnDialogue;

	std::string m_tilesetName;
	std::string m_layoutName;

	NetworkId m_tableOid;

	std::map<int, UIPage*> m_tileIdToTileMap;

	enum GameState
	{
		GS_notReady,
		GS_ready,
		GS_inProgress,
	};

	GameState m_currentState;

	void clearBoard();
	void clearTiles();
	void populateTiles();
	void refreshTileButtonState(int id);
	void refreshTilesByValue(int tileValue);
	void updateTilesLeftCount();
	void updateScoreText();

	void handleTileClicked(int tileId);
	void setSelectedTile(int tileId);
	void setHighlightTileValueById(int id);

	bool checkGameOver(bool playSound = true);

	void submitScore();

	void handleMoveExecuted(int tile1, int tile2);
	void doShuffle();
	void doUndo();
	bool doNewGame(std::string const & layoutName, std::string const & tilesetName);

	void showNewGameWindow();
	void hideNewGameWindow();
	void showGameOverWindow();
	void hideGameOverWindow();
	void showHelpWindow();
	void hideHelpWindow();
	void showExitWindow();
	void hideExitWindow();

	void hideAllDialogueWindows();

	bool waitingForDialogue();

};




#endif

