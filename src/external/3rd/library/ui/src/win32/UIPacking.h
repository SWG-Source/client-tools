// ======================================================================
//
// UIPacking.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UIPacking_H
#define INCLUDED_UIPacking_H

#pragma warning (disable:4786)

#include "UITypes.h"
#include "UiMemoryBlockManagerMacros.h"

// ======================================================================

namespace UIPacking
{

	//-----------------------------------------------------------------

	struct Rational
	{
		long numerator;
		long denominator;

		Rational () : numerator (1), denominator (1) {}
		Rational (long n, long d) : numerator (n), denominator (d) {}

		inline void  set (long n, long d) { numerator = n; denominator = d; }
	};

	//-----------------------------------------------------------------

	struct SizeInfo
	{
		UI_MEMORY_BLOCK_MANAGER_INTERFACE;
	public:

		enum Type
		{
			Fixed        = 0,
			Absolute     = 1,
			HalfAbsolute = 2,
			Proportional = 3,
			UserDefined  = 4
		};

		static const size_t MAX_AUTO_TYPES;

		Type     m_type;
		Rational m_proportion;
		float    m_userProportion;

		SizeInfo () : m_type (Fixed), m_proportion (2, 3), m_userProportion (0.5) {}
		long        performPacking (int wNear, int wFar, int pFar) const;
		void        resetInfo (long wSize, long pSize);
	};


	//-----------------------------------------------------------------

	struct LocationInfo
	{
		UI_MEMORY_BLOCK_MANAGER_INTERFACE;
	public:

		enum Region
		{
			Near            = 0,
			Center          = 1,
			Far             = 2
		};

		static const size_t MAX_REGIONS;

		enum Type
		{
			Fixed            = 0,
			Proportional     = 1,
			UserDefined      = 2
		};

		static const size_t MAX_AUTO_TYPES;

		Type      m_type;
		Region    m_widgetRegion;
		Region    m_parentRegion;
		Rational  m_proportion;
		float     m_userProportion;
		LocationInfo () : m_type (Fixed), m_widgetRegion (Near), m_parentRegion (Near), m_proportion (), m_userProportion (0.5) {}

		inline long      getFixedDistance () const { return m_proportion.numerator; }
		
		void      findPackingPoints (long wNear, long wFar, long pFar, long & wPoint, long & pPoint) const;
		void      resetInfo (long wNear, long wFar, long pFar);
		long      performPacking              (int wNear,int wFar,int pFar) const;

		bool      rulesMatch (const LocationInfo & rhs) const;

	};
	
	//-----------------------------------------------------------------
	//-----------------------------------------------------------------
	//-----------------------------------------------------------------
	//-----------------------------------------------------------------

	class Manager
	{
	public:

		Manager ();
		~Manager ();

		/*
		struct Hasher
		{
			std::hash<const char *> m_hash;

			inline size_t operator () (const UINarrowString & arg1) const
			{
				return m_hash (arg1.c_str ());
			}
		};
*/


		struct CompareNoCase
		{
			inline bool operator () (const std::string & arg1, const std::string & arg2) const
			{
				return _stricmp (arg1.c_str (), arg2.c_str ()) < 0;
			}
		};

		typedef ui_stdmap <std::string, SizeInfo *, CompareNoCase>::fwd     SizeInfoMap_t;
		typedef ui_stdmap <std::string, LocationInfo *, CompareNoCase>::fwd LocationInfoMap_t;

		const LocationInfo *  getLocationInfo (const std::string & str) const;
		const SizeInfo *      getSizeInfo     (const std::string & str) const;

	private:
		Manager (const Manager & rhs);
		Manager &        operator=    (const Manager & rhs);

		SizeInfoMap_t *         m_sizeInfoMap;
		LocationInfoMap_t *     m_locationInfoMap;
		
	};
	
	//-----------------------------------------------------------------

}
// ======================================================================


#endif
