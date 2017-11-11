//======================================================================
//
// SwgCuiQuestBuilder.h
// copyright (c) 2009 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiQuestBuilder_H
#define INCLUDED_SwgCuiQuestBuilder_H

//======================================================================
#include "clientUserInterface/CuiMediator.h"

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/CrcLowerString.h"
#include "sharedGame/CollectionsDataTable.h"

#include "UIEventCallback.h"
#include <map>
//----------------------------------------------------------------------
class CuiMessageBox;
class CuiWidget3dObjectListViewer;
class PlayerObject;

class UIButton;
class UICheckbox;
class UIComboBox;
class UIPage;
class UITable;
class UIText;
class UITextbox;
class UIComposite;
//----------------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}


struct TaskSlotInfo;
class SwgCuiQuestBuilder: public CuiMediator, public UIEventCallback
{
public:
	explicit SwgCuiQuestBuilder(UIPage & page);
	~SwgCuiQuestBuilder();

	void         OnButtonPressed(UIWidget *context );
	void		 OnGenericSelectionChanged (UIWidget* context);
	void		 OnTextboxChanged(UIWidget* context);
	void		 OnCheckboxSet(UIWidget* context );
	void		 OnCheckboxUnset(UIWidget* context );

	void		 OnCollectionsChanged(PlayerObject const & player);

	bool         OnMessage(UIWidget *context, const UIMessage & msg );

protected:
	virtual void performActivate();
	virtual void performDeactivate();

private:
	//disabled
	SwgCuiQuestBuilder(const SwgCuiQuestBuilder & rhs);
	SwgCuiQuestBuilder & operator= (const SwgCuiQuestBuilder & rhs);

	struct TaskSlotInfo
	{
		std::string	   slotName;
		CrcLowerString slotCrc;
		int			   categoryFlags;

		int			   quantity;
		int			   recipeRequirement;
	};

	void		 createSlotMap();
	void		 populateTasksTable(const char * stringNameFilter = NULL, const char * secondNameFilter = NULL, int currentPage = 0);
	void         updateTableTaskCount(const char * taskDisplayName, int newQuantity, int recipeCount);
	void		 addTask();

	void		 loadTasksFromRecipeData();

	void		 refreshCollectionData();

	// Task creator functions.
	UIPage*		 addKillTask(TaskSlotInfo const * taskInfo, std::string const taskData = "");
	UIPage*		 addLocationTask(TaskSlotInfo const * taskInfo, std::string const taskData = "");
	UIPage*		 addLootTask(TaskSlotInfo const * taskInfo, std::string const taskData = "");
	UIPage*		 addPvpKillTask(TaskSlotInfo const * taskInfo, std::string const taskData = "");
	UIPage*		 addPerformTask(TaskSlotInfo const * taskInfo, std::string const taskData = "");
	UIPage*		 addCommTask(TaskSlotInfo const * taskInfo, std::string const taskData = "");
	UIPage*		 addCraftTask(TaskSlotInfo const * taskInfo, std::string const taskData = "");
	UIPage*		 addPvpDestroyTask(TaskSlotInfo const * taskInfo, std::string const taskData = "");
	UIPage*		 addKillLootTask(TaskSlotInfo const * taskInfo, std::string const taskData = "");
	UIPage*		 addSpawnTask(TaskSlotInfo const * taskInfo, std::string const taskData = "");


	// Task parameter population functions
	bool		 getKillTaskInfo(UIPage * page, std::vector<std::string> & parameters);
	bool		 getLocationTaskInfo(UIPage * page, std::vector<std::string> & parameters);
	bool		 getLootTaskInfo(UIPage * page, std::vector<std::string> & parameters);
	bool         getPvpKillTaskInfo(UIPage * page, std::vector<std::string> & parameters);
	bool         getPerformTaskInfo(UIPage * page, std::vector<std::string> & parameters);
	bool         getCommTaskInfo(UIPage * page, std::vector<std::string> & parameters);
	bool         getCraftTaskInfo(UIPage * page, std::vector<std::string> & parameters);
	bool		 getPvpDestroyTaskInfo(UIPage * page, std::vector<std::string> & parameters);
	bool		 getKillLootTaskInfo(UIPage * page, std::vector<std::string> & parameters);
	bool		 getSpawnTaskInfo(UIPage *  page, std::vector<std::string> & parameters);

	void		 requestCreateQuest();
	void		 clearTaskRecipe(bool doCleanup = true);

	void		 updateQuestInfo();
	void		 captureLocation(UIPage * page, int & taskFlags);

	void		 OnConfirmClearQuest(const CuiMessageBox & box);
	void		 OnConfirmClearRewards(const CuiMessageBox & box);
	void		 OnConfirmChangeDraft(const CuiMessageBox & box);


	static const UILowerString KillSpeciesProperty;
	static const UILowerString KillCreatureProperty;
	static const UILowerString TemplateProperty;
	static const UILowerString FilterProperty;


	MessageDispatch::Callback * m_callback;

	UIButton *					m_close;
	UIButton *					m_addTask;
	UIButton *					m_createQuest;
	UIButton *					m_clearRewards;
	UIButton *					m_clearQuest;


	UITable *					m_taskTable;
	UIComboBox *				m_taskCategory;
	UITextbox *					m_taskFilter;

	UIPage *					m_killTask;
	UIPage *					m_locationTask;
	UIPage *					m_lootTask;
	UIPage *					m_pvpKillTask;
	UIPage *					m_performTask;
	UIPage *					m_commTask;
	UIPage *					m_craftTask;
	UIPage *				    m_pvpDestroyTask;
	UIPage *					m_killLootTask;
	UIPage *					m_spawnTask;

	UIComposite *				m_taskComposite;

	UITextbox *					m_questTitle;
	UIText *					m_questDescription;

	UIText *					m_questLevel;
	UIText *					m_questTaskCount;
	UIText *					m_requiredText;
	UITextbox *					m_creditsText;

	UICheckbox *				m_hideTasksCheck;
	UICheckbox *				m_showOnlyRecipeTasksCheck;
	UICheckbox *				m_recipeCheck;
	UICheckbox *				m_overrideCheck;
	UICheckbox *				m_shareCheck;

	UIComboBox *				m_shareCombo;
	UIButton *					m_prevTaskPage;
	UIButton *					m_nextTaskPage;

	CuiWidget3dObjectListViewer* m_lootViewers[4];

	typedef std::map<CrcLowerString, TaskSlotInfo> SlotMap;
	SlotMap						m_slotMap;

	std::vector<std::string>    m_performSongs;
	std::vector<std::string>    m_performDances;


};

//======================================================================

#endif