//======================================================================
//
// UIPropertyCategories.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIPropertyCategories_H
#define INCLUDED_UIPropertyCategories_H

//======================================================================

class UILowerString;

namespace UIPropertyCategories
{
	// ----------------------------------------------------

	enum Category
	{
		C_ALL_CATEGORIES = -1,

		C_Basic,
		C_Text,
		C_Icon,
		C_Behavior,
		C_AdvancedBehavior,
		C_Appearance,
		C_AdvancedAppearance,
		C_UserDefined,

		C_NUM_CATEGORIES
	};

	// ----------------------------------------------------

	extern const UILowerString s_propertyCategoryNames[];

	// ----------------------------------------------------

	class CategoryMask
	{
	public:
		CategoryMask()                      : m_mask(0)        {}
		CategoryMask(const CategoryMask &x) : m_mask(x.m_mask) {}

		void setHasBasic(bool i)              { _setBit(C_Basic, i); }
		void setHasText(bool i)               { _setBit(C_Text, i); }
		void setHasIcon(bool i)               { _setBit(C_Icon, i); }
		void setHasBehavior(bool i)           { _setBit(C_Behavior, i); }
		void setHasAdvancedBehavior(bool i)   { _setBit(C_AdvancedBehavior, i); }
		void setHasAppearance(bool i)         { _setBit(C_Appearance, i); }
		void setHasAdvancedAppearance(bool i) { _setBit(C_AdvancedAppearance, i); }
		void setHasUserDefined(bool i)        { _setBit(C_UserDefined, i); }

		bool getHasBasic()              const { return _getBit(C_Basic); }
		bool getHasText()               const { return _getBit(C_Text); }
		bool getHasIcon()               const { return _getBit(C_Icon); }
		bool getHasBehavior()           const { return _getBit(C_Behavior); }
		bool getHasAdvancedBehavior()   const { return _getBit(C_AdvancedBehavior); }
		bool getHasAppearance()         const { return _getBit(C_Appearance); }
		bool getHasAdvancedAppearance() const { return _getBit(C_AdvancedAppearance); }
		bool getHasUserDefined()        const { return _getBit(C_UserDefined); }

		bool getHasCategory(Category c) const { return _getBit(int(c)); }

		void _setBit(int bit, bool value)
		{
			if (value)
			{
				m_mask|=(1<<bit);
			}
			else
			{
				m_mask&=~(1<<bit);
			}
		}

		bool _getBit(int bit) const
		{
			return (m_mask&(1<<bit))!=0;
		}

		unsigned m_mask;
	};
}

//======================================================================

#endif
