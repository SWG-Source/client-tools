#ifndef __UIUTILS_H__
#define __UIUTILS_H__

#include "UIString.h"
#include "UIStyle.h"
#include "UITypes.h"
#include "UnicodeUtils.h"

class UIBaseObject;

static const UIString::value_type whitespace [] = { ' ', '\n', '\t', 0 };

namespace UIUtils
{
	
	typedef ui_stdlist<long>::fwd LongList;

	void RunTimeDiagnosticDump( const UIBaseObject *DumpingObject, const char *Message, const UIBaseObject *ContextObject = 0 );

	template <class T> inline unsigned long hextoi( const T *p )
	{
		unsigned long r = 0;
		for( ; *p; ++p )
		{
			r <<= 4;
			
			if( *p >= '0' && *p <= '9' )
				r |= *p - '0';
			else if( *p >= 'a' && *p <= 'f' )
				r |= *p - 'a' + 10;
			else if( *p >= 'A' && *p <= 'F' )
				r |= *p - 'A' + 10;
		}
		return r;
	}

	bool					ParseBoolean   ( const UIString &, bool &    );
	bool					ParseLong      ( const UIString &, long &    );
	bool					ParseInteger   ( const UIString &, int  &    );
	bool					ParseFloat     ( const UIString &, float &   );
	bool					ParsePoint     ( const UIString &, UIPoint & );
	bool					ParseFloatPoint( const UIString &, UIFloatPoint & );
	bool					ParseRect      ( const UIString &, UIRect &  );
	bool					ParseColor     ( const UIString &, UIColor & );	
	bool					ParseColorOrPalette ( const UIString &, UIColor & );	
	bool					ParseColorARGB ( const UIString &, UIColor & );	
	int                     ParseTwoTokens ( const UIString &, UIString [] );
	int						ParseLongVector( const UIString &, ui_stdvector<long>::fwd &);

	/// Delimited integer format utilities
	Unicode::String			FormatDelimitedInteger ( Unicode::String const & in);
	void					SetDigitGroupingSymbol ( const char newSymbol );
	// End delimited integer format utilities

	bool					ParseLayout     ( const UIString &, UIStyle::Layout &  );

	bool					FormatBoolean   ( UIString &, const bool      );
	bool					FormatLong      ( UIString &, const long      );
	bool					FormatInteger   ( UIString &, const int       );
	bool					FormatFloat     ( UIString &, const float     );
	bool					FormatPoint     ( UIString &, const UIPoint & );
	bool					FormatFloatPoint( UIString &, const UIFloatPoint & );
	bool					FormatRect      ( UIString &, const UIRect &  );
	bool					FormatColor     ( UIString &, const UIColor & );
	bool					FormatColorARGB ( UIString &, const UIColor & );
	bool					FormatLayout    ( UIString &, const UIStyle::Layout    );
	bool					FormatLongList  ( UIString &, const LongList &);

	// ClipRect returns false if there is no intersection
	inline bool ClipRect( UIRect &RectToClip, UIScalar left, UIScalar top, UIScalar right, UIScalar bottom )
	{
		if( RectToClip.top >= bottom )
			return false;
		if( RectToClip.left >= right )
			return false;
		if( RectToClip.bottom <= top )
			return false;
		if( RectToClip.right <= left )
			return false;

		if( RectToClip.top < top )
			RectToClip.top = top;
		if( RectToClip.left < left )
			RectToClip.left = left;
		if( RectToClip.bottom > bottom )
			RectToClip.bottom = bottom;
		if( RectToClip.right > right )
			RectToClip.right = right;

		return true;
	}

	inline bool ClipRect( UIRect &RectToClip, const UIRect &RectToClipAgainst )
	{
		return ClipRect( RectToClip, RectToClipAgainst.left, RectToClipAgainst.top, RectToClipAgainst.right, RectToClipAgainst.bottom );
	}

	inline bool RectanglesIntersect( const UIRect &r1, const UIRect &r2 )
	{
		if( r1.top > r2.bottom )
			return false;
		if( r1.left > r2.right )
			return false;
		if( r1.bottom < r2.top )
			return false;
		if( r1.right < r2.left )
			return false;

		return true;
	}

	inline void UnionRect( UIRect &RectToUnion, const UIRect &RectToAddToUnion )
	{
		if( RectToAddToUnion.top < RectToUnion.top )
			RectToUnion.top = RectToAddToUnion.top;
		if( RectToAddToUnion.left < RectToUnion.left )
			RectToUnion.left = RectToAddToUnion.left;
		if( RectToAddToUnion.bottom > RectToUnion.bottom )
			RectToUnion.bottom = RectToAddToUnion.bottom;
		if( RectToAddToUnion.right > RectToUnion.right )
			RectToUnion.right = RectToAddToUnion.right;
	}
	
	
	//======================================================================================
	bool FormatBoolean(UINarrowString & result, const bool b);
	bool FormatLong(UINarrowString & result, const long val);
	bool FormatFloat(UINarrowString & result, const float f);
	bool FormatPoint(UINarrowString & result, const UIPoint & pt);
	bool FormatRect(UINarrowString & result, const UIRect & rect);
	bool FormatColor(UINarrowString & result, const UIColor & color);
	bool FormatColorARGB(UINarrowString & result, const UIColor & color);
	bool FormatLayout(UINarrowString & result, const UIStyle::Layout lay);

	//-----------------------------------------------------------------
	inline bool ParseBoolean ( const UINarrowString & str, bool & val   )
	{
		return ParseBoolean (Unicode::narrowToWide (str), val);
	}
	//-----------------------------------------------------------------
	
	inline bool	ParseLong ( const UINarrowString & str, long &  val  )
	{
		return ParseLong (Unicode::narrowToWide (str), val);
	}
	//-----------------------------------------------------------------
	
	inline bool	ParseFloat   ( const UINarrowString & str, float & val )
	{
		return ParseFloat (Unicode::narrowToWide (str), val);
	}
	//-----------------------------------------------------------------
	
	inline bool	ParsePoint   ( const UINarrowString & str, UIPoint & val )
	{
		return ParsePoint (Unicode::narrowToWide (str), val);
	}
	//-----------------------------------------------------------------
	
	inline bool	ParseRect    ( const UINarrowString & str, UIRect & val )
	{
		return ParseRect (Unicode::narrowToWide (str), val);
	}
	//-----------------------------------------------------------------
	
	inline bool	ParseColor   ( const UINarrowString & str, UIColor & val )	
	{
		return ParseColor (Unicode::narrowToWide (str), val);
	}
	//-----------------------------------------------------------------
	
	inline bool	ParseColorARGB   ( const UINarrowString & str, UIColor & val )	
	{
		return ParseColorARGB (Unicode::narrowToWide (str), val);
	}

	//----------------------------------------------------------------------

	inline bool FormatInteger ( UIString &Out, const int In )
	{
		return FormatLong (Out, static_cast<long>(In));
	}

	//----------------------------------------------------------------------
	
	inline bool ParseInteger( const UIString &s, int &in)
	{
		long tmp;
		
		if (ParseLong (s, tmp))
		{
			in = static_cast<int>(tmp);
			return true;
		}
		
		return false;
	}

	//----------------------------------------------------------------------

	bool MovePointLeftOneWord  (const Unicode::String & str, int startpos, int & newpos);
	bool MovePointRightOneWord (const Unicode::String & str, int startpos, int & newpos);

	//-----------------------------------------------------------------

	// ======================================================================
	/**
	 * Return the square of a number.
	 *
	 * This routine is templated, and implemented in terms of the * operator.
	 *
	 * @return The square of its argument
	 */
	template<class T>
	inline T sqr(const T &t)
	{
		return (t * t);
	}

	// ----------------------------------------------------------------------
	/**
	 * Make sure value is between two values.
	 * 
	 * This routine is templated, and implemented in terms of the > and > operators.
	 * 
	 * @param minT  Minimum variable
	 * @param t  Variable to clamp against min and max
	 * @param maxT  Minimum variable
	 * @return If t is less than min, return min; if t is greater than max, return max
	 * otherwise, return t
	 */

	template<class T>inline const T clamp(const T &minT, const T &t, const T &maxT)
	{
		return (t < minT) ? minT : ((t > maxT) ? maxT : t);
	}

	// ----------------------------------------------------------------------
	/**
	 * returns whether a value is within a given range, including the range end points.
	 * 
	 * This routine will fatal in debug builds if max is less than min.
	 * 
	 * @return True if value falls within the range min .. max, including min and max,
	 * false otherwise.
	 */

	template<class T>inline bool WithinRangeInclusiveInclusive(const T &rangeMin, const T &value, const T &rangeMax)
	{
		return (value >= rangeMin) && (value <= rangeMax);
	}

	// ----------------------------------------------------------------------
	/**
	 * returns whether a value is within a given range, excluding the range end points.
	 * 
	 * This routine will fatal in debug builds if max is less than min.
	 * 
	 * @return True if value falls within the range min .. max, not including min and max,
	 * false otherwise.
	 */

	template<class T>inline bool WithinRangeExclusiveExclusive(const T &rangeMin, const T &value, const T &rangeMax)
	{
		return (value > rangeMin) && (value < rangeMax);
	}

	// ----------------------------------------------------------------------
	/**
	 * returns true if a value falls within the range (base - epsilon) .. (base + epsilon),
	 * including range endpoints.
	 * 
	 * This routine will fatal during debug builds if epsilon is less than zero.
	 * 
	 * @return true if value is within the range (base - epsilon) .. (base + epsilon), including
	 * the range endpoints.  Otherwise returns false.
	 */

	template<class T>inline bool WithinEpsilonInclusive(const T &base, const T &value, const T &epsilon)
	{
		return (value >= base - epsilon) && (value <= base + epsilon);
	}

	// ----------------------------------------------------------------------
	/**
	 * returns true if a value falls within the range (base - epsilon) .. (base + epsilon),
	 * excluding range endpoints.
	 * 
	 * This routine will fatal during debug builds if epsilon is less than zero.
	 * 
	 * @return true if value is within the range (base - epsilon) .. (base + epsilon), including
	 * the range endpoints.  Otherwise returns false.
	 */

	template<class T>inline bool WithinEpsilonExclusive(const T &base, const T &value, const T &epsilon)
	{
		return (value > base - epsilon) && (value < base + epsilon);
	}


	// ----------------------------------------------------------------------
	/**
	 * Determine if a number is a power of two.
	 * 
	 * This routine runs in O(1) time.
	 * 
	 * @param value  Value to see if it is a power of two
	 * @return True if the value is a power of two, otherwise false
	 */

	template<class T> inline bool IsPowerOfTwo(T value)
	{
		return ((value - 1) & (value)) == 0;
	}

	// ----------------------------------------------------------------------
	/**
	 * Returns the number of bits set in an integral type.
	 * 
	 * Type T must provide integral-type bitwise operations and semantics.
	 * 
	 * @param value  Value to count the number of bits
	 */

	template<class T> inline int GetBitCount(T value)
	{
		int count = 0;
		while (value)
		{
			value &= (value-1);
			++count;
		}

		return count;
	}

	// ----------------------------------------------------------------------
	/**
	 * Returns the index of the first bit set in an integral type.
	 * 
	 * Type T must provide integral-type bitwise operations and semantics.
	 * 
	 * @param value  Value to find the first set bit
	 * @return Returns the zero-based index of the first bit set in an
	 * integral type, with the least significant byte being index 0.
	 * 
	 * Returns -1 if no bits are set in the value.
	 */

	template<class T>inline int GetFirstBitSet(T value)
	{
		if (!value)
			return -1;

		int count;
		for (count = 0; !(value & static_cast<T>(0x01)); ++count, value >>= 1)
			{}

		return count;
	}

	// ----------------------------------------------------------------------
	/**
	 * Calculate the factorial of the specified value.
	 * 
	 * This routine takes O(n) time to compute.
	 * 
	 * @param number  The number to compute the factorial of
	 * @return The factorial of the specified value
	 */

	inline int factorial(int number)
	{
		int i, result;

		for (result = 1, i = 2; i <= number; ++i)
			result *= i;

		return result;
	}

	// ----------------------------------------------------------------------
	/**
	 * Calculate the number of combinations of i elements taken from a set of n elements.
	 * 
	 * This routine computes: n! / (i! * (n-i)!).
	 * 
	 * @param n  Number of elements in the set
	 * @param i  Number of elements to pick from the set
	 * @return The number of distinct combinations
	 */

	inline int choose(int n, int i)
	{
		return factorial(n) / (factorial(i) * factorial(n - i));
	}

	// ----------------------------------------------------------------------
	/**
	 * Linear interpolate from start to end along t, where t is from 0..1.
	 * 
	 * t must be from 0..1
	 * 
	 * @return A value from start to end
	 */

	template<class T> const T linearInterpolate (const T& start, const T& end, const float t)
	{
		return static_cast<T>((end - start) * t) + start;
	}

	// ----------------------------------------------------------------------
	/**
	 * A non-linear, smooth interpolation from start to end along t, where t is from 0..1.
	 * 
	 * (-2 * (end - start) * t^3) + (3 * (end - start) * t^2) + start
	 *
	 * t must be from 0..1
	 * 
	 * @return A value from start to end
	 */

	template<class T> const T cubicInterpolate(const T& start, const T& end, const float t)
	{
		float const tSquared = sqr(t);
		float const tCubed = tSquared * t;
		T const & diff = end - start;
		return static_cast<T>((diff * (tCubed * 2.f)) + (diff * (tSquared * 3.f)) + start);
	}

	// ----------------------------------------------------------------------
	/**
	 * A lint-friendly boolean compare.
	 * 
	 * @return A value from start to end
	 */

	inline bool boolEqual(bool const lhs, bool const rhs)
	{
		return (lhs && rhs) || (!lhs && !rhs);
	}
	

	//-----------------------------------------------------------------
	
	template <class UIObject, UITypeID type>
	UIObject * asUIObject(UIBaseObject * const object)
	{
		if (object && object->IsA(type))
		{
			return static_cast<UIObject  *>(object);
		}
		
		return NULL;
	}
	
	//-----------------------------------------------------------------
	
	template <class UIObject, UITypeID type>
	UIObject const * asUIObject(UIBaseObject const * const object)
	{
		if (object && object->IsA(type))
		{
			return static_cast<UIObject const *>(object);
		}
		
		return NULL;
	}
	
	// ======================================================================
}

#define UI_ASOBJECT(UIClass, object) UIUtils::asUIObject<UIClass, T##UIClass>(object)

#endif // __UIUTILS_H__
