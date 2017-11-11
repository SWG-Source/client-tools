//======================================================================
//
// SwgCuiAvatarSimple.h
// copyright (c) 2005 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAvatarSimple_H
#define INCLUDED_SwgCuiAvatarSimple_H

//======================================================================

#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiMediator.h"
#include "swgClientUserInterface/SwgCuiAvatarCustomizationBase_CustomizationGroup.h"
#include "SwgCuiAvatarCreationHelper.h"
#include "UIEventCallback.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

#include <map>
#include <vector>

class CreatureObject;
class CuiWidget3dObjectListViewer;
class TangibleObject;
class UITextbox;
class UIList;


namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

class SwgCuiAvatarSimple
: public CuiMediator
, public UIEventCallback
{
public:
	typedef Watcher<CreatureObject> CreatureObjectWatcher;
	typedef Watcher<TangibleObject> TangibleObjectWatcher;
	typedef std::vector<CreatureObject> CreatureObjectsVector;
	typedef std::vector<TangibleObjectWatcher> TangibleObjectsVector;
	typedef std::map<std::string, TangibleObjectWatcher> TangibleMap;
	typedef std::vector<std::string> StringVector;

	struct Avatar
	{
		CreatureObjectWatcher m_creature;
		StringVector m_hairStyleTemplates;
		TangibleMap m_hairStyles;
	};

	typedef stdmap<std::string, Avatar>::fwd AvatarMap;

public:

	explicit SwgCuiAvatarSimple(UIPage & page);

	virtual void OnButtonPressed(UIWidget * context);

	virtual void performActivate();
	virtual void performDeactivate();
	virtual void update(float deltaTimeSecs);

	void onRandomNameChanged(const Unicode::String & name);
	void onCreationAborted(bool);
	void onCreationFinished(bool);

	void verifyAndLockName();
	void onVerifyAndLockNameFinished(const SwgCuiAvatarCreationHelper::Messages::VerifyAndLockNameResponse::Payload & payload);

private:
	virtual                 ~SwgCuiAvatarSimple();
	                        SwgCuiAvatarSimple();
	                        SwgCuiAvatarSimple(const SwgCuiAvatarSimple & rhs);
	SwgCuiAvatarSimple &    operator=(const SwgCuiAvatarSimple & rhs);

	enum Direction
	{
		D_Previous,
		D_Next,
		D_Random
	};

	enum Gender
	{
		G_Male,
		G_Female,
		G_numberOfGenders
	};

	struct SpeciesButton;

	void loadAllPlayerModelData();
	void initializeCustomization();
	void prefetchAnotherObject();
	void updateAvatarSelection();
	void randomize();
	void randomizeName();
	void randomizeGender();
	void selectGender(Gender gender);
	void selectSpecies(int const newSelection);
	void selectSpecies(Direction const direction);
	void selectSpecies(SpeciesButton const * speciesButton);
	void selectClothes(Direction const direction);
	void randomizeAppearance();
	void updateSpeciesDescription();

private:
	UIButton * m_buttonBack;
	UIButton * m_buttonNext;
	UIButton * m_buttonRandomName;
	UIButton * m_buttonSpeciesPrev;
	UIButton * m_buttonSpeciesNext;
	UIButton * m_buttonMale;
	UIButton * m_buttonFemale;
	UIButton * m_buttonGenderNext;
	UIButton * m_buttonGenderPrev;
	UIButton * m_buttonClothingPrev;
	UIButton * m_buttonClothingNext;
	UIButton * m_buttonRandomAppearance;
	UIButton * m_buttonRandomAll;
	UITextbox * m_textboxName;
	UITextbox * m_textboxSurname;
	UICheckbox * m_checkNewbieTutorial;
	UIText * m_textMale;
	UIText * m_textFemale;
	UIText * m_speciesSelection;
	UIText * m_genderSelection;
	UIText * m_clothingSelection;
	UIText * m_p2speciesDescription;
	UIText * m_p2speciesName;
	UIPage * m_surnameVisible;
	UIComposite * m_species1_male;
	UIComposite * m_species2_male;
	UIComposite * m_species1_female;
	UIComposite * m_species2_female;

	UIList * m_hiddenAvatarList;

	CuiWidget3dObjectListViewer * m_viewer;

	MessageDispatch::Callback * m_callback;

	StringVector * m_oneNameTemplateVector;

	AvatarMap * m_avatarMap;

	bool m_nameModified;

	Gender m_gender;

	typedef SwgCuiAvatarCustomizationBase::CustomizationGroup CustomizationGroup;
	typedef SwgCuiAvatarCustomizationBase::CustomizationGroupVector CustomizationGroupVector;
	CustomizationGroupVector * m_customizationGroups;
	CustomizationData * m_cdata;

	std::string m_currentProfessionName;

	bool m_morphInProgress;
	bool m_pendingCreationFinished;
	bool m_aborted;
	bool m_generatedFirstName;
	bool m_preloadingFinished;
	bool m_pendingNameVerified;

	float m_secondsUntilNextPrefetch;

	Object * m_boxObject;

	typedef std::vector<SpeciesButton *> SpeciesButtons;
	SpeciesButtons m_speciesButtons;
};

//======================================================================

#endif
