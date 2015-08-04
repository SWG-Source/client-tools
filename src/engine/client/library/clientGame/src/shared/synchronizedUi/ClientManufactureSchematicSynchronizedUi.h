// ======================================================================
//
// ClientManufactureSchematicSynchronizedUi.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_ClientManufactureSchematicSynchronizedUi_H
#define	INCLUDED_ClientManufactureSchematicSynchronizedUi_H

#include "Archive/AutoDeltaVariableCallback.h"
#include "Archive/AutoDeltaVector.h"
#include "clientGame/ClientSynchronizedUi.h"


class ManufactureSchematicObject;

namespace Crafting
{
	struct IngredientSlot;
};

//----------------------------------------------------------------------

class ClientManufactureSchematicSynchronizedUi : public ClientSynchronizedUi
{
public:

	struct Messages
	{
		struct Ready;
		struct Ingredients;
		struct Customization;
	};

	explicit ClientManufactureSchematicSynchronizedUi (ManufactureSchematicObject &);
	        ~ClientManufactureSchematicSynchronizedUi ();

	int   getIngredientCount () const;
	bool  getIngredient      (int index, Crafting::IngredientSlot & data) const;
	bool  getIngredient      (const StringId & name, Crafting::IngredientSlot & data) const;

	float getExperimentMod(void) const;

	int   getExperimentAttribCount(void) const;
	bool  getExperimentAttribData(int index, StringId & name, float & minValue, float & maxValue, float & resourceMaxValue, float & currentValue) const;

	const std::vector<std::string> & getAppearances () const;

	int                              getCustomizationCount () const;
	bool                             getCustomizationInfo  (int n, std::string & name, int & minIndex, int & currentIndex, int & maxIndex) const;

	bool                             setCustomizationInfo       (int n, int value);
	int                              findCustomizationByName    (const std::string & name);

	bool                             setCustomizationAppearance (int n);

private:
	ClientManufactureSchematicSynchronizedUi(const ClientManufactureSchematicSynchronizedUi& );
	ClientManufactureSchematicSynchronizedUi();
	ClientManufactureSchematicSynchronizedUi& operator= (const ClientManufactureSchematicSynchronizedUi &);

private:

	struct Callbacks
	{
		template <typename S, typename U> struct DefaultCallback
		{
			void modified (ClientManufactureSchematicSynchronizedUi & target, const U & old, const U & value, bool isLocal) const;
		};

		typedef DefaultCallback<Messages::Ready,         bool>  Ready;
		typedef DefaultCallback<Messages::Ingredients,   uint8> Ingredients;
		typedef DefaultCallback<Messages::Customization, uint8> Customization;
	};


	typedef std::vector<NetworkId> NetworkIdList;
	typedef std::vector<int>       IntList;

	// The following fields are parallel arrays for ingredients
	Archive::AutoDeltaVector<StringId>          m_slotName;            // slot name @todo: can we make this a string?
	Archive::AutoDeltaVector<int>               m_slotType;            // enum Crafting::IngredientType
	Archive::AutoDeltaVector<NetworkIdList, ClientManufactureSchematicSynchronizedUi>     m_slotIngredient;      // actual ingredients used per slot
	Archive::AutoDeltaVector<IntList>           m_slotIngredientCount; // number of ingredients used per slot
	Archive::AutoDeltaVector<float>             m_slotComplexity;      // cuurent slot complexity
	Archive::AutoDeltaVector<int>               m_slotDraftOption;     // the draft schematic slot option
	Archive::AutoDeltaVector<int>               m_slotDraftIndex;      // the draft schematic slot index
	Archive::AutoDeltaVariableCallback<uint8, Callbacks::Ingredients, ClientManufactureSchematicSynchronizedUi>           m_slotIngredientsChanged; // signal

	// The following fields are parallel arrays for experimentation
	Archive::AutoDeltaVector<StringId>      m_attributeName;        // attributes that can be experimented with
	Archive::AutoDeltaVector<float>         m_attributeValue;       // current value of the attributes
	Archive::AutoDeltaVector<float>         m_minAttribute;         // min value of the attribute
	Archive::AutoDeltaVector<float>         m_maxAttribute;         // max value of the attribute
	Archive::AutoDeltaVector<float>         m_resourceMaxAttribute; // max value of the attribute due to resource selection

	// The following fields are parallel arrays for customizations
	Archive::AutoDeltaVector<std::string>   m_customName;       // customization property name
	Archive::AutoDeltaVector<int>           m_customIndex;      // palette/decal current index
	Archive::AutoDeltaVector<int>           m_customMinIndex;   // palette/decal minimum index
	Archive::AutoDeltaVector<int>           m_customMaxIndex;   // palette/decal maximum index
	Archive::AutoDeltaVariableCallback<uint8, Callbacks::Customization, ClientManufactureSchematicSynchronizedUi>       m_customChanged;    // used to signal changes

	Archive::AutoDeltaVariable<float>     m_experimentMod;      // appoximent mod that will be applied to experimentation tries
	Archive::AutoDeltaVector<std::string> m_appearance;         // list of appearances the player may choose from

	Archive::AutoDeltaVariableCallback<bool, Callbacks::Ready, ClientManufactureSchematicSynchronizedUi> m_ready;
}; 

//----------------------------------------------------------------------


inline float ClientManufactureSchematicSynchronizedUi::getExperimentMod(void) const
{
	return m_experimentMod.get();
}

inline int ClientManufactureSchematicSynchronizedUi::getExperimentAttribCount(void) const
{
	return m_attributeName.size();
}

inline const std::vector<std::string> & ClientManufactureSchematicSynchronizedUi::getAppearances () const
{
	return m_appearance.get();
}


//----------------------------------------------------------------------

#endif
