#include "_precompile.h"

#include "UICanvas.h"
#include "UIDataSource.h"
#include "UIGridStyle.h"
#include "UIPalette.h"
#include "UIPropertyDescriptor.h"
#include "UIRectangleStyle.h"
#include "UIRenderHelper.h"
#include "UITemplate.h"
#include "UITextStyle.h"
#include "UITextStyleManager.h"
#include "UITooltipStyle.h"
#include "UIUtils.h"
#include "UIWidget.h"
#include "UIWidgetRectangleStyles.h"
#include "UIFontCharacter.h"

#include <vector>

//----------------------------------------------------------------------

const char * const UITooltipStyle::TypeName = "TooltipStyle";

const UILowerString UITooltipStyle::PropertyName::BackgroundOpacity = UILowerString ("BackgroundOpacity");
const UILowerString UITooltipStyle::PropertyName::DataSource        = UILowerString ("DataSource");
const UILowerString UITooltipStyle::PropertyName::GridStyle         = UILowerString ("GridStyle");
const UILowerString UITooltipStyle::PropertyName::MaxWidth          = UILowerString ("MaxWidth");
const UILowerString UITooltipStyle::PropertyName::Template          = UILowerString ("Template");
const UILowerString UITooltipStyle::PropertyName::TextColor         = UILowerString ("TextColor");
const UILowerString UITooltipStyle::PropertyName::TextPadding       = UILowerString ("TextPadding");
const UILowerString UITooltipStyle::PropertyName::TextStyle         = UILowerString ("TextStyle");

//======================================================================================
#define _TYPENAME UITooltipStyle

namespace UITooltipStyleNamespace
{
	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(DataSource, "", T_object),
		_DESCRIPTOR(MaxWidth, "", T_int),
		_DESCRIPTOR(Template, "", T_object),
	_GROUPEND(Basic, 3, int(UIPropertyCategories::C_Basic));
	//================================================================
	//================================================================
	// Appearance category.
	_GROUPBEGIN(Appearance)
		_DESCRIPTOR(GridStyle, "", T_object),
		_DESCRIPTOR(TextColor, "", T_color),
		_DESCRIPTOR(TextPadding, "", T_rect),
		_DESCRIPTOR(TextStyle, "", T_object),
		_DESCRIPTOR(BackgroundOpacity, "", T_float),
	_GROUPEND(Appearance, 3, int(UIPropertyCategories::C_Appearance));
	//================================================================
}
using namespace UITooltipStyleNamespace;
//======================================================================================

UITooltipStyle::UITooltipStyle () :
UIWidgetStyle (),
mTextPadding  (),
mTextColor         (UIColor::white),
mBackgroundOpacity (0.5f)
{
	mMaxWidth				= -1;
	mGridStyle				=  0;
	mTemplate				=  0;
	mDataSource				=  0;
	mItemInDataSource       =  0;
	mTextStyle              =  0;
}

//----------------------------------------------------------------------

UITooltipStyle::~UITooltipStyle ()
{
	SetGridStyle(0);
	SetDataSource(0);
	SetTemplate(0);
	SetTextStyle(0);
}

//----------------------------------------------------------------------

bool UITooltipStyle::IsA( const UITypeID Type ) const
{
	return (Type == TUITooltipStyle) || UIWidgetStyle::IsA( Type );
}

const char *UITooltipStyle::GetTypeName () const
{
	return TypeName;
}

UIBaseObject *UITooltipStyle::Clone () const
{
	return new UITooltipStyle;
}

void UITooltipStyle::SetGridStyle( UIGridStyle *NewGridStyle )
{
	if( NewGridStyle )
		NewGridStyle->Attach( this );

	if( mGridStyle )
		mGridStyle->Detach( this );

	mGridStyle = NewGridStyle;
}

void UITooltipStyle::SetDataSource( UIDataSource *NewDataSource )
{
	if( NewDataSource )
		NewDataSource->Attach( this );

	if( mDataSource )
		mDataSource->Detach( this );

	mDataSource = NewDataSource;
}

void UITooltipStyle::SetTemplate( UITemplate *NewTemplate )
{
	if( NewTemplate )
		NewTemplate->Attach( this );

	if( mTemplate )
		mTemplate->Detach( this );

	mTemplate = NewTemplate;
}

void UITooltipStyle::SetTextStyle( UITextStyle *NewTextStyle )
{
	if( NewTextStyle )
		NewTextStyle->Attach( this );

	if( mTextStyle )
		mTextStyle->Detach( this );

	mTextStyle = NewTextStyle;
}

void UITooltipStyle::SetItemInDataSource( long In )
{
	if( In != mItemInDataSource )
	{
		mCache.Clear();
		mItemInDataSource = In;
	}
}

//----------------------------------------------------------------------

void UITooltipStyle::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::DataSource  );
	In.push_back( PropertyName::GridStyle	 );
	In.push_back( PropertyName::Template	   );
	In.push_back( PropertyName::TextStyle   );

	UIWidgetStyle::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UITooltipStyle::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidgetStyle::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//----------------------------------------------------------------------

void UITooltipStyle::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	UIPalette::GetPropertyNamesForType (TUITooltipStyle, In);

	In.push_back( PropertyName::DataSource  );
	In.push_back( PropertyName::GridStyle	 );
	In.push_back( PropertyName::MaxWidth	   );
	In.push_back( PropertyName::Template	   );
	In.push_back( PropertyName::TextColor   );
	In.push_back( PropertyName::TextPadding );
	In.push_back( PropertyName::TextStyle   );
	In.push_back( PropertyName::BackgroundOpacity );

	UIWidgetStyle::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UITooltipStyle::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::DataSource )
	{
		UIBaseObject *NewDataSource = GetObjectFromPath( Value, TUIDataSource );

		if( NewDataSource || Value.empty() )
		{
			SetDataSource( reinterpret_cast<UIDataSource*>( NewDataSource ) );
			return true;
		}
		// Fall through
	}
	else if( Name == PropertyName::GridStyle )
	{
		UIBaseObject *NewStyle = GetObjectFromPath( Value, TUIGridStyle );

		if( NewStyle || Value.empty() )
		{
			SetGridStyle( reinterpret_cast<UIGridStyle *>( NewStyle ) );
			return true;
		}
		// Fall through
	}
	else if( Name == PropertyName::MaxWidth )
	{
		return UIUtils::ParseLong( Value, mMaxWidth );
	}
	else if( Name == PropertyName::Template )
	{
		UIBaseObject *NewTemplate = GetObjectFromPath( Value, TUITemplate );

		if( NewTemplate || Value.empty() )
		{
			SetTemplate( reinterpret_cast<UITemplate *>( NewTemplate ) );
			return true;
		}
		// Fall through
	}
	else if( Name == PropertyName::TextColor )
		return UIUtils::ParseColor( Value, mTextColor );
	else if( Name == PropertyName::TextPadding )
		return UIUtils::ParseRect( Value, mTextPadding );
	else if( Name == PropertyName::BackgroundOpacity )
		return UIUtils::ParseFloat( Value, mBackgroundOpacity );
	else if( Name == PropertyName::TextStyle )
	{
		UIBaseObject *NewTextStyle = UITextStyleManager::GetInstance()->GetFontForLogicalFont(Value);

		if( NewTextStyle || Value.empty() )
		{
			SetTextStyle( reinterpret_cast<UITextStyle *>( NewTextStyle ) );
			return true;
		}
		// Fall through
	}
	else
	{
		UIPalette::SetPropertyForObject (*this, Name, Value);
	}

	return UIWidgetStyle::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UITooltipStyle::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::DataSource )
	{
		if( mDataSource )
		{
			GetPathTo( Value, mDataSource );
			return true;
		}
	}
	else if( Name == PropertyName::GridStyle )
	{
		if( mGridStyle )
		{
			GetPathTo( Value, mGridStyle );
			return true;
		}
	}
	else if( Name == PropertyName::MaxWidth )
	{
		return UIUtils::FormatLong( Value, mMaxWidth );
	}
	else if( Name == PropertyName::Template )
	{
		if( mTemplate )
		{
			GetPathTo( Value, mTemplate );
			return true;
		}
	}
	else if( Name == PropertyName::TextColor )
		return UIUtils::FormatColor( Value, mTextColor );
	else if( Name == PropertyName::TextPadding )
		return UIUtils::FormatRect( Value, mTextPadding );
	else if( Name == PropertyName::BackgroundOpacity )
		return UIUtils::FormatFloat( Value, mBackgroundOpacity );
	else if( Name == PropertyName::TextStyle )
	{
		if( mTextStyle )
		{
			Value = mTextStyle->GetLogicalName();
			return true;
		}
	}

	return UIWidgetStyle::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

void UITooltipStyle::Render( UICanvas &DestinationCanvas, UITime AnimationState ) const
{
	UISize theSize = Measure();

	if( (theSize.x > 0) && (theSize.y > 0) )
	{
		if( mGridStyle )
			mGridStyle->Render( AnimationState, DestinationCanvas, theSize, UISize::one, 0 );

		if( mTemplate && mDataSource )
		{
			typedef std::vector<UIWidget *>                     UIWidgetVector;
			UIWidgetVector *Instance   = 0;
			UIDataList    & DataList   = mDataSource->GetData();
			long            DataItemID = mItemInDataSource;

			for( UIDataList::iterator i = DataList.begin(); i != DataList.end(); ++i )
			{
				if( DataItemID == 0 )
				{
					Instance = mCache.Get( *i );

					if( !Instance )
					{
						const_cast<UITooltipStyle *>( this )->mCache.Add( *i, mTemplate, const_cast<UITooltipStyle *>( this ) );
						Instance = mCache.Get( *i );
					}
					break;
				}
				else
					--DataItemID;
			}

			if( Instance )
				UIRenderHelper::RenderObjects( DestinationCanvas, *Instance );
		}
	}
}

//----------------------------------------------------------------------

void UITooltipStyle::Render( UICanvas &DestinationCanvas, const UIString &TooltipText, UITime AnimationState ) const
{
	UISize theSize = Measure( TooltipText );

	if( (theSize.x > 0) && (theSize.y > 0) )
	{
		if( mGridStyle )
			mGridStyle->Render( AnimationState, DestinationCanvas, theSize, UISize::one, 0 );

		if( mTextStyle )
		{
			DestinationCanvas.PushState();

			const UIRectangleStyle * const rs = GetRectangleStyles ().GetStyle (UIWidgetRectangleStyles::RS_Default);

			if (rs)
			{
				DestinationCanvas.ModifyOpacity (mBackgroundOpacity);
				rs->Render (0, DestinationCanvas, theSize);
				DestinationCanvas.RestoreState ();
			}

			DestinationCanvas.Translate(mTextPadding.left, mTextPadding.top);
			DestinationCanvas.SetColor( mTextColor );

			if( mMaxWidth > 0 )
				mTextStyle->RenderText( UITextStyle::Left, TooltipText, DestinationCanvas, UIPoint::zero, &mMaxWidth );
			else
				mTextStyle->RenderText( UITextStyle::Left, TooltipText, DestinationCanvas, UIPoint::zero );

			DestinationCanvas.PopState();
		}
	}
}

//----------------------------------------------------------------------

const UISize UITooltipStyle::Measure () const
{
	if( mTemplate )
		return mTemplate->GetSize();
	else
		return UISize::zero;
}

//----------------------------------------------------------------------

UISize UITooltipStyle::Measure( const UIString &TooltipText) const
{
	if( mTextStyle )
	{
		UISize TextSize;

		if( mMaxWidth > 0 )
			mTextStyle->MeasureWrappedText( TooltipText, mMaxWidth, TextSize );
		else
			mTextStyle->MeasureText( TooltipText, TextSize );

		TextSize.x += mTextPadding.left + mTextPadding.right;
		TextSize.y += mTextPadding.top + mTextPadding.bottom;

		return TextSize;
	}
	else
		return UISize::zero;
}

//----------------------------------------------------------------------

void UITooltipStyle::SetCharacterWidth(long NewWidth)
{
	mMaxWidth = NewWidth;

	if (mTextStyle) 
	{
		UIFontCharacter const * const c =  mTextStyle->GetCharacter(0);
		if (c) 
		{
			mMaxWidth = NewWidth * c->GetWidth();
		}
	}
}

//----------------------------------------------------------------------
