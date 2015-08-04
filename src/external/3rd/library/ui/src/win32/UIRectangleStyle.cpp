#include "_precompile.h"
#include "UIRectangleStyle.h"

#include "UICanvas.h"
#include "UIImageStyle.h"
#include "UIPalette.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UIWidget.h"
#include "UiMemoryBlockManager.h"

#include <cassert>
#include <vector>

//-----------------------------------------------------------------

const char * const UIRectangleStyle::TypeName                            = "RectangleStyle";

const UILowerString  UIRectangleStyle::PropertyName::North               = UILowerString ("North");
const UILowerString  UIRectangleStyle::PropertyName::NorthStretch        = UILowerString ("NorthStretch");
const UILowerString  UIRectangleStyle::PropertyName::NorthShrink         = UILowerString ("NorthShrink");
const UILowerString  UIRectangleStyle::PropertyName::NorthEast           = UILowerString ("NorthEast");
const UILowerString  UIRectangleStyle::PropertyName::NorthWest           = UILowerString ("NorthWest");
const UILowerString  UIRectangleStyle::PropertyName::South               = UILowerString ("South");
const UILowerString  UIRectangleStyle::PropertyName::SouthStretch        = UILowerString ("SouthStretch");
const UILowerString  UIRectangleStyle::PropertyName::SouthShrink         = UILowerString ("SouthShrink");
const UILowerString  UIRectangleStyle::PropertyName::SouthEast           = UILowerString ("SouthEast");
const UILowerString  UIRectangleStyle::PropertyName::SouthWest           = UILowerString ("SouthWest");
const UILowerString  UIRectangleStyle::PropertyName::East                = UILowerString ("East");
const UILowerString  UIRectangleStyle::PropertyName::EastStretch         = UILowerString ("EastStretch");
const UILowerString  UIRectangleStyle::PropertyName::EastShrink          = UILowerString ("EastShrink");
const UILowerString  UIRectangleStyle::PropertyName::West                = UILowerString ("West");
const UILowerString  UIRectangleStyle::PropertyName::WestStretch         = UILowerString ("WestStretch");
const UILowerString  UIRectangleStyle::PropertyName::WestShrink          = UILowerString ("WestShrink");
const UILowerString  UIRectangleStyle::PropertyName::Center              = UILowerString ("Center");
const UILowerString  UIRectangleStyle::PropertyName::CenterStretchH      = UILowerString ("CenterStretchH");
const UILowerString  UIRectangleStyle::PropertyName::CenterStretchV      = UILowerString ("CenterStretchV");
const UILowerString  UIRectangleStyle::PropertyName::CenterShrinkH       = UILowerString ("CenterShrinkH");
const UILowerString  UIRectangleStyle::PropertyName::CenterShrinkV       = UILowerString ("CenterShrinkV");
const UILowerString  UIRectangleStyle::PropertyName::Color               = UILowerString ("Color");
const UILowerString  UIRectangleStyle::PropertyName::Opacity             = UILowerString ("Opacity");
const UILowerString  UIRectangleStyle::PropertyName::RemainderVertical   = UILowerString ("RemainderVertical");
const UILowerString  UIRectangleStyle::PropertyName::RemainderHorizontal = UILowerString ("RemainderHorizontal");
const UILowerString  UIRectangleStyle::PropertyName::OpacityRelativeMin  = UILowerString ("OpacityRelativeMin");

//----------------------------------------------------------------------

const Unicode::String UIRectangleStyle::RemainderNames::None   = Unicode::narrowToWide ("None");
const Unicode::String UIRectangleStyle::RemainderNames::Front  = Unicode::narrowToWide ("Front");
const Unicode::String UIRectangleStyle::RemainderNames::Back   = Unicode::narrowToWide ("Back");

//======================================================================================
#define _TYPENAME UIRectangleStyle

namespace UIRectangleStyleNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(RemainderVertical, "", T_string), // ENUM
		_DESCRIPTOR(RemainderHorizontal, "", T_string), // ENUM
	_GROUPEND(Basic, 2, int(UIPropertyCategories::C_Basic));
	//================================================================

	//================================================================
	// Appearance category.
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(Color, "", T_color),
		_DESCRIPTOR(Opacity, "", T_float),
		_DESCRIPTOR(OpacityRelativeMin, "", T_float),
	_GROUPEND(Appearance, 2, int(UIPropertyCategories::C_Appearance));
	//================================================================
	//================================================================
	// Behavior category.
	_GROUPBEGIN(Behavior)
		_DESCRIPTOR(NorthStretch, "", T_bool),
		_DESCRIPTOR(SouthStretch, "", T_bool),
		_DESCRIPTOR(EastStretch, "", T_bool),
		_DESCRIPTOR(WestStretch, "", T_bool),
		_DESCRIPTOR(CenterStretchH, "", T_bool),
		_DESCRIPTOR(CenterStretchV, "", T_bool),
		_DESCRIPTOR(NorthShrink, "", T_bool),
		_DESCRIPTOR(SouthShrink, "", T_bool),
		_DESCRIPTOR(EastShrink, "", T_bool),
		_DESCRIPTOR(WestShrink, "", T_bool),
		_DESCRIPTOR(CenterShrinkH, "", T_bool),
		_DESCRIPTOR(CenterShrinkV, "", T_bool),
	_GROUPEND(Behavior, 2, int(UIPropertyCategories::C_Behavior));
	//================================================================

	//================================================================
	// AdvancedAppearance category.
	_GROUPBEGIN(AdvancedAppearance)
		_DESCRIPTOR(North, "", T_object),
		_DESCRIPTOR(South, "", T_object),
		_DESCRIPTOR(East, "", T_object),
		_DESCRIPTOR(West, "", T_object),
		_DESCRIPTOR(NorthEast, "", T_object),
		_DESCRIPTOR(NorthWest, "", T_object),
		_DESCRIPTOR(SouthEast, "", T_object),
		_DESCRIPTOR(SouthWest, "", T_object),
		_DESCRIPTOR(Center, "", T_object),
	_GROUPEND(AdvancedAppearance, 2, int(UIPropertyCategories::C_AdvancedAppearance));
	//================================================================
}
using namespace UIRectangleStyleNamespace;
//======================================================================================

UIRectangleStyle::UIRectangleStyle() :
UIStyle              (),
mColor               (UIColor::white),
mOpacity             (1.0),
mRemainderHorizontal (R_back),
mRemainderVertical   (R_back),
mOpacityRelativeMin  (0.0f)
{
	{
		for( size_t i= 0; i < static_cast<size_t>(I_LastImage); ++i )
			mImages[i] = 0;
	}
	{
		for( size_t i = 0; i < static_cast<size_t>(S_LastStretch); ++i )
		{
			mStretch[i] = true;
			mShrink[i] = true;
		}
	}
}

//-----------------------------------------------------------------

UIRectangleStyle::~UIRectangleStyle()
{
	for( size_t i = 0; i < static_cast<size_t>(I_LastImage); ++i )
		SetImage( static_cast<Image>( i ), 0 );
}

//-----------------------------------------------------------------

bool UIRectangleStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUIRectangleStyle) || UIStyle::IsA( Type );
}
//-----------------------------------------------------------------


const char *UIRectangleStyle::GetTypeName( void ) const
{
	return TypeName;
}

//-----------------------------------------------------------------

UIBaseObject *UIRectangleStyle::Clone( void ) const
{
	return new UIRectangleStyle;
}

//-----------------------------------------------------------------

void UIRectangleStyle::SetImage( const Image i, UIImageStyle *NewImage )
{
	if( NewImage )
		NewImage->Attach( this );

	if( mImages[i] )
		mImages[i]->Detach( this );

	mImages[i] = NewImage;
}

//-----------------------------------------------------------------

UIImageStyle *UIRectangleStyle::GetImage( const Image i ) const
{
	return mImages[i];
}

//-----------------------------------------------------------------

void UIRectangleStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::North );
	In.push_back( PropertyName::NorthStretch );
	In.push_back( PropertyName::NorthShrink );
	In.push_back( PropertyName::NorthEast );
	In.push_back( PropertyName::NorthWest );

	In.push_back( PropertyName::South );
	In.push_back( PropertyName::SouthStretch );
	In.push_back( PropertyName::SouthShrink );
	In.push_back( PropertyName::SouthEast );
	In.push_back( PropertyName::SouthWest );

	In.push_back( PropertyName::East );
	In.push_back( PropertyName::EastStretch );
	In.push_back( PropertyName::EastShrink );
	In.push_back( PropertyName::West );
	In.push_back( PropertyName::WestStretch );
	In.push_back( PropertyName::WestShrink );

	In.push_back( PropertyName::Center );
	In.push_back( PropertyName::CenterStretchH );
	In.push_back( PropertyName::CenterStretchV );
	In.push_back( PropertyName::CenterShrinkH );
	In.push_back( PropertyName::CenterShrinkV );


	UIStyle::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UIRectangleStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Behavior, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedAppearance, category, o_groups);
}

//----------------------------------------------------------------------

void UIRectangleStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	UIPalette::GetPropertyNamesForType (TUIRectangleStyle, In);

	In.push_back( PropertyName::North );
	In.push_back( PropertyName::NorthStretch );
	In.push_back( PropertyName::NorthShrink );
	In.push_back( PropertyName::NorthEast );
	In.push_back( PropertyName::NorthWest );

	In.push_back( PropertyName::South );
	In.push_back( PropertyName::SouthStretch );
	In.push_back( PropertyName::SouthShrink );
	In.push_back( PropertyName::SouthEast );
	In.push_back( PropertyName::SouthWest );

	In.push_back( PropertyName::East );
	In.push_back( PropertyName::EastStretch );
	In.push_back( PropertyName::EastShrink );
	In.push_back( PropertyName::West );
	In.push_back( PropertyName::WestStretch );
	In.push_back( PropertyName::WestShrink );

	In.push_back( PropertyName::Center );
	In.push_back( PropertyName::CenterStretchH );
	In.push_back( PropertyName::CenterStretchV );
	In.push_back( PropertyName::CenterShrinkH );
	In.push_back( PropertyName::CenterShrinkV );

	In.push_back( PropertyName::Color );
	In.push_back( PropertyName::Opacity );
	In.push_back( PropertyName::OpacityRelativeMin );

	In.push_back( PropertyName::RemainderVertical);
	In.push_back( PropertyName::RemainderHorizontal);

	UIStyle::GetPropertyNames( In, forCopy );
}

//-----------------------------------------------------------------

bool UIRectangleStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::RemainderHorizontal)
		return SetRemainder (Value, mRemainderHorizontal);
	else if (Name == PropertyName::RemainderVertical)
		return SetRemainder (Value, mRemainderVertical);

	Image ImageID = LookupImageIDByName( Name );

	if( ImageID != I_LastImage )
	{
		UIImageStyle *NewImage = static_cast<UIImageStyle * >(GetObjectFromPath( Value, TUIImageStyle ));

		if( NewImage || Value.empty() )
		{
			SetImage( ImageID, NewImage );
			return true;
		}
	}

	//----------------------------------------------------------------------

	const Stretch StretchID = LookupStretchIDByName( Name );

	if( StretchID != S_LastStretch )
	{
		bool retval;

		if (UIUtils::ParseBoolean (Value, retval))
		{
			mStretch [static_cast<size_t>(StretchID)] = retval;
			return true;
		}
	}

	//----------------------------------------------------------------------

	const Stretch StrinkID = LookupShrinkIDByName( Name );

	if( StrinkID != S_LastStretch )
	{
		bool retval;

		if (UIUtils::ParseBoolean (Value, retval))
		{
			mShrink [static_cast<size_t>(StrinkID)] = retval;
			return true;
		}
	}

	//----------------------------------------------------------------------

	if( Name == PropertyName::Color )
		return UIUtils::ParseColor( Value, mColor );
	else if( Name == PropertyName::Opacity )
		return UIUtils::ParseFloat( Value, mOpacity );
	else if( Name == PropertyName::OpacityRelativeMin )
		return UIUtils::ParseFloat( Value, mOpacityRelativeMin );
	else
	{
		UIPalette::SetPropertyForObject (*this, Name, Value);
	}

	return UIStyle::SetProperty( Name, Value );
}

//-----------------------------------------------------------------

bool UIRectangleStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::RemainderHorizontal)
		return GetRemainder (Value, mRemainderHorizontal);
	else if (Name == PropertyName::RemainderVertical)
		return GetRemainder (Value, mRemainderVertical);

	//----------------------------------------------------------------------

	const Image ImageID = LookupImageIDByName( Name );

	if( ImageID != I_LastImage )
	{
		const UIImageStyle * const imageStyle = mImages[static_cast<size_t>(ImageID)];
		if( imageStyle )
		{
			GetPathTo( Value, imageStyle );
			return true;
		}
	}

	//----------------------------------------------------------------------

	const Stretch StretchID = LookupStretchIDByName( Name );

	if( StretchID != S_LastStretch )
	{
		if (UIUtils::FormatBoolean (Value, mStretch [static_cast<size_t>(StretchID)]))
			return true;
	}

	//----------------------------------------------------------------------

	const Stretch ShrinkID = LookupShrinkIDByName( Name );

	if( ShrinkID != S_LastStretch )
	{
		if (UIUtils::FormatBoolean (Value, mShrink [static_cast<size_t>(ShrinkID)]))
			return true;
	}

	//----------------------------------------------------------------------

	if( Name == PropertyName::Color )
		return UIUtils::FormatColor( Value, mColor );
	else if( Name == PropertyName::Opacity )
		return UIUtils::FormatFloat( Value, mOpacity );
	else if( Name == PropertyName::OpacityRelativeMin )
		return UIUtils::FormatFloat( Value, mOpacityRelativeMin );

	return UIStyle::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UIRectangleStyle::SetRemainder (const Unicode::String & Value, Remainder & r)
{
	if (Unicode::caseInsensitiveCompare (Value, RemainderNames::None, 0, Value.size ()))
		r = R_none;
	else if (Unicode::caseInsensitiveCompare (Value, RemainderNames::Front, 0, Value.size ()))
		r = R_front;
	else if (Unicode::caseInsensitiveCompare (Value, RemainderNames::Back, 0, Value.size ()))
		r = R_back;
	else
		return false;

	return true;
}

//----------------------------------------------------------------------

bool UIRectangleStyle::GetRemainder (Unicode::String & Value, Remainder r)
{
	switch (r)
	{
	case R_none:
		Value = RemainderNames::None;
		break;
	case R_front:
		Value = RemainderNames::Front;
		break;
	case R_back:
		Value = RemainderNames::Back;
		break;
	}

	return true;
}

//-----------------------------------------------------------------

UIRectangleStyle::Image UIRectangleStyle::LookupImageIDByName( const UILowerString & Name ) const
{
	if (Name == PropertyName::North)
		return I_North;
	if (Name == PropertyName::NorthEast)
		return I_NorthEast;
	if (Name == PropertyName::NorthWest)
		return I_NorthWest;
	if (Name == PropertyName::South)
		return I_South;
	if (Name == PropertyName::SouthEast)
		return I_SouthEast;
	if (Name == PropertyName::SouthWest)
		return I_SouthWest;
	if (Name == PropertyName::East)
		return I_East;
	if (Name == PropertyName::West)
		return I_West;
	if (Name == PropertyName::Center)
		return I_Center;

	return I_LastImage;
}

//-----------------------------------------------------------------

UIRectangleStyle::Stretch UIRectangleStyle::LookupStretchIDByName( const UILowerString & Name ) const
{
	if (Name == PropertyName::NorthStretch)
		return S_North;
	if (Name == PropertyName::SouthStretch)
		return S_South;
	if (Name == PropertyName::EastStretch)
		return S_East;
	if (Name == PropertyName::WestStretch)
		return S_West;
	if (Name == PropertyName::CenterStretchH)
		return S_CenterH;
	if (Name == PropertyName::CenterStretchV)
		return S_CenterV;

	return S_LastStretch;
}

//-----------------------------------------------------------------

UIRectangleStyle::Stretch UIRectangleStyle::LookupShrinkIDByName( const UILowerString & Name ) const
{
	if (Name == PropertyName::NorthShrink)
		return S_North;
	if (Name == PropertyName::SouthShrink)
		return S_South;
	if (Name == PropertyName::EastShrink)
		return S_East;
	if (Name == PropertyName::WestShrink)
		return S_West;
	if (Name == PropertyName::CenterShrinkH)
		return S_CenterH;
	if (Name == PropertyName::CenterShrinkV)
		return S_CenterV;

	return S_LastStretch;
}

//-----------------------------------------------------------------

void UIRectangleStyle::tileImage (const UIImageStyle & style, UITime dTime, UICanvas &canvas, const UIPoint &location, const UISize & size, UISize numTiles, bool shrinkX, bool shrinkY, Remainder remainderHorizontal, Remainder remainderVertical)
{
	static const char * profilerName = "UIRectangleStyle::tileImage";
	UI_UNREF (profilerName);

	const UISize imageActualSize (style.GetSize ());
	const UISize imageSize (abs (imageActualSize.x), abs (imageActualSize.y));

	if (imageSize.x == 0 || imageSize.y == 0 || size.x == 0 || size.y == 0)
	{
		return;
	}

	if (numTiles.x == 0)
		numTiles.x = std::max (1L, size.x / imageSize.x);

	if (numTiles.y == 0)
		numTiles.y = std::max (1L, size.y / imageSize.y);

	const UISize diff (size.x % numTiles.x, size.y % numTiles.y);

	const UISize tileSize         ((size.x - diff.x) / numTiles.x, (size.y - diff.y) / numTiles.y);
	const UISize tileSizeLastCell (tileSize.x + diff.x, tileSize.y + diff.y);

	const UIPoint startLocation (location);
	UIPoint curLocation (location);

	UIFloatPoint srcScale         (1.0f, 1.0f);
	UIFloatPoint srcScaleLastCell (1.0f, 1.0f);


	//-- invert the scale factors for flipped images...

	if (imageActualSize.x < 0)
	{
		srcScaleLastCell.x = srcScale.x         = -1.0f;
	}
	else if (imageActualSize.y < 0)
	{
		srcScale.y         = srcScaleLastCell.y = -1.0f;
	}


	if (shrinkX)
	{
		if (tileSizeLastCell.x != imageActualSize.x)
			srcScaleLastCell.x = static_cast<float>(imageActualSize.x) / static_cast<float>(tileSizeLastCell.x);
	}
	else if (tileSizeLastCell.x > imageSize.x)
			srcScaleLastCell.x = static_cast<float>(imageActualSize.x) / static_cast<float>(tileSizeLastCell.x);

	if (shrinkY)
	{
		if (tileSizeLastCell.y != imageActualSize.y)
			srcScaleLastCell.y = static_cast<float>(imageActualSize.y) / static_cast<float>(tileSizeLastCell.y);
	}
	else if (tileSizeLastCell.y > imageSize.y)
			srcScaleLastCell.y = static_cast<float>(imageActualSize.y) / static_cast<float>(tileSizeLastCell.y);

	if (tileSize.y != imageSize.y)
		srcScale.y = static_cast<float>(imageActualSize.y) / static_cast<float>(tileSize.y);
	if (tileSize.x != imageSize.x)
		srcScale.x = static_cast<float>(imageActualSize.x) / static_cast<float>(tileSize.x);


	UISize       theSize (tileSize);
	UIFloatPoint theScale (srcScale);

	canvas.EnableFiltering( true );

	//-----------------------------------------------------------------
	//-- iterate through the rows

	for (int j = 0; j < numTiles.y; ++j, curLocation.y += theSize.y, curLocation.x = startLocation.x)
	{
		theSize.x  = tileSize.x;
		theScale.x = srcScale.x;

		//-- first cell
		if (remainderVertical == R_front && j == 0)
		{
			theSize.y  = tileSizeLastCell.y;
			theScale.y = srcScaleLastCell.y;
		}
		//-- last cell
		else if (remainderVertical == R_back && (j == numTiles.y - 1))
		{
			theSize.y  = tileSizeLastCell.y;
			theScale.y = srcScaleLastCell.y;
		}

		canvas.SetSourceScale (theScale.x, theScale.y);

		for (int i = 0; i < numTiles.x; ++i, curLocation.x += theSize.x)
		{
			if (remainderHorizontal == R_front)
			{
				 if (i == 0)
				 {
					theSize.x = tileSizeLastCell.x;
					theScale.x = srcScaleLastCell.x;
					canvas.SetSourceScale (theScale.x, theScale.y);
				 }
				 else if (i == 1)
				 {
					theSize.x = tileSize.x;
					theScale.x = srcScale.x;
					canvas.SetSourceScale (theScale.x, theScale.y);
				 }
			}
			else if (remainderHorizontal == R_back && (i == numTiles.x - 1))
			{
				theSize.x = tileSizeLastCell.x;
				theScale.x = srcScaleLastCell.x;
				canvas.SetSourceScale (theScale.x, theScale.y);
			}

			style.Render (dTime, canvas, curLocation, theSize);
		}
	}

	canvas.SetSourceScale (1.0f, 1.0f);
	canvas.EnableFiltering (false);
}

//-----------------------------------------------------------------

namespace
{

	inline void drawCorner (UITime dTime, UICanvas & canvas, const UIImageStyle & style, const UIPoint & location, const UISize & cornerSize)
	{
		UIFloatPoint srcScale (1.0f, 1.0f);

		if (cornerSize.x != style.GetWidth ())
			srcScale.x = static_cast<float>(style.GetWidth ()) / static_cast<float>(cornerSize.x);
		if (cornerSize.y != style.GetHeight ())
			srcScale.y = static_cast<float>(style.GetHeight ()) / static_cast<float>(cornerSize.y);

		canvas.SetSourceScale (srcScale.x, srcScale.y);
		style.Render (dTime, canvas, location, cornerSize);
	}
}

//-----------------------------------------------------------------

void UIRectangleStyle::Render( UITime dTime, UICanvas &canvas, const UISize & size ) const
{
	static const char * profilerName = "UIRectangleStyle::Render";
	UI_UNREF (profilerName);

	canvas.PushState();
	canvas.ModifyOpacity (mOpacity);
	canvas.ModifyColor (mColor);

	const float oldOpacity    = canvas.GetOpacity ();
	const float actualOpacity = UIWidget::ComputeRelativeOpacity (oldOpacity, mOpacityRelativeMin);
	canvas.SetOpacity (actualOpacity);

	UIRect centerRect (size);

	bool drawOk [I_LastImage];
	UISize imageSizes [I_LastImage];

	{
		for (int i = 0; i < I_LastImage; ++i)
		{
			imageSizes [i].x = mImages [i] ? abs (mImages [i]->GetWidth  ()) : 0L;
			imageSizes [i].y = mImages [i] ? abs (mImages [i]->GetHeight ()) : 0L;
		}
	}

	if ((drawOk [I_West]   = (mImages [I_West]  && imageSizes [I_West].x > 0)) == true)
		centerRect.left   += imageSizes [I_West].x;
	if ((drawOk [I_East]   = (mImages [I_East]  && imageSizes [I_East].x > 0)) == true)
		centerRect.right  -= imageSizes [I_East].x;
	if ((drawOk [I_South]  = (mImages [I_South] && imageSizes [I_South].y > 0)) == true)
		centerRect.bottom -= imageSizes [I_South].y;
	if ((drawOk [I_North]  = (mImages [I_North] && imageSizes [I_North].y > 0)) == true)
		centerRect.top    += imageSizes [I_North].y;

	//-----------------------------------------------------------------
	//-- draw the center piece
	if (mImages [I_Center])
	{
		const UISize numTiles (mStretch [S_CenterH] ? 1 : 0, mStretch [S_CenterV] ? 1 : 0);
		tileImage (*mImages [I_Center], dTime, canvas, centerRect.Location (), centerRect.Size (), numTiles, getShrink (S_CenterH), getShrink (S_CenterV), mRemainderHorizontal, mRemainderVertical);
	}

	//-----------------------------------------------------------------
	//-- draw any corners

	canvas.EnableFiltering (true);

	if ((drawOk [I_NorthEast] = mImages [I_NorthEast] && drawOk [I_North] && drawOk [I_East]) == true)
	{
		const UISize cornerSize (imageSizes [I_East].x, imageSizes [I_North].y);
		drawCorner (dTime, canvas, *mImages [I_NorthEast], UIPoint (centerRect.right, 0), cornerSize);
	}

	if ((drawOk [I_NorthWest] = mImages [I_NorthWest] && drawOk [I_North] && drawOk [I_West]) == true)
	{
		const UISize cornerSize (imageSizes [I_West].x, imageSizes [I_North].y);
		drawCorner (dTime, canvas, *mImages [I_NorthWest], UIPoint (0, 0), cornerSize);
	}

	if ((drawOk [I_SouthEast] = mImages [I_SouthEast] && drawOk [I_South] && drawOk [I_East]) == true)
	{
		const UISize cornerSize (imageSizes [I_East].x, imageSizes [I_South].y);
		drawCorner (dTime, canvas, *mImages [I_SouthEast], UIPoint (centerRect.right, centerRect.bottom), cornerSize);
	}

	if ((drawOk [I_SouthWest] = mImages [I_SouthWest] && drawOk [I_South] && drawOk [I_West]) == true)
	{
		const UISize cornerSize (imageSizes [I_West].x, imageSizes [I_South].y);
		drawCorner (dTime, canvas, *mImages [I_SouthWest], UIPoint (0, centerRect.bottom), cornerSize);
	}


	//-----------------------------------------------------------------
	//-- draw the edges

	if (drawOk [I_West])
	{
		const UIRect rect (0,                drawOk [I_NorthWest] ? centerRect.top : 0,
			               centerRect.left,  drawOk [I_SouthWest] ? centerRect.bottom : size.y);
		const UISize numTiles (1, mStretch [S_West] ? 1 : 0);
		tileImage (*mImages [I_West], dTime, canvas, rect.Location (), rect.Size (), numTiles, true, getShrink (S_West), mRemainderHorizontal, mRemainderVertical);
	}

	if (drawOk [I_East])
	{
		const UIRect rect (centerRect.right, drawOk [I_NorthEast] ? centerRect.top : 0,
			               size.x,           drawOk [I_SouthEast] ? centerRect.bottom : size.y);
		const UISize numTiles (1, mStretch [S_East] ? 1 : 0);
		tileImage (*mImages [I_East], dTime, canvas, rect.Location (), rect.Size (), numTiles, true, getShrink (S_East), mRemainderHorizontal, mRemainderVertical);
	}

	if (drawOk [I_North])
	{
		const UIRect rect (drawOk [I_NorthWest] ? centerRect.left : 0, 0,
			               drawOk [I_NorthEast] ? centerRect.right : size.x, centerRect.top);
		const UISize numTiles (mStretch [I_North] ? 1 : 0, 1);
		tileImage (*mImages [I_North], dTime, canvas, rect.Location (), rect.Size (), numTiles, getShrink (S_North), true, mRemainderHorizontal, mRemainderVertical);
	}

	if (drawOk [I_South])
	{
		const UIRect rect (drawOk [I_SouthWest] ? centerRect.left : 0, centerRect.bottom,
			               drawOk [I_SouthEast] ? centerRect.right : size.x, size.y);
		const UISize numTiles (mStretch [I_South] ? 1 : 0, 1);
		tileImage (*mImages [I_South], dTime, canvas, rect.Location (), rect.Size (), numTiles, getShrink (S_South), true, mRemainderHorizontal, mRemainderVertical);
	}

	canvas.EnableFiltering (false);
	canvas.PopState();
}

//-----------------------------------------------------------------
