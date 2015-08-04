//======================================================================
//
// SwgCuiIncubator.h
// copyright (c) 2007 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiIncubator_H
#define INCLUDED_SwgCuiIncubator_H

#include "clientUserInterface/CuiMediator.h"
#include "sharedObject/CachedNetworkId.h"
#include "UIEventCallback.h"

class ClientObject;
class CuiWidget3dObjectListViewer;
class Object;
class SwgCuiContainerProviderDefault;
class SwgCuiInventoryContainer;
class SwgCuiInventoryInfo;
class UIImage;
class UIPage;
class UIText;
class UIVolumePage;

namespace MessageDispatch
{
	class Emitter;
	class Callback;
};

class SwgCuiIncubator :
public CuiMediator,
public UIEventCallback
{
public:
	explicit                SwgCuiIncubator              (UIPage & page);
	virtual void            performActivate                  ();
	virtual void            performDeactivate                ();
	virtual void            OnButtonPressed                  (UIWidget *context );
	virtual void            OnCheckboxSet                    (UIWidget *context);
	virtual void            OnCheckboxUnset                  (UIWidget *context);
	virtual bool            OnMessage                        (UIWidget *context, const UIMessage & msg );
	virtual void            onSelectionChanged               (const std::pair<int, ClientObject *> & payload);
	virtual bool            close                            ();
	virtual bool            overrideDoubleClick              (const UIWidget & viewerWidget);
	virtual void            update                           (float deltaTimeSecs);

	// accessors
	void SetSessionNumber(int val);
	void SetTerminalId(const NetworkId& val);
	void SetPowerGaugeValue(int val);
	void SetInitialPointsSurvival(int val);
	void SetInitialPointsBeastialResilience(int val);
	void SetInitialPointsCunning(int val);
	void SetInitialPointsIntelligence(int val);
	void SetInitialPointsAggression(int val);
	void SetInitialPointsHuntersInstinct(int val);
	void SetTemperatureGaugeValue(int val);
	void SetNutrientGaugeValue(int val);
	void SetInitialCreatureColorIndex(int val);
	void SetCreatureTemplateName(const std::string& val);

	void initializeControls();

	static void createObjectTooltipString (const ClientObject & obj, Unicode::String & str);
	
private:
	~SwgCuiIncubator ();
	SwgCuiIncubator (const SwgCuiIncubator &);
	SwgCuiIncubator & operator= (const SwgCuiIncubator &);
	
	enum CheckboxType
	{
		CB_survival = 0,
		CB_beastialResilience,
		CB_cunning,
		CB_intelligence,
		CB_aggression,
		CB_huntersInstinct,

		CB_numCheckboxTypes
	};

	int getDefensiveSkillPointsLeft();
	void setDefensiveSkillPointsLeft(int val);
	int getIntelligenceSkillPointsLeft();
	void setIntelligenceSkillPointsLeft(int val);
	int getOffensiveSkillPointsLeft();
	void setOffensiveSkillPointsLeft(int val);

	void updateButtonStates();

	// checkbox handling
	void handleCheckbox(CheckboxType type,UICheckbox *cb, bool check);
	int getTotalCheckboxFamilyPoints(CheckboxType type);
	int getCheckboxIndex(CheckboxType type, UICheckbox *cb);
	UICheckbox** getCheckboxFamily(CheckboxType type);
	void resetChecksToInitialPoints();
	void distributePointsToSpend();

	// progress bars
	void updateTemperatureBar();
	void updateNutrientBar();

	// viewer
	void updateCreatureColor();

	// slot handling
	CuiWidget3dObjectListViewer* findSlotViewer(const UIWidget * context);
	int findSlotViewerIndex (const UIWidget * context);

	void setSlot(int slot, Object *obj);
	void clearSlot(int slot);

private:
	class ContainerFilter;
	class ProviderFilter;
	friend ProviderFilter;

	MessageDispatch::Callback *       m_callback;


	// start values from initial create message
	int m_sessionNumber;
	CachedNetworkId m_terminalId;
	int m_powerGaugeValue;
	int m_initialPointsSurvival;
	int m_initialPointsBeastialResilience;
	int m_initialPointsCunning;
	int m_initialPointsIntelligence;
	int m_initialPointsAggression;
	int m_initialPointsHuntersInstinct;
	int m_temperatureGaugeValue;
	int m_nutrientGaugeValue;
	int m_initialCreatureColorIndex;
	std::string m_creatureTemplateName;
	// end values from initial create message

	UIButton * m_buttonCancel;
	UIButton * m_buttonCommit;
	UIText * m_pointsDefensiveSkills;
	UIText * m_pointsIntelligenceSkills;
	UIText * m_pointsOffensiveSkills;
	UIImage* m_powerBar;
	UIPage* m_heatBar;
	UIPage* m_heatBarBack;
	UIPage* m_nutrientBar;
	UIPage* m_nutrientBarBack;
	UIButton * m_buttonAddHeat;
	UIButton * m_buttonSubtractHeat;
	UIButton * m_buttonAddNutrients;
	UIButton * m_buttonSubtractNutrients;
	CuiWidget3dObjectListViewer * m_viewer;
	ClientObject *m_creatureObject;
	SwgCuiInventoryInfo * m_inventoryInfo;  
	SwgCuiInventoryContainer * m_containerInventory;
	SwgCuiContainerProviderDefault *  m_containerProviderInventory;
	ContainerFilter *                 m_containerFilter;
	ProviderFilter * m_providerFilter;
	UICheckbox * m_checkFilter;
	int m_pointsToSpend;
	int m_newCreatureColorIndex;

	bool m_committed;
	UICheckbox* m_checkboxSurvival[10];
	UICheckbox* m_checkboxBeastialResilience[10];
	UICheckbox* m_checkboxCunning[10];
	UICheckbox* m_checkboxIntelligence[10];
	UICheckbox* m_checkboxAggression[10];
	UICheckbox* m_checkboxHuntersInstinct[10];
	
	CuiWidget3dObjectListViewer* m_slotViewer[4];
};

#endif

