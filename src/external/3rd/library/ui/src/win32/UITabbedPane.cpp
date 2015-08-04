#include "_precompile.h"

#include "UITabbedPane.h"
#include "UITabbedPaneStyle.h"
#include "UIImageStyle.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"
#include "UICanvas.h"
#include "UIDataSource.h"
#include "UIMessage.h"
#include "UIEventCallback.h"
#include "UIButton.h"
#include "UIButtonStyle.h"
#include "UIData.h"
#include "UIPage.h"

#include <cassert>
#include <list>
#include <vector>

const char * UITabbedPane::TypeName	= "TabbedPane";

//----------------------------------------------------------------------

const UILowerString UITabbedPane::PropertyName::DataSource = UILowerString("DataSource");
const UILowerString UITabbedPane::PropertyName::ActiveTab = UILowerString("ActiveTab");
const UILowerString UITabbedPane::PropertyName::Style = UILowerString("Style");
const UILowerString UITabbedPane::PropertyName::TargetPage = UILowerString("TargetPage");
const UILowerString UITabbedPane::PropertyName::TabObject = UILowerString("TabObject");

//-----------------------------------------------------------------

const UILowerString UITabbedPane::DataProperties::DATA_TARGET = UILowerString("Target");
const UILowerString UITabbedPane::DataProperties::DATA_NAME = UILowerString("Name");
const UILowerString UITabbedPane::DataProperties::DATA_DRAGTYPE = UIWidget::PropertyName::DragType;
const UILowerString UITabbedPane::DataProperties::DATA_EXTRA_PROPS = UILowerString("DATA_EXTRA_PROPS");
const UILowerString UITabbedPane::DataProperties::DATA_CONTEXT_CAPABLE = UIWidget::PropertyName::ContextCapable;
const UILowerString UITabbedPane::DataProperties::DATA_CONTEXT_TO_PARENT = UIWidget::PropertyName::ContextToParent;
const UILowerString UITabbedPane::DataProperties::DATA_DROP_TO_PARENT = UIWidget::PropertyName::DropToParent;
const UILowerString UITabbedPane::DataProperties::DATA_ICON_PATH = UILowerString("IconPath");
const UILowerString UITabbedPane::DataProperties::DATA_ICON_COLOR = UILowerString("IconColor");
const UILowerString UITabbedPane::DataProperties::Text = UIButton::PropertyName::Text;
const UILowerString UITabbedPane::DataProperties::LocalText = UIButton::PropertyName::LocalText;

//======================================================================================
#define _TYPENAME UITabbedPane

namespace UITabbedPaneNamespace
{
	//----------------------------------------------------------------------

	UIButton * getButton(UIBaseObject * const obj)
	{
		UIButton * button = NULL;

		if (obj) 
		{
			UIWidget * const widget = UI_ASOBJECT(UIWidget, obj);
			if (widget && widget->IsA(TUIButton)) 
			{
				button = static_cast<UIButton *>(widget);
			}
			else 
			{
				// do not make recursive.
				UIBaseObject::UIObjectList cv;
				widget->GetChildren(cv);
				
				for (UIBaseObject::UIObjectList::const_iterator it = cv.begin(); it != cv.end(); ++it)
				{
					button = UI_ASOBJECT(UIButton, *it);
					
					if (button) 
					{
						break;
					}
				}
			}
		}

		return button;
	}

	long const getChildIndex(const UIBaseObject::UIObjectList & olist, UIWidget const & widget)
	{
		long count = 0;

		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin(); it != olist.end(); ++it)
		{
			UIBaseObject * const obj = *it;
			UIButton const * const button = getButton(obj);
			if (!button)
				continue;

			if (obj == const_cast<UIWidget*>(&widget) || 
				button == &widget || 
				button == getButton(const_cast<UIWidget*>(&widget)))
				return count;

			count++;
		}

		return -1;
	}

	//----------------------------------------------------------------------

	UIButton * getChildByIndex(const UIBaseObject::UIObjectList & olist, long index)
	{
		long count = 0;

		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin(); it != olist.end(); ++it)
		{
			UIBaseObject * const obj = *it;
			UIButton * const button = getButton(obj);
			if (!button)
				continue;

			if (count == index)
				return button;

			count++;
		}

		return 0;
	}

	//================================================================
	// Basic category.
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(DataSource, "", T_object),
		_DESCRIPTOR(ActiveTab, "", T_int),
		_DESCRIPTOR(Style, "", T_object),
		_DESCRIPTOR(TargetPage, "", T_object),
	_GROUPEND(Basic, 3, int(UIPropertyCategories::C_Basic));
	//================================================================
}
using namespace UITabbedPaneNamespace;
//======================================================================================

UITabbedPane::UITabbedPane() :
UIPage(),
UIEventCallback(),
UINotification(),
mStyle(0),
mActiveTab(-1),
mDataSource(0),
mTargetPage(0),
mCallbackForwardingRecursionGuard(false),
mIgnoreDataChange(false),
mDirtySemaphore(true),
mTabObject()
{
}

//-----------------------------------------------------------------

UITabbedPane::~UITabbedPane()
{
	SetStyle(0);
	SetDataSource(0);
	SetTargetPage(0);
}

//-----------------------------------------------------------------

bool UITabbedPane::IsA(const UITypeID Type) const
{
	return(Type == TUITabbedPane) || UIPage::IsA(Type);
}

//-----------------------------------------------------------------

const char *UITabbedPane::GetTypeName(void) const
{
	return TypeName;
}

//-----------------------------------------------------------------

UIBaseObject *UITabbedPane::Clone(void) const
{
	return new UITabbedPane;
}

//-----------------------------------------------------------------

bool UITabbedPane::ProcessMessage(const UIMessage &msg)
{
	return UIPage::ProcessMessage(msg);
}

//-----------------------------------------------------------------

void UITabbedPane::Render(UICanvas &DestinationCanvas) const
{
	UIPage::Render(DestinationCanvas);
}

//-----------------------------------------------------------------

void UITabbedPane::SetStyle(UITabbedPaneStyle *NewStyle)
{
	if (NewStyle != mStyle) 
	{
		if (NewStyle)
			NewStyle->Attach(this);
		
		if (mStyle)
			mStyle->Detach(this);
		
		mStyle = NewStyle;
		
		RecreateButtons();
	}
}

//-----------------------------------------------------------------

UIStyle *UITabbedPane::GetStyle(void) const
{
	return mStyle;
};

//-----------------------------------------------------------------

void UITabbedPane::GetLinkPropertyNames(UIPropertyNameVector &In) const
{
	UIPage::GetLinkPropertyNames(In);

	In.push_back(UITabbedPane::PropertyName::DataSource);
	In.push_back(UITabbedPane::PropertyName::Style);
	In.push_back(UITabbedPane::PropertyName::TargetPage);
	In.push_back(UITabbedPane::PropertyName::TabObject);
}

//----------------------------------------------------------------------

void UITabbedPane::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIPage::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UITabbedPane::GetPropertyNames(UIPropertyNameVector &In, bool forCopy) const
{
	UIPage::GetPropertyNames(In, forCopy);

	In.push_back(UITabbedPane::PropertyName::DataSource);
	In.push_back(UITabbedPane::PropertyName::ActiveTab);
	In.push_back(UITabbedPane::PropertyName::Style);
	In.push_back(UITabbedPane::PropertyName::TargetPage);
	In.push_back(UITabbedPane::PropertyName::TabObject);
}

//-----------------------------------------------------------------

bool UITabbedPane::SetProperty(const UILowerString & Name, const UIString &Value)
{
	if (Name == PropertyName::DataSource)
	{
		UIDataSource *NewDataSource = static_cast<UIDataSource *>(GetObjectFromPath(Value, TUIDataSource));

		if (NewDataSource || Value.empty())
		{
			SetDataSource(NewDataSource);
			return true;
		}
	}
	else if (Name == PropertyName::TargetPage)
	{
		UIPage *NewTargetPage = static_cast<UIPage *>(GetObjectFromPath(Value, TUIPage));

		if (NewTargetPage || Value.empty())
		{
			SetTargetPage(NewTargetPage);
			return true;
		}
	}
	else if (Name == PropertyName::ActiveTab)
	{
		long val = 0;

		if (UIUtils::ParseLong(Value, val))
		{
			SetActiveTab(val);
			return true;
		}
		else
			SetActiveTab(Unicode::wideToNarrow(Value));

		return true;
	}
	else if (Name == PropertyName::Style)
	{
		UITabbedPaneStyle * const NewStyle = static_cast<UITabbedPaneStyle *>(GetObjectFromPath(Value, TUITabbedPaneStyle));
		
		if (NewStyle || Value.empty())
		{
			SetStyle(NewStyle);
			return true;
		}
	}
	else if (Name == PropertyName::TabObject)
	{
		UIWidget * newWidget = static_cast<UIWidget*>(GetObjectFromPath(Value, TUIWidget));

		if ((newWidget && !newWidget->IsTransient() &&(newWidget != mTabObject) && getButton(newWidget)) || Value.empty())
		{
			mTabObject = newWidget;
			RecreateButtons();
			return true;
		}
	}

	return UIPage::SetProperty(Name, Value);
}

//-----------------------------------------------------------------

bool UITabbedPane::GetProperty(const UILowerString & Name, UIString &Value) const
{
	if (Name == PropertyName::DataSource)
	{
		if (mDataSource)
		{
			GetPathTo(Value, mDataSource);
			return true;
		}
	}
	else if (Name == PropertyName::TargetPage)
	{
		if (mTargetPage)
		{
			GetPathTo(Value, mTargetPage);
			return true;
		}
	}
	else if (Name == PropertyName::ActiveTab)
	{
		if (UIUtils::FormatLong(Value, mActiveTab))
		{
			mDirtySemaphore = true;
			return true;
		}
	}
	else if (Name == PropertyName::Style)
	{
		if (mStyle)
		{
			GetPathTo(Value, mStyle);
			return true;
		}
	}
	else if (Name == PropertyName::TabObject)
	{
		if (mTabObject)
		{
			GetPathTo(Value, mTabObject);
			return true;
		}
	}

	return UIPage::GetProperty(Name, Value);
}

//-----------------------------------------------------------------

void UITabbedPane::Link(void)
{
	UIPage::Link();

	if (mStyle)
		mStyle->Link();

	RecreateButtons();
}

//-----------------------------------------------------------------

bool UITabbedPane::OnMessage(UIWidget *Context, const UIMessage & msg)
{
	assert(Context);

	if (mCallbackForwardingRecursionGuard)
		return true;

	//----------------------------------------------------------------------
	//-- a UIVolumePage always forwards messages from its children to any
	//-- listeners subscribed to the volume page

	if (Context != this)
	{
		mCallbackForwardingRecursionGuard = true;
		bool const keepGoing = SendUIMessageCallback(msg, Context);
		mCallbackForwardingRecursionGuard = false;

		return keepGoing;
	}

	return true;
}

//----------------------------------------------------------------------

void UITabbedPane::OnButtonPressed(UIWidget * context)
{
	assert(context);
	assert(context->IsA(TUIButton));

	long const index = getChildIndex(GetChildrenRef(), *context);
	SetActiveTab(index);
}

//-----------------------------------------------------------------

long UITabbedPane::GetTabCount(void) const
{
	if (!mDataSource)
		return 0;

	return mDataSource->GetChildCount();
};

//-----------------------------------------------------------------

void UITabbedPane::SetTargetPage(UIPage * NewTargetPage)
{
	if (NewTargetPage == mTargetPage)
		return;

	if (NewTargetPage)
	{
		NewTargetPage->Attach(this);
	}

	if (mTargetPage)
	{
		mTargetPage->Detach(this);
	}

	mTargetPage = NewTargetPage;
}

//----------------------------------------------------------------------

void UITabbedPane::SetDataSource(UIDataSource *NewDataSource)
{
	if (NewDataSource == mDataSource)
		return;

	if (NewDataSource)
	{
		NewDataSource->Listen(this);
		NewDataSource->Attach(this);
	}

	if (mDataSource)
	{
		mDataSource->StopListening(this);
		mDataSource->Detach(this);
	}

	mDataSource = NewDataSource;

	RecreateButtons();

	SetActiveTab(0);
}

//----------------------------------------------------------------------

void UITabbedPane::RecreateButtons()
{
	{
		mDirtySemaphore = true;

		UIBaseObject::UIObjectList olist;
		GetChildren(olist);

		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin(); it != olist.end(); ++it)
		{
			UIWidget * widget = UI_ASOBJECT(UIWidget, *it);
			if (widget && widget->IsTransient())
			{
				UIButton const * button = getButton(widget);

				if (button) 
				{
					const_cast<UIButton *>(button)->RemoveCallback(this);
				}
				
				RemoveChild(*it);
			}
		}
	}

	if (!mDataSource || !mStyle)
		return;

	UIButtonStyle * const style = mStyle->GetButtonStyle(UITabbedPaneStyle::BST_middle);
	if (!style)
		return;

	UIButtonStyle * styleLeft = mStyle->GetButtonStyle(UITabbedPaneStyle::BST_left);
	if (!styleLeft)
		styleLeft = style;

	UIButtonStyle * styleRight = mStyle->GetButtonStyle(UITabbedPaneStyle::BST_right);
	if (!styleRight)
		styleRight = style;

	UIDataList const & dataList = mDataSource->GetData();
	{
		const int dataCount = static_cast<int>(dataList.size());

		int index = 0;
		for (UIDataList::const_iterator it = dataList.begin(); it != dataList.end(); ++it, ++index)
		{
			UIData * const data = *it;
			UIWidget * const widget = mTabObject ? static_cast<UIWidget *>(mTabObject->DuplicateObject()) : static_cast<UIWidget*>(new UIButton);

			if (mTabObject && !mTabObject->IsA(TUIButton)) 
			{
				widget->SetGetsInput(false);
				widget->SetAbsorbsInput(false);
			}
			
			widget->SetTransient(true);
			widget->SetVisible(true);

			AddChild(widget);

			UIButton * const butt = getButton(widget);
			butt->AddCallback(this);

			if (index == 0 && dataCount > 1)
				butt->SetStyle(styleLeft);
			else if (index == dataCount - 1 && dataCount > 1)
				butt->SetStyle(styleRight);
			else
				butt->SetStyle(style);

			butt->SetGetsInput(true);
			butt->SetAbsorbsInput(true);



			UIString val;
			if (!data->GetProperty(DataProperties::LocalText, val) && !data->GetProperty(DataProperties::Text, val) && !data->GetProperty(DataProperties::DATA_NAME, val))
				val = Unicode::narrowToWide("UNNAMED");

			butt->SetName(data->GetName());
			butt->SetText(val);

			if (data->GetProperty(DataProperties::DATA_DRAGTYPE, val))
			{
				widget->SetDragable(true);
				widget->SetProperty(UIWidget::PropertyName::DragType, val);
			}

			if (data->GetProperty(DataProperties::DATA_EXTRA_PROPS, val))
			{
				const Unicode::unicode_char_t propsep [] = {';', 0};

				Unicode::String expression;
				size_t pos = 0;

				while (pos != Unicode::String::npos && Unicode::getFirstToken(val, pos, pos, expression, propsep))
				{
					const size_t breakpos = expression.find('=');
					if (breakpos != Unicode::String::npos)
					{
						const Unicode::String & lhs = expression.substr(0, breakpos);
						const Unicode::String & rhs = expression.substr(breakpos + 1);

						widget->SetProperty(UILowerString(Unicode::wideToNarrow(lhs)), rhs);
						butt->SetProperty(UILowerString(Unicode::wideToNarrow(lhs)), rhs);
					}
				}
			}

			bool contextCapable = false;
			if (data->GetPropertyBoolean(DataProperties::DATA_CONTEXT_CAPABLE, contextCapable))
			{
				butt->SetContextCapable(contextCapable, true);
				widget->SetContextCapable(contextCapable, true);
			}

			bool contextToParent = false;
			if (data->GetPropertyBoolean(DataProperties::DATA_CONTEXT_TO_PARENT, contextToParent))
			{
				butt->SetContextCapable(contextToParent, true);
				widget->SetContextCapable(contextToParent, true);
			}

			bool dropToParent = false;
			if (data->GetPropertyBoolean(DataProperties::DATA_DROP_TO_PARENT, dropToParent))
			{
				butt->SetDropToParent(dropToParent);
				UIString dragAccepts;
				GetProperty(UIWidget::PropertyName::DragAccepts, dragAccepts);
				butt->SetProperty(UIWidget::PropertyName::DragAccepts, dragAccepts);
			}

			Unicode::String iconPath;
			if (data->GetProperty(DataProperties::DATA_ICON_PATH, iconPath))
				butt->SetProperty(DataProperties::DATA_ICON_PATH, iconPath);

			UIColor iconColor;
			if (data->GetPropertyColor(DataProperties::DATA_ICON_COLOR, iconColor))
				butt->SetPropertyColor(UIButton::PropertyName::IconColor, iconColor);

			if (index == mActiveTab)
			{
				widget->SetEnabled(false);
				widget->SetActivated(true);
			}
			else
			{
				widget->SetEnabled(true);
				widget->SetActivated(false);
			}


			widget->Link();
		}
	}

	if (!dataList.empty())
	{
		if (mActiveTab < 0)
			SetActiveTab(0);
		else if (mActiveTab >= static_cast<int>(dataList.size()))
			SetActiveTab(static_cast<int>(dataList.size()) - 1);
	}

	LayoutButtons();
}

//----------------------------------------------------------------------

void UITabbedPane::LayoutButtons()
{
	UIPoint pos;
	long const height = GetHeight();
	UIBaseObject::UIObjectList const & olist = GetChildrenRef();
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin(); it != olist.end(); ++it)
	{
		UIWidget * const widget = UI_ASOBJECT(UIWidget, *it);
		if (!widget || !widget->IsVisible()) 
		{
			continue;
		}

		UIButton * const butt = getButton(widget);
		if (!butt) 
		{
			continue;
		}

		widget->SetLocation(pos);
		widget->SetHeight(height);

		// ?
		Unicode::String iconPath;
		butt->GetProperty(DataProperties::DATA_ICON_PATH, iconPath);
		butt->SetProperty(UIButton::PropertyName::Icon, iconPath);

		UISize size;
		if (butt->GetDesiredSize(size))
			widget->SetWidth(size.x);
		else
			widget->SetWidth(height);

		pos.x += widget->GetWidth();
	}

	SetMinimumScrollExtent(UISize(pos.x, 0L));
	SetScrollExtent(UISize(pos.x, 0L));
	SetScrollLocation(UIPoint::zero);
}

//----------------------------------------------------------------------

void UITabbedPane::SetScrollLocation(const UIPoint & pt)
{
	UIPoint myPt;

	bool found = false;
	const UIBaseObject::UIObjectList & olist = GetChildrenRef();

	if (!olist.empty())
	{
		const UIButton * const lastButton = static_cast<UIButton *>(olist.back());
		if (lastButton)
		{
			UISize scrollExtent;
			GetScrollExtent(scrollExtent);

			if (pt.x >=(scrollExtent.x - GetWidth()))
			{
				myPt = pt;
				myPt.x = std::min(pt.x, scrollExtent.x - GetWidth());
				UIPage::SetScrollLocation(myPt);
				return;
			}
		}
	}

	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin(); it != olist.end(); ++it)
	{
		UIButton * const butt = getButton(*it);
		if (!butt) 
		{
			continue;
		}

		UIPoint const & buttLoc = butt->GetLocation();

		//-- right
		if (buttLoc.x > pt.x)
		{
			found = true;
			break;
		}
		myPt.x = buttLoc.x;
	}

	UIPage::SetScrollLocation(myPt);
}

//----------------------------------------------------------------------

void UITabbedPane::GetScrollSizes(UISize &PageSize, UISize &LineSize, bool rightLeft, bool) const
{
	const UIPoint & scrollLocation = GetScrollLocation();
	UISize scrollExtent;
	GetScrollExtent(scrollExtent);

	PageSize.y = LineSize.y = 1L;
	
	UIPoint lastButtLoc;
	
	const UIBaseObject::UIObjectList & olist = GetChildrenRef();
	
	bool lastButtonOverhangs = false;
	if (!olist.empty())
	{
		const UIButton * const lastButton = static_cast<UIButton *>(olist.back());
		if (lastButton)
			lastButtonOverhangs = lastButton->GetLocation().x < scrollLocation.x + GetWidth();
	}
	
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin(); it != olist.end(); ++it)
	{
		UIButton * const butt = getButton(*it);
		if (!butt) 
		{
			continue;
		}

		UIPoint const & buttLoc = butt->GetLocation();
		
		//-- right
		if (rightLeft)
		{
			if (buttLoc.x > scrollLocation.x)
			{
				PageSize.x = LineSize.x =(buttLoc.x - scrollLocation.x);
				
				if ((scrollLocation.x - PageSize.x) < 0)
				{
					if (lastButtonOverhangs)
						PageSize.x = LineSize.x =(scrollExtent.x - GetWidth()) - scrollLocation.x;
				}

				return;
			}
		}
		//-- left
		else
		{
			if (buttLoc.x > scrollLocation.x)
			{
				break;
			}
		}

		lastButtLoc = buttLoc;
	}

	if (!rightLeft)
		PageSize.x = LineSize.x = abs(scrollLocation.x - lastButtLoc.x);
}

//----------------------------------------------------------------------

void UITabbedPane::Notify(UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, UINotification::Code NotificationCode)
{
	UI_UNREF(ContextObject);

	if (NotifyingObject == mDataSource)
	{
		if (mIgnoreDataChange)
			return;

		switch(NotificationCode)
		{
		case UINotification::ChildAdded:
			{
				UIData * const data = static_cast<UIData *>(ContextObject);

				if (!data->HasProperty(DataProperties::DATA_NAME))
					data->SetProperty(DataProperties::DATA_NAME, Unicode::narrowToWide("Tab"));
				if (!data->HasProperty(DataProperties::DATA_TARGET))
					data->SetProperty(DataProperties::DATA_TARGET, Unicode::narrowToWide("SomeWidget"));
			}
			//-- fallthrough
		case UINotification::ChildRemoved:
		case UINotification::ChildChanged:
			RecreateButtons();
			SetActiveTab(mActiveTab);
			break;
		}
	}
}

//----------------------------------------------------------------------

void UITabbedPane::SetActiveTab(const std::string & name)
{
	std::string tabName;

	const int count = GetTabCount();

	for (int i = 0; i < count; ++i)
	{
		GetTabName(i, tabName);
		if (!_stricmp(name.c_str(), tabName.c_str()))
		{		
			SetActiveTab(i);
			return;
		}
	}
}

//----------------------------------------------------------------------

void UITabbedPane::SetActiveTab(long index)
{
	if (!mDataSource)
		return;

	index = std::min(index, static_cast<long>(mDataSource->GetChildCount()) - 1L);

	if (!mDirtySemaphore && index == mActiveTab)
		return;

	mDirtySemaphore = false;
	mActiveTab = index;

	long count = 0;
	const UIBaseObject::UIObjectList & olist = GetChildrenRef();
	for (UIBaseObject::UIObjectList::const_iterator it = olist.begin(); it != olist.end(); ++it)
	{
		UIButton * const butt = getButton(*it);
		if (!butt) 
		{
			continue;
		}

		butt->SetEnabled(count != mActiveTab);
		butt->SetActivated(count == mActiveTab);

		if (count == mActiveTab)
		{
			const UIPoint & scrollLocation = GetScrollLocation();
		
			if (butt->GetLocation().x < scrollLocation.x)
				SetScrollLocation(butt->GetLocation());
			else if ((butt->GetLocation().x + butt->GetWidth()) >(scrollLocation.x + GetWidth()))
			{
				UIPoint pt = butt->GetLocation();
				pt.x += butt->GetWidth();
				pt.x -= GetWidth();
				
				UIPoint targetPt = pt;
				
				for (UIBaseObject::UIObjectList::const_iterator pit = olist.begin(); pit != olist.end(); ++pit)
				{
					UIButton * const prevButton = getButton(*pit);
					if (!prevButton) 
					{
						continue;
					}
					
					pt.x = prevButton->GetLocation().x;
					
					if (prevButton->GetLocation().x >= targetPt.x || prevButton == butt)
						break;
				}
				
				SetScrollLocation(pt);
			}
		}

		++count;
	}

	//----------------------------------------------------------------------
	//-- enable/disable the target's children

	if (mTargetPage)
	{
		UIData * data = mDataSource ? static_cast<UIData *>(mDataSource->GetChildByPosition(mActiveTab)) : 0;
		UIWidget * newWidget = 0;

		if (data)
		{
			UIString str;
			if (data->GetProperty(DataProperties::DATA_TARGET, str))
			{
				newWidget = static_cast<UIWidget *>(mTargetPage->GetObjectFromPath(str, TUIWidget));
			}
		}

		UIObjectList olist;
		mTargetPage->GetChildren(olist);

		for (UIObjectList::iterator it = olist.begin(); it != olist.end(); ++it)
		{
			if (!(*it)->IsA(TUIWidget))
				continue;

			UIWidget * const w = static_cast<UIWidget *>(*it);

			w->SetVisible(w == newWidget);
			w->SetEnabled(w == newWidget);
		}
	}

	SendCallback(&UIEventCallback::OnTabbedPaneChanged, UILowerString::null);
}

//----------------------------------------------------------------------

void UITabbedPane::SetSize(const UISize &NewSize)
{
	UIWidget::SetSize(NewSize);
	LayoutButtons();
}

//----------------------------------------------------------------------

long UITabbedPane::GetTabFromPoint(const UIPoint & point) const
{
	if (mTabObject) 
	{
		_asm nop;
	}

	UIWidget * const widget = GetWidgetFromPoint(point, false);

	if (widget)
		return getChildIndex(GetChildrenRef(), *widget);

	return 0;
}

//----------------------------------------------------------------------

UIData * UITabbedPane::GetTabData(long index) const
{
	if (!mDataSource)
		return 0;

	return mDataSource->GetChildByPosition(index);
}

//----------------------------------------------------------------------

bool UITabbedPane::GetTabName(long index, UINarrowString & name) const
{
	const UIData * const data = GetTabData(index);
	if (data)
	{
		name = data->GetName();
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

/**
* This is over-ridden because disabled children of the tabbed pane should be
* treated as enabled by the tabbed pane.
*/

UIWidget * UITabbedPane::GetWidgetFromPoint(const UIPoint & pt, bool) const
{
	return UIPage::GetWidgetFromPoint(pt, false);
}

//----------------------------------------------------------------------

UIButton * UITabbedPane::GetTabButton(long index)
{
	return getChildByIndex(GetChildrenRef(), index);
}

//----------------------------------------------------------------------

long UITabbedPane::FindTabIndex(const UIButton & button)
{
	return getChildIndex(GetChildrenRef(), button);
}

//----------------------------------------------------------------------

void UITabbedPane::SetButtonIcon(int index, UIImageStyle * icon)
{
	UIData * const data = GetTabData(index);

	if (data)
	{
		mIgnoreDataChange = true;

		if (icon)
			data->SetPropertyNarrow(DataProperties::DATA_ICON_PATH, icon->GetFullPath());
		else
			data->SetPropertyNarrow(DataProperties::DATA_ICON_PATH, std::string());

		mIgnoreDataChange = false;
	}

	UIButton * const button = GetTabButton(index);

	if (button)
	{
		if (icon)
			button->SetPropertyNarrow(DataProperties::DATA_ICON_PATH, icon->GetFullPath());
		else
			button->SetPropertyNarrow(DataProperties::DATA_ICON_PATH, std::string());
	}

	LayoutButtons();
}

//----------------------------------------------------------------------

void UITabbedPane::SetButtonIconColor(int index, const UIColor & color)
{
	UIData * const data = GetTabData(index);

	if (data)
	{
		mIgnoreDataChange = true;
		data->SetPropertyColor(DataProperties::DATA_ICON_COLOR, color);
		mIgnoreDataChange = false;
	}

	UIButton * const button = GetTabButton(index);

	if (button)
	{
		button->SetIconColor(color);
	}
}

//----------------------------------------------------------------------

void UITabbedPane::SetButtonText(int index, const Unicode::String & str)
{
	UIData * const data = GetTabData(index);

	if (data)
	{
		mIgnoreDataChange = true;
		data->SetProperty(DataProperties::LocalText, str);
		mIgnoreDataChange = false;
	}

	UIButton * const button = GetTabButton(index);

	if (button)
	{
		button->SetText(str);
	}

	LayoutButtons();

}

//----------------------------------------------------------------------

UIData * UITabbedPane::AppendTab(const Unicode::String & localLabel, UIWidget * tabTarget)
{
	if (mDataSource)
	{
		mDirtySemaphore = true;

		UIData * const data = new UIData;
		data->SetProperty(DataProperties::DATA_NAME, localLabel);
		data->SetProperty(DataProperties::LocalText, localLabel);

		if (tabTarget)
			data->SetPropertyNarrow(DataProperties::DATA_TARGET, tabTarget->GetFullPath());

		mDataSource->AddChild(data);
		return data;
	}

	return 0;
}

//----------------------------------------------------------------------

void UITabbedPane::Clear()
{
	if (mDataSource)
		mDataSource->Clear();

	mDirtySemaphore = true;
}

//----------------------------------------------------------------------

UIWidget * UITabbedPane::GetActiveWidget() const
{
	if (mTargetPage && mDataSource)
	{
		UIData const * const data = static_cast<UIData *>(mDataSource->GetChildByPosition(mActiveTab));
		if (data)
		{
			UIString str;
			if (data->GetProperty(DataProperties::DATA_TARGET, str))
				return static_cast<UIWidget *>(mTargetPage->GetObjectFromPath(str, TUIWidget));
		}
	}

	return 0;
}

//----------------------------------------------------------------------
