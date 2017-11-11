// ======================================================================
//
// DraftSchematic.h
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_DraftSchematic_H
#define INCLUDED_DraftSchematic_H

// ======================================================================

class DraftSchematic
{
public:

	class Attribute
	{
	public:

		enum ValueType
		{
			VT_integer,
			VT_armorRating,
			VT_damageType
		};

	public:

		Attribute ();
		~Attribute ();

		CString getName () const;

	public:

		CString m_nameStringTable;
		CString m_nameStringId;
		CString m_experimentStringTable;
		CString m_experimentStringId;
		ValueType m_valueType;
		CString m_minimumValue;
		CString m_maximumValue;
	};

	class Slot
	{
	public:

		Slot ();
		~Slot ();

		CString getName () const;

	public:

		bool m_optional;
		CString m_nameStringTable;
		CString m_nameStringId;
		CString m_ingredientType;
		CString m_ingredientName;
		int m_ingredientCount;
	};

public:

	static CString createServerObjectTemplateName (CString const & pathName);
	static CString createSharedObjectTemplateName (CString const & pathName);
	static void createDefaultAttribute (DraftSchematic::Attribute & attribute);
	static void createDefaultSlot (DraftSchematic::Slot & slot);

public:

	DraftSchematic ();
	~DraftSchematic ();

	bool load (char const * rootName);
	bool save (char const * rootName);

	CString const & getBaseServerObjectTemplate () const;
	void setBaseServerObjectTemplate (CString const & baseServerObjectTemplate);
	CString const & getBaseSharedObjectTemplate () const;
	void setBaseSharedObjectTemplate (CString const & baseSharedObjectTemplate);
	int getItemsPerContainer () const;
	void setItemsPerContainer (int itemsPerContainer);
	CString const & getCraftedServerObjectTemplate () const;
	void setCraftedServerObjectTemplate (CString const & craftedServerObjectTemplate);
	CString const & getCraftedSharedObjectTemplate () const;
	void setCraftedSharedObjectTemplate (CString const & craftedSharedObjectTemplate);
	CString const & getManufactureScript () const;
	void setManufactureScript (CString const & manufactureScript);
	int getComplexity () const;
	void setComplexity (int complexity);

	int getNumberOfSlots () const;
	Slot const * getSlot (int index) const;
	Slot * getSlot (int index);
	void addSlot (Slot * slot);
	void removeSlot (Slot * slot);

	int getNumberOfAttributes () const;
	Attribute const * getAttribute (int index) const;
	Attribute * getAttribute (int index);
	void addAttribute (Attribute * attribute);
	void removeAttribute (Attribute * attribute);

	bool getOverrideCraftingType () const;
	void setOverrideCraftingType (bool overrideCraftingType);
	CString const & getOverrideCraftingTypeValue () const;
	void setOverrideCraftingTypeValue (CString const & overrideCraftingTypeValue);
	bool getOverrideDestroyIngredients () const;
	void setOverrideDestroyIngredients (bool overrideDestroyIngredients);
	bool getOverrideDestroyIngredientsValue () const;
	void setOverrideDestroyIngredientsValue (bool overrideDestroyIngredientsValue);
	bool getOverrideXpType () const;
	void setOverrideXpType (bool overrideXpType);
	CString const & getOverrideXpTypeValue () const;
	void setOverrideXpTypeValue (CString const & overrideXpTypeValue);

	CString const & getOldServerObjectTemplate () const;
	CString const & getOldSharedObjectTemplate () const;
	CString generateServerObjectTemplate (CString const & serverObjectTemplateName, bool header = true) const;
	CString generateSharedObjectTemplate (CString const & sharedObjectTemplateName, bool header = true) const;

private:

	void reset ();
	bool parseServerObjectTemplate (CString file);
	bool parseSharedObjectTemplate (CString file);

private:

	//-- server tpf
	CString m_baseServerObjectTemplate;
	int m_itemsPerContainer;
	CString m_craftedServerObjectTemplate;

	typedef std::vector<Slot *> SlotList;
	SlotList m_slotList;

	CString m_manufactureScript;
	int m_complexity;

	bool m_overrideCraftingType;
	CString m_overrideCraftingTypeValue;

	bool m_overrideDestroyIngredients;
	bool m_overrideDestroyIngredientsValue;

	bool m_overrideXpType;
	CString m_overrideXpTypeValue;

	//-- shared tpf
	CString m_baseSharedObjectTemplate;

	typedef std::vector<Attribute *> AttributeList;
	AttributeList m_attributeList;

	CString m_craftedSharedObjectTemplate;

	//-- debug
	CString m_oldServerObjectTemplate;
	CString m_oldSharedObjectTemplate;
};

// ======================================================================

#endif
