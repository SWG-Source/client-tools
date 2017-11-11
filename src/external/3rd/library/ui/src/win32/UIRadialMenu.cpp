#include "_precompile.h"

#include "UIButton.h"
#include "UIButtonStyle.h"
#include "UICanvas.h"
#include "UIClock.h"
#include "UIData.h"
#include "UIDataSource.h"
#include "UIDataSourceContainer.h"
#include "UIEventCallback.h"
#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIMessage.h"
#include "UINamespace.h"
#include "UIPage.h"
#include "UIPopupMenu.h"
#include "UIPropertyDescriptor.h"
#include "UIRadialMenu.h"
#include "UIRadialMenuStyle.h"
#include "UIRectangleStyle.h"
#include "UIUtils.h"
#include "UIWidgetRectangleStyles.h"

#include <cassert>
#include <vector>
#include <cmath>

const char *UIRadialMenu::TypeName	= "RadialMenu";

//----------------------------------------------------------------------

const UILowerString UIRadialMenu::DataPropertyName::Text              = UILowerString ("Text");
const UILowerString UIRadialMenu::DataPropertyName::Icon              = UILowerString ("Icon");

const UILowerString UIRadialMenu::PropertyName::DataSource            = UILowerString ("DataSource");
const UILowerString UIRadialMenu::PropertyName::Style                 = UILowerString ("Style");
const UILowerString UIRadialMenu::PropertyName::PopupDataNamespace    = UILowerString ("PopupDataNamespace");
const UILowerString UIRadialMenu::PropertyName::RadialCenterPrototype = UILowerString ("RadialCenterPrototype");
const UILowerString UIRadialMenu::PropertyName::RadialCenterMargin    = UILowerString ("RadialCenterMargin");

//======================================================================================
#define _TYPENAME UIRadialMenu

namespace UIRadialMenuNamespace
{
	const float MY_PI = 3.1415926535897932384626433832795f;

	const int   s_max_buttons = UIRadialMenuStyle::BST_NumButtonStyles;
	const float s_max_buttons_recip = 1.0f / static_cast<float>(s_max_buttons);

	const UIRadialMenuStyle::ButtonStyleType s_buttonStyleTypes [s_max_buttons] =
	{
		UIRadialMenuStyle::BST_N,
		UIRadialMenuStyle::BST_NE,
		UIRadialMenuStyle::BST_E,
		UIRadialMenuStyle::BST_SE,
		UIRadialMenuStyle::BST_S,
		UIRadialMenuStyle::BST_SW,
		UIRadialMenuStyle::BST_W,
		UIRadialMenuStyle::BST_NW
	};

	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(DataSource, "", T_object),
		_DESCRIPTOR(PopupDataNamespace, "", T_object),
		_DESCRIPTOR(RadialCenterMargin, "", T_rect),
		_DESCRIPTOR(RadialCenterPrototype, "", T_object),
		_DESCRIPTOR(Style, "", T_object),
	_GROUPEND(Basic, 3, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UIRadialMenuNamespace;
//======================================================================================

UIRadialMenu::UIRadialMenu() :
UIPage               (),
mStyle               (0),
mDataSource          (0),
mSelectedName        (),
mHoverButtonIndex    (-1),
mHoverButtonTime     (0),
mLastNotifyTime      (0),
mPopupMenuVanishTime (0),
mPopupButtonIndex    (-1),
mPopupMenu           (0),
mPopupDataNamespace  (0),
mRecursionGuard      (false),
mRadialCenterWidget  (0),
mRadialCenterMargin  ()
{
	AddCallback (this);
	if (IsVisible ())
		UIClock::gUIClock ().ListenPerFrame (this);
	SetSelectable (true);
	SetPropertyBoolean (UIWidget::PropertyName::TextOpacityRelativeApply, true);
	SetPropertyFloat   (UIWidget::PropertyName::TextOpacityRelativeMin,   1.0f);
}

//-----------------------------------------------------------------

UIRadialMenu::~UIRadialMenu()
{
	RemoveCallback (this);
	UIClock::gUIClock ().StopListening (this);
	SetStyle(0);
	SetDataSource (0);
	SetPopupMenu (0);
	SetPopupDataNamespace (0);
	SetRadialCenterPrototype (0);
}

//-----------------------------------------------------------------

bool UIRadialMenu::IsA( const UITypeID Type ) const
{
	return (Type == TUIRadialMenu) || UIPage::IsA( Type );
}

//-----------------------------------------------------------------

const char *UIRadialMenu::GetTypeName () const
{
	return TypeName;
}

//-----------------------------------------------------------------

UIBaseObject *UIRadialMenu::Clone () const
{
	return new UIRadialMenu;
}

//-----------------------------------------------------------------

bool UIRadialMenu::ProcessMessage( const UIMessage &msg )
{
	if (UIPage::ProcessMessage (msg))
		return true;

	UIManager::gUIManager ().SetKeystrokePopsContext (false);

	bool processed = false;

	if (msg.Type == UIMessage::Character)
	{
		if (isdigit (msg.Keystroke) && msg.Keystroke != '0')
		{
			const int index = msg.Keystroke - '1';

			const UIBaseObject::UIObjectList & olist = GetChildrenRef ();

			int i = 0;
			for (UIObjectList::const_iterator it = olist.begin (); it != olist.end() && !processed; ++it)
			{
				UIBaseObject * const obj = *it;
				if (obj->IsA (TUIButton))
				{
					if (i++ == index)
					{
						static_cast<UIButton *>(obj)->Press ();
						processed = true;
					}
				}
			}
		}
	}

	return processed;
}

//-----------------------------------------------------------------

void UIRadialMenu::Render( UICanvas &DestinationCanvas ) const
{
	UIPage::Render (DestinationCanvas);
}

//-----------------------------------------------------------------

void UIRadialMenu::SetStyle( UIRadialMenuStyle *NewStyle )
{
	if (mStyle)
		mStyle->StopListening (this);

	AttachMember (mStyle, NewStyle);

	if (mStyle)
		mStyle->Listen (this);

	RecreateButtons ();
}

//-----------------------------------------------------------------

UIStyle *UIRadialMenu::GetStyle () const
{
	return mStyle;
};

//-----------------------------------------------------------------

void UIRadialMenu::GetLinkPropertyNames( UIPropertyNameVector &In ) const
{
	In.push_back (PropertyName::DataSource);
	In.push_back (PropertyName::PopupDataNamespace);
	In.push_back (PropertyName::RadialCenterPrototype);
	In.push_back (PropertyName::Style);

	UIPage::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UIRadialMenu::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIPage::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UIRadialMenu::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back (PropertyName::DataSource);
	In.push_back (PropertyName::PopupDataNamespace);
	In.push_back (PropertyName::RadialCenterMargin);
	In.push_back (PropertyName::RadialCenterPrototype);
	In.push_back (PropertyName::Style);

	UIPage::GetPropertyNames( In, forCopy );
}

//-----------------------------------------------------------------

bool UIRadialMenu::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::DataSource )
	{
		UIDataSource * const NewDataSource = static_cast<UIDataSource *>(GetObjectFromPath(Value, TUIDataSource));

		if( NewDataSource || Value.empty() )
		{
			SetDataSource (NewDataSource);
			return true;
		}
	}
	else if( Name == PropertyName::Style )
	{
		UIRadialMenuStyle * const NewStyle = static_cast<UIRadialMenuStyle *>(GetObjectFromPath(Value, TUIRadialMenuStyle));

		if (NewStyle || Value.empty())
		{
			SetStyle (NewStyle);
			return true;
		}
	}
	else if( Name == PropertyName::PopupDataNamespace )
	{
		UINamespace * const ns = static_cast<UINamespace *>(GetObjectFromPath(Value, TUINamespace));

		if (ns || Value.empty())
		{
			SetPopupDataNamespace (ns);
			return true;
		}
	}
	else if (Name == PropertyName::RadialCenterMargin)
	{
		if (UIUtils::ParseRect (Value, mRadialCenterMargin))
		{
			LayoutButtons ();
			return true;
		}
		return false;
	}

	else if (Name == PropertyName::RadialCenterPrototype)
	{
		UIWidget * const w = static_cast<UIWidget *>(GetObjectFromPath(Value, TUIWidget));

		if (w)
		{
			SetRadialCenterPrototype (w);
		}
	}

	return UIPage::SetProperty( Name, Value );
}

//-----------------------------------------------------------------

bool UIRadialMenu::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::DataSource )
	{
		if (mDataSource)
		{
			GetPathTo (Value, mDataSource);
			return true;
		}
	}
	else if( Name == PropertyName::Style )
	{
		if( mStyle )
		{
			GetPathTo( Value, mStyle );
			return true;
		}
	}
	else if( Name == PropertyName::PopupDataNamespace )
	{
		if( mPopupDataNamespace )
		{
			GetPathTo( Value, mPopupDataNamespace );
			return true;
		}
	}

	else if (Name == PropertyName::RadialCenterMargin)
	{
		return UIUtils::FormatRect (Value, mRadialCenterMargin);
	}

	return UIPage::GetProperty( Name, Value );
}

//-----------------------------------------------------------------

void UIRadialMenu::Link ()
{
	UIPage::Link();

	if (mStyle)
		mStyle->Link ();

	RecreateButtons ();
}

//-----------------------------------------------------------------

bool UIRadialMenu::OnMessage (UIWidget * context, const UIMessage & msg)
{
	assert (context);

	if (context->GetParent () != this)
	{
		return true;
	}

	if (msg.Type == UIMessage::MouseEnter)
	{
		mHoverButtonTime = 0;
		mHoverButtonIndex = 0;

		bool found = false;
		const UIBaseObject::UIObjectList & olist = GetChildrenRef ();
		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			const UIBaseObject * const obj = *it;
			if (obj == context)
			{
				found = true;
				break;
			}

			if (obj->IsA (TUIButton))
				++mHoverButtonIndex;
		}

		if (!found)
		{
			mHoverButtonIndex = -1;
			return true;
		}

		//----------------------------------------------------------------------
		//-- if the popupmenu is already visible for another button, try to spawn it here.

	}
	else if (msg.Type == UIMessage::MouseExit)
	{
		mHoverButtonTime     = 0;
		mHoverButtonIndex    = - 1;
		mPopupMenuVanishTime = 0;
	}


	return true;
}

//----------------------------------------------------------------------

void UIRadialMenu::OnButtonPressed (UIWidget * context)
{
	UIButton * const button = static_cast<UIButton *>(context);

	mSelectedName = button->GetName ();

	SendCallback( &UIEventCallback::OnPopupMenuSelection , UILowerString::null);
}

//----------------------------------------------------------------------

void UIRadialMenu::OnPopupMenuSelection (UIWidget * context)
{
	//-- forward popup menu selections
	if (context == mPopupMenu)
	{
		if (mRecursionGuard)
			return;

		mRecursionGuard = true;
		SendCallback( &UIEventCallback::OnPopupMenuSelection , context, UILowerString::null);
		mRecursionGuard = false;
	}
}

//----------------------------------------------------------------------

void UIRadialMenu::OnShow (UIWidget * context)
{
	if (context == this)
		UIClock::gUIClock ().ListenPerFrame (this);
}

//----------------------------------------------------------------------

void UIRadialMenu::OnHide (UIWidget * context)
{
	if (context == this)
		UIClock::gUIClock ().StopListening (this);
}

//-----------------------------------------------------------------

long UIRadialMenu::GetItemCount () const
{
	if (!mDataSource)
		return 0;

	return mDataSource->GetChildCount ();
};

//----------------------------------------------------------------------

void UIRadialMenu::SetDataSource( UIDataSource *NewDataSource )
{
//	if( NewDataSource == mDataSource )
//		return;

	if( NewDataSource )
	{
		NewDataSource->Listen( this );
		NewDataSource->Attach( this );
	}

	if( mDataSource )
	{
		mDataSource->StopListening( this );
		mDataSource->Detach( this );
	}

	mDataSource = NewDataSource;

	RecreateButtons ();
}

//----------------------------------------------------------------------

void UIRadialMenu::RecreateButtons ()
{
	UIBaseObject::UIObjectList olist;
	GetChildren (olist);

	{
		for (UIObjectList::iterator it = olist.begin (); it != olist.end (); ++it)
		{
			UIBaseObject * const obj = *it;
			if (obj->IsA (TUIButton))
			{
				static_cast<UIButton *>(obj)->RemoveCallback (this);
				RemoveChild (obj);
			}
		}
	}

	if (!mDataSource || !mStyle)
		return;

	UIObjectList dataList;

	mDataSource->GetChildren (dataList);

	{
		int index = 0;
		for (UIObjectList::iterator it = dataList.begin (); it != dataList.end () && index < s_max_buttons; ++it, ++index)
		{
			UIData * const data = static_cast<UIData *>(*it);

			const UIRadialMenuStyle::ButtonStyleType bst = s_buttonStyleTypes [index];

			UIButton * const butt = new UIButton;
			butt->SetStyle (mStyle->GetButtonStyle (bst));
			butt->SetIsPopupButton     (true);
			butt->SetIsAllMouseButtons (true);
			butt->AddCallback (this);

			butt->SetName (data->GetName ());

			UIString text;
			if (!data->GetProperty (DataPropertyName::Text, text))
				text = Unicode::narrowToWide ("UNNAMED");

			float opacity = 1.0f;
			if (data->GetPropertyFloat (UIWidget::PropertyName::Opacity, opacity))
				butt->SetOpacity (opacity);

			UIColor color;
			if (data->GetPropertyColor (UIWidget::PropertyName::Color, color))
				butt->SetColor(color);
			else
				butt->SetColor(UIColor::white);
			
			bool enabled;
			if (data->GetPropertyBoolean (UIWidget::PropertyName::Enabled, enabled))
				butt->SetEnabled (enabled);
			else
				butt->SetEnabled (true);

			UIString iconPath;
			if (data->GetProperty (DataPropertyName::Icon, iconPath))
				butt->SetProperty (UIButton::PropertyName::Icon, iconPath);

			butt->SetText (text);
			butt->SetTransient (true);

			AddChild (butt);

			butt->Link ();
		}
	}

	LayoutButtons ();
}


//----------------------------------------------------------------------

void UIRadialMenu::LayoutButtons ()
{
	if (!mStyle)
		return;

	const UIBaseObject::UIObjectList & olist = GetChildrenRef ();

	int buttonIndex = 0;

	const long radius    = mStyle->GetItemRadius ();
	UIRect rectFromCenter (-radius, -radius, radius + 1L, radius + 1L);

	UISize centerSize;

	if (mRadialCenterWidget)
	{
		mRadialCenterWidget->Center ();
		mRadialCenterWidget->SetVisible (true);

		centerSize = UISize (radius * 2L, radius * 2L);
		mRadialCenterWidget->SetMaximumSize (centerSize);
		mRadialCenterWidget->SetMinimumSize (centerSize);
		mRadialCenterWidget->SetMaximumSize (centerSize);
		mRadialCenterWidget->SetSize        (centerSize);
	}

	{
		const float f_radius = static_cast<float>(radius);
		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			UIBaseObject * const obj = *it;
			if (!obj->IsA (TUIButton))
				continue;

			UIButton * const butt = static_cast<UIButton *>(obj);

			if (!butt->IsVisible () || buttonIndex >= s_max_buttons)
				break;

			const UIRadialMenuStyle::ButtonStyleType bst = s_buttonStyleTypes [buttonIndex];

			const float angle = (MY_PI * 0.5f) -(buttonIndex * MY_PI * 2.0f) * s_max_buttons_recip;
			UIPoint buttLocation;

			{
				UISize dsize;
				butt->GetDesiredSize (dsize);
				butt->SetSize (dsize);
			}

			const UISize & bsize = butt->GetSize ();

			if (bst == UIRadialMenuStyle::BST_N)
			{
				buttLocation.x = -bsize.x / 2L;
				buttLocation.y = -radius - bsize.y;
			}
			else if (bst == UIRadialMenuStyle::BST_S)
			{
				buttLocation.x = -bsize.x / 2L;
				buttLocation.y = radius;
			}
			else if (bst == UIRadialMenuStyle::BST_E)
			{
				buttLocation.x = radius;
				buttLocation.y = -bsize.y / 2L;
			}
			else if (bst == UIRadialMenuStyle::BST_W)
			{
				buttLocation.x = -radius - bsize.x;
				buttLocation.y = -bsize.y / 2L;
			}
			else
			{
				buttLocation.x =  static_cast<long>(f_radius * cos (angle));
				buttLocation.y = -static_cast<long>(f_radius * sin (angle));

				if (buttLocation.x < 0L)
					buttLocation.x -= bsize.x;

				if (buttLocation.y < 0L)
					buttLocation.y -= bsize.y;
			}

			butt->SetLocation (buttLocation);

			const UIRect & buttRect = butt->GetRect ();

			rectFromCenter.left   = std::min (buttRect.left,    rectFromCenter.left);
			rectFromCenter.top    = std::min (buttRect.top,     rectFromCenter.top);
			rectFromCenter.right  = std::max (buttRect.right,   rectFromCenter.right);
			rectFromCenter.bottom = std::max (buttRect.bottom,  rectFromCenter.bottom);

			++buttonIndex;
		}
	}

	rectFromCenter.left   = std::min (rectFromCenter.left,    -rectFromCenter.right);
	rectFromCenter.right  = std::max (rectFromCenter.right,   -rectFromCenter.left);
	rectFromCenter.top    = std::min (rectFromCenter.top,     -rectFromCenter.bottom);
	rectFromCenter.bottom = std::max (rectFromCenter.bottom,  -rectFromCenter.top);

	const UISize & rectSize = rectFromCenter.Size ();
	SetMaximumSize (rectSize);
	SetMinimumSize (rectSize);
	SetMaximumSize (rectSize);
	SetSize (rectSize);

	if (mRadialCenterWidget)
	{
		mRadialCenterWidget->Center ();
	}

	{
		const UIPoint & rectLoc = rectFromCenter.Location ();

		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			UIBaseObject * const obj = *it;
			if (!obj->IsA (TUIButton))
				continue;

			UIButton * const butt = static_cast<UIButton *>(obj);

			if (!butt->IsVisible ())
				break;

			const UIPoint & buttLocation = butt->GetLocation ();
			butt->SetLocation (buttLocation - rectLoc);
		}
	}
}

//----------------------------------------------------------------------

void UIRadialMenu::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, UINotification::Code NotificationCode )
{
	UI_UNREF (ContextObject);
 
	if (NotifyingObject == mStyle)
	{
		RecreateButtons ();
		return;
	}

	if( NotifyingObject == mDataSource )
	{
		switch( NotificationCode )
		{
		case UINotification::ChildAdded:
		case UINotification::ChildRemoved:
		case UINotification::ChildChanged:
			RecreateButtons ();
			break;
		}

		return;
	}

	if (!mStyle)
		return;

	const unsigned long currentTime = UIClock::gUIClock ().GetTime ();

	if (mPopupMenu && !mPopupMenu->GetParent ())
		SetPopupMenu (0);

	//-- see if we need to vanish our popupmenu
	if (mPopupMenu && mPopupMenu->IsVisible () && !mPopupMenu->IsUnderMouse ())
	{
		if (mHoverButtonIndex < 0)
		{
			mPopupMenuVanishTime += (currentTime - mLastNotifyTime);

			//-- disappear timeout is 2x the appear timeout
			if (mPopupMenuVanishTime >= mStyle->GetItemPopupTimeout () * 2L)
			{
				mPopupMenu->SetVisible (false);
				mPopupButtonIndex = -1;
				SetPopupMenu (0);
			}
		}
	}

	//-- see if we  need to spawn a new popupmenu or make the current one visible
	if (mHoverButtonIndex >= 0 && (!mPopupMenu || !mPopupMenu->IsVisible () || !mPopupMenu->GetParent () || mHoverButtonIndex != mPopupButtonIndex))
	{
		mHoverButtonTime += (currentTime - mLastNotifyTime);

		if ((mPopupMenu && mPopupMenu->IsVisible ()) || mHoverButtonTime >= mStyle->GetItemPopupTimeout ())
		{
			SpawnPopupMenu (mHoverButtonIndex);
		}
	}

	mLastNotifyTime = currentTime;
}

//-----------------------------------------------------------------

bool UIRadialMenu::AddItem (const Unicode::NarrowString & name, const Unicode::NarrowString & globalText, bool enabled, float opacity)
{

	if (!mDataSource)
	{
		UIDataSource * const source = new UIDataSource;
		assert (source);
		source->SetName ("DefaultDataSource");

		//-- the data source will be deleted when the RadialMenu detaches from it in the dtor
		SetDataSource (source);
	}

	assert (mDataSource);

	UIData * const data = new UIData;
	data->SetName (name);
	data->SetProperty (DataPropertyName::Text, Unicode::narrowToWide (globalText));
	data->SetPropertyFloat  (UIWidget::PropertyName::Opacity, opacity);

	if (!enabled)
	{
		UIString enabledString;
		if (UIUtils::FormatBoolean (enabledString, enabled))
			data->SetProperty (UIWidget::PropertyName::Enabled, enabledString);
	}

	mDataSource->AddChild (data);

	return true;
}

//-----------------------------------------------------------------

void UIRadialMenu::ClearItems ()
{
	if (mDataSource)
		mDataSource->Clear ();

	SetPopupDataNamespace (0);
}

//----------------------------------------------------------------------

void UIRadialMenu::SetPopupMenu (UIPopupMenu * menu)
{
	if (menu)
		menu->Attach (0);

	if (mPopupMenu)
	{
		UIManager::gUIManager ().PopContextWidgets (mPopupMenu);
		mPopupMenu->RemoveCallback (this);
		mPopupMenu->Detach (0);
		mPopupMenu = 0;
	}

	mPopupMenu = menu;

	if (mPopupMenu)
	{
		mPopupMenu->AddCallback (this);
		UIManager::gUIManager ().PushContextWidget (*mPopupMenu);
	}
}

//----------------------------------------------------------------------

void UIRadialMenu::SpawnPopupMenu (int index)
{
	UIDataSourceContainer * const dsc    = GetPopupDataSourceContainer (index);

	if (!dsc)
	{
		if (mPopupMenu)
			SetPopupMenu (0);

		return;
	}

	if (!mPopupMenu)
	{
		UIPopupMenu * const pop = new UIPopupMenu(this);
		assert (pop);
		SetPopupMenu (pop);
	}

	const long nodeCount = GetChildrenRef ().size ();

	if (nodeCount && mPopupMenu)
	{
		const UIRadialMenuStyle::ButtonStyleType bst = s_buttonStyleTypes [index];

		UIPopupMenuStyle * popupStyle = 0;

		if (mStyle)
		{
			popupStyle = mStyle->GetPopupStyle (bst);
			if (!popupStyle)
				popupStyle = mStyle->GetPopupStyle (UIRadialMenuStyle::BST_N);
		}

		mPopupMenu->SetStyle (popupStyle);
	}

	mPopupMenu->SetDataSourceContainer (dsc);

	mPopupMenu->Link ();
	mPopupMenu->SetTransient (true);
	mPopupMenu->SetVisible (true);
	mPopupMenu->SetEnabled (true);

	assert (mPopupMenu);

	if (!mPopupMenu->GetParent ())
		UIManager::gUIManager ().PushContextWidget (*mPopupMenu);

	PlacePopup (index, mPopupMenu);
}

//----------------------------------------------------------------------

const UIButton * UIRadialMenu::GetRadialButton (int index) const
{
	if (index < 0)
		return 0;

	int count = 0;
	const UIBaseObject::UIObjectList & olist = GetChildrenRef ();
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		const UIBaseObject * const obj = *it;
		if (!obj->IsA (TUIButton))
			continue;

		if (count == index)
			return static_cast<const UIButton *>(obj);

		++count;
	}

	return 0;
}

//----------------------------------------------------------------------

void UIRadialMenu::PlacePopup (int index, UIPopupMenu * pop)
{
	assert (pop);

	const UIButton * const butt = GetRadialButton (index);

	if (!butt)
		return;

	UIPoint location (butt->GetWorldLocation ());

	// pop out the top
	if (index < 3)
		location.y -= pop->GetHeight ();
	// pop out the bottom
	else
		location.y += butt->GetHeight ();

	long desiredWidth = butt->GetWidth ();

	desiredWidth -= 8;
	location.x   += 4;

	long left  = location.x;
	long right = location.x + desiredWidth;

	pop->SetMinimumDesiredWidth (desiredWidth);

	//-- if it's not the size we want, just try to center it on the center of left/right
	if (desiredWidth != pop->GetWidth ())
	{
		location.x = (right + left - pop->GetWidth ()) / 2;
	}

	//----------------------------------------------------------------------
	//-- clipping

	{
		//-- clip it to the bottom of the screen

		const UIPage * parentPage = 0;

		if (pop->GetParent ())
		{
			if (pop->GetParent ()->IsA (TUIPage))
			{
				parentPage = static_cast<const UIPage *>(pop->GetParent ());
				if (location.y + pop->GetHeight () >= parentPage->GetHeight ())
					location.y = parentPage->GetHeight () - pop->GetHeight ();
			}
		}

		// clip it to the top of the screen

		if (location.y < 0)
			location.y = 0;
		//-- clip it to the right of the screen

		if (parentPage)
		{
			if (location.x + pop->GetWidth () >= parentPage->GetWidth ())
				location.x = parentPage->GetWidth () - pop->GetWidth ();
		}

		//-- clip it to the left of the screen

		if (location.x < 0)
			location.x = 0;
	}

	pop->SetLocation (location);
}

//----------------------------------------------------------------------

UIDataSourceContainer * UIRadialMenu::GetPopupDataSourceContainer (int index, bool create)
{
	if (!mDataSource)
		return 0;

	const UIData * const data = mDataSource->GetChildByPosition (index);

	if (!data)
		return 0;

	return GetPopupDataSourceContainer (data->GetName (), create);
}

//----------------------------------------------------------------------

UIDataSourceContainer * UIRadialMenu::GetPopupDataSourceContainer (const UINarrowString & name, bool create)
{
	if (!mPopupDataNamespace)
	{
		if (create)
		{
			UINamespace * const ns = new UINamespace;
			ns->SetName ("MyPopupDataNamespace");
			SetPopupDataNamespace (ns);
			assert (mPopupDataNamespace);
		}
		else
			return 0;
	}

	UIBaseObject * const child = mPopupDataNamespace->GetChild (name.c_str ());

	if (child)
	{
		if (!child->IsA (TUIDataSourceContainer))
			return 0;

		return static_cast<UIDataSourceContainer *>(child);
	}

	if (create)
	{
		UIDataSourceContainer * const dsc = new UIDataSourceContainer;
		dsc->SetName (name);
		mPopupDataNamespace->AddChild (dsc);
		return dsc;
	}

	return 0;
}

//----------------------------------------------------------------------

void  UIRadialMenu::SetPopupDataNamespace (UINamespace * ns)
{
	RemoveProperty (PropertyName::PopupDataNamespace);

	if (ns)
		ns->Attach (this);

	if (mPopupDataNamespace)
		mPopupDataNamespace->Detach (this);

	mPopupDataNamespace = ns;
}

//----------------------------------------------------------------------

UIDataSource * UIRadialMenu::AddPopupDataSourceItem (UIDataSourceContainer * dsc, const UINarrowString & name, const UINarrowString & globalLabel, bool enabled, float opacity)
{
	return AddPopupDataSourceItem (dsc, name, Unicode::narrowToWide (globalLabel), enabled, opacity);
}

//----------------------------------------------------------------------

/**
* Add a PopupDatasource if needed.
*
* @return the UIData, whether it pre-existed or was created
*/

UIDataSource * UIRadialMenu::AddPopupDataSourceItem (UIDataSourceContainer * dsc, const UINarrowString & name, const UIString & localLabel, bool enabled, float opacity)
{
	assert (dsc);
	UIDataSource * ds = static_cast<UIDataSource *>(dsc->GetChild (name.c_str ()));
	if (!ds)
	{
		ds = new UIDataSource;
		assert (ds);
		ds->SetName (name);

		//-- todo: this DataPropertyName::Text must remain the same as the UIPopupMenu's DATA_TEXT!!
		ds->SetProperty (DataPropertyName::Text, localLabel);

		if (!enabled)
			ds->SetPropertyBoolean (UIWidget::PropertyName::Enabled, enabled);

		ds->SetPropertyFloat (UIWidget::PropertyName::Opacity, opacity);

		dsc->AddChild (ds);
	}
	return ds;
}

//----------------------------------------------------------------------

void UIRadialMenu::SetRadialCenterPrototype (UIWidget * proto)
{
	if (mRadialCenterWidget)
		RemoveChild (mRadialCenterWidget);

	UIWidget * const dupe = proto ? static_cast<UIWidget *>(proto->DuplicateObject ()) : 0;

	AttachMember (mRadialCenterWidget, dupe);

	if (mRadialCenterWidget)
	{
		mRadialCenterWidget->RemoveProperty (UIWidget::PropertyName::PackSize);
		mRadialCenterWidget->RemoveProperty (UIWidget::PropertyName::PackLocation);
		AddChild (mRadialCenterWidget);
		MoveChild (mRadialCenterWidget, UIBaseObject::Bottom);
	}

	LayoutButtons ();
}

//----------------------------------------------------------------------
