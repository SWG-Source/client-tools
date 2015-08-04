#include "_precompile.h"

#include "UIButton.h"
#include "UIButtonStyle.h"
#include "UICanvas.h"
#include "UIClock.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIDataSourceContainer.h"
#include "UIEventCallback.h"
#include "UIImage.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UIPopupMenuStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIRectangleStyle.h"
#include "UIUtils.h"
#include "UIWidgetRectangleStyles.h"

#include <cassert>
#include <list>
#include <vector>

const char * const UIPopupMenu::TypeName = "PopupMenu";

// ======================================================================
#define _TYPENAME UIPopupMenu

namespace UIPopupMenuNamespace
{
	UIPage * findRootPage(UIBaseObject *);

	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(DataSource, "", T_object),
		_DESCRIPTOR(Style, "", T_object),
	_GROUPEND(Basic, 3, int(UIPropertyCategories::C_Basic));
	//================================================================
}

using namespace UIPopupMenuNamespace;

// ======================================================================

UIPage * UIPopupMenuNamespace::findRootPage(UIBaseObject * const object)
{
	UIBaseObject * parent = (object != 0) ? object->GetParent() : 0;
	UIPage * parentPage = 0;

	while(parent != 0)
	{
		if (object->IsA(TUIPage))
		{
			parentPage = static_cast<UIPage *>(parent);
		}
		parent = parent->GetParent();
	}

	return parentPage;
}

// ======================================================================

const UILowerString UIPopupMenu::PropertyName::DataSource    = UILowerString ("DataSource");
const UILowerString UIPopupMenu::PropertyName::Style         = UILowerString ("Style");

const UILowerString UIPopupMenu::DataProperties::Text        = UILowerString ("Text");
const UILowerString UIPopupMenu::DataProperties::LocalText   = UILowerString ("LocalText");
const UILowerString UIPopupMenu::DataProperties::SubMenu     = UILowerString ("SubMenu");
const UILowerString UIPopupMenu::DataProperties::Opacity     = UILowerString ("Opacity");
const UILowerString UIPopupMenu::DataProperties::IsLabel     = UILowerString ("IsLabel");

//----------------------------------------------------------------------

UIPopupMenu::UIPopupMenu() :
UIPage               (),
UIEventCallback      (),
UINotification       (),
mOwningPage          (0),
mStyle               (0),
mDataSourceContainer (0),
mSelectedName        (),
mMinimumDesiredWidth (0),
mSelectedIndex       (0),
mPopupLocation       (),
mSubMenu             (0),
mSubMenuButton       (0),
mHoverButton         (0),
mHoverCountDown      (0),
mPopupButtonBehavior (true),
mOffset              (),
mButtonWasPressed    (false)
{
	UIClock::gUIClock ().ListenPerFrame (this);
}

//-----------------------------------------------------------------

UIPopupMenu::UIPopupMenu(UIPage * const owningPage) :
UIPage               (),
UIEventCallback      (),
UINotification       (),
mOwningPage          (owningPage),
mStyle               (0),
mDataSourceContainer (0),
mSelectedName        (),
mMinimumDesiredWidth (0),
mSelectedIndex       (0),
mPopupLocation       (),
mSubMenu             (0),
mSubMenuButton       (0),
mHoverButton         (0),
mHoverCountDown      (0),
mPopupButtonBehavior (true),
mOffset              (),
mButtonWasPressed    (false)
{
	UIClock::gUIClock ().ListenPerFrame (this);
}

//-----------------------------------------------------------------

UIPopupMenu::~UIPopupMenu()
{
	UIClock::gUIClock ().StopListening (this);
	SetStyle               (0);
	SetDataSourceContainer (0);
	SetSubMenu             (0, 0);
	SetHoverButton         (0);
}

//-----------------------------------------------------------------

bool UIPopupMenu::IsA( const UITypeID Type ) const
{
	return (Type == TUIPopupMenu) || UIPage::IsA( Type );
}

//-----------------------------------------------------------------

const char *UIPopupMenu::GetTypeName () const
{
	return TypeName;
}

//-----------------------------------------------------------------

UIBaseObject *UIPopupMenu::Clone () const
{
	return new UIPopupMenu(mOwningPage);
}

//-----------------------------------------------------------------

void UIPopupMenu::SetStyle( UIPopupMenuStyle *NewStyle )
{
	RemoveProperty (PropertyName::Style);
	AttachMember (mStyle, NewStyle);

	RecreateButtons ();
}

//-----------------------------------------------------------------

UIStyle *UIPopupMenu::GetStyle () const
{
	return mStyle;
};

//-----------------------------------------------------------------

void UIPopupMenu::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back (UIPopupMenu::PropertyName::DataSource);
	In.push_back (UIPopupMenu::PropertyName::Style);

	UIPage::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UIPopupMenu::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIPage::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIPopupMenu::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back (UIPopupMenu::PropertyName::DataSource);
	In.push_back (UIPopupMenu::PropertyName::Style);

	UIPage::GetPropertyNames( In, forCopy );
}

//-----------------------------------------------------------------

bool UIPopupMenu::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if(Name == PropertyName::DataSource )
	{
		UIDataSourceContainer * const NewDataSource = static_cast<UIDataSourceContainer *>(GetObjectFromPath(Value, TUIDataSourceContainer));

		if( NewDataSource || Value.empty() )
		{
			SetDataSourceContainer (NewDataSource);
			return true;
		}
	}
	else if(Name == PropertyName::Style )
	{
		UIPopupMenuStyle * const NewStyle = static_cast<UIPopupMenuStyle *>(GetObjectFromPath(Value, TUIPopupMenuStyle));

		if (NewStyle || Value.empty())
		{
			SetStyle (NewStyle);
			return true;
		}
	}
	return UIPage::SetProperty( Name, Value );
}

//-----------------------------------------------------------------

bool UIPopupMenu::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if(Name == PropertyName::DataSource )
	{
		if (mDataSourceContainer)
		{
			GetPathTo (Value, mDataSourceContainer);
			return true;
		}
	}
	else if(Name == PropertyName::Style )
	{
		if( mStyle )
		{
			GetPathTo( Value, mStyle );
			return true;
		}
	}

	return UIPage::GetProperty( Name, Value );
}

//-----------------------------------------------------------------

void UIPopupMenu::Link ()
{
	UIPage::Link();

	if (mStyle)
		mStyle->Link ();

	RecreateButtons ();
}

//----------------------------------------------------------------------

bool UIPopupMenu::ProcessMessage (const UIMessage & msg)
{
	mButtonWasPressed = false;

	if (UIPage::ProcessMessage (msg))
		return true;

	if (mButtonWasPressed)
	{
//		UIManager::gUIManager ().PopContextWidgetsNextFrame (this);
		return false;
	}

	//- catch right-click on disabled buttons
	if ((msg.Type == UIMessage::RightMouseDown || msg.Type == UIMessage::RightMouseUp))
	{
		UIWidget * const wid = GetWidgetFromPoint (msg.MouseCoords, true);
		if (!wid)
			return true;

		return false;
	}

	//-- block double clicking
	else if (msg.Type == UIMessage::LeftMouseDoubleClick || msg.Type == UIMessage::MiddleMouseDoubleClick || msg.Type == UIMessage::RightMouseDoubleClick)
		return true;

	return false;
}

//-----------------------------------------------------------------

bool UIPopupMenu::OnMessage         (UIWidget *context, const UIMessage & msg)
{
	if (!mDataSourceContainer)
		return true;

	if (context->GetParent () != this || !context->IsA (TUIButton))
		return true;

	UIButton * const button = static_cast<UIButton *>(context);

	if (msg.Type == UIMessage::ContextRequest)
	{
		if (msg.Data != 0)
			return true;

		spawnSubMenu (*button);

		return false;
	}
	else if (button->IsContextCapable (true) && (msg.Type == UIMessage::RightMouseDown || msg.Type == UIMessage::RightMouseUp))
		return false;
	else if (msg.Type == UIMessage::LeftMouseDoubleClick || msg.Type == UIMessage::MiddleMouseDoubleClick || msg.Type == UIMessage::RightMouseDoubleClick)
		return false;

	return true;
}

//----------------------------------------------------------------------

void UIPopupMenu::OnHoverIn (UIWidget * context)
{
	if (!mDataSourceContainer)
		return;

	if (context->GetParent () != this || !context->IsA (TUIButton))
		return;

	UIButton * const button = static_cast<UIButton *>(context);
	const long index        = GetButtonIndex (button);
	if (index < 0)
		return;

	SetHoverButton (button);
}

//----------------------------------------------------------------------

void UIPopupMenu::OnHoverOut (UIWidget * context)
{
	if (!mDataSourceContainer)
		return;

	if (context->GetParent () != this || !context->IsA (TUIButton))
		return;

	UIButton * const button = static_cast<UIButton *>(context);
	const long index        = GetButtonIndex (button);
	if (index < 0)
		return;

	if (button == mHoverButton)
		SetHoverButton (0);
}

//----------------------------------------------------------------------

void UIPopupMenu::OnButtonPressed (UIWidget * context)
{
	mButtonWasPressed = true;

	UIButton * const button = static_cast<UIButton *>(context);

	if (button->IsContextCapable (true))
	{
		spawnSubMenu (*button);
	}
	else
	{
		mSelectedName = button->GetName ();
		mSelectedIndex = GetButtonIndex (button);

		SendCallback( &UIEventCallback::OnPopupMenuSelection , UILowerString::null);
	}
}

//-----------------------------------------------------------------

long UIPopupMenu::GetItemCount () const
{
	if (!mDataSourceContainer)
		return 0;

	return mDataSourceContainer->GetChildCount ();
};

//----------------------------------------------------------------------

void UIPopupMenu::SetDataSourceContainer( UIDataSourceContainer *NewDataSource )
{
	RemoveProperty (PropertyName::DataSource);
	AttachMember (mDataSourceContainer, NewDataSource);
	RecreateButtons ();
}

//----------------------------------------------------------------------

void UIPopupMenu::SetSubMenu        (UIPopupMenu * pop, UIButton * button )
{
	if (mSubMenuButton && mSubMenuButton != button)
	{
		mSubMenuButton->SetIsToggleButton  (false);
		mSubMenuButton->SetIsToggleDown    (false);
	}


	if (mSubMenu)
		mSubMenu->RemoveCallback (this);

	if (pop)
		pop->AddCallback (this);

	AttachMember (mSubMenu,       pop);
	AttachMember (mSubMenuButton, button);

	if (button)
	{
		button->SetIsToggleButton   (true);
		button->SetIsToggleDown     (true);
	}
}

//----------------------------------------------------------------------

void UIPopupMenu::SetHoverButton    (UIButton * button)
{
	if (AttachMember (mHoverButton, button))
	{
		if (mSubMenu && mHoverButton)
		{
			UIManager::gUIManager ().PopContextWidgets (mSubMenu);
			SetSubMenu (0, 0);
		}

		mHoverCountDown = 10;

//		if (button)
//			button->SetActivated (true);
	}
}

//----------------------------------------------------------------------

void UIPopupMenu::RecreateButtons ()
{
	typedef std::vector<std::string> StringVector;
	StringVector oldSubMenuButtonNames;
	if (mSubMenu && mSubMenuButton)
	{
		UIPopupMenu * curPop   = this;

		while (curPop)
		{
			if (curPop->mSubMenu && curPop->mSubMenuButton)
			{
				oldSubMenuButtonNames.push_back (curPop->mSubMenuButton->GetName ());
				curPop = curPop->mSubMenu;
			}
			else
				break;
		}

		UIManager::gUIManager ().PopContextWidgets (mSubMenu);
	}

	SetSubMenu (0, 0);
	SetPackDirty (true);

	UIBaseObject::UIObjectList olist;
	GetChildren (olist);

	{
		for (UIObjectList::iterator it = olist.begin (); it != olist.end (); ++it)
		{
			UIBaseObject * const obj = *it;
			if (obj->IsA (TUIButton))
				static_cast<UIButton *>(obj)->RemoveCallback (this);

			RemoveChild (obj);
		}
	}

	if (!mDataSourceContainer || !mStyle)
		return;

	//----------------------------------------------------------------------

	const UIWidgetRectangleStyles & rstyles = mStyle->GetRectangleStyles ();

	static const UILowerString RStyleProps [] =
	{
		UIWidgetRectangleStyles::PropertyName::Default,
		UIWidgetRectangleStyles::PropertyName::Disabled,
		UIWidgetRectangleStyles::PropertyName::Selected,
		UIWidgetRectangleStyles::PropertyName::MouseOver,
		UIWidgetRectangleStyles::PropertyName::MouseOverSelected,
		UIWidgetRectangleStyles::PropertyName::Activated,
		UIWidgetRectangleStyles::PropertyName::MouseOverActivated
	};

	static const int numProps = sizeof (RStyleProps) / sizeof (RStyleProps [0]);

	{
		Unicode::String str;
		for (int i = 0; i < UIWidgetRectangleStyles::RS_LastStyle; ++i)
		{
			UIWidgetRectangleStyles::RectangleStyle styleId = static_cast<UIWidgetRectangleStyles::RectangleStyle>(i);

			const UIRectangleStyle * const rstyle = rstyles.GetStyle (styleId);

			if (rstyle)
				GetPathTo (str, rstyle);
			else
				str.clear ();

			if (i < numProps)
			{
				const UILowerString & prop = RStyleProps [i];
				SetProperty (prop, str);
			}
		}
	}

	//----------------------------------------------------------------------

	SetBackgroundOpacity (mStyle->GetBackgroundOpacity ());

	//----------------------------------------------------------------------

	UIButtonStyle * const style = mStyle->GetButtonStyle ();

	if (!style)
		return;

	UIButtonStyle * styleLabel = mStyle->GetButtonStyleLabel ();
	if (!styleLabel)
		styleLabel = style;

	const UIDataSourceContainer::DataSourceBaseList & dataSourceList = mDataSourceContainer->GetDataSourceBaseList ();
	typedef std::vector<UIButton *> ButtonVector;
	ButtonVector bv;

	{
		int index = 0;
		for (UIDataSourceContainer::DataSourceBaseList::const_iterator it = dataSourceList.begin (); it != dataSourceList.end (); ++it, ++index)
		{
			const UIDataSourceBase * const dsb = *it;

			if (!dsb->IsA (TUIDataSource))
				break;

			const UIDataSource * const ds = static_cast<const UIDataSource *>(*it);

			UIButton * const butt = new UIButton;

			butt->SetIsPopupButton (mPopupButtonBehavior);

			Unicode::String dummy;
			if (ds->GetProperty (DataProperties::SubMenu, dummy))
			{
				butt->SetContextCapable (true, true);
				butt->SetIsPopupEatPress (true);
			}
			else
			{
				butt->SetIsAllMouseButtons (true);
			}

			butt->AddCallback (this);
			butt->SetStyle (style);
			butt->SetPropertyLong(UIButton::PropertyName::MaxTextLines, 0);
			butt->SetName (ds->GetName ());

			UIString text;
			if (!ds->GetProperty (DataProperties::LocalText, text) &&
				!ds->GetProperty (DataProperties::Text, text))
				text.clear ();

			bool isLabel = false;
			if (ds->GetPropertyBoolean (DataProperties::IsLabel, isLabel))
			{
				butt->SetGetsInput (false);
				butt->SetStyle     (styleLabel);
			}

			float opacity = 1.0f;
			if (ds->GetPropertyFloat (DataProperties::Opacity, opacity))
				butt->SetOpacity (opacity);

			bool enabled;
			if (ds->GetPropertyBoolean (UIWidget::PropertyName::Enabled, enabled))
				butt->SetEnabled (enabled);
			else
				butt->SetEnabled (true);

			butt->SetText (text);
			butt->SetTransient (true);

			AddChild (butt);

			if (butt->IsContextCapable (true))
			{
				UIImage * const image = new UIImage;
				image->SetVisible (true);
				image->SetStyle   (mStyle->GetIconSubmenu ());
				AddChild (image);
			}
		}
	}

	LayoutButtons ();

	//----------------------------------------------------------------------
	//-- restore open states of old menus

	if (!oldSubMenuButtonNames.empty ())
	{
		UIPopupMenu * curPop   = this;

		StringVector::const_iterator it = oldSubMenuButtonNames.begin ();

		while (curPop && it != oldSubMenuButtonNames.end ())
		{
			const std::string & oldSubMenuButtonName = *it;
			++it;
			UIButton * const oldSubMenuButton = static_cast<UIButton *>(curPop->GetObjectFromPath (oldSubMenuButtonName.c_str (), TUIButton));
			if (oldSubMenuButton)
			{
				curPop->spawnSubMenu (*oldSubMenuButton);
				curPop = curPop->mSubMenu;
				curPop->RecreateButtons ();
			}
			else
				break;
		}
	}
}

//----------------------------------------------------------------------

void UIPopupMenu::LayoutButtons ()
{
	if (!mStyle)
		return;

	UIButtonStyle * const bstyle = mStyle->GetButtonStyle ();

	if (!bstyle)
		return;

	long maxWidth = mMinimumDesiredWidth;
	const long itemHeight = mStyle->GetItemHeight ();

	const UIBaseObject::UIObjectList & olist = GetChildrenRef ();

	{
		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			if (!(*it)->IsA (TUIButton))
				continue;

			UIButton * const butt = static_cast<UIButton *>(*it);

			UISize size;
			if (butt->GetDesiredSize (size))
				maxWidth = std::max (maxWidth, size.x + size.y * 2L);
		}
	}

	const long RIGHT_ICON_MARGIN = 2L;
	const UIRect & bmargin = bstyle->GetTextMargin (UIButtonStyle::Normal);
	UIPoint iconSize (bmargin.right - RIGHT_ICON_MARGIN, itemHeight - bmargin.top - bmargin.bottom);
	iconSize.y = iconSize.x = std::min (iconSize.x, iconSize.y);

	UIPoint pos;
	long buttonHeight = itemHeight;
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it, pos.y += buttonHeight)
	{
		if (!(*it)->IsA (TUIButton))
			continue;

		UIButton * const butt = static_cast<UIButton *>(*it);
		butt->SetLocation (pos);

		Unicode::String buttonText;
		butt->GetProperty (DataProperties::Text, buttonText);
		
		short numLines = 1;
		Unicode::String::size_type position = buttonText.find(Unicode::unicode_char_t('\n'));
  		while (position != Unicode::String::npos)
		{
			++numLines;
   			position = buttonText.find(Unicode::unicode_char_t('\n'), position + 1);
		}

		buttonHeight =	itemHeight * numLines;
		butt->SetSize (UISize (maxWidth, buttonHeight));

		if (butt->IsContextCapable (true))
		{
			++it;

			if (it == olist.end ())
			{
				assert (false);
				break;
			}

			UIBaseObject * const objImage = *it;

			if (objImage->IsA (TUIImage))
			{
				UIImage * const image = static_cast<UIImage *>(objImage);
				image->SetSize (iconSize);
				image->SetLocation (UIPoint (pos.x + maxWidth - iconSize.x - RIGHT_ICON_MARGIN, pos.y + (buttonHeight - iconSize.y) / 2L));
			}
			else
			{
				assert (false);
			}
		}
	}

	SetMaximumSize (UISize (16384L, 16384L));
	SetMinimumSize (UISize (0L, 0L));

	const UISize size (maxWidth, pos.y);

	SetMaximumSize (size);
	SetMinimumSize (size);
	SetMaximumSize (size);
}

//----------------------------------------------------------------------

void UIPopupMenu::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, UINotification::Code NotificationCode )
{
	UI_UNREF (ContextObject);

	if( NotifyingObject == mDataSourceContainer )
	{
		switch( NotificationCode )
		{
		case UINotification::ChildAdded:
			{
//				UIData * const data = static_cast<UIData *>(ContextObject);
			}
			//-- fall through
		case UINotification::ChildRemoved:
		case UINotification::ChildChanged:
			RecreateButtons ();
			//			SetActiveTab (mActiveTab);
			break;
		}
	}
	else
	{
		if (mHoverButton)
		{
			bool hoverPositive = mHoverCountDown > 0;

			if (hoverPositive)
			{
				mHoverCountDown -= UIClock::gUIClock ().GetLastFrameTime ();

				if (mHoverCountDown <= 0)
				{
					spawnSubMenu (*mHoverButton);
					//				SetHoverButton (0);
					//-- spawn submenu
				}
			}
		}
	}
}

//----------------------------------------------------------------------

const UIDataSource * UIPopupMenu::GetSelectedData   () const
{
	if (mDataSourceContainer)
	{
		UIDataSourceBase * const dsb = mDataSourceContainer->GetChildByPositionLinear (mSelectedIndex);
		if (dsb->IsA (TUIDataSource))
			return static_cast<UIDataSource *>(dsb);
	}
	return 0;
}

//----------------------------------------------------------------------

UIDataSource * UIPopupMenu::AddItem  (const Unicode::NarrowString & name, const Unicode::String & localLabel)
{
	UIDataSource * const ds = AddItem (name, name);

	if (ds)
	{
		ds->SetProperty (DataProperties::Text, localLabel);
		ds->SetProperty (DataProperties::LocalText, localLabel);
	}

	return ds;
}

//-----------------------------------------------------------------

UIDataSource * UIPopupMenu::AddItem (const Unicode::NarrowString & name, const Unicode::NarrowString & globalText)
{
	if (!mDataSourceContainer)
	{
		UIDataSourceContainer * const source = new UIDataSourceContainer;
		assert (source);
		source->SetName ("DefaultDataSource");

		//-- the data source will be deleted when the popupmenu detaches from it in the dtor
		SetDataSourceContainer (source);
	}

	assert (mDataSourceContainer);

	UIDataSource * const ds = new UIDataSource;
	ds->SetName (name);
	ds->SetProperty (DataProperties::Text, Unicode::narrowToWide (globalText));

	mDataSourceContainer->AddChild (ds);

	return ds;
}

//-----------------------------------------------------------------

void UIPopupMenu::ClearItems ()
{
	if (mDataSourceContainer)
		mDataSourceContainer->Clear ();
}

//----------------------------------------------------------------------

void UIPopupMenu::SetMinimumDesiredWidth (long width)
{
	mMinimumDesiredWidth = width;
	LayoutButtons ();
}

//----------------------------------------------------------------------

long UIPopupMenu::GetButtonIndex (const UIButton * button) const
{
	long count = 0;

	const UIBaseObject::UIObjectList & olist = GetChildrenRef ();

	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		if (!(*it)->IsA (TUIButton))
			continue;

		if (button == static_cast<const UIBaseObject *>(*it))
			return count;

		++count;
	}

	return -1;
}

//----------------------------------------------------------------------

void UIPopupMenu::SetPopupLocation  (const UIPoint & popupLocation)
{
	mPopupLocation = popupLocation;
}

//----------------------------------------------------------------------

void UIPopupMenu::spawnSubMenu (UIButton & button)
{
	const long index        = GetButtonIndex (&button);
	if (index < 0)
		return;

	UIDataSourceBase * const dsb = mDataSourceContainer->GetChildByPositionLinear (index);

	if (!dsb || !dsb->IsA (TUIDataSource))
		return;

	std::string subMenuStr;
	dsb->GetPropertyNarrow (DataProperties::SubMenu, subMenuStr);

	if (!subMenuStr.empty ())
	{
		if (mSubMenu)
		{
			UIManager::gUIManager ().PopContextWidgets (mSubMenu);
			SetSubMenu (0, 0);
		}

		UIDataSourceContainer * const dsc = static_cast<UIDataSourceContainer *>(dsb->GetObjectFromPath (subMenuStr.c_str (), TUIDataSourceContainer));
		if (dsc)
		{
			UIPopupMenu * const pop = new UIPopupMenu(findRootPage(&button));
			pop->SetName                (Unicode::wideToNarrow (button.GetLocalText ()));
			pop->SetStyle               (mStyle);
			pop->SetDataSourceContainer (dsc);

			UIPoint pt = button.GetWorldLocation ();
			pt.x += button.GetWidth  ();
			pt.y += button.GetHeight () / 4L;
			pt.x -= button.GetHeight () / 2L;

			pop->SetLocation   (pt);
			UIManager::gUIManager ().PushContextWidget (*pop);
			SetSubMenu (pop, &button);
		}
	}
}

//----------------------------------------------------------------------

void UIPopupMenu::OnPopupMenuSelection (UIWidget * context)
{
	if (context == mSubMenu)
	{
		mSelectedName  = mSubMenu->GetSelectedName ();
		mSelectedIndex = GetButtonIndex (mSubMenuButton);
		SendCallback( &UIEventCallback::OnPopupMenuSelection , UILowerString::null);
	}
}

//----------------------------------------------------------------------

void UIPopupMenu::SetPopupButtonBehavior (bool b)
{
	mPopupButtonBehavior = b;
}

//----------------------------------------------------------------------

void UIPopupMenu::SetOffsetIndex         (int offsetIndex)
{
	mOffset.x = 0L;

	if (mStyle)
	{
		const long itemHeight = mStyle->GetItemHeight ();
		mOffset.y = - offsetIndex * itemHeight - (itemHeight / 2L);
	}
	else
		mOffset.y = 0L;
}

//----------------------------------------------------------------------

UIPopupMenu * UIPopupMenu::CreatePopupMenu       (UIWidget & widget)
{
	UIPopupMenuStyle * const style = widget.FindPopupStyle ();

	if (style)
	{
		UIPopupMenu * const pop = new UIPopupMenu(findRootPage(&widget));
		pop->SetStyle (style);
		return pop;
	}

	return 0;
}

//----------------------------------------------------------------------

UIPage * UIPopupMenu::GetOwningPage() const
{
	return mOwningPage;
}

//----------------------------------------------------------------------
