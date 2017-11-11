// ======================================================================
//
// SwgCuiResourceExtraction.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction.h"

#include "sharedMessageDispatch/Message.h"
#include "sharedMessageDispatch/Transceiver.h"

#include "clientGame/ClientInstallationSynchronizedUi.h"
#include "clientGame/Game.h"
#include "clientGame/InstallationObject.h"
#include "clientGame/ResourceIconManager.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientUserInterface/CuiManager.h"
#include "clientUserInterface/CuiMediatorFactory.h"
#include "clientUserInterface/CuiMessageBox.h"
#include "clientUserInterface/CuiResourceManager.h"
#include "clientUserInterface/CuiStringIdsResource.h"
#include "clientUserInterface/CuiWidget3dObjectListViewer.h"
#include "clientUserInterface/CuiWorkspace.h"

#include "swgClientUserInterface/SwgCuiMediatorTypes.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_Hopper.h"
#include "swgClientUserInterface/SwgCuiResourceExtraction_SetResource.h"

#include "UIButton.h"
#include "UIData.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UIPie.h"
#include "UIRunner.h"
#include "UIText.h"
#include "UITextBox.h"
#include "UIVolumePage.h"

#include <cstdio>

// ======================================================================

namespace SwgCuiResourceExtractionNamespace
{
	const UIString     pieFontSmall  (Unicode::narrowToWide("/Fonts.arial_black_14"));
	const UIString     pieFontLarge  (Unicode::narrowToWide("/Fonts.arial_black_20"));
	const UIString     waitingPercent(Unicode::narrowToWide(""));

	const float maxRange = 32.0f;
}

using namespace SwgCuiResourceExtractionNamespace;

//----------------------------------------------------------------------

SwgCuiResourceExtraction::SwgCuiResourceExtraction (UIPage & page)
: CuiMediator                   ("SwgCuiResourceExtraction", page),
  UIEventCallback               (),
  MessageDispatch::Receiver     (),
  m_extractionCallback          (new MessageDispatch::Callback),
  m_window                      (0),
  m_buttonTurnOn                (0),
  m_buttonTurnOff               (0),
  m_buttonChangeResource        (0),
  m_buttonManageHopper          (0),
  m_textTitleBar                (0),
  m_textExtractionRate          (0),
  m_textExtractionRateSpec      (0),
  m_textHopperPercent           (0),
  m_textIntactPercent           (0),
  m_currentResourceName         (0),
  m_currentResourceIcon         (0),
  m_currentResourceEfficiencyText(0),
  m_pageCurrentResourceVolume   (0),
  m_damagePie                   (0),
  m_hopperPie                   (0),
  m_currentResourceBar          (0),
  m_currentResourceBarBack      (0),
  m_buttonOk                    (0),
  m_runnerL                     (0),
  m_runnerR                     (0),
  m_mediatorHopper              (0),
  m_mediatorSetResource         (0),
  m_hopperActive                (false),
  m_harvesterActive             (true)
{
	//set the page states
	IGNORE_RETURN (setState    (MS_closeable));
	IGNORE_RETURN (setState    (MS_closeDeactivates));

	//main page data
	getCodeDataObject (TUIPage,       m_window,                    "window");
	getCodeDataObject (TUIButton,     m_buttonTurnOn,              "switchon");
	getCodeDataObject (TUIButton,     m_buttonTurnOff,             "switchoff");
	getCodeDataObject (TUIButton,     m_buttonChangeResource,      "buttonchangeresource");
	getCodeDataObject (TUIButton,     m_buttonManageHopper,        "buttonaccesshopper");
	getCodeDataObject (TUIText,       m_textTitleBar,              "titlebar");
	getCodeDataObject (TUIText,       m_textExtractionRate,        "percentActual");
	getCodeDataObject (TUIText,       m_textExtractionRateSpec,    "percentSpec");
	getCodeDataObject (TUIText,       m_textHopperPercent,         "piecapacitypercent");
	getCodeDataObject (TUIText,       m_textIntactPercent,         "piehealthpercent");
	getCodeDataObject (TUIPie,        m_damagePie,                 "piehealth");
	getCodeDataObject (TUIPie,        m_hopperPie,                 "piecapacity");
	getCodeDataObject (TUIText,       m_currentResourceName,       "nameresource");
	getCodeDataObject (TUIWidget,     m_currentResourceBar,        "barresource");
	getCodeDataObject (TUIWidget,     m_currentResourceBarBack,    "barbackresource");
	getCodeDataObject (TUIRunner,     m_runnerL,                   "strobeL");
	getCodeDataObject (TUIRunner,     m_runnerR,                   "strobeR");
	getCodeDataObject (TUIButton,     m_buttonOk,                  "buttonok");

	UIWidget * viewerWidget = 0;
	getCodeDataObject (TUIWidget,       viewerWidget, "iconresource");
	m_currentResourceIcon = NON_NULL (dynamic_cast<CuiWidget3dObjectListViewer *>(viewerWidget));
	m_currentResourceIcon->SetDragable             (false);
	m_currentResourceIcon->SetContextCapable       (false, false);
	m_currentResourceIcon->setRotateSpeed          (0.0f);
	m_currentResourceIcon->setCameraLookAtCenter   (true);
	m_currentResourceIcon->setDragYawOk            (false);
	m_currentResourceIcon->setPaused               (false);
	m_currentResourceIcon->setCameraForceTarget    (true);
	m_currentResourceIcon->setAutoZoomOutOnly      (true);
	m_currentResourceIcon->setCameraTransformToObj (true);

	getCodeDataObject (TUIText,       m_currentResourceEfficiencyText, "efficiencytext");

	//set the titlebar
	onChangeName();

	m_currentResourceBar->SetWidth (0);

	m_runnerL->SetPaused(true);
	m_runnerR->SetPaused(true);
}

//-----------------------------------------------------------------

SwgCuiResourceExtraction::~SwgCuiResourceExtraction ()
{
	delete m_extractionCallback;
	m_extractionCallback          = 0;

	m_window                      = 0;
	m_buttonTurnOn                = 0;
	m_buttonTurnOff               = 0;
	m_buttonChangeResource        = 0;
	m_textTitleBar                = 0;
	m_pageCurrentResourceVolume   = 0;
	m_currentResourceName         = 0;
	m_currentResourceIcon         = 0;
}

//-----------------------------------------------------------------

void SwgCuiResourceExtraction::performActivate ()
{
	CuiResourceManager::startListening();
	CuiResourceManager::requestResourceData();

	CuiManager::requestPointer (true);

	CuiResourceManager::requestResourceData ();

	InstallationObject * const harvester = CuiResourceManager::getHarvesterInstallation ();
	if(!harvester)
	{
		DEBUG_WARNING(true, ("No harvester"));
		return;
	}

	setAssociatedObjectId(harvester->getNetworkId());
	setMaxRangeFromObject(maxRange);

	Object * const player                = Game::getPlayer ();
	if(!player)
	{
		DEBUG_WARNING(true, ("No player"));
		return;
	}
	harvester->getHarvesterResourceData    (*player);

	m_buttonTurnOn->AddCallback              (this);
	m_buttonTurnOff->AddCallback             (this);
	m_buttonChangeResource->AddCallback      (this);
	m_buttonManageHopper->AddCallback        (this);
	m_runnerL->AddCallback                   (this);
	m_runnerR->AddCallback                   (this);
	m_buttonOk->AddCallback                  (this);

	m_currentResourceName->SetText           (CuiStringIdsResource::res_waiting.localize());
	m_currentResourceEfficiencyText->SetText (SwgCuiResourceExtractionNamespace::waitingPercent);
	m_textExtractionRate->SetText            (SwgCuiResourceExtractionNamespace::waitingPercent);
	m_textExtractionRateSpec->SetText        (SwgCuiResourceExtractionNamespace::waitingPercent);
	m_textHopperPercent->SetText             (SwgCuiResourceExtractionNamespace::waitingPercent);

	m_currentResourceIcon->clearObjects();

	update ();

	m_extractionCallback->connect (*this, &SwgCuiResourceExtraction::onChangeActive,             static_cast<ClientInstallationSynchronizedUi::Messages::Active *>                (0));
	m_extractionCallback->connect (*this, &SwgCuiResourceExtraction::onChangeExtractionRate,     static_cast<ClientInstallationSynchronizedUi::Messages::ExtractionRate *>        (0));
	m_extractionCallback->connect (*this, &SwgCuiResourceExtraction::onChangeExtractionRateMax,  static_cast<ClientInstallationSynchronizedUi::Messages::ExtractionRateMax *>     (0));
	m_extractionCallback->connect (*this, &SwgCuiResourceExtraction::onChangeResourcePool,       static_cast<ClientInstallationSynchronizedUi::Messages::ResourcePool *>          (0));
	m_extractionCallback->connect (*this, &SwgCuiResourceExtraction::onChangeResourcePoolList,   static_cast<ClientInstallationSynchronizedUi::Messages::ResourcePoolsModified *> (0));
	m_extractionCallback->connect (*this, &SwgCuiResourceExtraction::onChangeHopper,             static_cast<ClientInstallationSynchronizedUi::Messages::Hopper *>                (0));
	m_extractionCallback->connect (*this, &SwgCuiResourceExtraction::onChangeHopperMax,          static_cast<ClientInstallationSynchronizedUi::Messages::HopperMax *>             (0));
	m_extractionCallback->connect (*this, &SwgCuiResourceExtraction::onUpdateResourcePoolData,   static_cast<CuiResourceManager::Messages::PoolsChanged *>                        (0));
	m_extractionCallback->connect (*this, &SwgCuiResourceExtraction::onChangeCondition,          static_cast<ClientInstallationSynchronizedUi::Messages::Condition *>             (0));
	m_extractionCallback->connect (*this, &SwgCuiResourceExtraction::onDataReady,                static_cast<ClientInstallationSynchronizedUi::Messages::InstallationSynchronizedUiReady *>(0));	

	if(m_hopperActive)
	{
		showHopperPage();
		m_hopperActive = false;
	}

	updateActivateButtons();
}

//-----------------------------------------------------------------

void SwgCuiResourceExtraction::performDeactivate ()
{
	CuiResourceManager::stopListening();

	if(m_mediatorHopper && m_mediatorHopper->isActive())
	{
		m_hopperActive = true;
	}

	m_extractionCallback->disconnect (*this, &SwgCuiResourceExtraction::onChangeActive,             static_cast<ClientInstallationSynchronizedUi::Messages::Active *>         (0));
	m_extractionCallback->disconnect (*this, &SwgCuiResourceExtraction::onChangeExtractionRate,     static_cast<ClientInstallationSynchronizedUi::Messages::ExtractionRate *> (0));
	m_extractionCallback->disconnect (*this, &SwgCuiResourceExtraction::onChangeExtractionRateMax,  static_cast<ClientInstallationSynchronizedUi::Messages::ExtractionRateMax *>     (0));
	m_extractionCallback->disconnect (*this, &SwgCuiResourceExtraction::onChangeResourcePool,       static_cast<ClientInstallationSynchronizedUi::Messages::ResourcePool *>       (0));
	m_extractionCallback->disconnect (*this, &SwgCuiResourceExtraction::onChangeResourcePoolList,   static_cast<ClientInstallationSynchronizedUi::Messages::ResourcePoolsModified *>       (0));
	m_extractionCallback->disconnect (*this, &SwgCuiResourceExtraction::onChangeHopper,             static_cast<ClientInstallationSynchronizedUi::Messages::Hopper *>         (0));
	m_extractionCallback->disconnect (*this, &SwgCuiResourceExtraction::onChangeHopperMax,          static_cast<ClientInstallationSynchronizedUi::Messages::HopperMax *>             (0));
	m_extractionCallback->disconnect (*this, &SwgCuiResourceExtraction::onUpdateResourcePoolData,   static_cast<CuiResourceManager::Messages::PoolsChanged *>               (0));
	m_extractionCallback->disconnect (*this, &SwgCuiResourceExtraction::onChangeCondition,          static_cast<ClientInstallationSynchronizedUi::Messages::Condition *>             (0));

	m_currentResourceIcon->clearObjects();

	m_buttonTurnOn->RemoveCallback              (this);
	m_buttonTurnOff->RemoveCallback             (this);
	m_buttonChangeResource->RemoveCallback      (this);
	m_buttonManageHopper->RemoveCallback        (this);
	m_runnerL->RemoveCallback                   (this);
	m_runnerR->RemoveCallback                   (this);
	m_buttonOk->RemoveCallback                  (this);

	CuiManager::requestPointer (false);

	setIsUpdating(false);
}

//-----------------------------------------------------------------

void SwgCuiResourceExtraction::receiveMessage (const MessageDispatch::Emitter & , const MessageDispatch::MessageBase & message)
{
	if (message.isType (Game::Messages::SCENE_CHANGED))
	{

	}
}

//-----------------------------------------------------------------

void SwgCuiResourceExtraction::OnButtonPressed( UIWidget *context )
{
	InstallationObject * const harvester = CuiResourceManager::getHarvesterInstallation ();
	if(!harvester)
	{
		DEBUG_WARNING(true, ("No harvester"));
		return;
	}
	Object * const player                = Game::getPlayer ();
	if(!player)
	{
		DEBUG_WARNING(true, ("No player"));
		return;
	}

	//turn harvester on
	if (m_buttonTurnOn == context)
	{
		const ClientInstallationSynchronizedUi * const sync = CuiResourceManager::getSynchronized ();
		const NetworkId & selectedResourcePool = sync->getSelectedResourcePool ();
		if(selectedResourcePool == NetworkId::cms_invalid)
		{
			CuiMessageBox::createInfoBox (CuiStringIdsResource::res_activatedwithoutresource.localize());
		}
		else
		{
			if (sync)
			{
				setActive(true, false);
			}
		}
	}

	//turn harvester off
	else if (m_buttonTurnOff == context)
	{
		const ClientInstallationSynchronizedUi * const sync = CuiResourceManager::getSynchronized ();
		if (sync)
		{
			setActive(false, false);
		}
	}

	//manage the harvester hopper
	else if (m_buttonManageHopper == context)
	{
		showHopperPage();
	}

	//bring up the resource list page
	else if (m_buttonChangeResource == context)
	{
		harvester->getHarvesterResourceData    (*player);

		m_mediatorSetResource = safe_cast<SwgCuiResourceExtraction_SetResource*>(CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_ResourceExtraction_SetResource));
	}

	else if (m_buttonOk == context)
	{
		deactivate();
	}
}

//-----------------------------------------------------------------

void SwgCuiResourceExtraction::showHopperPage()
{
	m_mediatorHopper = safe_cast<SwgCuiResourceExtraction_Hopper*>(CuiMediatorFactory::toggleInWorkspace (CuiMediatorTypes::WS_ResourceExtraction_Hopper));
}

//-----------------------------------------------------------------

void SwgCuiResourceExtraction::update ()
{
	const ClientInstallationSynchronizedUi * const sync = CuiResourceManager::getSynchronized ();

	if (!sync)
		return;

	onChangeExtractionRate    (*sync);
	onChangeExtractionRateMax (*sync);
	onChangeActive            (*sync);
	onChangeResourcePool      (*sync);
	onChangeHopper            (*sync);
	onChangeHopperMax         (*sync);
	onChangeName              ();
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::updateActivateButtons()
{
	if(m_harvesterActive)
	{
		m_buttonTurnOn->SetEnabled(false);
		m_buttonTurnOff->SetEnabled(true);
	}
	else
	{
		m_buttonTurnOn->SetEnabled(true);
		m_buttonTurnOff->SetEnabled(false);
	}
}

//----------------------------------------------------------------------

//set the title bar to the harvester name
void SwgCuiResourceExtraction::onChangeName ()
{
	const ClientObject * const obj = CuiResourceManager::getHarvester ();
	if (obj)
	{
		m_textTitleBar->SetText (obj->getLocalizedName ());
	}
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::onChangeExtractionRate  (const ClientInstallationSynchronizedUi & sync)
{
	if (sync.getOwner ()->getNetworkId () != CuiResourceManager::getHarvesterId ())
		return;

	char buf [128];
	_snprintf (buf, 128, "%.2f", sync.getExtractionRateInstalled ());
	m_textExtractionRate->SetText (Unicode::narrowToWide (buf));
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::onChangeExtractionRateMax (const ClientInstallationSynchronizedUi & sync)
{
	if (sync.getOwner ()->getNetworkId () != CuiResourceManager::getHarvesterId ())
		return;

	const int max = sync.getExtractionRateMax ();

	char buf [128];
	_snprintf (buf, 128, "%d", max);
	m_textExtractionRateSpec->SetText (Unicode::narrowToWide (buf));
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::onChangeActive (const ClientInstallationSynchronizedUi & sync)
{
	if (sync.getOwner ()->getNetworkId () != CuiResourceManager::getHarvesterId ())
		return;

	setActive(sync.getActive(), true);

	updateActivateButtons();
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::onChangeResourcePool (const ClientInstallationSynchronizedUi & sync)
{
	if (sync.getOwner ()->getNetworkId () != CuiResourceManager::getHarvesterId ())
		return;

	updateResourceList(sync);

	const NetworkId & selectedResourcePool = sync.getSelectedResourcePool ();
	const CuiResourceManager::ResourceInfo * const info = CuiResourceManager::findResourceInfo (selectedResourcePool);
	DEBUG_WARNING(!info, ("ResourceInfo not available for resource %d", selectedResourcePool.getValue()));

	//no resource selected, set visual data accordingly
	if(selectedResourcePool == NetworkId::cms_invalid)
	{
		m_currentResourceName->SetText (CuiStringIdsResource::res_noresourceselected.localize());
		m_currentResourceIcon->clearObjects();
		m_currentResourceIcon->setViewDirty(true);
		const char* buf = "0%";
		m_currentResourceEfficiencyText->SetText(Unicode::narrowToWide(buf));
	}

	//set the resource name
	Unicode::String name;
	IGNORE_RETURN (ResourceTypeManager::createTypeDisplayLabel(selectedResourcePool, name));
	m_currentResourceName->SetText (name);

	//set the resource efficiency text
	const int efficiency = info ? info->efficiency : 0;
	char buf[64];
	_snprintf (buf, 64, "%d%%", efficiency);
	m_currentResourceEfficiencyText->SetText(Unicode::narrowToWide(buf));

	//set the resource icon
	Object* obj = ResourceIconManager::getObjectForType(selectedResourcePool);
	if (obj)
	{
		m_currentResourceIcon->setObject(obj);
		m_currentResourceIcon->setViewDirty(true);
	}
	else
	{
		DEBUG_WARNING(true, ("Could not build an icon object for resource %d", selectedResourcePool.getValue()));
	}
	updateCurrentResourceBar();
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::onChangeHopper (const ClientInstallationSynchronizedUi & sync)
{
	if (sync.getOwner ()->getNetworkId () != CuiResourceManager::getHarvesterId ())
		return;

	updateResourceList(sync);

	const float hopperContentsCur = sync.getHopperContentsCur ();
	const int   hopperContentsMax = sync.getHopperContentsMax ();
	const int   hopperPercent  = hopperContentsMax > 0 ? (static_cast<int>(hopperContentsCur * 100 / hopperContentsMax)) : 0;

	DEBUG_WARNING(hopperPercent > 100, ("Hopper contents is %d%% which is > 100%%", hopperPercent));

	char buf [128];
	_snprintf (buf, 128, "%d%%", hopperPercent);
	m_textHopperPercent->SetText (Unicode::narrowToWide (buf));

	//shrink the font if we have more than 99% to show
	if(hopperPercent < 100)
		IGNORE_RETURN (m_textHopperPercent->SetProperty(UILowerString("Font"), SwgCuiResourceExtractionNamespace::pieFontLarge));
	else
		IGNORE_RETURN (m_textHopperPercent->SetProperty(UILowerString("Font"), SwgCuiResourceExtractionNamespace::pieFontSmall));

	//update the pie piece
	const float hopperPercentFloat = static_cast<float>(hopperPercent) / 100;
	_snprintf (buf, 128, "%.2f", hopperPercentFloat);
	IGNORE_RETURN (m_hopperPie->SetProperty(UILowerString("PieValue"), Unicode::narrowToWide(buf)));

	if(m_mediatorHopper && m_mediatorHopper->isActive())
		m_mediatorHopper->update ();
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::onChangeHopperMax (const ClientInstallationSynchronizedUi & sync)
{
	if (sync.getOwner ()->getNetworkId () != CuiResourceManager::getHarvesterId ())
		return;

	updateResourceList(sync);

	const float hopperContentsCur = sync.getHopperContentsCur ();
	const int   hopperContentsMax = sync.getHopperContentsMax ();
	const int   hopperPercent  = hopperContentsMax > 0 ? (static_cast<int>(hopperContentsCur * 100 / hopperContentsMax)) : 0;

	DEBUG_WARNING(hopperPercent > 100, ("Hopper contents is %d%% which is > 100%%", hopperPercent));

	char buf [128];
	_snprintf (buf, 128, "%d%%", hopperPercent);
	m_textHopperPercent->SetText (Unicode::narrowToWide (buf));

	//shrink the font if we have more than 99% to show
	if(hopperPercent < 100)
		IGNORE_RETURN (m_textHopperPercent->SetProperty(UILowerString("Font"), SwgCuiResourceExtractionNamespace::pieFontLarge));
	else
		IGNORE_RETURN (m_textHopperPercent->SetProperty(UILowerString("Font"), SwgCuiResourceExtractionNamespace::pieFontSmall));

	//update the pie piece
	const float hopperPercentFloat = static_cast<float>(hopperPercent) / 100;
	_snprintf (buf, 128, "%.2f", hopperPercentFloat);
	IGNORE_RETURN (m_hopperPie->SetProperty(UILowerString("PieValue"), Unicode::narrowToWide(buf)));

	if(m_mediatorHopper && m_mediatorHopper->isActive())
		m_mediatorHopper->update ();

	if(m_mediatorSetResource && m_mediatorSetResource->isActive())
		m_mediatorSetResource->update ();
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::onUpdateResourcePoolData (const int & )
{
	//-- force a repopulate
	const ClientInstallationSynchronizedUi * const sync = CuiResourceManager::getSynchronized ();
	if (sync)
		onChangeResourcePoolList (*sync);
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::onChangeCondition (const ClientInstallationSynchronizedUi & sync)
{
	if (sync.getOwner ()->getNetworkId () != CuiResourceManager::getHarvesterId ())
		return;

	const int8 condition = sync.getCondition();
	//update the pie piece
	const float conditionPercentFloat = static_cast<float>(condition) / 100;
	char buf [128];
	_snprintf (buf, 128, "%.2f", conditionPercentFloat);
	IGNORE_RETURN (m_damagePie->SetProperty(UILowerString("PieValue"), Unicode::narrowToWide(buf)));

	if(m_mediatorHopper && m_mediatorHopper->isActive())
		m_mediatorHopper->update ();
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::onChangeResourcePoolList (const ClientInstallationSynchronizedUi & sync)
{
	updateResourceList(sync);

	update();

	if(m_mediatorHopper && m_mediatorHopper->isActive ())
			m_mediatorHopper->update ();

	if(m_mediatorSetResource && m_mediatorSetResource->isActive ())
			m_mediatorSetResource->update ();
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::updateResourceList(const ClientInstallationSynchronizedUi & sync) const
{
	const ClientInstallationSynchronizedUi::ResourceTypeIdVector & resourceTypeIds         = sync.getResourceTypesId     ();
	const ClientInstallationSynchronizedUi::ResourceTypeNameVector & resourceTypeNames     = sync.getResourceTypesName   ();
	const ClientInstallationSynchronizedUi::ResourceTypeParentVector & resourceTypeParents = sync.getResourceTypesParent ();

	size_t i = resourceTypeIds.size();
	size_t j = resourceTypeNames.size();
	size_t k = resourceTypeParents.size();
	DEBUG_FATAL(i != j || j != k, ("different sizes of resource data"));
	UNREF(j);
	UNREF(k);

	//register all the resource types for future use
	for(size_t a = i; a != i; ++a)
	{
		ResourceTypeManager::setTypeInfo(resourceTypeIds[a], Unicode::narrowToWide(resourceTypeNames[a]), resourceTypeParents[a]);
	}
}

//----------------------------------------------------------------------

/** Update the harvester status and GUI to the new active state
 */
void SwgCuiResourceExtraction::setActive (bool active, bool calledByServer)
{
	if(active != m_harvesterActive)
	{
		m_harvesterActive = active;

		InstallationObject * const harvester = CuiResourceManager::getHarvesterInstallation ();
		if(!harvester)
		{
			DEBUG_WARNING(true, ("No harvester"));
			return;
		}
		Object * const player                = Game::getPlayer ();
		if(!player)
		{
			DEBUG_WARNING(true, ("No player"));
			return;
		}

		if(!calledByServer)
			harvester->setHarvesterActive (*player, active);
	}

	if(calledByServer)
	{
		m_runnerL->SetPaused(!active);
		m_runnerR->SetPaused(!active);
		//clear runners if they're off (to avoid orphaning lights)
		if(!active)
		{
			m_runnerL->ToggleAll(false);
			m_runnerR->ToggleAll(false);
		}
	}
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::updateCurrentResourceBar()
{
	const ClientInstallationSynchronizedUi * const sync = CuiResourceManager::getSynchronized ();
	if (!sync)
		return;

	const NetworkId & id = sync->getSelectedResourcePool();

	const CuiResourceManager::ResourceInfo * const info = CuiResourceManager::findResourceInfo (id);
	DEBUG_WARNING(!info, ("ResourceInfo not available for resource %d", id.getValue()));

	const int efficiency = info ? info->efficiency : 0;

	float efficiencyPercent = static_cast<float>(efficiency) / 100.0f;

	int maxWidth = m_currentResourceBarBack->GetSize().x;

	m_currentResourceBar->SetWidth (static_cast<int>(maxWidth * efficiencyPercent));
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::toggleShowData (bool showData)
{
	//hiding the data window automatically displays the "waiting" page, in UI script
	m_window->SetVisible(showData);
}

//----------------------------------------------------------------------

void SwgCuiResourceExtraction::onDataReady (const ClientInstallationSynchronizedUi & sync)
{
	if (sync.getOwner ()->getNetworkId () != CuiResourceManager::getHarvesterId ())
		return;
	toggleShowData(true);
}

// ======================================================================
