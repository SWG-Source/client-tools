#ifndef __UIRectangleStyle_H__
#define __UIRectangleStyle_H__

#include "UiMemoryBlockManagerMacros.h"
#include "UIStyle.h"

#include <bitset>

class UIImageStyle;
class UiMemoryBlockManager;

class UIRectangleStyle : public UIStyle
{
public:

	enum Stretch
	{
		S_North,
		S_South,
		S_East,
		S_West,
		S_CenterH,
		S_CenterV,
		S_LastStretch
	};

	enum Image
	{
		I_North,
		I_South,
		I_East,
		I_West,
		I_Center,
		I_NorthEast,
		I_NorthWest,
		I_SouthEast,
		I_SouthWest,
		I_LastImage
	};

	enum Remainder
	{
		R_none,
		R_front,
		R_back
	};


	struct RemainderNames
	{
		static const Unicode::String None;
		static const Unicode::String Front;
		static const Unicode::String Back;
	};

	static const char    * const TypeName;

	struct PropertyName
	{
		static const UILowerString  Color;
		static const UILowerString  Opacity;

		static const UILowerString  North;
		static const UILowerString  NorthStretch;
		static const UILowerString  NorthShrink;
		static const UILowerString  NorthEast;
		static const UILowerString  NorthWest;

		static const UILowerString  South;
		static const UILowerString  SouthStretch;
		static const UILowerString  SouthShrink;
		static const UILowerString  SouthEast;
		static const UILowerString  SouthWest;

		static const UILowerString  East;
		static const UILowerString  EastStretch;
		static const UILowerString  EastShrink;
		static const UILowerString  West;
		static const UILowerString  WestStretch;
		static const UILowerString  WestShrink;

		static const UILowerString  Center;
		static const UILowerString  CenterStretchH;
		static const UILowerString  CenterStretchV;
		static const UILowerString  CenterShrinkH;
		static const UILowerString  CenterShrinkV;

		static const UILowerString  RemainderVertical;
		static const UILowerString  RemainderHorizontal;

		static const UILowerString  OpacityRelativeMin;
	};

	                      UIRectangleStyle();
	virtual              ~UIRectangleStyle();

	virtual bool          IsA( const UITypeID ) const;
	virtual const char   *GetTypeName( void ) const;
	virtual UIBaseObject *Clone( void ) const;

	virtual void          GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void          GetPropertyNames( UIPropertyNameVector &, bool forCopy  ) const;
	virtual void          GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool          SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool          GetProperty( const UILowerString & Name, UIString &Value ) const;

	void                  SetImage( const Image, UIImageStyle * );
	UIImageStyle         *GetImage( const Image ) const;

	void                  Render( UITime, UICanvas &, const UISize & size ) const;

	bool getShrink (Stretch s) const;

	//-- this allowmini bool flag is a hack to allow prevention of minimizing the edges, but allow the center to minimize
	static void           tileImage (const UIImageStyle & style, UITime dTime, UICanvas &canvas, const UIPoint &location, const UISize & size, UISize numTiles, bool shrinkX, bool shrinkY, Remainder remainderHorizontal, Remainder remainderVertical);

	float                 GetOpacityRelativeMin () const;
	
private:

	static bool           SetRemainder (const Unicode::String & Value, Remainder & r);
	static bool           GetRemainder (Unicode::String & Value, Remainder r);

	Image                 LookupImageIDByName   (const UILowerString & Name) const;
	Stretch               LookupStretchIDByName (const UILowerString & Name) const;
	Stretch               LookupShrinkIDByName  (const UILowerString & Name) const;

	UIImageStyle         *mImages[I_LastImage];

	std::bitset<S_LastStretch> mStretch;
	std::bitset<S_LastStretch> mShrink;

	UIColor               mColor;
	float                 mOpacity;

	Remainder             mRemainderHorizontal;
	Remainder             mRemainderVertical;

	float                 mOpacityRelativeMin;
};

//----------------------------------------------------------------------

inline bool UIRectangleStyle::getShrink (Stretch s) const
{
	return mShrink [static_cast<size_t>(s)];
}

//----------------------------------------------------------------------

inline float UIRectangleStyle::GetOpacityRelativeMin () const
{
	return mOpacityRelativeMin;
}

//----------------------------------------------------------------------


#endif // __UIRectangleStyle_H__
