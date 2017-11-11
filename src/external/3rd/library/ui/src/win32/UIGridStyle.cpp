#include "_precompile.h"

#include "UIGridStyle.h"
#include "UIImageStyle.h"
#include "UICanvas.h"
#include "UIPropertyDescriptor.h"

#include <vector>

const char *UIGridStyle::TypeName = "GridStyle";

const UILowerString UIGridStyle::PropertyName::NormalBackground    = UILowerString ("Normal.Background");
const UILowerString UIGridStyle::PropertyName::NormalJunction      = UILowerString ("Normal.Junction");
const UILowerString UIGridStyle::PropertyName::NormalVStartCap     = UILowerString ("Normal.VStartCap");
const UILowerString UIGridStyle::PropertyName::NormalVBackground   = UILowerString ("Normal.VBackground");
const UILowerString UIGridStyle::PropertyName::NormalVEndCap       = UILowerString ("Normal.VEndCap");
const UILowerString UIGridStyle::PropertyName::NormalHStartCap		 = UILowerString ("Normal.HStartCap");
const UILowerString UIGridStyle::PropertyName::NormalHBackground	 = UILowerString ("Normal.HBackground");
const UILowerString UIGridStyle::PropertyName::NormalHEndCap			 = UILowerString ("Normal.HEndCap");
const UILowerString UIGridStyle::PropertyName::SelectedBackground  = UILowerString ("Selected.Background");
const UILowerString UIGridStyle::PropertyName::SelectedJunction    = UILowerString ("Selected.Junction");
const UILowerString UIGridStyle::PropertyName::SelectedVStartCap   = UILowerString ("Selected.VStartCap");
const UILowerString UIGridStyle::PropertyName::SelectedVBackground = UILowerString ("Selected.VBackground");
const UILowerString UIGridStyle::PropertyName::SelectedVEndCap     = UILowerString ("Selected.VEndCap");
const UILowerString UIGridStyle::PropertyName::SelectedHStartCap   = UILowerString ("Selected.HStartCap");
const UILowerString UIGridStyle::PropertyName::SelectedHBackground = UILowerString ("Selected.HBackground");
const UILowerString UIGridStyle::PropertyName::SelectedHEndCap     = UILowerString ("Selected.HEndCap");

//======================================================================================
#define _TYPENAME UIGridStyle

namespace UIGridStyleNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(NormalBackground, "", T_object),
		_DESCRIPTOR(NormalJunction, "", T_object),
		_DESCRIPTOR(NormalVStartCap, "", T_object),
		_DESCRIPTOR(NormalVBackground, "", T_object),
		_DESCRIPTOR(NormalVEndCap, "", T_object),
		_DESCRIPTOR(NormalHStartCap, "", T_object),
		_DESCRIPTOR(NormalHBackground, "", T_object),
		_DESCRIPTOR(NormalHEndCap, "", T_object),
		_DESCRIPTOR(SelectedBackground, "", T_object),
		_DESCRIPTOR(SelectedJunction, "", T_object),
		_DESCRIPTOR(SelectedVStartCap, "", T_object),
		_DESCRIPTOR(SelectedVBackground, "", T_object),
		_DESCRIPTOR(SelectedVEndCap, "", T_object),
		_DESCRIPTOR(SelectedHStartCap, "", T_object),
		_DESCRIPTOR(SelectedHBackground, "", T_object),
		_DESCRIPTOR(SelectedHEndCap, "", T_object),
	_GROUPEND(Basic, 3, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UIGridStyleNamespace;
//======================================================================================

typedef UIGridStyle::Image Image;

UIGridStyle::UIGridStyle()
{
	for( int i= 0; i < LastImage; ++i )
		mImages[i] = 0;
}

UIGridStyle::~UIGridStyle()
{
	for( int i= 0; i < LastImage; ++i )
	{
		if( mImages[i] )
			mImages[i]->Detach( this );
	}
}

bool UIGridStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUIGridStyle) || UIWidgetStyle::IsA( Type );
}

const char *UIGridStyle::GetTypeName( void ) const
{
	return TypeName;
}

UIBaseObject *UIGridStyle::Clone( void ) const
{
	return new UIGridStyle;
}

void UIGridStyle::SetImage( const Image i, UIImageStyle *NewImage )
{
	if( NewImage )
		NewImage->Attach( this );

	if( mImages[i] )
		mImages[i]->Detach( this );

	mImages[i] = NewImage;
}

UIImageStyle *UIGridStyle::GetImage( const Image i ) const
{
	static const Image remap[LastImage] =
	{
		NormalBackground,
		NormalJunction,
		NormalVStartCap,
		NormalVBackground,
		NormalVEndCap,
		NormalHStartCap,
		NormalHBackground,
		NormalHEndCap,

		NormalBackground,
		NormalJunction,
		NormalVStartCap,
		NormalVBackground,
		NormalVEndCap,
		NormalHStartCap,
		NormalHBackground,
		NormalHEndCap,
	};

	if( mImages[i] )
		return mImages[i];
	else
		return mImages[remap[i]];
}

void UIGridStyle::Render( UITime AnimationTime, UICanvas &DestinationCanvas, const UISize &CellSize, const UISize &CellCount, const bool *CellSelectionState ) const
{
	UIRect	ClippingRect;
	UIPoint	FirstCell;
	UIPoint	LastCell;
	long		x, y;

	if( CellSize.x < 1 || CellSize.y < 1 )
		return;

	UIImageStyle *BackgroundNormal			= GetImage( NormalBackground  );
	UIImageStyle *JunctionNormal				= GetImage( NormalJunction    );
	UIImageStyle *VStartCapNormal				= GetImage( NormalVStartCap   );
	UIImageStyle *VBackgroundNormal			= GetImage( NormalVBackground );
	UIImageStyle *VEndCapNormal					= GetImage( NormalVEndCap     );
	UIImageStyle *HStartCapNormal				= GetImage( NormalHStartCap   );
	UIImageStyle *HBackgroundNormal			= GetImage( NormalHBackground );
	UIImageStyle *HEndCapNormal					= GetImage( NormalHEndCap     );

	UIImageStyle *BackgroundSelected		= GetImage( SelectedBackground  );
#if 0
	UIImageStyle *JunctionSelected			= GetImage( SelectedJunction    );
	UIImageStyle *VStartCapSelected			= GetImage( SelectedVStartCap   );
	UIImageStyle *VBackgroundSelected		= GetImage( SelectedVBackground );
	UIImageStyle *VEndCapSelected				= GetImage( SelectedVEndCap     );
	UIImageStyle *HStartCapSelected			= GetImage( SelectedHStartCap   );
	UIImageStyle *HBackgroundSelected		= GetImage( SelectedHBackground );
	UIImageStyle *HEndCapSelected       = GetImage( SelectedHEndCap     );
#endif

	long          JunctionWidth					= JunctionNormal    ? JunctionNormal->GetWidth() : 0;
	long          JunctionHeight				= JunctionNormal    ? JunctionNormal->GetHeight() : 0;
	long          BackgroundWidth				= BackgroundNormal  ? BackgroundNormal->GetWidth() : 0;
	long          BackgroundHeight			= BackgroundNormal  ? BackgroundNormal->GetHeight() : 0;
	long          HStartCapWidth				= HStartCapNormal   ? HStartCapNormal->GetWidth() : 0;
	long          HEndCapWidth					= HEndCapNormal     ? HEndCapNormal->GetWidth() : 0;
	long					HBackgroundWidth			= HBackgroundNormal ? HBackgroundNormal->GetWidth() : 0;
	long          VStartCapHeight				= VStartCapNormal   ? VStartCapNormal->GetHeight() : 0;
	long          VEndCapHeight					= VEndCapNormal     ? VEndCapNormal->GetHeight() : 0;
	long					VBackgroundHeight			= VBackgroundNormal ? VBackgroundNormal->GetHeight() : 0;

	DestinationCanvas.GetClip( ClippingRect );

	FirstCell.x = ClippingRect.left / CellSize.x;
	FirstCell.y = ClippingRect.top  / CellSize.y;

	LastCell.x  = (ClippingRect.right  + CellSize.x) / CellSize.x;
	LastCell.y  = (ClippingRect.bottom + CellSize.y) / CellSize.y;

	UIPoint	FirstCellLocation( FirstCell.x * CellSize.x, FirstCell.y * CellSize.y );

	// Clamp in case we have a clipping region that is
	// larger than the amount of content we have to render
	if( FirstCell.x < 0 )
		FirstCell.x = 0;
	if( LastCell.x > CellCount.x )
		LastCell.x = CellCount.x;

	if( FirstCell.y < 0 )
		FirstCell.y = 0;
	if( LastCell.y > CellCount.y )
		LastCell.y = CellCount.y;

	DestinationCanvas.PushState();

	// Draw Cell Backgrounds
	if( BackgroundNormal && BackgroundSelected && BackgroundHeight > 0 && BackgroundWidth > 0)
	{
		for( y = FirstCell.y; y < LastCell.y; ++y )
		{
			long RowY  = y * CellSize.y + JunctionHeight;
			long LastY = RowY + CellSize.y;

			for( x = FirstCell.x; x < LastCell.x; ++x )
			{
				long ColX  = x * CellSize.x + JunctionWidth;
				long LastX = ColX + CellSize.x;

				DestinationCanvas.Clip( UIRect( 0, 0, LastX, LastY ) );

				if( CellSelectionState && CellSelectionState[y*CellCount.x + x] )
				{
					for( int yp = RowY; yp < LastY; yp += BackgroundHeight )
					{
						for( int xp = ColX; xp < LastX; xp += BackgroundWidth )
							BackgroundSelected->Render( AnimationTime, DestinationCanvas, UIPoint( xp, yp ) );
					}
				}
				else
				{
					for( int yp = RowY; yp < LastY; yp += BackgroundHeight )
					{
						for( int xp = ColX; xp < LastX; xp += BackgroundWidth )
							BackgroundNormal->Render( AnimationTime, DestinationCanvas, UIPoint( xp, yp ) );
					}
				}

				DestinationCanvas.RestoreClip();
			}
		}
	}

	// Draw Horizontal Edges
	for( x = FirstCell.x; x < LastCell.x; ++x )
	{
		long ColX    = x * CellSize.x;
		long bgStart = ColX + JunctionWidth + HStartCapWidth;
		long bgEnd   = ColX + CellSize.x - HEndCapWidth;

		for( y = FirstCell.y; y <= LastCell.y; ++y )
		{
			long RowY = y * CellSize.y;

			if( HStartCapWidth )
				HStartCapNormal->Render( AnimationTime, DestinationCanvas, UIPoint( ColX + JunctionWidth, RowY ) );

			if( HEndCapWidth )
				HEndCapNormal->Render( AnimationTime, DestinationCanvas, UIPoint( bgEnd, RowY ) );
		}

		if( HBackgroundWidth > 0)
		{
			DestinationCanvas.Clip( UIRect( 0, 0, bgEnd, CellSize.y * CellCount.y + 1 ) );

			for( y = FirstCell.y; y <= LastCell.y; ++y )
			{
				long RowY = y * CellSize.y;

				for( int xp = bgStart; xp < bgEnd; xp += HBackgroundWidth )
					HBackgroundNormal->Render( AnimationTime, DestinationCanvas, UIPoint( xp, RowY ) );
			}

			DestinationCanvas.RestoreClip();
		}
	}

	// Draw Vertical Edges
	for( y = FirstCell.y; y < LastCell.y; ++y )
	{
		long RowY    = y * CellSize.y;
		long bgStart = RowY + JunctionHeight + VStartCapHeight;
		long bgEnd   = RowY + CellSize.y - VEndCapHeight;

		for( x = FirstCell.x; x <= LastCell.x; ++x )
		{
			long ColX = x * CellSize.x;

			if( VStartCapHeight )
				VStartCapNormal->Render( AnimationTime, DestinationCanvas, UIPoint( ColX, RowY + JunctionHeight ) );

			if( VEndCapHeight )
				VEndCapNormal->Render( AnimationTime, DestinationCanvas, UIPoint( ColX, bgEnd ) );
		}

		if( VBackgroundHeight > 0)
		{
			DestinationCanvas.Clip( UIRect( 0, 0, CellSize.x * CellCount.x + 1, bgEnd ) );

			for( x = FirstCell.x; x <= LastCell.x; ++x )
			{
				long ColX = x * CellSize.x;

				for( int yp = bgStart; yp < bgEnd; yp += VBackgroundHeight )
					VBackgroundNormal->Render( AnimationTime, DestinationCanvas, UIPoint( ColX, yp ) );
			}

			DestinationCanvas.RestoreClip();
		}
	}

	// Draw Junctions
	if( JunctionWidth )
	{
		for( x = FirstCell.x; x <= LastCell.x; ++x )
		{
			long ColX = x * CellSize.x;

			for( y = FirstCell.y; y <= LastCell.y; ++y )
				JunctionNormal->Render( AnimationTime, DestinationCanvas, UIPoint( ColX, y * CellSize.y ) );
		}
	}

	DestinationCanvas.PopState();
}

//----------------------------------------------------------------------

void UIGridStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	GetPropertyNames (In, false);
}

//----------------------------------------------------------------------

void UIGridStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
}

//----------------------------------------------------------------------

void UIGridStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::NormalBackground    );
	In.push_back( PropertyName::NormalJunction      );
	In.push_back( PropertyName::NormalVStartCap     );
	In.push_back( PropertyName::NormalVBackground   );
	In.push_back( PropertyName::NormalVEndCap       );
	In.push_back( PropertyName::NormalHStartCap     );
	In.push_back( PropertyName::NormalHBackground   );
	In.push_back( PropertyName::NormalHEndCap       );

	In.push_back( PropertyName::SelectedBackground  );
	In.push_back( PropertyName::SelectedJunction    );
	In.push_back( PropertyName::SelectedVStartCap   );
	In.push_back( PropertyName::SelectedVBackground );
	In.push_back( PropertyName::SelectedVEndCap     );
	In.push_back( PropertyName::SelectedHStartCap   );
	In.push_back( PropertyName::SelectedHBackground );
	In.push_back( PropertyName::SelectedHEndCap     );

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}

bool UIGridStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	UIGridStyle::Image ImageID = LookupImageIDByName( Name );

	if( ImageID != UIGridStyle::LastImage )
	{
		UIBaseObject *NewImage = GetObjectFromPath( Value.c_str(), TUIImageStyle );

		if( NewImage || Value.empty() )
		{
			SetImage( ImageID, reinterpret_cast<UIImageStyle *>(NewImage) );
			return true;
		}
	}

	return UIWidgetStyle::SetProperty( Name, Value );
}

bool UIGridStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	UIGridStyle::Image ImageID = LookupImageIDByName( Name );

	if( ImageID != UIGridStyle::LastImage  )
	{
		if( mImages[ImageID] )
		{
			GetPathTo( Value, mImages[ImageID] );
			return true;
		}
	}
	return UIWidgetStyle::GetProperty( Name, Value );
}

UIGridStyle::Image UIGridStyle::LookupImageIDByName( const UILowerString & Name ) const
{
	if( Name == PropertyName::NormalBackground )
		return UIGridStyle::NormalBackground;
	else if( Name == PropertyName::NormalJunction )
		return UIGridStyle::NormalJunction;
	else if( Name == PropertyName::NormalVStartCap )
		return UIGridStyle::NormalVStartCap;
	else if( Name == PropertyName::NormalVBackground )
		return UIGridStyle::NormalVBackground;
	else if( Name == PropertyName::NormalVEndCap )
		return UIGridStyle::NormalVEndCap;
	else if( Name == PropertyName::NormalHStartCap )
		return UIGridStyle::NormalHStartCap;
	else if( Name == PropertyName::NormalHBackground )
		return UIGridStyle::NormalHBackground;
	else if( Name == PropertyName::NormalHEndCap )
		return UIGridStyle::NormalHEndCap;
	else if( Name == PropertyName::SelectedBackground )
		return UIGridStyle::SelectedBackground;
	else if( Name == PropertyName::SelectedJunction )
		return UIGridStyle::SelectedJunction;
	else if( Name == PropertyName::SelectedVStartCap )
		return UIGridStyle::SelectedVStartCap;
	else if( Name == PropertyName::SelectedVBackground )
		return UIGridStyle::SelectedVBackground;
	else if( Name == PropertyName::SelectedVEndCap )
		return UIGridStyle::SelectedVEndCap;
	else if( Name == PropertyName::SelectedHStartCap )
		return UIGridStyle::SelectedHStartCap;
	else if( Name == PropertyName::SelectedHBackground )
		return UIGridStyle::SelectedHBackground;
	else if( Name == PropertyName::SelectedHEndCap )
		return UIGridStyle::SelectedHEndCap;
	else
		return UIGridStyle::LastImage;
}