#include "_precompile.h"

#include "UIBoundaryPolygon.h"
#include "UIButton.h"
#include "UIButtonStyle.h"
#include "UICanvas.h"
#include "UIEventCallback.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIText.h"
#include "UITextStyle.h"
#include "UIUtils.h"
#include "UiMemoryBlockManager.h"
#include "UIPropertyDescriptor.h"
#include "UnicodeUtils.h"

#include <cassert>
#include <vector>

//----------------------------------------------------------------------

const char * const UIButton::TypeName                        = "Button";

//----------------------------------------------------------------------

const UILowerString UIButton::PropertyName::Icon              = UILowerString ("Icon");
const UILowerString UIButton::PropertyName::IconAlignment     = UILowerString ("IconAlignment");
const UILowerString UIButton::PropertyName::IconColor         = UILowerString ("IconColor");
const UILowerString UIButton::PropertyName::IconMargin        = UILowerString ("IconMargin");
const UILowerString UIButton::PropertyName::IconMaxSize       = UILowerString ("IconMaxSize");
const UILowerString UIButton::PropertyName::IconMinSize       = UILowerString ("IconMinSize");
const UILowerString UIButton::PropertyName::IconShrink        = UILowerString ("IconShrink");
const UILowerString UIButton::PropertyName::IconStretch       = UILowerString ("IconStretch");
const UILowerString UIButton::PropertyName::IsCancelButton    = UILowerString ("IsCancelButton");
const UILowerString UIButton::PropertyName::IsDefaultButton   = UILowerString ("IsDefaultButton");
const UILowerString UIButton::PropertyName::LocalText         = UILowerString ("LocalText");
const UILowerString UIButton::PropertyName::MaxTextLines      = UILowerString ("MaxTextLines");
const UILowerString UIButton::PropertyName::MouseOverSound    = UILowerString ("MouseOverSound");
const UILowerString UIButton::PropertyName::OnPress           = UILowerString ("OnPress");
const UILowerString UIButton::PropertyName::PressSound        = UILowerString ("PressSound");
const UILowerString UIButton::PropertyName::ReleaseSound      = UILowerString ("ReleaseSound");
const UILowerString UIButton::PropertyName::Style             = UILowerString ("Style");
const UILowerString UIButton::PropertyName::Text              = UILowerString ("Text");
const UILowerString UIButton::PropertyName::TextCapital       = UILowerString ("TextCapital");
const UILowerString UIButton::PropertyName::Toggle            = UILowerString ("Toggle");
const UILowerString UIButton::PropertyName::ToggleDown        = UILowerString ("ToggleDown");
const UILowerString UIButton::PropertyName::RenderIconBehind  = UILowerString ("RenderIconBehind");

//----------------------------------------------------------------------

const UILowerString UIButton::CategoryName::Icon              = UILowerString ("Icon");
const UILowerString UIButton::CategoryName::Text              = UILowerString ("Text");
const UILowerString UIButton::CategoryName::Behavior          = UILowerString ("Behavior");

//----------------------------------------------------------------------

const UILowerString UIButton::MethodName::Press        = UILowerString ("Press");

//----------------------------------------------------------------------

const Unicode::String UIButton::IconAlignmentNames::None           = Unicode::narrowToWide ("None");
const Unicode::String UIButton::IconAlignmentNames::Left           = Unicode::narrowToWide ("Left");
const Unicode::String UIButton::IconAlignmentNames::Center         = Unicode::narrowToWide ("Center");
const Unicode::String UIButton::IconAlignmentNames::Right          = Unicode::narrowToWide ("Right");

//======================================================================================
#define _TYPENAME UIButton

namespace UIButtonNamespace
{
	//----------------------------------------------------------------

	typedef UIButtonStyle::UIButtonState UIButtonState;

	UIButtonState          GetControlState (const UIButton & button)
	{
		if (button.IsActivated ())
			return UIButtonStyle::Pressed;
		else if (!button.IsEnabled())
			return UIButtonStyle::Disabled;
		else if (button.IsUnderMouse())
			return UIButtonStyle::MouseOver;
		else if (button.IsSelected())
			return UIButtonStyle::Selected;
		else
			return UIButtonStyle::Normal;
	}

	UIImage * s_buttonIconImage = 0;

	//----------------------------------------------------------------

	//================================================================
	// Text category
	_GROUPBEGIN(Text)
		_DESCRIPTOR(LocalText,   "",T_string),
		_DESCRIPTOR(MaxTextLines,"",T_string),
		_DESCRIPTOR(Text,        "",T_string),
		_DESCRIPTOR(TextCapital, "",T_bool)
	_GROUPEND(Text, 2, 0);
	//================================================================
	//================================================================
	// Icon category
	_GROUPBEGIN(Icon)
		_DESCRIPTOR(Icon,"",T_object),
		_DESCRIPTOR(IconAlignment,"",T_string),
		_DESCRIPTOR(IconColor,"",T_color),
		_DESCRIPTOR(IconMargin,"",T_rect),
		_DESCRIPTOR(IconMaxSize,"",T_point),
		_DESCRIPTOR(IconMinSize,"",T_point),
		_DESCRIPTOR(IconShrink,"",T_bool),
		_DESCRIPTOR(IconStretch,"",T_bool),
		_DESCRIPTOR(RenderIconBehind,"",T_bool),
		_DESCRIPTOR(Style,"",T_object)
	_GROUPEND(Icon, 2, 0);

	//================================================================
	// Behavior category
	_GROUPBEGIN(Behavior)
		_DESCRIPTOR(IsCancelButton,"",T_bool),
		_DESCRIPTOR(IsDefaultButton,"",T_bool),
		_DESCRIPTOR(MouseOverSound,"",T_string),
		_DESCRIPTOR(OnPress,"",T_string),
		_DESCRIPTOR(PressSound,"",T_string),
		_DESCRIPTOR(ReleaseSound,"",T_string),
		_DESCRIPTOR(Toggle,"",T_string),
		_DESCRIPTOR(ToggleDown,"",T_string)
	_GROUPEND(Behavior, 2, 0);
	//================================================================
}
using namespace UIButtonNamespace;

//======================================================================================

UIButton::UIButton    () :
UIWidget              (),
mStyle                (0),
mText                 (),
mLocalText            (),
mButtonAttributeBits  (BABF_AutoPressByDrag),
mIcon                 (0),
mIconOffsetLastRender (0),
mIconMargin           (2L, 2L, 2L, 2L),
mIconColor            (UIColor::white),
mIconAlignment        (IA_left),
mIconMaxSize          (16384L,16384L),
mIconMinSize          (0L,0L),
mMaxTextLines         (1)
{
	SetSelectable (false);

	//-- a Button's default background opacity is 1.0f
	UIWidget::SetBackgroundOpacity (1.0f);

	if (!s_buttonIconImage)
		s_buttonIconImage = new UIImage;

	s_buttonIconImage->Attach (this);
}

//======================================================================================

UIButton::~UIButton()
{
	SetStyle (0);
	SetIcon  (0);

	assert (s_buttonIconImage);
	if (s_buttonIconImage->Detach (this))
		s_buttonIconImage = 0;
}

//======================================================================================

bool UIButton::IsA( const UITypeID QueriedType ) const
{
	return QueriedType == TUIButton || UIWidget::IsA( QueriedType );
}

//======================================================================================

const char *UIButton::GetTypeName() const
{
	return TypeName;
}

//======================================================================================

UIBaseObject *UIButton::Clone() const
{
	return new UIButton;
}

//======================================================================================

void UIButton::SetText( const UIString &NewText )
{
	if (&mText != &NewText)
		mText = NewText;

	UIManager::gUIManager ().CreateLocalizedString (mText, mLocalText);
	SetLocalText (mLocalText);
}

//======================================================================================

void UIButton::GetText( UIString &Out ) const
{
	Out = mText;
}

//======================================================================================

void UIButton::SetLocalText( const UIString &NewText )
{
	mLocalText = NewText;

	if (GetTextCapital ())
		mLocalText = Unicode::toUpper (mLocalText);
}

//======================================================================================

void UIButton::GetLocalText( UIString &Out ) const
{
	Out = mLocalText;
}

//======================================================================================

void UIButton::SetIsDefaultButton( const bool IsDefault )
{
	if( IsDefault )
		mButtonAttributeBits |= BABF_IsDefault;
	else
		mButtonAttributeBits &= ~BABF_IsDefault;
}

//======================================================================================

void UIButton::SetIsCancelButton( const bool IsCancel )
{
	if( IsCancel )
		mButtonAttributeBits |= BABF_IsCancel;
	else
		mButtonAttributeBits &= ~BABF_IsCancel;
}

//======================================================================================

void UIButton::SetIsPopupButton( const bool IsDefault )
{
	if( IsDefault )
		mButtonAttributeBits |= BABF_Popup;
	else
		mButtonAttributeBits &= ~BABF_Popup;
}

//======================================================================================

bool UIButton::CanSelect() const
{
	return IsSelectable ();
}

//======================================================================================

bool UIButton::ProcessMessage( const UIMessage &msg )
{
	const bool oldUnderMouse = IsUnderMouse ();

	bool retVal = IsAbsorbsInput();
	
	if( UIWidget::ProcessMessage( msg ) )
		return retVal;

	if( !IsEnabled() )
		return false;

	switch( msg.Type )
	{
	case UIMessage::Character:
		if (IsPopupButton ())
			return false;

		if( msg.Keystroke == ' ')
		{
			Press ();
			return retVal;
		}
		return false;

	case UIMessage::KeyDown:
	case UIMessage::KeyRepeat:

		if (IsPopupButton ())
			return false;

		if( msg.Keystroke == UIMessage::Space )
		{
			SetActivated (true);
			EnterPressedState ();
			return retVal;
		}
		else if( msg.Keystroke == UIMessage::Escape )
		{
			if (IsPressedByMouse ())
			{
				SetActivated (false);
				SetIsPressedByMouse (false);
				return retVal;
			}

			return false;
		}

		return false;

	case UIMessage::KeyUp:

		if( msg.Keystroke == UIMessage::Space )
		{
			if( IsPopupButton () || (IsActivated () && !IsPressedByMouse ()))
			{
				SetActivated (false);
				Press();
				return retVal;
			}
		}

		break;

	case UIMessage::RightMouseDown:
	case UIMessage::MiddleMouseDown:
		if (!IsAllMouseButtons ())
			return false;
		//-- fall through if is all mouse buttons enabled
	case UIMessage::LeftMouseDown:


		///if (IsPopupButton ())
//			return handleButtonRelease ();
//		else
			return handleButtonPress () && retVal;

	case UIMessage::RightMouseUp:
	case UIMessage::MiddleMouseUp:
		if (!IsAllMouseButtons ())
			return false;
		//-- fall through if is all mouse buttons enabled

	case UIMessage::LeftMouseUp:

		return handleButtonRelease () && retVal;

	case UIMessage::MouseMove:
		{
			if (IsPopupButton ())
				return false;

			if( IsPressedByMouse ())
			{
				if( (msg.MouseCoords.x < 0) || (msg.MouseCoords.y < 0) ||
					(msg.MouseCoords.x > GetWidth()) || (msg.MouseCoords.y > GetHeight()) )
				{
					if ((mButtonAttributeBits & BABF_PressedStateSticky) == 0)
						SetActivated (false);
				}
				else
				{
					SetActivated (true);
				}
				return retVal;
			}
		}

		break;

	case UIMessage::MouseEnter:
		if (!oldUnderMouse && IsUnderMouse ())
		{
			MouseOver();
			return retVal;
		}
		break;

	case UIMessage::MouseExit:
		SetIsPressedByMouse (false);
		SetActivated (IsToggleDown ());
		break;

	case UIMessage::LeftMouseDoubleClick:
	case UIMessage::RightMouseDoubleClick:
	case UIMessage::MiddleMouseDoubleClick:
		return retVal;
	}

	return false;
}

//----------------------------------------------------------------------

bool UIButton::handleButtonRelease ()
{
	if( IsPopupButton () || IsPressedByMouse ())
	{
		SetIsPressedByMouse (false);

		if( IsPopupButton () || IsActivated () )
		{
			SetActivated (false);
			Press();

			if (!IsPopupButton () || IsPopupEatPress ())
				return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

bool UIButton::handleButtonPress ()
{
	if (!IsPopupButton ())
	{
		SetActivated (true);
		EnterPressedState ();

		SetIsPressedByMouse (true);
		return true;
	}

	return true;
}

//----------------------------------------------------------------------

bool UIButton::calculateRenderPoint (UIPoint & renderPoint) const
{
	mIconOffsetLastRender = 0;

	if (!mStyle || !mIcon)
		return false;

	s_buttonIconImage->SetStyle (mIcon);

//	const UIRect & textMargin       = mStyle->GetTextMargin (ControlState);

	const long availableHeight = GetHeight () - mIconMargin.top  - mIconMargin.bottom;
	const long availableWidth  = GetWidth ()  - mIconMargin.left - mIconMargin.right;

	UIPoint renderSize (abs (mIcon->GetWidth ()), abs (mIcon->GetHeight ()));

	if (renderSize.x == 0L || renderSize.y == 0L)
		return false;

	const bool stretchable = (mButtonAttributeBits & BABF_IconStretch) != 0;
	const bool shrinkable  = (mButtonAttributeBits & BABF_IconShrink) != 0;

	{
		float factor = 1.0f;

		if (renderSize.x > availableWidth)
		{
			if (shrinkable)
				factor = static_cast<float>(availableWidth) / static_cast<float>(renderSize.x);
		}
		else if (renderSize.x < availableWidth)
		{
			if (stretchable)
				factor = static_cast<float>(availableWidth) / static_cast<float>(renderSize.x);
		}

		if (renderSize.y > availableHeight)
		{
			if (shrinkable)
				factor = std::min (factor, static_cast<float>(availableHeight) / static_cast<float>(renderSize.y));
			else
				factor = std::min (factor, 1.0f);
		}
		else if (renderSize.y < availableHeight)
		{
			if (stretchable)
				factor = std::min (factor, static_cast<float>(availableHeight) / static_cast<float>(renderSize.y));
		}

		renderSize *= factor;
	}

	//-- clamp icon sizes
	renderSize.x = std::max (mIconMinSize.x, renderSize.x);
	renderSize.y = std::max (mIconMinSize.y, renderSize.y);
	renderSize.x = std::min (mIconMaxSize.x, renderSize.x);
	renderSize.y = std::min (mIconMaxSize.y, renderSize.y);

	if (renderSize.x == 0L || renderSize.y == 0L)
		return false;

	s_buttonIconImage->SetSize (renderSize);

	renderPoint.y = mIconMargin.top + (availableHeight - renderSize.y) / 2L;

	if (mIconAlignment == IA_left)
	{
		renderPoint.x = mIconMargin.left;
		mIconOffsetLastRender = renderPoint.x + renderSize.x + mIconMargin.right + mIconMargin.left;
	}
	else if (mIconAlignment == IA_center)
	{
		renderPoint.x = mIconMargin.left + (availableWidth - renderSize.x) / 2L;
		mIconOffsetLastRender = renderPoint.x + renderSize.x + mIconMargin.right + mIconMargin.left;
//		renderpoint.x = textMargin.left + mIconMargin.left;
//		renderpoint.y = textMargin.top + mIconMargin.top + (availableHeight - renderSize.y) / 2L;
	}
	else if (mIconAlignment == IA_right)
	{
		renderPoint.x = GetWidth () - mIconMargin.right - renderSize.x;
		mIconOffsetLastRender = renderPoint.x - mIconMargin.left;
	}
	else
		assert (false);

	return true;
}

//======================================================================================

void UIButton::RenderBackground ( UICanvas &DestinationCanvas ) const
{
	UIWidget::RenderDefault (DestinationCanvas, mStyle ? &mStyle->GetRectangleStyles () : 0);
}

//----------------------------------------------------------------------

void UIButton::RenderIcon (UICanvas & DestinationCanvas) const
{
	if (!mStyle)
		return;

	UIPoint renderPoint;

	if (!calculateRenderPoint (renderPoint))
		return;

	const UIButtonState ControlState = GetControlState (*this);
	const UIColor & styleIconColor = mStyle->GetIconColor (ControlState);

	DestinationCanvas.PushState   ();
	DestinationCanvas.ModifyColor (mIconColor);
	DestinationCanvas.ModifyColor (styleIconColor);
	DestinationCanvas.Translate   (renderPoint);

	s_buttonIconImage->Render (DestinationCanvas);

	DestinationCanvas.PopState    ();
}

//----------------------------------------------------------------------

void UIButton::RenderText ( UICanvas &DestinationCanvas ) const
{
	if( !mStyle || mText.empty())
		return;

	const UIButtonState ControlState = GetControlState (*this);

	const UITextStyle * const textStyle = mStyle->GetTextStyle  (ControlState);

	if (!textStyle)
		return;

	const UIRect & textMargin           = mStyle->GetTextMargin (ControlState);
	UIPoint RenderOrigin = textMargin.Location ();

	long wrapWidth = GetWidth ();

	if (mIconAlignment == IA_left)
	{
		RenderOrigin.x = std::max (mIconOffsetLastRender, textMargin.left);
		wrapWidth     -= RenderOrigin.x + textMargin.right;
	}
	else if (mIconAlignment == IA_center)
	{
		RenderOrigin.x = std::max (mIconOffsetLastRender, textMargin.left);
	}
	else if (mIconAlignment == IA_right)
	{
		const long stop = std::min (mIconOffsetLastRender, wrapWidth - textMargin.right);
		wrapWidth = stop - textMargin.left;
	}

	const long usableVerticalSpace = GetHeight () - textMargin.top - textMargin.bottom;

	const int tav = mStyle->GetTextAlignmentVertical ();

	if (tav == UIText::TAV_center)
	{
		UISize measuredTextSize;
		if (mMaxTextLines == 1)
			measuredTextSize.y = textStyle->GetLeading () * mMaxTextLines;
		else
		{
			textStyle->GetWrappedTextInfo (mLocalText, -1, wrapWidth, measuredTextSize.x, measuredTextSize.y, 0, 0, UITextStyle::UseLastCharAdvance, true, true);
			if (mMaxTextLines > 0)
				measuredTextSize.y = std::min (measuredTextSize.y, textStyle->GetLeading () * mMaxTextLines);
		}
		RenderOrigin.y = std::max (RenderOrigin.y, RenderOrigin.y + (usableVerticalSpace - measuredTextSize.y) / 2L);
	}
	else if (tav == UIText::TAV_bottom)
	{
		UISize measuredTextSize;
		if (mMaxTextLines == 1)
			measuredTextSize.y = textStyle->GetLeading () * mMaxTextLines;
		else
		{
			textStyle->MeasureWrappedText (mLocalText, wrapWidth, measuredTextSize);		
			if (mMaxTextLines > 0)
				measuredTextSize.y = std::min (measuredTextSize.y, textStyle->GetLeading () * mMaxTextLines);
		}
		RenderOrigin.y = std::max (RenderOrigin.y, RenderOrigin.y + (usableVerticalSpace - measuredTextSize.y));
	}

	DestinationCanvas.PushState();
	{
		DestinationCanvas.Clip     (RenderOrigin.x, textMargin.top, RenderOrigin.x + wrapWidth, textMargin.top + usableVerticalSpace);
		DestinationCanvas.SetColor (mStyle->GetTextStyleColor( ControlState ) );
		textStyle->RenderText      (static_cast<UITextStyle::Alignment>(mStyle->GetTextAlignment ()),  mLocalText, DestinationCanvas, RenderOrigin, &wrapWidth, mMaxTextLines, true, true );
	}
	DestinationCanvas.PopState();
}

//----------------------------------------------------------------------

bool UIButton::GetDesiredSize     (UISize & size) const
{
	const bool retval = GetTextSize (size);

	UIRect textMargin;

	if (mStyle)
	{
		const UIButtonState ControlState = GetControlState (*this);
		textMargin    = mStyle->GetTextMargin (ControlState);
		size.x        += textMargin.left + textMargin.right;
		size.y        += textMargin.top  + textMargin.bottom;
	}
	else
	{

	}

	if (!mIcon)
		return retval;

	UIPoint renderPoint;
	if (!calculateRenderPoint (renderPoint))
		return retval;

	if (mIconAlignment == IA_left)
	{
		size.x += std::max (0L, mIconOffsetLastRender - textMargin.left);
	}
	else if (mIconAlignment == IA_center)
	{
		size.x += std::max (0L, mIconOffsetLastRender - textMargin.left);
	}
	else if (mIconAlignment == IA_right)
	{
		size.x += std::max (0L, mIconOffsetLastRender - textMargin.right);
	}

	return true;
}

//----------------------------------------------------------------------

bool UIButton::GetTextSize (UISize & size) const
{
	if (mStyle)
	{
		UIButtonState	ControlState = GetControlState (*this);
		UITextStyle * const Style = mStyle->GetTextStyle( ControlState );

		if( Style )
		{
			Style->MeasureText( mLocalText, size );
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------

void UIButton::ApplyStyle (const UIButtonStyle * NewStyle)
{
	UIBoundaryPolygon * boundary = 0;
	static const std::string boundaryName ("UIButton::Boundary");
	boundary = static_cast<UIBoundaryPolygon *>(findBoundary (boundaryName));
	
	if (NewStyle)
	{
		SetTextCapital (NewStyle->GetTextCapital ());
		
		const UIRect & margin = NewStyle->GetMarginHotSpot ();
		const bool zeroMargin = (margin.left == 0 && margin.right == 0 && margin.top == 0 && margin.bottom == 0);
		
		if (zeroMargin)
		{
			if (boundary)
			{
				removeBoundary (*boundary);
				delete boundary;
			}
		}
		else
		{
			if (!boundary)
			{
				boundary = new UIBoundaryPolygon (boundaryName);
				addBoundary (*boundary);
			}
			
			boundary->makeMarginBoundary (GetSize (), margin);
		}
	}
	else if (boundary)
	{
		removeBoundary (*boundary);
		delete boundary;
	}
}

//======================================================================================

void UIButton::SetStyle( UIButtonStyle *NewStyle )
{
	RemoveProperty (PropertyName::Style);

	if (AttachMember (mStyle, NewStyle))
	{
		ApplyStyle (NewStyle);
	}
}

//======================================================================================

UIStyle *UIButton::GetStyle() const
{
	return mStyle;
};

//======================================================================================

void UIButton::Press ()
{
	if (IsEnabled ())
	{
		if (IsToggleButton ())
			SetIsToggleDown (!IsToggleDown ());

		LeavePressedState ();

		SendCallback( &UIEventCallback::OnButtonPressed, PropertyName::OnPress );

		NotifyActionListener ();
	}
}

//======================================================================================

void UIButton::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back( PropertyName::Icon       );
	In.push_back( PropertyName::Style );
	In.push_back( PropertyName::Text );

	UIWidget::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UIButton::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIWidget::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Text, category, o_groups);
	GROUP_TEST_AND_PUSH(Icon, category, o_groups);
	GROUP_TEST_AND_PUSH(Behavior, category, o_groups);
}

//----------------------------------------------------------------------

void UIButton::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back( PropertyName::Icon       );
	In.push_back( PropertyName::IconAlignment);
	In.push_back( PropertyName::IconColor );
	In.push_back( PropertyName::IconMargin );
	In.push_back( PropertyName::IconMaxSize);
	In.push_back( PropertyName::IconMinSize);
	In.push_back( PropertyName::IconShrink);
	In.push_back( PropertyName::IconStretch);
	In.push_back( PropertyName::IsCancelButton	);
	In.push_back( PropertyName::IsDefaultButton );
	In.push_back( PropertyName::LocalText );
	In.push_back( PropertyName::MaxTextLines);
	In.push_back( PropertyName::MouseOverSound );
	In.push_back( PropertyName::OnPress );
	In.push_back( PropertyName::PressSound );
	In.push_back( PropertyName::ReleaseSound );
	In.push_back( PropertyName::Style );
	In.push_back( PropertyName::Text );
	In.push_back( PropertyName::TextCapital );
	In.push_back( PropertyName::Toggle);
	In.push_back( PropertyName::ToggleDown);
	In.push_back( PropertyName::RenderIconBehind );

	UIWidget::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

void UIButton::GetCategories(UIPropertyCategories::CategoryMask &o_categories) const
{
	o_categories.setHasText(true);
	o_categories.setHasIcon(true);
	o_categories.setHasBehavior(true);

	UIWidget::GetCategories(o_categories);
}

//----------------------------------------------------------------------

void UIButton::GetPropertiesInCategory (UIPropertyCategories::Category category, UIPropertyNameVector & In) const
{
	if (category == UIPropertyCategories::C_Text)
	{		
		In.push_back( PropertyName::LocalText );
		In.push_back( PropertyName::MaxTextLines);
		In.push_back( PropertyName::Text );
		In.push_back( PropertyName::TextCapital );		
	}
	else if(category == UIPropertyCategories::C_Icon)
	{
		In.push_back( PropertyName::Icon       );
		In.push_back( PropertyName::IconAlignment);
		In.push_back( PropertyName::IconColor );
		In.push_back( PropertyName::IconMargin );
		In.push_back( PropertyName::IconMaxSize);
		In.push_back( PropertyName::IconMinSize);
		In.push_back( PropertyName::IconShrink);
		In.push_back( PropertyName::IconStretch);
		In.push_back( PropertyName::RenderIconBehind );
		In.push_back( PropertyName::Style );
	}
	else if(category == UIPropertyCategories::C_Behavior)
	{
		In.push_back( PropertyName::IsCancelButton	);
		In.push_back( PropertyName::IsDefaultButton );
		In.push_back( PropertyName::MouseOverSound );
		In.push_back( PropertyName::OnPress );
		In.push_back( PropertyName::PressSound );
		In.push_back( PropertyName::ReleaseSound );
		In.push_back( PropertyName::Toggle);
		In.push_back( PropertyName::ToggleDown);
	}

	UIWidget::GetPropertiesInCategory(category, In);
}

//======================================================================================

bool UIButton::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::IsCancelButton )
	{
		bool IsCancel;

		if( UIUtils::ParseBoolean( Value, IsCancel ) )
		{
			SetIsCancelButton( IsCancel );
			return true;
		}
		return false;
	}
	else if( Name == PropertyName::IsDefaultButton )
	{
		bool IsDefault;

		if( UIUtils::ParseBoolean( Value, IsDefault ) )
		{
			SetIsDefaultButton( IsDefault );
			return true;
		}
		return false;
	}
	else if( Name == PropertyName::Style )
	{
		UIBaseObject * const NewStyle = GetObjectFromPath( Value, TUIButtonStyle );

		if( NewStyle || Value.empty() )
		{
			SetStyle( static_cast<UIButtonStyle *>(NewStyle) );
		}
	}
	else if (Name == PropertyName::Text )
	{
		SetText( Value );
		return true;
	}
	else if( Name == PropertyName::LocalText )
	{
		SetLocalText( Value );
		return true;
	}
	else if( Name == PropertyName::TextCapital )
	{
		bool b = false;
		if (UIUtils::ParseBoolean (Value, b))
		{
			SetTextCapital (b);
			return true;
		}
		return false;
	}
	else if( Name == PropertyName::Icon )
	{
		UIImageStyle * const style = static_cast<UIImageStyle *>(GetObjectFromPath (Value, TUIImageStyle));

		if (style || Value.empty ())
		{
			SetIcon (style);
		}
	}
	else if( Name == PropertyName::IconStretch )
	{
		bool b = false;

		if (UIUtils::ParseBoolean (Value, b))
		{
			if (b)
			{
				mButtonAttributeBits |= BABF_IconStretch;
			}
			else
			{
				mButtonAttributeBits &= ~BABF_IconStretch;
			}
		}
	}
	else if( Name == PropertyName::IconShrink )
	{
		bool b = false;

		if (UIUtils::ParseBoolean (Value, b))
		{
			if (b)
				mButtonAttributeBits |= BABF_IconShrink;
			else
				mButtonAttributeBits &= ~BABF_IconShrink;
		}
	}
	else if (Name == PropertyName::IconColor)
	{
		return UIUtils::ParseColor (Value, mIconColor);
	}
	else if (Name == PropertyName::IconMargin)
	{
		return UIUtils::ParseRect (Value, mIconMargin);
	}
	else if( Name == PropertyName::Toggle )
	{
		bool b = false;

		if (UIUtils::ParseBoolean (Value, b))
			SetIsToggleButton (b);
	}
	else if( Name == PropertyName::ToggleDown )
	{
		bool b = false;

		if (UIUtils::ParseBoolean (Value, b))
			SetIsToggleDown (b);
	}

	else if( Name == MethodName::Press )
	{
		Press ();
		return true;
	}
	else if (Name == PropertyName::IconAlignment)
	{
		const IconAlignment align = GetIconAlignmentFromString (Value);

		if (align != IA_none)
		{
			SetIconAlignment (align);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::IconMaxSize)
	{
		return UIUtils::ParsePoint (Value, mIconMaxSize);
	}
	else if (Name == PropertyName::IconMinSize)
	{
		return UIUtils::ParsePoint (Value, mIconMinSize);
	}
	else if (Name == PropertyName::MaxTextLines)
	{
		if (UIUtils::ParseLong (Value, mMaxTextLines))
		{
			mMaxTextLines = std::max (mMaxTextLines, -1L);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::RenderIconBehind)
	{
		bool b = false;

		if (UIUtils::ParseBoolean (Value, b))
		{
			SetRenderIconBehind(b);
		}
	}

	return UIWidget::SetProperty( Name, Value );
}

//======================================================================================

bool UIButton::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::IsCancelButton )
		return UIUtils::FormatBoolean( Value, IsCancelButton() );
	else if( Name == PropertyName::IsDefaultButton )
		return UIUtils::FormatBoolean( Value, IsDefaultButton() );
	else if( Name == PropertyName::Style )
	{
		if( mStyle )
		{
			GetPathTo( Value, mStyle );
			return true;
		}
	}
	else if( Name == PropertyName::Text )
	{
		GetText( Value );
		return true;
	}
	else if( Name == PropertyName::LocalText )
	{
		GetLocalText( Value );
		return true;
	}
	else if (Name == PropertyName::TextCapital)
		return UIUtils::FormatBoolean (Value, GetTextCapital ());
	else if( Name == PropertyName::Icon )
	{
		if( mIcon )
		{
			GetPathTo( Value, mIcon );
			return true;
		}
	}
	else if( Name == PropertyName::IconStretch )
	{
		return UIUtils::FormatBoolean (Value, (mButtonAttributeBits & BABF_IconStretch) != 0);
	}
	else if( Name == PropertyName::IconShrink )
	{
		return UIUtils::FormatBoolean (Value, (mButtonAttributeBits & BABF_IconShrink) != 0);
	}
	else if (Name == PropertyName::IconColor)
	{
		return UIUtils::FormatColor (Value, mIconColor);
	}
	else if (Name == PropertyName::IconMargin)
	{
		return UIUtils::FormatRect (Value, mIconMargin);
	}
	else if( Name == PropertyName::Toggle )
	{
		return UIUtils::FormatBoolean (Value, IsToggleButton ());
	}
	else if( Name == PropertyName::ToggleDown )
	{
		return UIUtils::FormatBoolean (Value, IsToggleDown ());
	}
	else if (Name == PropertyName::IconAlignment)
	{
		GetStringFromIconAlignment (mIconAlignment, Value);
		return true;
	}
	else if (Name == PropertyName::IconMaxSize)
	{
		return UIUtils::FormatPoint (Value, mIconMaxSize);
	}
	else if (Name == PropertyName::IconMinSize)
	{
		return UIUtils::FormatPoint (Value, mIconMinSize);
	}
	else if (Name == PropertyName::MaxTextLines)
	{
		return UIUtils::FormatLong (Value, mMaxTextLines);
	}
	else if (Name == PropertyName::RenderIconBehind)
	{
		return UIUtils::FormatBoolean(Value, IsRenderIconBehind());
	}


	return UIWidget::GetProperty( Name, Value );
}

//======================================================================================

void UIButton::EnterPressedState ()
{
	std::string SoundToPlay;

	if( !GetPropertyNarrow ( UIButton::PropertyName::PressSound, SoundToPlay ) && mStyle )
		mStyle->GetPropertyNarrow ( UIButtonStyle::PropertyName::PressSound, SoundToPlay );

	if( !SoundToPlay.empty() )
		UIManager::gUIManager().PlaySound( SoundToPlay.c_str () );
}

//======================================================================================

void UIButton::LeavePressedState ()
{
	std::string SoundToPlay;

	if( !GetPropertyNarrow ( UIButton::PropertyName::ReleaseSound, SoundToPlay ) && mStyle )
		mStyle->GetPropertyNarrow ( UIButtonStyle::PropertyName::ReleaseSound, SoundToPlay );

	if( !SoundToPlay.empty() )
		UIManager::gUIManager().PlaySound( SoundToPlay.c_str () );
}

//======================================================================================

void UIButton::MouseOver ()
{
	std::string SoundToPlay;

	if( !GetPropertyNarrow ( UIButton::PropertyName::MouseOverSound, SoundToPlay ) && mStyle )
		mStyle->GetPropertyNarrow ( UIButtonStyle::PropertyName::MouseOverSound, SoundToPlay );

	if( !SoundToPlay.empty() )
		UIManager::gUIManager().PlaySound( SoundToPlay.c_str () );
}

//----------------------------------------------------------------------

void UIButton::SetIcon (UIImageStyle * icon)
{
	AttachMember (mIcon, icon);
}

//----------------------------------------------------------------------

void UIButton::SetIconColor           (const UIColor & color)
{
	mIconColor = color;
}

//----------------------------------------------------------------------

void UIButton::SetPressedStateSticky (bool b)
{
	if (b)
		mButtonAttributeBits |= BABF_PressedStateSticky;
	else
		mButtonAttributeBits &= ~BABF_PressedStateSticky;
}

//----------------------------------------------------------------------

void UIButton::SetIsAllMouseButtons     (const bool b)
{
	if (b)
		mButtonAttributeBits |= BABF_AllMouseButtons;
	else
		mButtonAttributeBits &= ~BABF_AllMouseButtons;
}

//----------------------------------------------------------------------

void UIButton::SetIsToggleButton     (const bool b)
{
	if (b)
		mButtonAttributeBits |= BABF_Toggle;
	else
		mButtonAttributeBits &= ~BABF_Toggle;
}

//----------------------------------------------------------------------

void UIButton::SetIsToggleDown    (const bool b)
{
	if (b)
	{
		mButtonAttributeBits |= BABF_ToggleDown;
		SetActivated (true);
	}
	else
	{
		mButtonAttributeBits &= ~BABF_ToggleDown;
		SetActivated (IsPressedByMouse ());
	}
}

//----------------------------------------------------------------------

void UIButton::SetIsPopupEatPress   (const bool b)
{
	if (b)
		mButtonAttributeBits |= BABF_PopupEatPress;
	else
		mButtonAttributeBits &= ~BABF_PopupEatPress;
}

//----------------------------------------------------------------------

void UIButton::SetIsPressedByMouse    (bool b)
{
	if (b)
		mButtonAttributeBits |= BABF_PressedByMouse;
	else
		mButtonAttributeBits &= ~BABF_PressedByMouse;
}

//----------------------------------------------------------------------

void UIButton::SetIconAlignment       (IconAlignment ia)
{
	mIconAlignment = ia;
}

//----------------------------------------------------------------------

UIButton::IconAlignment UIButton::GetIconAlignmentFromString (const Unicode::String & str)
{
	const int size = str.size ();
	Unicode::String alignStr;
	for (int i = 0; i < IA_numAlignments; ++i)
	{
		const IconAlignment align = static_cast<IconAlignment>(i);
		GetStringFromIconAlignment (align, alignStr);

		if (Unicode::caseInsensitiveCompare (str, alignStr, 0, size))
			return align;
	}

	return IA_none;
}

//----------------------------------------------------------------------

void UIButton::GetStringFromIconAlignment (IconAlignment align, Unicode::String & str)
{
	switch (align)
	{
	case IA_left:
		str = IconAlignmentNames::Left;
		break;
	case IA_center:
		str = IconAlignmentNames::Center;
		break;
	case IA_right:
		str = IconAlignmentNames::Right;
		break;
	default:
		str = IconAlignmentNames::None;
		break;
	}
}

//----------------------------------------------------------------------

void UIButton::SetTextCapital (bool b)
{
	const bool cur = GetTextCapital ();

	if (b && !cur)
	{
		mButtonAttributeBits |= BABF_TextCapital;
	}
	else if (!b && cur)
	{
		mButtonAttributeBits &= ~BABF_TextCapital;
	}
	else
		return;

	ResetLocalizedStrings ();
}

//----------------------------------------------------------------------

void UIButton::SetAutoPressByDrag (bool b)
{
	if (b)
		mButtonAttributeBits |= BABF_AutoPressByDrag;
	else
		mButtonAttributeBits &= ~BABF_AutoPressByDrag;
}

//----------------------------------------------------------------------

void UIButton::Render( UICanvas &DestinationCanvas ) const
{
	if (IsRenderIconBehind())
	{
		RenderIcon       (DestinationCanvas);
		RenderText       (DestinationCanvas);
		RenderBackground (DestinationCanvas);
	}
	else
	{
		RenderBackground (DestinationCanvas);
		RenderIcon       (DestinationCanvas);
		RenderText       (DestinationCanvas);
	}
}


