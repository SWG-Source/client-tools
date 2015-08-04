//===================================================================
//
// ManufactureSchematicObject.cpp
// copyright 2001, Sony Online Entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ManufactureSchematicObject.h"

#include "clientGame/ClientCommandQueue.h"
#include "clientGame/ClientManufactureSchematicObjectTemplate.h"
#include "clientGame/ClientManufactureSchematicSynchronizedUi.h"
#include "clientGame/ClientObject.h"
#include "clientGame/Game.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/SharedDraftSchematicObjectTemplate.h"
#include "sharedGame/SharedTangibleObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedNetworkMessages/MessageQueueCraftFillSlot.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Controller.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"

//----------------------------------------------------------------------

namespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<const ManufactureSchematicObject &, ManufactureSchematicObject::Messages::ChangeSignal > 
			changeSignal;
		MessageDispatch::Transceiver<const ManufactureSchematicObject &, ManufactureSchematicObject::Messages::Ingredients > 
			ingredients;
		MessageDispatch::Transceiver<const ManufactureSchematicObject &, ManufactureSchematicObject::Messages::Customization > 
			customization;
		MessageDispatch::Transceiver<const ManufactureSchematicObject &, ManufactureSchematicObject::Messages::Ready > 
			ready;
	}

	Controller * findController ()
	{
		return Game::getPlayer () ? Game::getPlayer ()->getController () : 0;
	}
}

//----------------------------------------------------------------------

ManufactureSchematicObject::ManufactureSchematicObject(const SharedManufactureSchematicObjectTemplate* newTemplate) :
	IntangibleObject               (newTemplate),
	m_draftSchematicSharedTemplate (),
	m_itemsPerContainer            (0),
	m_attributes                   (),
	m_manufactureTime              (0),
	m_customAppearance             (),
	m_appearanceData               (),
	m_isCrafting                   (false),
	m_schematicChangedSignal       (0),
	m_customAppearanceSelection    (-1)
{
	addSharedVariable    (m_attributes);
	addSharedVariable    (m_itemsPerContainer);
	addSharedVariable    (m_manufactureTime);

	addSharedVariable_np (m_appearanceData);
	addSharedVariable_np (m_customAppearance);
	addSharedVariable_np (m_draftSchematicSharedTemplate);
	addSharedVariable_np (m_isCrafting);
	addSharedVariable_np (m_schematicChangedSignal);

	m_schematicChangedSignal.setSourceObject (this);
	m_customAppearance.setSourceObject (this);
	m_appearanceData.setSourceObject (this);
	m_draftSchematicSharedTemplate.setSourceObject (this);
}

//-------------------------------------------------------------------
	
ManufactureSchematicObject::~ManufactureSchematicObject()
{
}

//----------------------------------------------------------------------

void ManufactureSchematicObject::endBaselines()
{
	IntangibleObject::endBaselines();

static const uint32 listen = Crc::normalizeAndCalculate("synchronizedUiListen");

	if (m_isCrafting.get())
	{
		// set up syncronization needed for crafting
		setSynchronizedUi (createSynchronizedUi ());
		// tell the server we want it's data
		ClientCommandQueue::enqueueCommand(listen, getNetworkId(), Unicode::String());
	}
}

//----------------------------------------------------------------------

ClientSynchronizedUi * ManufactureSchematicObject::createSynchronizedUi ()
{
	return new ClientManufactureSchematicSynchronizedUi(*this);
}

//----------------------------------------------------------------------

void ManufactureSchematicObject::transferToSlot (const NetworkId & id, int slot, int option, uint8 sequence)
{
	Controller * const controller = findController ();
	
	if (controller)
	{
		MessageQueueCraftFillSlot * const msg = new MessageQueueCraftFillSlot (id, slot, option, sequence);
		
		//-- enqueue message
		controller->appendMessage (CM_fillSchematicSlotMessage, 0.0f, msg, 
			GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_SERVER);
	}
}

//----------------------------------------------------------------------

int ManufactureSchematicObject::getIngredientCount() const
{
	const ClientManufactureSchematicSynchronizedUi * const sync = safe_cast<const ClientManufactureSchematicSynchronizedUi *>(getClientSynchronizedUi ());
	if (sync)
		return sync->getIngredientCount();
	return 0;
}

//----------------------------------------------------------------------

bool ManufactureSchematicObject::getIngredient (int index, Crafting::IngredientSlot & data) const
{
	const ClientManufactureSchematicSynchronizedUi * const sync = safe_cast<const ClientManufactureSchematicSynchronizedUi *>(getClientSynchronizedUi ());
	if (sync)
		return sync->getIngredient(index, data);
	return false;
}

//----------------------------------------------------------------------

bool ManufactureSchematicObject::getIngredient (const StringId & name, Crafting::IngredientSlot & data) const
{
	const ClientManufactureSchematicSynchronizedUi * const sync = safe_cast<const ClientManufactureSchematicSynchronizedUi *>(getClientSynchronizedUi ());
	if (sync)
		return sync->getIngredient(name, data);
	return false;
}

//----------------------------------------------------------------------

void ManufactureSchematicObject::Callbacks::ChangeSignal::modified (ManufactureSchematicObject & target, const uint8 &, const uint8 &, bool) const
{
	Transceivers::changeSignal.emitMessage (target);
}

//----------------------------------------------------------------------

void ManufactureSchematicObject::Callbacks::ChangeDraft::modified (ManufactureSchematicObject & target, const uint32 & old, const uint32 & current, bool) const
{
	// if we have a custom appearance, use that instead of the draft schematic
	if (!target.getCustomAppearanceName().empty())
		return;

	if (old == current)
		return;

	const SharedDraftSchematicObjectTemplate * const draftTemplate = safe_cast<
		const SharedDraftSchematicObjectTemplate *>(ObjectTemplateList::fetch(
		ObjectTemplateList::lookUp(current)));
	if (draftTemplate)
	{
		Appearance * const app = AppearanceTemplateList::createAppearance (draftTemplate->getCraftedAppearanceFilename ().c_str ());
		draftTemplate->releaseReference ();

		if (app)
		{
			target.setAppearance (app);
			Transceivers::changeSignal.emitMessage (target);
			return;
		}
		else
			WARNING (true, ("invalid draft schematic appearance for manf schem: %lu", current));
	}
	else
		WARNING (true, ("invalid draft schematic for manf schem: %lu", current));

	target.setAppearance (0);
	Transceivers::changeSignal.emitMessage (target);
}

//----------------------------------------------------------------------

void ManufactureSchematicObject::Callbacks::ChangeAppearance::modified (ManufactureSchematicObject & target, const std::string & old, const std::string & current, bool) const
{
	if (old == current)
		return;

	const SharedTangibleObjectTemplate * const customTemplate = dynamic_cast<
		const SharedTangibleObjectTemplate *>(ObjectTemplateList::fetch(current.c_str()));
	if (customTemplate)
	{
		const std::string appearanceName(customTemplate->getAppearanceFilename());

		// see if we need to create a customization property for the schematic
		CustomizationDataProperty *cdProperty = safe_cast<CustomizationDataProperty*>(target.getProperty( CustomizationDataProperty::getClassPropertyId()));
		if (cdProperty == NULL)
			customTemplate->createCustomizationDataPropertyAsNeeded(target, true);
		
		customTemplate->releaseReference ();

		Appearance * const app = AppearanceTemplateList::createAppearance(appearanceName.c_str());
		if (app)
		{
			target.setAppearance (app);
			Transceivers::changeSignal.emitMessage (target);

			if (!target.m_appearanceData.get().empty())
			{
				// reset the custom colors
				std::string tempData(target.m_appearanceData.get());
				target.m_appearanceData = "";
				target.m_appearanceData = tempData;
			}
			return;
		}
		else
			WARNING (true, ("invalid custom appearance for manf schem: %s", current.c_str ()));
	}
	else
		WARNING (true, ("invalid custom template for manf schem: %s", current.c_str ()));

	target.setAppearance (0);
	Transceivers::changeSignal.emitMessage (target);
}

//----------------------------------------------------------------------

void ManufactureSchematicObject::Callbacks::ChangeAppearanceData::modified (ManufactureSchematicObject & target, const std::string & old, const std::string & current, bool) const
{
	if (old == current || current.empty())
		return;

	CustomizationDataProperty *cdProperty = safe_cast<CustomizationDataProperty*>(target.getProperty( CustomizationDataProperty::getClassPropertyId()));
	if (cdProperty == NULL)
	{
		// assume the custom appearance template hasn't been set yet; this function 
		// should automatically get called again when it has
		return;
	}
 
	// retrieve the CustomizationData instance associated with the property
	CustomizationData *const customizationData = cdProperty->fetchCustomizationData();
	NOT_NULL(customizationData);

	//-- initialize CustomizationData variable values from the string
	customizationData->loadLocalDataFromString(current);

	//-- release local reference
	customizationData->release();
}

//----------------------------------------------------------------------

float ManufactureSchematicObject::getExperimentMod(void) const
{
	const ClientManufactureSchematicSynchronizedUi * const sync = safe_cast<const ClientManufactureSchematicSynchronizedUi *>(getClientSynchronizedUi ());
	if (sync)
		return sync->getExperimentMod();
	return 0;
}

//----------------------------------------------------------------------

int ManufactureSchematicObject::getExperimentAttribCount(void) const
{
	const ClientManufactureSchematicSynchronizedUi * const sync = safe_cast<const ClientManufactureSchematicSynchronizedUi *>(getClientSynchronizedUi ());
	if (sync)
		return sync->getExperimentAttribCount();
	return 0;
}

//----------------------------------------------------------------------

bool ManufactureSchematicObject::getExperimentAttribData(int index, StringId & name, float & minValue, float & maxValue, float & resourceMaxValue, float & currentValue) const
{
	const ClientManufactureSchematicSynchronizedUi * const sync = safe_cast<const ClientManufactureSchematicSynchronizedUi *>(getClientSynchronizedUi ());
	if (sync)
		return sync->getExperimentAttribData(index, name, minValue, maxValue, resourceMaxValue, currentValue);
	return false;
}

//----------------------------------------------------------------------

void ManufactureSchematicObject::onIngredientsChanged ()
{
	Transceivers::changeSignal.emitMessage (*this);
	Transceivers::ingredients.emitMessage (*this);
}

//----------------------------------------------------------------------

void ManufactureSchematicObject::onReady ()
{
	Transceivers::ready.emitMessage (*this);
}

//----------------------------------------------------------------------

void ManufactureSchematicObject::onCustomizationChanged     ()
{
}

//----------------------------------------------------------------------

uint32 ManufactureSchematicObject::getDraftSchematicSharedTemplate () const
{
	return m_draftSchematicSharedTemplate.get ();
}

//----------------------------------------------------------------------

const std::string & ManufactureSchematicObject::getCustomAppearanceName () const
{
	return m_customAppearance.get ();
}

const std::string & ManufactureSchematicObject::getCustomAppearanceData () const
{
	return m_appearanceData.get ();
}

//----------------------------------------------------------------------

int ManufactureSchematicObject::getCustomizationCount        () const
{
	const ClientManufactureSchematicSynchronizedUi * const sync = safe_cast<const ClientManufactureSchematicSynchronizedUi *>(getClientSynchronizedUi ());
	if (sync)
		return sync->getCustomizationCount ();
	return 0;
}

//----------------------------------------------------------------------

bool ManufactureSchematicObject::getCustomizationInfo         (int n, std::string & name, int & minIndex, int & index, int & maxIndex) const
{
	const ClientManufactureSchematicSynchronizedUi * const sync = safe_cast<const ClientManufactureSchematicSynchronizedUi *>(getClientSynchronizedUi ());
	if (sync)
		return sync->getCustomizationInfo (n, name, minIndex, index, maxIndex);
	return false;
}

//----------------------------------------------------------------------

bool ManufactureSchematicObject::getCustomizationValue      (int n, int & index) const
{
	const ClientManufactureSchematicSynchronizedUi * const sync = safe_cast<const ClientManufactureSchematicSynchronizedUi *>(getClientSynchronizedUi ());
	if (sync)
	{
		std::string name;
		int minIndex = 0;
		int maxIndex = 0;
		return sync->getCustomizationInfo (n, name, minIndex, index, maxIndex);
	}
	return false;

}

//----------------------------------------------------------------------

int ManufactureSchematicObject::getCustomAppearanceCount   () const
{
	const ClientManufactureSchematicSynchronizedUi * const sync = safe_cast<const ClientManufactureSchematicSynchronizedUi *>(getClientSynchronizedUi ());
	if (sync)
		return static_cast<int>(sync->getAppearances ().size ());
	return 0;
}

//----------------------------------------------------------------------

bool ManufactureSchematicObject::getCustomAppearanceName    (int n, std::string & name) const
{
	if (n < 0)
		return false;

	const ClientManufactureSchematicSynchronizedUi * const sync = safe_cast<const ClientManufactureSchematicSynchronizedUi *>(getClientSynchronizedUi ());
	if (sync)
	{
		const std::vector<std::string> & appNames = sync->getAppearances ();
		if (n < static_cast<int>(appNames.size ()))
		{
			name = appNames [n];
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------

bool ManufactureSchematicObject::setCustomizationInfo       (int n, int value)
{
	if (n < 0)
		return false;

	ClientManufactureSchematicSynchronizedUi * const sync = safe_cast<ClientManufactureSchematicSynchronizedUi *>(getClientSynchronizedUi ());
	if (sync)
	{
		return sync->setCustomizationInfo (n, value);
	}
	return false;
}

//----------------------------------------------------------------------

bool ManufactureSchematicObject::setCustomizationInfo       (const std::string & name, int value)
{
	ClientManufactureSchematicSynchronizedUi * const sync = safe_cast<ClientManufactureSchematicSynchronizedUi *>(getClientSynchronizedUi ());
	if (sync)
	{
		const int n = sync->findCustomizationByName (name);
		if (n >= 0)
		{
			return sync->setCustomizationInfo (n, value);
		}
	}
	return false;
}
//----------------------------------------------------------------------

bool ManufactureSchematicObject::setCustomizationAppearance       (int n)
{
	if (n >= getCustomAppearanceCount ())
		return false;

	m_customAppearanceSelection = n;
	return true;
}

//===================================================================
