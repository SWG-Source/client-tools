//======================================================================
//
// UIComboBox.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UIComboBox.h"

#include "UIButton.h"
#include "UIButtonStyle.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIList.h"
#include "UIListStyle.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPopupMenu.h"
#include "UIPopupMenuStyle.h"
#include "UIPopupMenuStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIScrollbar.h"
#include "UISliderbarStyle.h"
#include "UITextbox.h"
#include "UITextboxStyle.h"
#include "UIUtils.h"
#include "UnicodeUtils.h"

#include <vector>

//======================================================================

const char * const UIComboBox::TypeName = "ComboBox";

const UILowerString UIComboBox::PropertyName::ButtonMargin    = UILowerString ("ButtonMargin");
const UILowerString UIComboBox::PropertyName::ButtonStyle     = UILowerString ("ButtonStyle");
const UILowerString UIComboBox::PropertyName::ButtonWidth     = UILowerString ("ButtonWidth");
const UILowerString UIComboBox::PropertyName::Editable        = UILowerString ("Editable");
const UILowerString UIComboBox::PropertyName::ListMargin      = UILowerString ("ListMargin");
const UILowerString UIComboBox::PropertyName::ListStyle       = UILowerString ("ListStyle");
const UILowerString UIComboBox::PropertyName::ScrollbarMargin = UILowerString ("ScrollbarMargin");
const UILowerString UIComboBox::PropertyName::ScrollbarStyle  = UILowerString ("ScrollbarStyle");
const UILowerString UIComboBox::PropertyName::TextboxMargin   = UILowerString ("TextboxMargin");
const UILowerString UIComboBox::PropertyName::TextboxStyle    = UILowerString ("TextboxStyle");
const UILowerString UIComboBox::PropertyName::DataSource      = UILowerString ("DataSource");
const UILowerString UIComboBox::PropertyName::DropDownHeight  = UILowerString ("DropDownHeight");
const UILowerString UIComboBox::PropertyName::SelectedIndex   = UILowerString ("SelectedIndex");
const UILowerString UIComboBox::PropertyName::SelectedText    = UILowerString ("SelectedText");
const UILowerString UIComboBox::PropertyName::ScrollbarWidth  = UILowerString ("ScrollbarWidth");
const UILowerString UIComboBox::PropertyName::SelectedItem    = UILowerString ("SelectedItem");

//======================================================================================
#define _TYPENAME UIComboBox

namespace UIComboBoxNamespace
{
	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Editable, "", T_bool),
		_DESCRIPTOR(ListMargin, "", T_rect),
		_DESCRIPTOR(ListStyle, "", T_object),
		_DESCRIPTOR(DropDownHeight, "", T_int),
		_DESCRIPTOR(ButtonMargin, "", T_rect),
		_DESCRIPTOR(ButtonStyle, "", T_object),
		_DESCRIPTOR(ButtonWidth, "", T_int),
		_DESCRIPTOR(ScrollbarMargin, "", T_rect),
		_DESCRIPTOR(ScrollbarStyle, "", T_object),
		_DESCRIPTOR(ScrollbarWidth, "", T_int),
		_DESCRIPTOR(TextboxMargin, "", T_rect),
		_DESCRIPTOR(TextboxStyle, "", T_object),
		_DESCRIPTOR(SelectedIndex, "", T_int),
		_DESCRIPTOR(SelectedText, "", T_string),
		_DESCRIPTOR(DataSource, "", T_object),
	_GROUPEND(Basic, 3, 0);
	//================================================================
}
using namespace UIComboBoxNamespace;

//======================================================================================

UIComboBox::UIComboBox () :
UIPage (),
UIEventCallback   (),
mScroll           (0),
mButton           (new UIButton),
mTextbox          (new UITextbox),
mList             (0),
mPopupPage        (0),
mButtonStyle      (0),
mTextboxStyle     (0),
mListStyle        (0),
mScrollbarStyle   (0),
mDataSource       (0),
mEditable         (false),
mMouseIsDown      (false),
mDropDownHeight   (64L),
mSelectedIndex    (-1),
mTextboxMargin    (),
mButtonMargin     (),
mListMargin       (),
mScrollbarMargin  (),
mScrollbarWidth   (16L),
mScrollbarOpacity (-1.0f)
{
	mButton->Attach (this);
	mTextbox->Attach (this);

	mButton->SetName  ("ComboButton");
	mTextbox->SetName ("ComboTextbox");

	mButton->SetTransient (true);
	mTextbox->SetTransient (true);

	SetSize             (UISize (64L, 16L));
	mTextbox->SetHeight (16L);
	mButton->SetHeight  (16L);

	AddChild (mButton);
	AddChild (mTextbox);

	SetButtonWidth      (16L);

	mButton->SetProperty  (UIWidget::PropertyName::PackSize,     Unicode::narrowToWide ("f,a"));
	mButton->SetProperty  (UIWidget::PropertyName::PackLocation, Unicode::narrowToWide ("fff"));
	mTextbox->SetProperty (UIWidget::PropertyName::PackSize,     Unicode::narrowToWide ("a,a"));
	mTextbox->SetProperty (PropertyName::Editable,               Unicode::narrowToWide ("false"));

	mButton->AddCallback (this);
	this->AddCallback (this);
	mTextbox->AddCallback (this);

	SetSelectable (true);
}

//----------------------------------------------------------------------

UIComboBox::~UIComboBox ()
{
	mButton->RemoveCallback (this);
	this->RemoveCallback (this);
	mTextbox->RemoveCallback (this);

	mButton->Detach (this);
	mTextbox->Detach (this);

	SetButtonStyle       (0);
	SetTextboxStyle      (0);
	SetListStyle         (0);
	SetScrollbarStyle    (0);
	SetDataSource        (0);

	if (mPopupPage)
	{
		mPopupPage->RemoveCallback (this);
		mPopupPage->Detach (this);
		mPopupPage = 0;
	}

	if (mList)
	{
		mList->RemoveCallback (this);
		mList->Detach (this);
		mList = 0;
	}

	if (mScroll)
	{
		mScroll->Detach (this);
		mScroll = 0;
	}
}

//----------------------------------------------------------------------

bool UIComboBox::IsA (const UITypeID type) const
{
	return type == TUIComboBox || UIPage::IsA (type);
}

//----------------------------------------------------------------------

const char *UIComboBox::GetTypeName () const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject *UIComboBox::Clone () const
{
	return new UIComboBox;
}

//----------------------------------------------------------------------

void UIComboBox::GetLinkPropertyNames( UIPropertyNameVector & in) const
{
	in.push_back (PropertyName::ButtonStyle    );
	in.push_back (PropertyName::ListStyle      );
	in.push_back (PropertyName::ScrollbarStyle );
	in.push_back (PropertyName::TextboxStyle   );
	in.push_back (PropertyName::DataSource     );
	in.push_back (PropertyName::SelectedText   );

	UIPage::GetLinkPropertyNames (in);
}

//----------------------------------------------------------------------

void UIComboBox::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIPage::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIComboBox::GetPropertyNames( UIPropertyNameVector & in, bool forCopy) const
{
	// PropertyName::SelectedItem is a special property
	// and is not returned by this function

	in.push_back (PropertyName::ButtonMargin   );
	in.push_back (PropertyName::ButtonStyle    );
	in.push_back (PropertyName::ButtonWidth    );
	in.push_back (PropertyName::Editable       );
	in.push_back (PropertyName::ListMargin     );
	in.push_back (PropertyName::ListStyle      );
	in.push_back (PropertyName::ScrollbarMargin);
	in.push_back (PropertyName::ScrollbarStyle );
	in.push_back (PropertyName::ScrollbarWidth );
	in.push_back (PropertyName::TextboxMargin  );
	in.push_back (PropertyName::TextboxStyle   );
	in.push_back (PropertyName::DataSource     );
	in.push_back (PropertyName::DropDownHeight );
	in.push_back (PropertyName::SelectedIndex  );
	in.push_back (PropertyName::SelectedText   );

	UIPage::GetPropertyNames (in, forCopy);
}

//----------------------------------------------------------------------

bool  UIComboBox::SetProperty (const UILowerString & Name, const UIString &Value)
{
	const size_t pSeparator = Name.get ().find ('.');

	if( pSeparator != std::string::npos)
	{
		const int len = pSeparator;

		if( Name.equals (PropertyName::SelectedItem.c_str (), len))
		{
			UIData * const selectedDataObject = GetDataAtIndex(GetSelectedIndex());

			if (selectedDataObject)
				selectedDataObject->SetProperty( UILowerString (Name.get ().substr (pSeparator + 1)), Value );

			return true;
		}
	}

	if (Name == PropertyName::ButtonStyle    )
	{
		UIBaseObject * const object = GetObjectFromPath( Value, TUIButtonStyle );

		if( object || Value.empty() )
		{
			SetButtonStyle( static_cast<UIButtonStyle *>(object) );
			return true;
		}
	}
	else if (Name == PropertyName::ButtonWidth    )
	{
		long width = -1;
		if (UIUtils::ParseLong (Value, width))
		{
			SetButtonWidth (width);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::SelectedIndex )
	{
		long tmp = -1;
		if (UIUtils::ParseLong (Value, tmp))
		{
			SetSelectedIndex (tmp);
			return true;
		}
		return false;
	}

	else if (Name == PropertyName::SelectedText)
	{
		if (mEditable)
		{
			SetSelectedIndex (-1);
			mTextbox->SetText (Value);
			return true;
		}
		return false;
	}

	else if (Name == PropertyName::DropDownHeight    )
	{
		return UIUtils::ParseLong (Value, mDropDownHeight);
	}
	else if (Name == PropertyName::Editable       )
	{
		if (UIUtils::ParseBoolean (Value, mEditable))
		{
			return mTextbox->SetProperty (UITextbox::PropertyName::Editable, Value);
		}
		else
			return false;
	}
	else if (Name == PropertyName::ListStyle )
	{
		UIBaseObject * const object = GetObjectFromPath( Value, TUIListStyle );

		if( object || Value.empty() )
		{
			SetListStyle( static_cast<UIListStyle *>(object) );
			return true;
		}
	}
	else if (Name == PropertyName::ScrollbarStyle )
	{
		UIBaseObject * const object = GetObjectFromPath( Value, TUISliderbarStyle );

		if( object || Value.empty() )
		{
			SetScrollbarStyle( static_cast<UISliderbarStyle *>(object) );
			return true;
		}
	}

	else if (Name == PropertyName::TextboxStyle   )
	{
		UIBaseObject * const object = GetObjectFromPath( Value, TUITextboxStyle );

		if( object || Value.empty() )
		{
			SetTextboxStyle( static_cast<UITextboxStyle *>(object) );
			return true;
		}
	}

	else if (Name == PropertyName::DataSource   )
	{
		UIBaseObject * const object = GetObjectFromPath( Value, TUIDataSource );

		if( object || Value.empty() )
		{
			SetDataSource( static_cast<UIDataSource *>(object) );
			return true;
		}
	}

	else if (Name == PropertyName::ButtonMargin )
	{
		UIRect rect;
		if (UIUtils::ParseRect (Value, rect))
		{
			SetButtonMargin (rect);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::ListMargin )
	{
		UIRect rect;
		if (UIUtils::ParseRect (Value, rect))
		{
			SetListMargin (rect);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::ScrollbarMargin )
	{
		UIRect rect;
		if (UIUtils::ParseRect (Value, rect))
		{
			SetScrollbarMargin (rect);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::TextboxMargin )
	{
		UIRect rect;
		if (UIUtils::ParseRect (Value, rect))
		{
			SetTextboxMargin (rect);
			return true;
		}
		return false;
	}
	else if (Name == PropertyName::ScrollbarWidth)
	{
		UIUtils::ParseLong (Value, mScrollbarWidth);
	}

	return UIPage::SetProperty (Name, Value);
}

//----------------------------------------------------------------------

bool  UIComboBox::GetProperty (const UILowerString & Name, UIString &Value) const
{
	const size_t pSeparator = Name.get ().find ('.');

	if( pSeparator != std::string::npos)
	{
		const int len = pSeparator;

		if( Name.equals (PropertyName::SelectedItem.c_str (), len))
		{
			UIData const * const selectedDataObject = GetDataAtIndex(GetSelectedIndex());

			if (selectedDataObject)
				selectedDataObject->GetProperty( UILowerString (Name.get ().substr (pSeparator + 1)), Value );

			return true;
		}
	}

	if (Name == PropertyName::ButtonStyle    )
	{
		if( mButtonStyle )
		{
			GetPathTo( Value, mButtonStyle );
			return true;
		}
	}
	else if (Name == PropertyName::ButtonWidth    )
	{
		return UIUtils::FormatLong (Value, mButton->GetWidth ());
	}
	else if (Name == PropertyName::DropDownHeight    )
	{
		return UIUtils::FormatLong (Value, mDropDownHeight);
	}
	else if (Name == PropertyName::SelectedIndex    )
	{
		return UIUtils::FormatLong (Value, mSelectedIndex);
	}
	else if (Name == PropertyName::SelectedText)
	{
		mTextbox->GetLocalText (Value);
		return true;
	}
	else if (Name == PropertyName::Editable       )
	{
		return UIUtils::FormatBoolean (Value, mEditable);
	}
	else if (Name == PropertyName::ListStyle )
	{
		if( mListStyle )
		{
			GetPathTo( Value, mListStyle );
			return true;
		}
	}
	else if (Name == PropertyName::ScrollbarStyle )
	{
		if( mScrollbarStyle )
		{
			GetPathTo( Value, mScrollbarStyle );
			return true;
		}
	}

	else if (Name == PropertyName::TextboxStyle   )
	{
		if( mTextboxStyle )
		{
			GetPathTo( Value, mTextboxStyle );
			return true;
		}
	}
	else if (Name == PropertyName::DataSource   )
	{
		if( mDataSource )
		{
			GetPathTo( Value, mDataSource );
			return true;
		}
	}

	else if (Name == PropertyName::ButtonMargin )
	{
		return UIUtils::FormatRect (Value, mButtonMargin);
	}
	else if (Name == PropertyName::ListMargin )
	{
		return UIUtils::FormatRect (Value, mListMargin);
	}
	else if (Name == PropertyName::ScrollbarMargin )
	{
		return UIUtils::FormatRect (Value, mScrollbarMargin);
	}
	else if (Name == PropertyName::TextboxMargin )
	{
		return UIUtils::FormatRect (Value, mTextboxMargin);
	}
	else if (Name == PropertyName::ScrollbarWidth)
	{
		UIUtils::FormatLong (Value, mScrollbarWidth);
	}

	return UIPage::GetProperty (Name, Value);

}

//----------------------------------------------------------------------

bool  UIComboBox::OnMessage            (UIWidget *context, const UIMessage & msg)
{
	if (context == this)
	{
		if (msg.Type == UIMessage::MouseWheel)
		{
			mMouseIsDown = false;
			PerformPopup ();
			return false;
		}
	}
	else if (context == mPopupPage)
	{
		if (mList && mPopupPage->IsVisible ())
		{
			if (
				((msg.Type == UIMessage::KeyDown || msg.Type == UIMessage::KeyRepeat) && msg.Keystroke == UIMessage::Enter) ||
				(msg.Type == UIMessage::Character && msg.Keystroke == ' '))
			{
				const long oldIndex = GetSelectedIndex ();
				SetSelectedIndex (mList->GetLastSelectedRow ());
				return oldIndex == GetSelectedIndex ();
			}
			
			if (msg.MouseCoords.y < mList->GetLocation ().y)
			{
				if (msg.Type == UIMessage::MouseWheel)
				{
					return !mList->ProcessMessage (msg);
				}
				else if (msg.Type == UIMessage::LeftMouseDown)
				{
					SetSelectedIndex (mList->GetLastSelectedRow ());
					return true;
				}
			}
		}
	}
	else if (context == mTextbox)
	{
		if (msg.Type == UIMessage::LeftMouseDown)
		{
			mMouseIsDown = true;
			return false;
		}

		else if (mMouseIsDown && msg.Type == UIMessage::LeftMouseUp)
		{
			mMouseIsDown = false;

			if (!mEditable)
				PerformPopup ();
			return false;
		}

		else if (msg.Type == UIMessage::LeftMouseDoubleClick)
		{
			PerformPopup ();
			return false;
		}

		else if (msg.Type == UIMessage::KeyDown || msg.Type == UIMessage::KeyRepeat)
		{
			if (msg.Keystroke == UIMessage::Enter)
			{
				if (mEditable)
				{
					const long oldIndex = GetSelectedIndex ();
					SetSelectedIndex  (-1, false);
					return oldIndex == -1;
				}
			}
			else if (msg.Keystroke == UIMessage::DownArrow)
			{
				if (!mPopupPage || !mPopupPage->IsVisible ())
				{
					PerformPopup ();
					return false;
				}
			}
		}
	}

	return true;
}


//----------------------------------------------------------------------

void  UIComboBox::OnHide               (UIWidget *context)
{
	if (context == mPopupPage)
	{
		SetFocus ();
//		SetSelectedIndex (mList->GetLastSelectedRow ());
	}
}

//----------------------------------------------------------------------

void  UIComboBox::OnButtonPressed      (UIWidget *context)
{
	if (context == mButton)
	{
		PerformPopup ();
	}
}

//----------------------------------------------------------------------

void UIComboBox::OnGenericSelectionChanged (UIWidget * context)
{
	if (context == mList)
	{
		if (mList->GetSelectionFinalized ())
			SetSelectedIndex (mList->GetLastSelectedRow ());
	}
}

//----------------------------------------------------------------------

void UIComboBox::SetButtonStyle       (UIButtonStyle * style)
{
	RemoveProperty (PropertyName::ButtonStyle);

	if (AttachMember (mButtonStyle, style))
		mButton->SetStyle (style);
}

//----------------------------------------------------------------------

void UIComboBox::SetTextboxStyle      (UITextboxStyle * style)
{
	RemoveProperty (PropertyName::TextboxStyle);

	if (AttachMember (mTextboxStyle, style))
		mTextbox->SetStyle (style);
}

//----------------------------------------------------------------------

void UIComboBox::SetListStyle    (UIListStyle * style)
{
	RemoveProperty (PropertyName::ListStyle);

	if (AttachMember (mListStyle, style) && mList)
		mList->SetStyle (mListStyle);
}

//----------------------------------------------------------------------

void UIComboBox::SetScrollbarStyle    (UISliderbarStyle * style)
{
	RemoveProperty (PropertyName::ScrollbarStyle);

	if (AttachMember (mScrollbarStyle, style) && mScroll)
		mScroll->SetStyle (mScrollbarStyle);
}

//----------------------------------------------------------------------

void UIComboBox::SetDataSource        (UIDataSource * ds)
{
	RemoveProperty (PropertyName::DataSource);

	if (AttachMember (mDataSource, ds))
	{
		if (mPopupPage && mPopupPage->IsVisible ())
			mPopupPage->SetVisible (false);

		SetSelectedIndex (mSelectedIndex);
	}
}

//----------------------------------------------------------------------

void UIComboBox::SetButtonWidth (long width)
{
	mButton->SetSize     (UISize (width, GetHeight ()));
	UpdateLayout ();
}

//----------------------------------------------------------------------

void UIComboBox::Link ()
{
	UIPage::Link ();

	if (mButtonStyle)
		mButtonStyle->Link ();
	if (mTextboxStyle)
		mTextboxStyle->Link ();
	if (mListStyle)
		mListStyle->Link ();
	if (mScrollbarStyle)
		mScrollbarStyle->Link ();

	if (mList)
		mList->SetStyle (mListStyle);

	if (mScroll)
		mScroll->SetStyle (mScrollbarStyle);
}

//----------------------------------------------------------------------

void UIComboBox::PerformPopup ()
{
	mTextbox->ClearSelection ();

	if (!mPopupPage)
	{
		mPopupPage = new UIPage;
		mPopupPage->Attach (this);
		mPopupPage->SetName ("TransientComboPopup");
		mPopupPage->AddCallback (this);
		mPopupPage->SetTransient (true);
	}

	if (!mList)
	{
		mList = new UIList;
		mList->Attach (this);
		mList->SetSelectionAllowedMultiRow (false);
		mList->SetScrollWheelSelection     (true);
		mList->SetName ("TransientComboList");
		mList->AddCallback (this);
		mList->SetTransient (true);
		mPopupPage->AddChild (mList);
	}

	const long dropDownWidth = GetWidth () - mButton->GetWidth () - mButtonMargin.left - mButtonMargin.right - mListMargin.left - mListMargin.right;

	mList->SetSelectionMovesWithMouse (true);
	mList->SetSelectionFinalized (false);
	mList->SetSize (UISize::zero);
	mList->SetDataSource (mDataSource);
	mList->SetBackgroundOpacity (GetBackgroundOpacity ());
	mList->SetStyle (mListStyle);
	mList->UpdateFromStyle ();

	UISize extent;
	mList->GetScrollExtent (extent);
	mList->SetSize (UISize (dropDownWidth, std::min (extent.y, mDropDownHeight - mListMargin.top - mListMargin.bottom)));
	mList->SetLocation (UIPoint (mListMargin.left, GetHeight () + mListMargin.top));

	if (mSelectedIndex >= 0)
		mList->SelectRow (mSelectedIndex);
	else
		mList->SelectRow (0L);

	mPopupPage->SetHeight (mList->GetHeight () + GetHeight () + mListMargin.top + mListMargin.bottom);
	mPopupPage->SetWidth (GetWidth ());

	if (extent.y > mDropDownHeight)
	{
		if (!mScroll)
		{
			mScroll = new UIScrollbar;
			mScroll->Attach (this);
			mScroll->SetName ("TransientComboScroll");
			mScroll->SetTransient (true);
			mPopupPage->AddChild (mScroll);
		}

		mScroll->SetStyle (mScrollbarStyle);
		mScroll->SetSize (UISize (mScrollbarWidth, mDropDownHeight - mScrollbarMargin.top - mScrollbarMargin.bottom));
		mScroll->SetVisible (true);

		if (mScrollbarOpacity >= 0.0f)
			mScroll->SetOpacity (mScrollbarOpacity);
		else
			mScroll->SetOpacity (GetBackgroundOpacity ());

		mScroll->SetLocation (GetWidth () - mScroll->GetWidth () - mScrollbarMargin.right, GetHeight () + mScrollbarMargin.top);
		mScroll->AttachToControl (mList);
	}
	else
	{
		if (mScroll)
			mScroll->SetVisible (false);
	}

	mPopupPage->SetBackgroundColor (UIColor::green);
	mPopupPage->SetBackgroundOpacity (0.0f);
	mPopupPage->SetLocation (GetWorldLocation ());

	if (UIManager::isUIReady()) 
	{
		UIManager::gUIManager().PushContextWidget (*mPopupPage);
	}

	mList->SetFocus ();
}

//-----------------------------------------------------------------

void UIComboBox::SetSelectedIndex (long index, bool doUpdateValue)
{
	const long oldIndex = mSelectedIndex;

	if (mDataSource)
		mSelectedIndex = std::min (index, static_cast<long>(mDataSource->GetChildCount () - 1));
	else
		mSelectedIndex = -1;

	if (mSelectedIndex != oldIndex)
		SendCallback( &UIEventCallback::OnGenericSelectionChanged, UILowerString::null );

	if (doUpdateValue)
		UpdateValue ();

	//-- can't do this as a result of a message from the popup (yet)
	if (mPopupPage && UIManager::isUIReady())
	{
		UIManager::gUIManager ().PopContextWidgetsNextFrame (mPopupPage);
	}
}

//-----------------------------------------------------------------

void UIComboBox::UpdateValue ()
{
	if (mDataSource && mSelectedIndex >= 0)
	{
		const UIData * const data = mDataSource->GetChildByPosition (static_cast<size_t>(mSelectedIndex));

		if (data)
		{
			UIString str;

			if (data->GetProperty (UITextbox::PropertyName::LocalText, str) || data->GetProperty (UITextbox::PropertyName::Text, str))
			{
				mTextbox->SetText (str);
				return;
			}
		}
	}

	mTextbox->SetText (UIString ());
}

//----------------------------------------------------------------------

bool UIComboBox::AddChild( UIBaseObject * child)
{
	if (UIPage::AddChild (child))
	{
		if (child->IsA (TUIWidget))
			static_cast<UIWidget *>(child)->SetTransient (true);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool UIComboBox::InsertChildBefore (UIBaseObject * child, const UIBaseObject * childToPrecede)
{
	if (UIPage::InsertChildBefore (child, childToPrecede))
	{
		if (child->IsA (TUIWidget))
			static_cast<UIWidget *>(child)->SetTransient (true);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool UIComboBox::InsertChildAfter  (UIBaseObject * child, const UIBaseObject * childTosucceed)
{
	if (UIPage::InsertChildAfter (child, childTosucceed))
	{
		if (child->IsA (TUIWidget))
			static_cast<UIWidget *>(child)->SetTransient (true);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

UIData * UIComboBox::GetDataAtIndex (long index)
{
	if (!mDataSource || index < 0 || index >= static_cast<long>(mDataSource->GetChildCount ()))
		return 0;

	return mDataSource->GetChildByPosition (static_cast<size_t>(index));
}

//----------------------------------------------------------------------

const UIData * UIComboBox::GetDataAtIndex (long index) const
{
	if (!mDataSource || index < 0 || index >= static_cast<long>(mDataSource->GetChildCount ()))
		return 0;

	return mDataSource->GetChildByPosition (static_cast<size_t>(index));
}

//----------------------------------------------------------------------

UIDataSource * UIComboBox::GetDataSource ()
{
	return mDataSource;
}

//----------------------------------------------------------------------

const UIDataSource * UIComboBox::GetDataSource () const
{
	return mDataSource;
}

//----------------------------------------------------------------------

void UIComboBox::Pack()
{
	UIPage::Pack ();
	ApplyPacking (mTextbox);
	ApplyPacking (mButton);
}

//----------------------------------------------------------------------

void UIComboBox::SetTextboxMargin   (const UIRect & rect)
{
	if (mTextboxMargin  != rect)
	{
		mTextboxMargin  = rect;
		UpdateLayout ();
	}
}

//----------------------------------------------------------------------

void UIComboBox::SetButtonMargin    (const UIRect & rect)
{
	if (mButtonMargin   != rect)
	{
		mButtonMargin   = rect;
		UpdateLayout ();
	}
}

//----------------------------------------------------------------------

void UIComboBox::SetListMargin      (const UIRect & rect)
{
	if (mListMargin     != rect)
	{
		mListMargin     = rect;
		UpdateLayout ();
	}
}

//----------------------------------------------------------------------

void UIComboBox::SetScrollbarMargin (const UIRect & rect)
{
	if (mScrollbarMargin!= rect)
	{
		mScrollbarMargin = rect;
		UpdateLayout ();
	}
}

//----------------------------------------------------------------------

void UIComboBox::UpdateLayout ()
{
	const UISize & size = GetSize ();

	const long buttonX = size.x - mButton->GetWidth () - mButtonMargin.right;

	mButton->SetSize (UISize (mButton->GetWidth (), size.y - mButtonMargin.top - mButtonMargin.bottom));

	mButton->SetLocation (buttonX, mButtonMargin.top);
	mTextbox->SetSize    (UISize (buttonX - mTextboxMargin.left - mTextboxMargin.right - mButtonMargin.left,
		size.y - mTextboxMargin.top - mTextboxMargin.bottom));

	mTextbox->SetLocation (mTextboxMargin.Location ());
}

//----------------------------------------------------------------------

void UIComboBox::Clear ()
{
	if (mDataSource)
		mDataSource->Clear();
}

//----------------------------------------------------------------------

UIData * UIComboBox::AddItem (const Unicode::String &localizedString, const std::string &name)
{
	UIDataSource * const dataSource = GetDataSource();

	if (dataSource != NULL)
	{
		UIData * const data = new UIData;
		data->SetProperty(UIList::DataProperties::LOCALTEXT, localizedString);
		data->SetName(name);
		dataSource->AddChild(data);

		return data;
	}

	return 0;
}

//----------------------------------------------------------------------

int UIComboBox::GetItemCount() const
{
	int result = 0;

	UIDataSource const * const dataSource = GetDataSource();

	if (dataSource != NULL)
	{
		result = static_cast<int>(dataSource->GetChildCount());
	}

	return result;
}

//----------------------------------------------------------------------

bool UIComboBox::GetIndexLocalText(int const index, Unicode::String &localText) const
{
	bool result = false;
	UIData const * const data = GetDataAtIndex(index);

	if (data != NULL)
	{
		data->GetProperty(UIList::DataProperties::LOCALTEXT, localText);
		result = true;
	}

	return result;
}

//----------------------------------------------------------------------

bool UIComboBox::GetIndexName(int const index, std::string &name) const
{
	bool result = false;
	UIData const * const data = GetDataAtIndex(index);

	if (data != NULL)
	{
		name = data->GetName();
		result = true;
	}

	return result;
}

//----------------------------------------------------------------------

void UIComboBox::GetSelectedIndexLocalText(Unicode::String &localText) const
{
	if (!GetIndexLocalText(GetSelectedIndex(), localText))
	{
		localText = Unicode::String();
	}
}

//----------------------------------------------------------------------

void UIComboBox::GetSelectedIndexName(std::string &name) const
{
	if (!GetIndexName(GetSelectedIndex(), name))
	{
		name = std::string();
	}
}

//======================================================================
