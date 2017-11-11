// ======================================================================
//
// ClientManufactureSchematicSynchronizedUi.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ClientManufactureSchematicSynchronizedUi.h"

#include "clientGame/InstallationObject.h"
#include "clientGame/ManufactureSchematicObject.h"
#include "clientUserInterface/CuiCraftManager.h"
#include "sharedGame/CraftingData.h"
#include "sharedMessageDispatch/Transceiver.h"


//-----------------------------------------------------------------------

/**
 * Class constructor.
 *
 * @param owner		manf schematic creating this class
 */

ClientManufactureSchematicSynchronizedUi::ClientManufactureSchematicSynchronizedUi (ManufactureSchematicObject & owner) :
	ClientSynchronizedUi (owner),
	m_slotName(),
	m_slotType(),
	m_slotIngredient(),
	m_slotIngredientCount(),
	m_slotComplexity(),
	m_slotDraftOption(),
	m_slotDraftIndex(),
	m_slotIngredientsChanged (0),
	m_attributeName(),
	m_attributeValue(),
	m_minAttribute(),
	m_maxAttribute(),
	m_resourceMaxAttribute(),
	m_customName(),
	m_customIndex(),
	m_customMinIndex(),
	m_customMaxIndex(),
	m_customChanged(0),
	m_experimentMod(0),
	m_appearance(),
	m_ready(false)
{
	addToUiPackage(m_slotName);
	addToUiPackage(m_slotType);
	addToUiPackage(m_slotIngredient);
	addToUiPackage(m_slotIngredientCount);
	addToUiPackage(m_slotComplexity);
	addToUiPackage(m_slotDraftOption);
	addToUiPackage(m_slotDraftIndex);
	addToUiPackage(m_slotIngredientsChanged);
	addToUiPackage(m_attributeName);
	addToUiPackage(m_attributeValue);
	addToUiPackage(m_minAttribute);
	addToUiPackage(m_maxAttribute);
	addToUiPackage(m_resourceMaxAttribute);
	addToUiPackage(m_customName);
	addToUiPackage(m_customIndex);
	addToUiPackage(m_customMinIndex);
	addToUiPackage(m_customMaxIndex);
	addToUiPackage(m_customChanged);
	addToUiPackage(m_experimentMod);
	addToUiPackage(m_appearance);
	addToUiPackage(m_ready);

	m_ready.setSourceObject(this);
	m_slotIngredientsChanged.setSourceObject(this);
	m_customChanged.setSourceObject(this);
}

//-----------------------------------------------------------------------

/**
 * Class destructor.
 */
ClientManufactureSchematicSynchronizedUi::~ClientManufactureSchematicSynchronizedUi()
{
}

//-----------------------------------------------------------------------

void ClientManufactureSchematicSynchronizedUi::Callbacks::Ready::modified(ClientManufactureSchematicSynchronizedUi & target, const bool & , const bool & , bool) const
{
	ManufactureSchematicObject * const manf = static_cast<ManufactureSchematicObject * >(target.getOwner ());
	if (manf)
		manf->onReady ();
}

//----------------------------------------------------------------------

void ClientManufactureSchematicSynchronizedUi::Callbacks::Ingredients::modified(ClientManufactureSchematicSynchronizedUi & target, const uint8 & , const uint8 & , bool) const
{
	ManufactureSchematicObject * const manf = static_cast<ManufactureSchematicObject * >(target.getOwner ());
	if (manf)
		manf->onIngredientsChanged ();
}

//----------------------------------------------------------------------

void ClientManufactureSchematicSynchronizedUi::Callbacks::Customization::modified(ClientManufactureSchematicSynchronizedUi & target, const uint8 & , const uint8 & , bool) const
{
	ManufactureSchematicObject * const manf = static_cast<ManufactureSchematicObject * >(target.getOwner ());
	if (manf)
		manf->onCustomizationChanged ();
}
//-----------------------------------------------------------------------

/**
 * Returns the number of ingredient slots available for the schematic.
 *
 * @return the number of slots
 */
int ClientManufactureSchematicSynchronizedUi::getIngredientCount() const
{
	return m_slotName.size();
}	// ClientManufactureSchematicSynchronizedUi::getIngredientCount

//-----------------------------------------------------------------------

/**
 * Returns data about a schematic slot.
 *
 * @param index		index of the slot to get
 * @param data		structure to fill with the slot data
 *
 * @return true if data is filled with valid data, false if not
 */
bool ClientManufactureSchematicSynchronizedUi::getIngredient(int index, Crafting::IngredientSlot & data) const
{
	if (index < 0 || index >= static_cast<int>(m_slotName.size()))
		return false;

	data.ingredientType = static_cast<Crafting::IngredientType>(m_slotType[index]);
	data.name = m_slotName[index];
	data.complexity = m_slotComplexity[index];
	data.draftSlotOption = m_slotDraftOption[index];
	data.draftSlotIndex = m_slotDraftIndex[index];
	
	const NetworkIdList & ingredientIds = m_slotIngredient[index];
	const IntList & ingredientCounts = m_slotIngredientCount[index];
	int count = ingredientIds.size();

	data.ingredients.clear ();

	for (int i = 0; i < count; ++i)
	{
		data.ingredients.push_back(Crafting::SimpleIngredientPtr(
			new Crafting::SimpleIngredient(CachedNetworkId(ingredientIds[i]), 
			ingredientCounts[i], NetworkId::cms_invalid, 0)));
	}

	return true;
}	// ClientManufactureSchematicSynchronizedUi::getIngredient(int)

//-----------------------------------------------------------------------

/**
 * Returns data about a schematic slot.
 *
 * @param name		name of the slot to get
 * @param data		structure to fill with the slot data
 *
 * @return true if data is filled with valid data, false if not
 */
bool ClientManufactureSchematicSynchronizedUi::getIngredient(const StringId & name, 
	Crafting::IngredientSlot & data) const
{
	int index = m_slotName.find(name);
	if (index < 0)
		return false;

	return getIngredient(index, data);
}	// ClientManufactureSchematicSynchronizedUi::getIngredient(const StringId &)

//----------------------------------------------------------------------

/**
 *
 */
bool ClientManufactureSchematicSynchronizedUi::getExperimentAttribData(int index, 
	StringId & name, float & minValue, float & maxValue, float & resourceMaxValue, 
	float & currentValue) const
{
	if (index < 0 || index >= static_cast<int>(m_attributeName.size()))
		return false;
	name = m_attributeName[index];
	minValue = m_minAttribute[index];
	maxValue = m_maxAttribute[index];
	resourceMaxValue = m_resourceMaxAttribute[index];
	currentValue = m_attributeValue[index];
	return true;
}

//----------------------------------------------------------------------

int ClientManufactureSchematicSynchronizedUi::getCustomizationCount () const
{
	DEBUG_FATAL (m_customName.size () != m_customIndex.size () ||
		m_customName.size () != m_customMinIndex.size () ||
		m_customName.size () != m_customMaxIndex.size (), ("bad vector"));

	return static_cast<int>(m_customName.size ());
}

//----------------------------------------------------------------------

bool ClientManufactureSchematicSynchronizedUi::getCustomizationInfo  (int n, std::string & name, int & minIndex, int & currentIndex, int & maxIndex) const
{
	DEBUG_FATAL (m_customName.size () != m_customIndex.size () ||
		m_customName.size () != m_customMinIndex.size () ||
		m_customName.size () != m_customMaxIndex.size (), ("bad vector"));

	if (n < 0 || n >= static_cast<int>(m_customName.size ()))
		return false;

	name         = m_customName     [n];
	currentIndex = m_customIndex    [n];
	minIndex     = m_customMinIndex [n];
	maxIndex     = m_customMaxIndex [n];

	return true;
}

//----------------------------------------------------------------------

bool ClientManufactureSchematicSynchronizedUi::setCustomizationInfo       (int n, int value)
{
	DEBUG_FATAL (m_customName.size () != m_customIndex.size () ||
		m_customName.size () != m_customMinIndex.size () ||
		m_customName.size () != m_customMaxIndex.size (), ("bad vector"));

	if (n < 0 || n >= static_cast<int>(m_customName.size ()))
		return false;

	m_customIndex.set  (n, value);

	return true;
}

//----------------------------------------------------------------------

int ClientManufactureSchematicSynchronizedUi::findCustomizationByName    (const std::string & name)
{
	DEBUG_FATAL (m_customName.size () != m_customIndex.size () ||
		m_customName.size () != m_customMinIndex.size () ||
		m_customName.size () != m_customMaxIndex.size (), ("bad vector"));

	const int count = static_cast<int>(m_customName.size ());

	for (int i = 0; i < count; ++i)
	{
		if (m_customName.get (i) == name)
			return i;
	}

	return -1;
}

//----------------------------------------------------------------------

	bool                             setCustomizationAppearance (int n);

//-----------------------------------------------------------------------

