//======================================================================
//
// SwgCuiAvatarSimple.cpp
// copyright(c) 2002 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiAvatarSimple.h"

#include "clientGame/ClientImageDesignerManager.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/ContainerInterface.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/GameNetwork.h"
#include "clientGame/PlayerCreationManagerClient.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiCharacterHairManager.h"
#include "clientUserInterface/CuiCharacterLoadoutManager.h"
#include "clientUserInterface/CuiLoginManagerClusterInfo.h"
#include "clientUserInterface/CuiLoginManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiPreferences.h"
#include "clientUserInterface/CuiStringIdsServer.h"
#include "clientUserInterface/CuiStringIds.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer_TextOverlay.h"
#include "clientGame/Game.h"
#include "clientUserInterface/CuiTransition.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/PlayerCreationManager.h"
#include "sharedCollision/ExtentList.h"
#include "sharedObject/Container.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/NameErrors.h"
#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "UIButton.h"
#include "UIComposite.h"
#include "UICheckbox.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIList.h"
#include "UITextbox.h"
#include "UIText.h"

#include <algorithm>

//======================================================================

namespace SwgCuiAvatarSimpleNamespace
{
	namespace Properties
	{
		const UILowerString OneNameTemplates = UILowerString("OneNameTemplates");
	}

	std::string const cs_sharedTemplatePrefix = "object/creature/player/shared_";
	std::string const cs_sharedTemplateSuffix = ".iff";
	UILowerString const s_maleProp   = UILowerString("appearanceMale");
	UILowerString const s_femaleProp = UILowerString("appearanceFemale");
	float const cs_secondsUntilNextPrefetch = 10.0f;

	CuiMessageBox * s_verifyMessageBox = 0;
	void verifyMessageBoxClosed(const CuiMessageBox & box);

	int cutString(Unicode::String const & str, Unicode::String & one, Unicode::String & two);
	CreatureObject * createPlayerModel(std::string const & templateName);
	void deleteAvatarMap(SwgCuiAvatarSimple::AvatarMap * & avatarMap);
	TangibleObject * createHair(std::string const & templateName, CustomizationData * m_cdata);
	void randomizeHairColors(TangibleObject * const hair, std::string const & varName, int const value = -1);
	void abortBackToSelection();
	void onMessageBoxClosedConfirmCancel(const CuiMessageBox & box);

	std::string getCodeDataName(std::string const & templateName);

	static std::string const s_jediString("jedi");
}

//======================================================================

void SwgCuiAvatarSimpleNamespace::verifyMessageBoxClosed(const CuiMessageBox & box)
{
	if (s_verifyMessageBox == &box)
	{
		s_verifyMessageBox = 0;
	}
}

//----------------------------------------------------------------------

int SwgCuiAvatarSimpleNamespace::cutString(Unicode::String const & str, Unicode::String & one, Unicode::String & two)
{
	// TODO move over to using Unicode::tokenize
	size_t endpos = 0;
	if (Unicode::getFirstToken(str, 0, endpos, one))
	{
		if (endpos != Unicode::String::npos && Unicode::getFirstToken(str, endpos + 1, endpos, two)) //lint !e650 !e737 //stupid msvc
			return 2;

		return 1;
	}

	return 0;
}

//----------------------------------------------------------------------

CreatureObject * SwgCuiAvatarSimpleNamespace::createPlayerModel(std::string const & templateName)
{
	CreatureObject * const creature = safe_cast<CreatureObject*>(ObjectTemplate::createObject(templateName.c_str()));
	if (creature == 0)
	{
		WARNING(true,("SwgCuiAvatarCreation failed [%s]", templateName.c_str()));
		return 0;
	}

	SharedObjectTemplate const * const tmpl = dynamic_cast<const SharedObjectTemplate *>(creature->getObjectTemplate());
	
	if (tmpl != 0)
	{
		//-- set avatar scale to the midpoint
		creature->setScaleFactor((tmpl->getScaleMax() + tmpl->getScaleMin()) * 0.5f);
	}

	CreatureController * const controller = new CreatureController(creature);
	creature->setController(controller);
	creature->endBaselines();

	Appearance * const appearance = creature->getAppearance();
	if (appearance != 0)
	{
		SkeletalAppearance2 * const skeletalAppearance = appearance->asSkeletalAppearance2();
		if (skeletalAppearance != 0)
		{
			//-- preload all lod assets
			int const count = skeletalAppearance->getDetailLevelCount();
			for (int i = 0; i < count; ++i)
			{
				IGNORE_RETURN(skeletalAppearance->isDetailLevelAvailable(i));
			}
		}
	}

	if (ConfigClientGame::getCharacterCreationLoadoutsEnabled())
	{
		CuiCharacterLoadoutManager::setupLoadout(*creature);
		CuiCharacterHairManager::setupDefaultHair(*creature);
	}

	return creature;
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimpleNamespace::deleteAvatarMap(SwgCuiAvatarSimple::AvatarMap * & avatarMap)
{
	{
		CreatureObject const * const player = SwgCuiAvatarCreationHelper::getCreature();

		SwgCuiAvatarSimple::AvatarMap::iterator ii = avatarMap->begin();
		SwgCuiAvatarSimple::AvatarMap::iterator iiEnd = avatarMap->end();

		for (; ii != iiEnd; ++ii)
		{
			SwgCuiAvatarSimple::Avatar & avatar = ii->second;

			{
				CreatureObject * const toDelete = avatar.m_creature;

				if ((toDelete != 0) && (toDelete != player))
				{
					delete toDelete;
				}
			}

			{
				SwgCuiAvatarSimple::TangibleMap::iterator jj = avatar.m_hairStyles.begin();
				SwgCuiAvatarSimple::TangibleMap::iterator jjEnd = avatar.m_hairStyles.end();

				for (; jj != jjEnd; ++jj)
				{
					TangibleObject * const toDelete = jj->second;
					delete toDelete;
				}
			}
		}
	}

	avatarMap->clear();
}

//----------------------------------------------------------------------

TangibleObject * SwgCuiAvatarSimpleNamespace::createHair(std::string const & templateName, CustomizationData * m_cdata)
{
	TangibleObject * const hair = safe_cast<TangibleObject *>(ObjectTemplate::createObject(templateName.c_str()));

	if (hair != 0)
	{
		hair->setNetworkId(ClientImageDesignerManager::getNextClientNetworkId());
		hair->endBaselines();

		CustomizationData * const data = hair->fetchCustomizationData();
		if (data != 0)
		{
			data->mountRemoteCustomizationData(*m_cdata, CustomizationManager::cms_shared_owner, CustomizationManager::cms_shared_owner_no_slash);
			data->release();
		}
	}
	else
	{
		WARNING(true,("Invalid hair object template '%s'.", templateName.c_str()));
	}

	return hair;
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimpleNamespace::randomizeHairColors(TangibleObject * const hair, std::string const & varName, int const value)
{
	if (varName.empty())
	{
		return;
	}

	NOT_NULL(hair);

	int v = value;

	CustomizationData * const cdata = hair->fetchCustomizationData();
	if (cdata != 0)
	{
		CustomizationManager::PathType type = CustomizationManager::PT_none;
		PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(CustomizationManager::findVariable(*cdata, varName, type));

		if (var != 0)
		{
			PaletteArgb const * const palette = var->fetchPalette();

			if (palette != 0)
			{
				SharedImageDesignerManager::PaletteValues const paletteValues = SharedImageDesignerManager::getPaletteValuesForPaletteAndLevel(palette, SharedImageDesignerManager::IDPL_NOT_A_DESIGNER);
				int range_begin = 0, range_end = 0;

				var->getRange(range_begin, range_end);

				if (paletteValues.maxIndex < range_end)
				{
					range_end = paletteValues.maxIndex;
				}

				if (v < 0)
				{
					v = Random::random(range_begin, range_end - 1);
				}
				var->setValue(v);
				palette->release();
			}
		}
		//-- Release local reference.
		cdata->release();
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimpleNamespace::abortBackToSelection()
{
	GameNetwork::setAcceptSceneCommand(false);
	GameNetwork::disconnectConnectionServer();
	SwgCuiAvatarCreationHelper::purgePool();
	CuiTransition::startTransition(CuiMediatorTypes::AvatarSimple, CuiMediatorTypes::AvatarSelection);
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimpleNamespace::onMessageBoxClosedConfirmCancel(const CuiMessageBox & box)
{
	if (box.completedAffirmative ())
	{
		abortBackToSelection();
	}
}

//----------------------------------------------------------------------

std::string SwgCuiAvatarSimpleNamespace::getCodeDataName(std::string const & templateName)
{
	std::string::size_type begin = templateName.rfind("shared_") + strlen("shared") + 1;
	std::string::size_type end = templateName.rfind(".iff");

	//WARNING(true, ("%d %d", begin, end));

	std::string codeDataName("species_");
	codeDataName += templateName.substr(begin, end - begin);

	//WARNING(true, ("%s %s", templateName.c_str(), codeDataName.c_str()));

	return codeDataName;
}

//======================================================================

using namespace SwgCuiAvatarSimpleNamespace;

//======================================================================

struct SwgCuiAvatarSimple::SpeciesButton
{
	int m_index;
	std::string m_maleCodeDataName;
	std::string m_femaleCodeDataName;
	UIButton * m_maleButton;
	UIButton * m_femaleButton;

	SpeciesButton();
	~SpeciesButton();
};

//----------------------------------------------------------------------

SwgCuiAvatarSimple::SpeciesButton::SpeciesButton()
: m_index(0)
, m_maleCodeDataName()
, m_femaleCodeDataName()
, m_maleButton(0)
, m_femaleButton(0)
{
}

//----------------------------------------------------------------------

SwgCuiAvatarSimple::SpeciesButton::~SpeciesButton()
{
	m_maleButton = 0;
	m_femaleButton = 0;
}

//======================================================================

SwgCuiAvatarSimple::SwgCuiAvatarSimple(UIPage & page)
: CuiMediator("SwgCuiAvatarSimple", page)
, UIEventCallback()
, m_buttonBack(0)
, m_buttonNext(0)
, m_buttonRandomName(0)
, m_buttonSpeciesPrev(0)
, m_buttonSpeciesNext(0)
, m_buttonMale(0)
, m_buttonFemale(0)
, m_buttonGenderNext(0)
, m_buttonGenderPrev(0)
, m_buttonClothingNext(0)
, m_buttonClothingPrev(0)
, m_buttonRandomAppearance(0)
, m_buttonRandomAll(0)
, m_textboxName(0)
, m_textboxSurname(0)
, m_checkNewbieTutorial(0)
, m_textMale(0)
, m_textFemale(0)
, m_speciesSelection(0)
, m_genderSelection(0)
, m_clothingSelection(0)
, m_p2speciesDescription(0)
, m_p2speciesName(0)
, m_surnameVisible(0)
, m_species1_male(0)
, m_species2_male(0)
, m_species1_female(0)
, m_species2_female(0)
, m_hiddenAvatarList(0)
, m_viewer(0)
, m_callback(new MessageDispatch::Callback)
, m_oneNameTemplateVector(new StringVector)
, m_avatarMap(new AvatarMap)
, m_nameModified(false)
, m_gender(G_Male)
, m_customizationGroups(new CustomizationGroupVector)
, m_cdata(0)
, m_morphInProgress(false)
, m_pendingCreationFinished(false)
, m_aborted(false)
, m_generatedFirstName(false)
, m_preloadingFinished(false)
, m_pendingNameVerified(false)
, m_secondsUntilNextPrefetch(0.0f)
, m_boxObject(0)
, m_speciesButtons()
{
	//----------------------------------------------------------------------
	//-- find the template names that only have one name(wookiees)

	UIData const * const codeData = NON_NULL(getCodeData());

	if (codeData != 0)
	{
		std::string string;
		if (codeData->GetPropertyNarrow(Properties::OneNameTemplates, string))
		{
			size_t endpos = 0;
			std::string token;
			while(Unicode::getFirstToken(string, endpos, endpos, token))
			{
				std::string const file(cs_sharedTemplatePrefix + token + cs_sharedTemplateSuffix);
				m_oneNameTemplateVector->push_back(file);

				if (endpos == Unicode::String::npos)
				{
					break;
				}
				++endpos;
			}
		}
	}

	std::sort(m_oneNameTemplateVector->begin(), m_oneNameTemplateVector->end());

	getCodeDataObject(TUIButton, m_buttonBack, "buttonBack");
	getCodeDataObject(TUIButton, m_buttonNext, "buttonNext");
	getCodeDataObject(TUIButton, m_buttonRandomName, "buttonRandomName");
	getCodeDataObject(TUIButton, m_buttonSpeciesPrev, "buttonSpeciesPrev");
	getCodeDataObject(TUIButton, m_buttonSpeciesNext, "buttonSpeciesNext");
	getCodeDataObject(TUIButton, m_buttonMale, "buttonMale");
	getCodeDataObject(TUIButton, m_buttonFemale, "buttonFemale");
	getCodeDataObject(TUIButton, m_buttonClothingPrev, "buttonClothingPrev");
	getCodeDataObject(TUIButton, m_buttonClothingNext, "buttonClothingNext");
	getCodeDataObject(TUIButton, m_buttonGenderNext, "buttonGenderNext");
	getCodeDataObject(TUIButton, m_buttonGenderPrev, "buttonGenderPrev");
	getCodeDataObject(TUIButton, m_buttonRandomAppearance, "buttonRandomAppearance");
	getCodeDataObject(TUIButton, m_buttonRandomAll, "buttonRandomAll");
	getCodeDataObject(TUITextbox, m_textboxName, "textboxName");
	getCodeDataObject(TUITextbox, m_textboxSurname, "textboxSurname");
	getCodeDataObject(TUICheckbox, m_checkNewbieTutorial, "checkNewbieTutorial");
	getCodeDataObject(TUIText, m_textMale, "textMale");
	getCodeDataObject(TUIText, m_textFemale, "textFemale");
	getCodeDataObject(TUIText, m_speciesSelection, "speciesSelection");
	getCodeDataObject(TUIText, m_genderSelection, "genderSelection");
	getCodeDataObject(TUIText, m_clothingSelection, "clothingSelection");
	getCodeDataObject(TUIText, m_p2speciesDescription, "p2speciesDescription");
	getCodeDataObject(TUIText, m_p2speciesName, "p2speciesName");
	getCodeDataObject(TUIPage, m_surnameVisible, "surnameVisible");
	getCodeDataObject(TUIList, m_hiddenAvatarList, "hiddenAvatarList");
	getCodeDataObject(TUIComposite, m_species1_male, "species1_male");
	getCodeDataObject(TUIComposite, m_species2_male, "species2_male");
	getCodeDataObject(TUIComposite, m_species1_female, "species1_female");
	getCodeDataObject(TUIComposite, m_species2_female, "species2_female");

	{
		UIWidget * widget = 0;
		getCodeDataObject(TUIWidget, widget, "viewer");
		m_viewer = NON_NULL(dynamic_cast<CuiWidget3dObjectListViewer *>(widget));
	}

	registerMediatorObject(*m_buttonBack, true);
	registerMediatorObject(*m_buttonNext, true);
	registerMediatorObject(*m_buttonRandomName, true);
	registerMediatorObject(*m_buttonSpeciesPrev, true);
	registerMediatorObject(*m_buttonSpeciesNext, true);
	registerMediatorObject(*m_buttonMale, true);
	registerMediatorObject(*m_buttonFemale, true);
	registerMediatorObject(*m_buttonClothingNext, true);
	registerMediatorObject(*m_buttonClothingPrev, true);
	registerMediatorObject(*m_buttonGenderNext, true);
	registerMediatorObject(*m_buttonGenderPrev, true);
	registerMediatorObject(*m_buttonRandomAppearance, true);
	registerMediatorObject(*m_buttonRandomAll, true);
	registerMediatorObject(*m_textboxName, true);
	registerMediatorObject(*m_textboxSurname, true);
	registerMediatorObject(*m_checkNewbieTutorial, true);
	registerMediatorObject(*m_textMale, true);
	registerMediatorObject(*m_textFemale, true);
	registerMediatorObject(*m_speciesSelection, true);
	registerMediatorObject(*m_genderSelection, true);
	registerMediatorObject(*m_clothingSelection, true);
	registerMediatorObject(*m_p2speciesDescription, true);
	registerMediatorObject(*m_p2speciesName, true);
	registerMediatorObject(*m_surnameVisible, true);
	registerMediatorObject(*m_hiddenAvatarList, true);
	registerMediatorObject(*m_species1_male, true);
	registerMediatorObject(*m_species2_male, true);
	registerMediatorObject(*m_species1_female, true);
	registerMediatorObject(*m_species2_female, true);

	m_boxObject = new Object();
	Appearance * const appearance = new Appearance(0);
	AxialBox const box(Vector(-0.2f, -0.2f, -0.2f), Vector(0.2f, 2.0f, 0.2f));// + Vector(0.0f, 1.0f, 0.0f));
	BoxExtent * const extent = new BoxExtent(box);

	appearance->setExtent(extent);
	m_boxObject->setAppearance(appearance);

	int const rowCount = m_hiddenAvatarList->GetRowCount();

	for (int i = 0; i < rowCount; ++i)
	{
		UIData const * const selectedData = m_hiddenAvatarList->GetDataAtRow(i);

		NOT_NULL(selectedData);

		UINarrowString maleTemplateName;
		selectedData->GetPropertyNarrow(s_maleProp, maleTemplateName);

		UINarrowString femaleTemplateName;
		selectedData->GetPropertyNarrow(s_femaleProp, femaleTemplateName);

		//WARNING(true, ("%s %s", maleTemplateName.c_str(), femaleTemplateName.c_str()));

		SpeciesButton * const speciesButton = new SpeciesButton;

		speciesButton->m_index = i;
		speciesButton->m_maleCodeDataName = getCodeDataName(maleTemplateName);
		speciesButton->m_femaleCodeDataName = getCodeDataName(femaleTemplateName);

		getCodeDataObject(TUIButton, speciesButton->m_maleButton, speciesButton->m_maleCodeDataName.c_str());
		getCodeDataObject(TUIButton, speciesButton->m_femaleButton, speciesButton->m_femaleCodeDataName.c_str());

		registerMediatorObject(*speciesButton->m_maleButton, true);
		registerMediatorObject(*speciesButton->m_femaleButton, true);

		//WARNING(true, ("%s %s", speciesButton->m_maleCodeDataName.c_str(), speciesButton->m_femaleCodeDataName.c_str()));

		m_speciesButtons.push_back(speciesButton);
	}
}

//----------------------------------------------------------------------

SwgCuiAvatarSimple::~SwgCuiAvatarSimple()
{
	if (s_verifyMessageBox != 0)
	{
		s_verifyMessageBox->closeMessageBox();
	}

	m_buttonBack = 0;
	m_buttonNext = 0;
	m_buttonRandomName = 0;
	m_buttonSpeciesPrev = 0;
	m_buttonSpeciesNext = 0;
	m_buttonMale = 0;
	m_buttonFemale = 0;
	m_buttonClothingPrev = 0;
	m_buttonClothingNext = 0;
	m_buttonGenderNext = 0;
	m_buttonGenderPrev = 0;
	m_buttonRandomAppearance = 0;
	m_buttonRandomAll = 0;
	m_textboxName = 0;
	m_textboxSurname = 0;
	m_checkNewbieTutorial = 0;
	m_textMale = 0;
	m_textFemale = 0;
	m_textFemale = 0;
	m_viewer = 0;
	m_hiddenAvatarList = 0;
	m_speciesSelection = 0;
	m_genderSelection = 0;
	m_clothingSelection = 0;
	m_p2speciesDescription = 0;
	m_p2speciesName = 0;
	m_surnameVisible = 0;
	m_species1_male = 0;
	m_species2_male = 0;
	m_species1_female = 0;
	m_species2_female = 0;

	delete m_callback;
	m_callback = 0;

	delete m_oneNameTemplateVector;
	m_oneNameTemplateVector = 0;

	delete m_avatarMap;
	m_avatarMap = 0;

	m_customizationGroups->clear();
	delete m_customizationGroups;
	m_customizationGroups = 0;

	Appearance * const appearance = m_boxObject->getAppearance();
	Extent * const extent = (appearance != 0) ? const_cast<Extent *>(appearance->getExtent()) : 0;

	if (extent != 0)
	{
		IGNORE_RETURN(ExtentList::fetch(extent));
	}

	delete m_boxObject;
	m_boxObject = 0;

	{
		SpeciesButtons::iterator ii = m_speciesButtons.begin();
		SpeciesButtons::iterator iiEnd = m_speciesButtons.end();

		for (; ii != iiEnd; ++ii)
		{
			SpeciesButton * const speciesButton = *ii;
			delete speciesButton;
		}

		m_speciesButtons.clear();
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::performActivate()
{
	CuiMediator::performActivate();

	setIsUpdating(true);
	setPointerInputActive(true);
	setKeyboardInputActive(true);
	setInputToggleActive(false);

	SwgCuiAvatarCreationHelper::setCreatingJedi(false);

	m_hiddenAvatarList->SelectRow(0);

	bool const creatingFirstCharacter = SwgCuiAvatarCreationHelper::getCreature() == 0;

	if (creatingFirstCharacter )
	{
		m_generatedFirstName = false;
		m_gender = G_Male;
	}
	else
	{
		m_currentProfessionName = SwgCuiAvatarCreationHelper::getProfession();
		m_clothingSelection->SetText(Unicode::narrowToWide("@cc_" + m_currentProfessionName));
	}

	loadAllPlayerModelData();
	updateAvatarSelection();

	if (creatingFirstCharacter )
	{
		selectClothes(D_Random);
		randomizeAppearance();
	}

	m_callback->connect(*this, &SwgCuiAvatarSimple::onRandomNameChanged, static_cast<SwgCuiAvatarCreationHelper::Messages::RandomNameChanged *>(0));
	m_callback->connect(*this, &SwgCuiAvatarSimple::onCreationAborted, static_cast<SwgCuiAvatarCreationHelper::Messages::Aborted*>(0));
	m_callback->connect(*this, &SwgCuiAvatarSimple::onCreationFinished, static_cast<SwgCuiAvatarCreationHelper::Messages::CreationFinished *>(0));
	m_callback->connect(*this, &SwgCuiAvatarSimple::onVerifyAndLockNameFinished, static_cast<SwgCuiAvatarCreationHelper::Messages::VerifyAndLockNameResponse *>(0));

	m_textboxName->SetFocus();

	m_viewer->SetLocalTooltip(CuiStringIds::tooltip_viewer_3d_controls.localize());
	m_viewer->setViewDirty(true);
	m_viewer->setAutoZoomOutOnly(false);
	m_viewer->setCameraZoomInWhileTurn(false);
	m_viewer->setCameraLookAtCenter(true);
	m_viewer->setCameraForceTarget(false);
	m_viewer->recomputeZoom();
	m_viewer->setCameraForceTarget(false);
	m_viewer->setViewDirty(true);
	m_viewer->setRotationSlowsToStop(true);
	m_viewer->setCameraTransformToObj(true);
	m_viewer->setCameraLookAtBone("root");

	CuiTransition::signalTransitionReady(CuiMediatorTypes::AvatarSimple);

	m_pendingCreationFinished = false;
	m_aborted = false;
	m_preloadingFinished = false;
	m_pendingNameVerified = false;
	m_secondsUntilNextPrefetch = 0.0f;

	if (!m_generatedFirstName)
	{
		SwgCuiAvatarCreationHelper::requestRandomName(true);
		m_generatedFirstName = true;
	}

	updateSpeciesDescription();

	m_checkNewbieTutorial->SetVisible(CuiLoginManager::canSkipTutorial());
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::performDeactivate()
{
	CuiMediator::performDeactivate();

	setIsUpdating(false);
	setPointerInputActive(false);
	setKeyboardInputActive(false);
	setInputToggleActive(true);

	m_callback->disconnect(*this, &SwgCuiAvatarSimple::onRandomNameChanged, static_cast<SwgCuiAvatarCreationHelper::Messages::CreationFinished *>(0));
	m_callback->disconnect(*this, &SwgCuiAvatarSimple::onCreationAborted, static_cast<SwgCuiAvatarCreationHelper::Messages::Aborted*>(0));
	m_callback->disconnect(*this, &SwgCuiAvatarSimple::onCreationFinished, static_cast<SwgCuiAvatarCreationHelper::Messages::CreationFinished *>(0));
	m_callback->disconnect(*this, &SwgCuiAvatarSimple::onVerifyAndLockNameFinished, static_cast<SwgCuiAvatarCreationHelper::Messages::VerifyAndLockNameResponse *>(0));

	m_viewer->clearObjects();

	deleteAvatarMap(m_avatarMap);
	m_avatarMap->clear();

	if (m_cdata != 0)
	{
		m_cdata->release();
		m_cdata = 0;
	}

	m_pendingNameVerified = false;

	if (s_verifyMessageBox != 0)
	{
		s_verifyMessageBox->closeMessageBox();
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::OnButtonPressed(UIWidget * const context)
{
	CuiPreferences::setUseNewbieTutorial(m_checkNewbieTutorial->IsChecked());

	if (context == m_buttonBack)
	{
		CuiMessageBox::createYesNoBox(CuiStringIds::avatar_create_confirm_cancel.localize(), onMessageBoxClosedConfirmCancel);
	}
	else if (context == m_buttonNext)
	{
		verifyAndLockName();
	}
	else if (context == m_buttonRandomName)
	{
		randomizeName();
	}
	else if (context == m_buttonMale)
	{
		if (m_gender != G_Male)
		{
			selectGender(G_Male);
		}
	}
	else if (context == m_buttonFemale)
	{
		if (m_gender != G_Female)
		{
			selectGender(G_Female);
		}
	}
	else if (context == m_buttonSpeciesPrev)
	{
		selectSpecies(D_Previous);
		updateAvatarSelection();
	}
	else if (context == m_buttonSpeciesNext)
	{
		selectSpecies(D_Next);
		updateAvatarSelection();
	}
	else if (context == m_buttonRandomAppearance)
	{
		randomizeAppearance();
		updateAvatarSelection();
	}
	else if ((context == m_buttonGenderNext) || (context == m_buttonGenderPrev))
	{
		// we only have 2 genders so both buttons do the same thing
		int nextGender = static_cast<int>(m_gender) + 1;
		nextGender %= static_cast<int>(G_numberOfGenders);
		selectGender(static_cast<Gender>(nextGender));
	}
	else if (context == m_buttonClothingPrev)
	{
		selectClothes(D_Previous);
		updateAvatarSelection();
	}
	else if (context == m_buttonClothingNext)
	{
		selectClothes(D_Next);
		updateAvatarSelection();
	}
	else if (context == m_buttonRandomAll)
	{
		randomize();
		updateAvatarSelection();
	}
	else
	{
		SpeciesButtons::iterator ii = m_speciesButtons.begin();
		SpeciesButtons::iterator iiEnd = m_speciesButtons.end();

		for (; ii != iiEnd; ++ii)
		{
			SpeciesButton * const speciesButton = *ii;
			NOT_NULL(speciesButton);

			if ((context == speciesButton->m_maleButton)
				|| (context == speciesButton->m_femaleButton))
			{
				selectSpecies(speciesButton);
				updateAvatarSelection();
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::onRandomNameChanged(const Unicode::String & name)
{
	if (m_nameModified)
		return;

	Unicode::String firstname;
	Unicode::String surname;

	cutString(name, firstname, surname);

	m_textboxName->SetLocalText(firstname);
	m_textboxSurname->SetLocalText(surname);

	CreatureObject * const playerAvatar = SwgCuiAvatarCreationHelper::getCreature();

	if (playerAvatar != 0)
	{
		playerAvatar->setObjectName(name);
	}

	m_pendingNameVerified = false;
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::onCreationAborted(bool)
{
	m_aborted = true;

	if (m_pendingCreationFinished)
	{
		WARNING (true,("Got abort message in the same frame as pending creation finished message ."));
	}
	else
	{
		GameNetwork::setAcceptSceneCommand(false);
		GameNetwork::disconnectConnectionServer();
		CuiMediatorFactory::activate(CuiMediatorTypes::AvatarSelection);
		SwgCuiAvatarCreationHelper::requestRandomName(false);
		SwgCuiAvatarCreationHelper::purgePool();
		deactivate();
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::onCreationFinished(bool b)
{
	if (b)
	{
		if (m_aborted)
		{
			WARNING (true,("Got creation finished message in the same frame as user abort message."));
		}
		else
		{
			m_pendingCreationFinished = false;

			CreatureObject * const player = SwgCuiAvatarCreationHelper::getCreature();

			Unicode::String playerName;

			if (player == 0)		
			{
				if (!SwgCuiAvatarCreationHelper::wasLastCreationAutomatic(playerName))
				{
					WARNING(true, ("No player"));
					return;
				}
			}
			else
			{
				playerName = player->getObjectName ();
			}

			ConfigClientGame::setLauncherAvatarName(std::string());
			ConfigClientGame::setLauncherClusterId(CuiLoginManager::getConnectedClusterId ());
			ConfigClientGame::setAvatarName(Unicode::wideToNarrow (playerName));
			ConfigClientGame::setCentralServerName(CuiLoginManager::getConnectedClusterName ());
			ConfigClientGame::setNextAutoConnectToGameServer(true);

			CuiMediatorFactory::activate(CuiMediatorTypes::AvatarSelection);

			SwgCuiAvatarCreationHelper::purgePool();
			deactivate();
		}
	}	
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::verifyAndLockName()
{
	Unicode::String const & firstname = m_textboxName->GetLocalText();
	Unicode::String const & surname = m_textboxSurname->GetLocalText();

	if (firstname.empty())
	{
		Unicode::String str;
		NameErrors::nameDeclinedEmpty.localize(str);
		IGNORE_RETURN(CuiMessageBox::createInfoBox(str));
		return;
	}

	Unicode::String playerName;
	bool const hasLastName = m_surnameVisible->IsVisible() && !surname.empty();

	if (hasLastName)
	{
		playerName = firstname + (Unicode::narrowToWide(" ") + surname);
	}
	else
	{
		playerName = firstname;
	}

	CreatureObject * const playerAvatar = SwgCuiAvatarCreationHelper::getCreature();
	NOT_NULL(playerAvatar);
	playerAvatar->setObjectName(playerName);

	if (s_verifyMessageBox != 0)
	{
		s_verifyMessageBox->closeMessageBox();
	}

	s_verifyMessageBox = CuiMessageBox::createMessageBox(CuiStringIds::avatar_verifying_name.localize(), verifyMessageBoxClosed);
	s_verifyMessageBox->setRunner(true);

	SwgCuiAvatarCreationHelper::VerifyAndLockName();

	m_pendingNameVerified = true;
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::onVerifyAndLockNameFinished(const SwgCuiAvatarCreationHelper::Messages::VerifyAndLockNameResponse::Payload & payload)
{
	if (m_pendingNameVerified)
	{
		if (payload.success)
		{
			Unicode::String const & firstname = m_textboxName->GetLocalText();
			Unicode::String const & surname = m_textboxSurname->GetLocalText();

			if (firstname.empty())
			{
				Unicode::String str;
				NameErrors::nameDeclinedEmpty.localize(str);
				IGNORE_RETURN(CuiMessageBox::createInfoBox(str));
				return;
			}

			Unicode::String playerName;
			bool const hasLastName = m_surnameVisible->IsVisible() && !surname.empty();

			if (hasLastName)
			{
				playerName = firstname + (Unicode::narrowToWide(" ") + surname);
			}
			else
			{
				playerName = firstname;
			}

			CreatureObject * const playerAvatar = SwgCuiAvatarCreationHelper::getCreature();
			NOT_NULL(playerAvatar);

			if (playerAvatar->getObjectName() == playerName)
			{
				CreatureObject * const playerAvatar = SwgCuiAvatarCreationHelper::getCreature();
				NOT_NULL(playerAvatar);

				if (hasLastName)
				{
					playerAvatar->setObjectName(firstname + (Unicode::narrowToWide(" ") + surname));
				}
				else
				{
					playerAvatar->setObjectName(firstname);
				}

				SwgCuiAvatarCreationHelper::purgeExtraPoolMembers();
				SwgCuiAvatarCreationHelper::setCreatureCustomized(true);

				CuiTransition::startTransition(CuiMediatorTypes::AvatarSimple, CuiMediatorTypes::AvatarCustomize);
			}
		}
		else
		{
			if (s_verifyMessageBox != 0)
			{
				s_verifyMessageBox->closeMessageBox();
			}

			CuiMessageBox::createInfoBox(payload.errorMessage.localize(), verifyMessageBoxClosed);
		}
	}

	m_pendingNameVerified = false;
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::update(float deltaTimeSecs)
{
	CuiMediator::update(deltaTimeSecs);

	if (m_morphInProgress)
	{
		bool somethingChanged = false;

		float const frameTime = Clock::frameTime();

		int changeCount = 0;

		CreatureObject * const playerAvatar = SwgCuiAvatarCreationHelper::getCreature();
		NOT_NULL(playerAvatar);

		CustomizationGroupVector::iterator ii = m_customizationGroups->begin();
		CustomizationGroupVector::iterator iiEnd = m_customizationGroups->end();

		for (; ii != iiEnd; ++ii)
		{
			CustomizationGroup & group = *ii;
			changeCount += group.incrementTarget(*playerAvatar, frameTime);
		}

		somethingChanged =(changeCount != 0);
		m_morphInProgress = somethingChanged;
	}

	if (!m_preloadingFinished)
	{
		m_secondsUntilNextPrefetch += deltaTimeSecs;
		if (m_secondsUntilNextPrefetch > cs_secondsUntilNextPrefetch)
		{
			m_secondsUntilNextPrefetch = 0.0f;
			prefetchAnotherObject();
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::prefetchAnotherObject()
{
	{
		AvatarMap::iterator ii = m_avatarMap->begin();
		AvatarMap::iterator iiEnd = m_avatarMap->end();

		for (; ii != iiEnd; ++ii)
		{
			CreatureObject * avatar = (*ii).second.m_creature;

			if (avatar == 0)
			{
				std::string const & templateName = (*ii).first;

				avatar = createPlayerModel(templateName);

				if (avatar != 0)
				{
					(*m_avatarMap)[templateName].m_creature = avatar;
					return;
				}
			}
		}
	}

	{
		AvatarMap::iterator ii = m_avatarMap->begin();
		AvatarMap::iterator iiEnd = m_avatarMap->end();

		for (; ii != iiEnd; ++ii)
		{
			Avatar & avatar = (*ii).second;

			StringVector::const_iterator jj = avatar.m_hairStyleTemplates.begin();
			StringVector::const_iterator jjEnd = avatar.m_hairStyleTemplates.end();

			for (; jj != jjEnd; ++jj)
			{
				std::string const & templateName = *jj;

				TangibleMap::const_iterator kk = avatar.m_hairStyles.find(templateName);

				if (kk == avatar.m_hairStyles.end())
				{
					if (avatar.m_hairStyles[templateName] == 0)
					{
						TangibleObject * const hair = createHair(templateName, m_cdata);
						if (hair != 0)
						{
							avatar.m_hairStyles[templateName] = hair;
							return;
						}
					}
				}
			}
		}
	}

	m_preloadingFinished = true;
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::updateAvatarSelection()
{
	CreatureObject * playerAvatar = SwgCuiAvatarCreationHelper::getCreature();

	UIData const * const selectedData = m_hiddenAvatarList->GetDataAtRow(m_hiddenAvatarList->GetLastSelectedRow());

	NOT_NULL(selectedData);

	UINarrowString templateName;
	selectedData->GetPropertyNarrow(m_gender == G_Male ? s_maleProp : s_femaleProp, templateName);

	CreatureObject const * const originalAvatar = playerAvatar;

	AvatarMap::const_iterator ii = m_avatarMap->find(templateName);
	if (ii != m_avatarMap->end())
	{
		playerAvatar = (*ii).second.m_creature;
	}

	if (playerAvatar == 0)
	{
		playerAvatar = createPlayerModel(templateName);
		if (playerAvatar != 0)
		{
			(*m_avatarMap)[templateName].m_creature = playerAvatar;
		}
		else
		{
			IGNORE_RETURN(CuiMessageBox::createInfoBox(CuiStringIds::avatar_err_avatar_not_found.localize()));
			m_hiddenAvatarList->SelectRow(-1);
			return;
		}
	}

	if ((playerAvatar != originalAvatar) || (m_viewer->getLastRenderObject() == 0))
	{
		playerAvatar->resetRotateTranslate_o2p();

		m_viewer->clearObjects();

		m_viewer->addObject(*playerAvatar);

		TangibleObject * const pedestal = SwgCuiAvatarCreationHelper::getPedestal();

		if (pedestal != 0)
		{
			pedestal->resetRotateTranslate_o2p();
			m_viewer->addObject(*pedestal);

			BoxExtent const * const box = dynamic_cast<const BoxExtent *>(pedestal->getAppearance()->getExtent());
			if (box != 0)
			{
				pedestal->move_o(Vector::negativeUnitY *(box->getTop() + SwgCuiAvatarCreationHelper::getPedestalOffsetFromTop()));
			}
		}
		else
		{
			WARNING(true,("No pedestal for SwgCuiAvatarCreation"));
		}

		m_viewer->addObject(*m_boxObject);
	}

	IGNORE_RETURN(SwgCuiAvatarCreationHelper::setCreature(*playerAvatar));
	SwgCuiAvatarCreationHelper::setCreatureCustomized(true);

	m_viewer->setPaused(false);

	bool const isMale = m_gender == G_Male;
	bool const isFemale = m_gender == G_Female;

	m_textMale->SetVisible(isMale);
	m_species1_male->SetVisible(isMale);
	m_species2_male->SetVisible(isMale);

	m_textFemale->SetVisible(isFemale);
	m_species1_female->SetVisible(isFemale);
	m_species2_female->SetVisible(isFemale);

	if (isMale)
	{
		m_genderSelection->SetText(m_textMale->GetText());
	}
	else if (isFemale)
	{
		m_genderSelection->SetText(m_textFemale->GetText());
	}

	if (std::binary_search(m_oneNameTemplateVector->begin(), m_oneNameTemplateVector->end(), std::string(playerAvatar->getObjectTemplateName())))
	{
		m_surnameVisible->SetVisible(false);
	}
	else
	{
		m_surnameVisible->SetVisible(true);
	}

	if ((playerAvatar != 0) &&(originalAvatar != playerAvatar))
	{
		initializeCustomization();
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::loadAllPlayerModelData()
{
	std::string dummyDefaultHair;
	int dummyHairSkillModValue = 0;

	std::string avatarTemplateName;

	{
		CreatureObject * const playerAvatar = SwgCuiAvatarCreationHelper::getCreature();

		if (playerAvatar != 0)
		{
			avatarTemplateName = playerAvatar->getTemplateName();

			Avatar & avatar = (*m_avatarMap)[avatarTemplateName];

			avatar.m_creature = playerAvatar;

			if (!CuiCharacterHairManager::loadHairStyles(avatarTemplateName, avatar.m_hairStyleTemplates, dummyDefaultHair, dummyHairSkillModValue))
			{
				WARNING_STRICT_FATAL(!avatarTemplateName.empty(),("Error loading hairstyles for template %s", avatarTemplateName.c_str()));
			}

			if (m_cdata != 0)
			{
				m_cdata->release();
				m_cdata = 0;
			}

			m_cdata = playerAvatar->fetchCustomizationData();
		}
	}

	UIDataSource * const dataSource = m_hiddenAvatarList->GetDataSource();

	if (dataSource != 0)
	{
		UIDataList const & olist = dataSource->GetData();

		UIDataList::const_iterator ii = olist.begin();
		UIDataList::const_iterator iiEnd = olist.end();

		for (int i = 0; ii != iiEnd; ++i, ++ii)
		{
			UIData const * const data = *ii;
			NOT_NULL(data);

			{
				std::string templateName;
				if (data->GetPropertyNarrow(s_maleProp, templateName))
				{
					if (avatarTemplateName == templateName)
					{
						m_hiddenAvatarList->SelectRow(i);
					}

					AvatarMap::const_iterator ii = m_avatarMap->find(templateName);
					if (ii == m_avatarMap->end() || (*ii).second.m_creature == 0)
					{
						Avatar & avatar = (*m_avatarMap)[templateName];

						if (!CuiCharacterHairManager::loadHairStyles(templateName, avatar.m_hairStyleTemplates, dummyDefaultHair, dummyHairSkillModValue))
						{
							WARNING_STRICT_FATAL(!templateName.empty(),("Error loading hairstyles for template %s", templateName.c_str()));
						}
					}
				}
			}

			{
				std::string templateName;
				if (data->GetPropertyNarrow(s_femaleProp, templateName))
				{
					if (avatarTemplateName == templateName)
					{
						m_hiddenAvatarList->SelectRow(i);
					}

					AvatarMap::const_iterator ii = m_avatarMap->find(templateName);
					if (ii == m_avatarMap->end() || (*ii).second.m_creature == 0)
					{
						Avatar & avatar =(*m_avatarMap)[templateName];

						if (!CuiCharacterHairManager::loadHairStyles(templateName, avatar.m_hairStyleTemplates, dummyDefaultHair, dummyHairSkillModValue))
						{
							WARNING_STRICT_FATAL(!templateName.empty(),("Error loading hairstyles for template %s", templateName.c_str()));
						}
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::initializeCustomization()
{
	CreatureObject * const playerAvatar = SwgCuiAvatarCreationHelper::getCreature();
	NOT_NULL(playerAvatar);

	m_customizationGroups->clear();

	Unicode::String species(Unicode::narrowToWide(playerAvatar->getObjectTemplateName()));

	std::string species_gender;

	// @todo: replace species hack code below with real code
	Unicode::String::size_type sharedPos = species.find(Unicode::narrowToWide("shared_"), 0);
	const Unicode::String::size_type iffPos = species.find(Unicode::narrowToWide(".iff"), 0);
	if (sharedPos != Unicode::String::npos && iffPos != Unicode::String::npos)
	{
		sharedPos += strlen("shared_");
		species = species.substr(sharedPos, iffPos - sharedPos);
		species_gender = Unicode::wideToNarrow(species);
		species = Unicode::narrowToWide("creature.") + species;
	}

	if (m_cdata != 0)
	{
		m_cdata->release();
		m_cdata = 0;
	}

	m_cdata = playerAvatar->fetchCustomizationData();
	playerAvatar->clientSetMood(0);
	playerAvatar->setAnimationMood("ui");

	CustomizationManager::CustomizationSpeciesMap const & customizationMap = CustomizationManager::getCustomizationSpeciesMap(species_gender);

	for (CustomizationManager::CustomizationSpeciesMap::const_iterator i = customizationMap.begin(); i != customizationMap.end(); ++i)
	{
		std::string const & customizationGroupName = i->first;
		CustomizationManager::CustomizationVector const & customizationVector = i->second;
		CustomizationGroup const group(*playerAvatar, *m_cdata, species_gender, customizationGroupName, customizationVector);
		m_customizationGroups->push_back(group);
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::randomize()
{
	Direction const species = static_cast<Direction>(Random::random(1));
	Direction const clothes = static_cast<Direction>(Random::random(1));

	randomizeGender();
	selectSpecies(species);
	selectClothes(clothes);
	randomizeAppearance();

	// name needs to be called dead last because Species and Gender might be changed
	// and the newly randomized name could be placed on the wrong character
	randomizeName();
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::randomizeName()
{
	m_nameModified = false;
	SwgCuiAvatarCreationHelper::requestRandomName(true);
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::randomizeGender()
{
	m_gender = static_cast<Gender>(Random::random(static_cast<int>(G_numberOfGenders) - 1));

	CreatureObject * const playerAvatar = SwgCuiAvatarCreationHelper::getCreature();
	NOT_NULL(playerAvatar);

	PlayerCreationManagerClient::strip(*playerAvatar);
	PlayerCreationManagerClient::setupPlayer(*playerAvatar, m_currentProfessionName);
	SwgCuiAvatarCreationHelper::setProfession(m_currentProfessionName);
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::selectGender(Gender gender)
{
	m_gender = gender;
	updateAvatarSelection();

	CreatureObject * const playerAvatar = SwgCuiAvatarCreationHelper::getCreature();
	NOT_NULL(playerAvatar);

	PlayerCreationManagerClient::strip(*playerAvatar);
	PlayerCreationManagerClient::setupPlayer(*playerAvatar, m_currentProfessionName);
	SwgCuiAvatarCreationHelper::setProfession(m_currentProfessionName);
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::selectSpecies(Direction const direction)
{
	int const rowCount = m_hiddenAvatarList->GetRowCount();
	int const lastSelectedRow = m_hiddenAvatarList->GetLastSelectedRow();
	int const offset = (direction == D_Previous) ? -1 : 1;
	int nextSelection = (lastSelectedRow + offset);

	if (nextSelection < 0)
	{
		nextSelection = rowCount - 1;
	}

	nextSelection %= rowCount;

	selectSpecies(nextSelection);
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::selectSpecies(SpeciesButton const * speciesButton)
{
	NOT_NULL(speciesButton);
	selectSpecies(speciesButton->m_index);
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::selectSpecies(int const newSelection)
{
	int const rowCount = m_hiddenAvatarList->GetRowCount();

	if ((newSelection < 0) || (newSelection > rowCount))
	{
		return;
	}
	
	m_hiddenAvatarList->SelectRow(newSelection);

	// we need to force an update Avatar Selection for the species change.
	// kind of nasty having to do this here.

	updateAvatarSelection();
	updateSpeciesDescription();

	CreatureObject * playerAvatar = SwgCuiAvatarCreationHelper::getCreature();
	NOT_NULL(playerAvatar);

	PlayerCreationManagerClient::strip(*playerAvatar);
	PlayerCreationManagerClient::setupPlayer(*playerAvatar, m_currentProfessionName);
	SwgCuiAvatarCreationHelper::setProfession(m_currentProfessionName);
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::selectClothes(Direction const direction)
{
	PlayerCreationManager::StringVector professions;
	PlayerCreationManager::getProfessionVector(professions, "");

	{
		PlayerCreationManager::StringVector::iterator ii = std::find(professions.begin(), professions.end(), s_jediString);

		if (ii != professions.end())
		{
			professions.erase(ii);
		}
	}

	if (!professions.empty())
	{
		int const numberOfProfessions = professions.size();

		if (direction == D_Random)
		{
			int const index = Random::random(numberOfProfessions - 1);
			m_currentProfessionName = professions[index];
		}

		PlayerCreationManager::StringVector::const_iterator ii = professions.begin();
		PlayerCreationManager::StringVector::const_iterator iiEnd = professions.end();

		int currentIndex = 0;

		for (int i = 0; ii != iiEnd; ++ii, ++i)
		{
			std::string profession = *ii;

			if (profession == m_currentProfessionName)
			{
				currentIndex = i;
				break;
			}
		}

		int const offset = (direction == D_Previous) ? -1 : 1;
		int nextSelection = (currentIndex + offset);

		if (nextSelection < 0)
		{
			nextSelection = numberOfProfessions - 1;
		}

		nextSelection %= numberOfProfessions;

		m_currentProfessionName = professions[nextSelection];

		m_clothingSelection->SetText(Unicode::narrowToWide("@cc_" + m_currentProfessionName));

		CreatureObject * const playerAvatar = SwgCuiAvatarCreationHelper::getCreature();
		NOT_NULL(playerAvatar);

		PlayerCreationManagerClient::strip(*playerAvatar);
		PlayerCreationManagerClient::setupPlayer(*playerAvatar, m_currentProfessionName);
		SwgCuiAvatarCreationHelper::setProfession(m_currentProfessionName);
	}
	else
	{
		WARNING(true, ("No professions available"));
	}
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::randomizeAppearance()
{
	typedef std::map<std::string, int> StringIntMap;
	StringIntMap colorValues;
	StringIntMap colorValuesLinked;
	StringIntMap hairColorValues;
	std::string hairColorVarNames[2];

	NOT_NULL(m_cdata);
	{
		for (CustomizationGroupVector::iterator ii = m_customizationGroups->begin(); ii != m_customizationGroups->end(); ++ii)
		{
			CustomizationGroup & group = *ii;

			group.randomize(false);

			//-- store the hair color variables names

			if (group.isHair || group.isColorLinkedToHair)
			{
				hairColorVarNames[0] = group.varColorLinkedToHair[0];
				hairColorVarNames[1] = group.varColorLinkedToHair[1];
			}

			{
				for (int i = 0; i < 2; ++i)
				{
					if (group.varColors[i].empty())
					{
						continue;
					}

					if (group.isVarColorHair[i])
					{
						continue;
					}

					//-- don't randomize the color multiple times
					StringIntMap::const_iterator ii = colorValues.find(group.varColors[i]);

					int value = 0;

					if (ii == colorValues.end())
					{
						PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(CustomizationManager::findVariable(*m_cdata, group.varColors[i]));

						if (var != 0)
						{
							PaletteArgb const * const palette = var->fetchPalette();
							if (palette)
							{
								SharedImageDesignerManager::PaletteValues const paletteValues = SharedImageDesignerManager::getPaletteValuesForPaletteAndLevel(palette, SharedImageDesignerManager::IDPL_NOT_A_DESIGNER);
								int range_begin = 0;
								int range_end = 0;

								var->getRange(range_begin, range_end);

								if (paletteValues.maxIndex < range_end)
								{
									range_end = paletteValues.maxIndex;
								}

								value = Random::random(range_begin, range_end - 1);
								var->setValue(value);
								colorValues.insert(std::make_pair(group.varColors[i], value));

								if (!group.varColorLinkedToSelf[i][0].empty())
								{
									colorValuesLinked.insert(std::make_pair(group.varColorLinkedToSelf[i][0], value));
								}
								if (!group.varColorLinkedToSelf[i][1].empty())
								{
									colorValuesLinked.insert(std::make_pair(group.varColorLinkedToSelf[i][1], value));
								}

								palette->release();
							}
						}
					}
					else
					{
						value =(*ii).second;
					}

					if (group.isColorLinkedToHair)
					{
						hairColorValues[group.varColorLinkedToHair[i]] = value;
					}
				}
			}
		}
	}

	//-- bring the linked variables up to spec
	{
		for (StringIntMap::const_iterator ii = colorValuesLinked.begin(); ii != colorValuesLinked.end(); ++ii)
		{
			const std::string & varName =(*ii).first;
			const int value =(*ii).second;
			PaletteColorCustomizationVariable * const var = dynamic_cast<PaletteColorCustomizationVariable *>(CustomizationManager::findVariable(*m_cdata, varName));

			if (var)
			{
				var->setValue(value);
			}
		}
	}


	{
		CreatureObject * const playerAvatar = SwgCuiAvatarCreationHelper::getCreature();
		NOT_NULL(playerAvatar);

		TangibleObject * const oldHair = safe_cast<TangibleObject *>(ContainerInterface::getObjectInSlot(*playerAvatar, "hair"));
		std::string const templateName = playerAvatar->getObjectTemplateName();

		AvatarMap::const_iterator ii = m_avatarMap->find(templateName);
		if (ii != m_avatarMap->end())
		{
			SlottedContainer * const slotted = ContainerInterface::getSlottedContainer(*playerAvatar);
			NOT_NULL(slotted);

			if (oldHair != 0)
			{
				Container::ContainerErrorCode tmp = Container::CEC_Success;
				IGNORE_RETURN(slotted->remove(*oldHair, tmp));

				//-- This is required to force TangibleObject's m_containerChangeMap to be
				//-- correct for the next alter of creature
				if (oldHair->alter(1.0f))
				{
					oldHair->conclude();
				}
			}

			Avatar const & avatar = ii->second;

			if (!avatar.m_hairStyleTemplates.empty())
			{
				int const index = Random::random(avatar.m_hairStyleTemplates.size() - 1);

				std::string const & hairTemplateName = avatar.m_hairStyleTemplates[index];

				TangibleObject * const newHair = createHair(hairTemplateName, m_cdata);

				if (newHair != 0)
				{
					int arrangementIndex = 0;
					Container::ContainerErrorCode tmp = Container::CEC_Success;

					if (slotted->getFirstUnoccupiedArrangement (*newHair, arrangementIndex, tmp))
					{
						slotted->add(*newHair, arrangementIndex, tmp);
					}
				}
			}

			if (oldHair != 0)
			{
				//-- we removed some hair.  It needs to be re-linked to the player model
				//-- or possibly destroyed , 
				{
					//-- The TangibleObject m_containerChangeMap refers to the removal of objects
					//-- before deleting or re-linking customization on the objects, we must force
					//-- the TangibleObject creature to process the changes

					if (playerAvatar->alter(1.0f))
					{
						playerAvatar->conclude();
					}

					//createCustomizationLink(*oldHair);

					//-- none of the hair widgets own it, so destroy it
					//-- it should now be safe to delete the previously worn objects
					delete oldHair;
				}
			}
		}
	}

	{
		CreatureObject * const playerAvatar = SwgCuiAvatarCreationHelper::getCreature();
		NOT_NULL(playerAvatar);

		TangibleObject * const hair = safe_cast<TangibleObject *>(ContainerInterface::getObjectInSlot(*playerAvatar, "hair"));

		if (hair != 0)
		{
			//-- don't randomize the hair color twice
			if (hairColorVarNames[1] == hairColorVarNames[0])
			{
				hairColorVarNames[1].clear();
			}

			//-- update or randomize hair colors as needed
			for (int i = 0; i < 2; ++i)
			{
				if (!hairColorVarNames[i].empty())
				{
					StringIntMap::const_iterator ii = hairColorValues.find(hairColorVarNames[i]);

					if (ii == hairColorValues.end())
					{
						randomizeHairColors(hair, hairColorVarNames[i]);
					}
					else
					{
						randomizeHairColors(hair, hairColorVarNames[i],(*ii).second);
					}
				}
			}
		}
	}

	m_morphInProgress = true;
}

//----------------------------------------------------------------------

void SwgCuiAvatarSimple::updateSpeciesDescription()
{
	UIData const * const selectedData = m_hiddenAvatarList->GetDataAtRow(m_hiddenAvatarList->GetLastSelectedRow());

	NOT_NULL(selectedData);

	{
		UIString selectionName;
		selectedData->GetProperty(UILowerString("LocalText"), selectionName);

		m_speciesSelection->SetLocalText(selectionName);
		m_p2speciesName->SetLocalText(selectionName);
	}

	{
		UIString description;

		std::string descriptionEncoded;

		if (!selectedData->GetPropertyNarrow(UILowerString ("Description"), descriptionEncoded))
		{
			description = Unicode::narrowToWide("NO DESCRIPTION SPECIFIED");
		}
		else
		{
			description = StringId(descriptionEncoded).localize();
		}

		description.push_back('\n');

		m_p2speciesDescription->SetLocalText(description);
		m_p2speciesDescription->ScrollToPoint(UIPoint::zero);
		m_p2speciesDescription->SetTextUnroll(true);
		m_p2speciesDescription->ResetTextUnroll();
	}
}

//======================================================================

