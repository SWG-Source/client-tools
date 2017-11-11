//===================================================================
//
// ManufactureSchematicObject.h
// copyright 2001, Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_ManufactureSchematicObject_H
#define INCLUDED_ManufactureSchematicObject_H

//===================================================================

#include "Archive/AutoDeltaMap.h"
#include "clientGame/IntangibleObject.h"
#include "sharedObject/CachedNetworkId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkIdArchive.h"

//===================================================================

class SharedManufactureSchematicObjectTemplate;
class StringId;

namespace Crafting
{
	struct IngredientSlot;
};


//===================================================================

class ManufactureSchematicObject : public IntangibleObject
{
public:

	struct Messages
	{
		struct ChangeSignal;
		struct Ingredients;
		struct Customization;
		struct Ready;
	};

	explicit                    ManufactureSchematicObject (const SharedManufactureSchematicObjectTemplate* newTemplate);
	virtual                    ~ManufactureSchematicObject ();

	virtual void                endBaselines();

	bool                        isReadyToCraft(void) const;

	void                        transferToSlot             (const NetworkId & id, int slot, int option, uint8 sequence);

	int                         getIngredientCount         () const;
	bool                        getIngredient              (int index, Crafting::IngredientSlot & data) const;
	bool                        getIngredient              (const StringId & name, Crafting::IngredientSlot & data) const;

	typedef stdvector<StringId>::fwd StringIdVector;

	float                       getExperimentMod(void) const;

	int                         getExperimentAttribCount(void) const;
	bool                        getExperimentAttribData(int index, StringId & name, float & minValue, float & maxValue, float & resourceMaxValue, float & currentValue) const;

	int                         getCustomizationCount      () const;
	bool                        getCustomizationInfo       (int n, std::string & name, int & minIndex, int & index, int & maxIndex) const;
	bool                        getCustomizationValue      (int n, int & index) const;
	int                         getCustomAppearanceCount   () const;
	bool                        getCustomAppearanceName    (int n, std::string & appearanceName) const;

	bool                        setCustomizationInfo       (int n, int value);
	bool                        setCustomizationInfo       (const std::string & name, int value);

	bool                        setCustomizationAppearance (int n);
	int                         getCustomizationAppearance () const;

	void                        onCustomizationChanged     ();
	void                        onIngredientsChanged       ();
	void                        onReady                    ();

	uint32                      getDraftSchematicSharedTemplate () const;
	const std::string &         getCustomAppearanceName () const;
	const std::string &         getCustomAppearanceData () const;

protected:
	virtual ClientSynchronizedUi * createSynchronizedUi ();

private:
 	                            ManufactureSchematicObject ();
	                            ManufactureSchematicObject (const ManufactureSchematicObject& rhs);
	ManufactureSchematicObject&	operator=                  (const ManufactureSchematicObject& rhs);

private:

	//----------------------------------------------------------------------
	//-- AutoDeltaVariable Callbacks

	struct Callbacks
	{
		template <typename U> struct DefaultCallback
		{
			void modified (ManufactureSchematicObject & target, const U & old, const U & value, bool isLocal) const;
		};

		typedef DefaultCallback<uint8>   ChangeSignal;
		typedef DefaultCallback<uint32>  ChangeDraft;
		
		template <typename U> struct AppearanceCallback
		{
			void modified (ManufactureSchematicObject & target, const U & old, const U & value, bool isLocal) const;
		};
		typedef AppearanceCallback<std::string>      ChangeAppearance;

		template <typename U> struct AppearanceDataCallback
		{
			void modified (ManufactureSchematicObject & target, const U & old, const U & value, bool isLocal) const;
		};
		typedef AppearanceDataCallback<std::string>  ChangeAppearanceData;
	};

	friend Callbacks::ChangeSignal;
	friend Callbacks::ChangeAppearance;

	typedef Archive::AutoDeltaVariable<Unicode::String> AutoDeltaUnicodeString;
	typedef Archive::AutoDeltaVariable<std::string> AutoDeltaString;
	typedef Archive::AutoDeltaVariableCallback<uint32, Callbacks::ChangeDraft, ManufactureSchematicObject> AutoDeltaDraftCallback;
	typedef Archive::AutoDeltaVariableCallback<std::string, Callbacks::ChangeAppearance, ManufactureSchematicObject> AutoDeltaAppearanceCallback;
	typedef Archive::AutoDeltaVariableCallback<std::string, Callbacks::ChangeAppearanceData, ManufactureSchematicObject> AutoDeltaAppearanceDataCallback;

	// persisted vars
	Archive::AutoDeltaVariable<int>                       m_itemsPerContainer;
	Archive::AutoDeltaMap<StringId, float>                m_attributes;
	Archive::AutoDeltaVariable<float>                     m_manufactureTime;

	// not persisted vars
	AutoDeltaDraftCallback                                m_draftSchematicSharedTemplate;
	AutoDeltaAppearanceCallback                           m_customAppearance;   // this is the custom appearance template
	AutoDeltaAppearanceDataCallback                       m_appearanceData;     // this is the custom colors
	Archive::AutoDeltaVariable<bool>                      m_isCrafting;         // flag that this schematic is being used in a crafing session
	Archive::AutoDeltaVariableCallback<uint8, Callbacks::ChangeSignal, ManufactureSchematicObject> m_schematicChangedSignal;

	int                                                   m_customAppearanceSelection;
};


//----------------------------------------------------------------------

inline bool ManufactureSchematicObject::isReadyToCraft(void) const
{
	return getIngredientCount() > 0;
}

//----------------------------------------------------------------------

inline int ManufactureSchematicObject::getCustomizationAppearance () const
{
	return m_customAppearanceSelection;
}

//===================================================================

#endif
