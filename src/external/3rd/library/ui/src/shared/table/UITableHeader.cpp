// ======================================================================
//
// UITableHeader.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "_precompile.h"
#include "UITableHeader.h"

#include "UIButton.h"
#include "UIButtonStyle.h"
#include "UICursor.h"
#include "UIImageStyle.h"
#include "UIMessage.h"
#include "UITable.h"
#include "UITableModel.h"
#include "UIPropertyDescriptor.h"
#include "UIUtils.h"

#include <cassert>
#include <list>
#include <vector>

// ======================================================================

const char * const UITableHeader::TypeName  = "TableHeader";

const UILowerString UITableHeader::PropertyName::Table            = UILowerString ("Table");
const UILowerString UITableHeader::PropertyName::ButtonStyle      = UILowerString ("ButtonStyle");
const UILowerString UITableHeader::PropertyName::ImageSortUp      = UILowerString ("ImageSortUp");
const UILowerString UITableHeader::PropertyName::ImageSortDown    = UILowerString ("ImageSortDown");
const UILowerString UITableHeader::PropertyName::CursorResize     = UILowerString ("CursorResize");
const UILowerString UITableHeader::PropertyName::PadFront         = UILowerString ("PadFront");
const UILowerString UITableHeader::PropertyName::PadBack          = UILowerString ("PadBack");

//======================================================================================
#define _TYPENAME UITableHeader

namespace UITableHeaderNamespace
{
	int findButtonIndex (UIButton * button, const UIBaseObject::UIObjectList & olist)
	{
		int index = 0;
		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			if ((*it)->IsA (TUIButton))
			{
				if (*it == button)
					return index;

				++index;
			}
		}

		return -1;
	}

	int findDividerColumnRight (UIPage * page, const UIBaseObject::UIObjectList & olist)
	{
		int index = 1;
		for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
		{
			if ((*it)->IsA (TUIPage))
			{
				if (*it == page)
					return index;

				++index;
			}
		}

		return -1;
	}

	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(PadFront, "", T_bool),
		_DESCRIPTOR(PadBack, "", T_bool),
		_DESCRIPTOR(Table, "", T_object),
	_GROUPEND(Basic, 3, 0);
	//================================================================

	//================================================================
	// AdvancedAppearance category
	_GROUPBEGIN(AdvancedAppearance)
		_DESCRIPTOR(ButtonStyle, "", T_object),
		_DESCRIPTOR(ImageSortUp, "", T_object),
		_DESCRIPTOR(ImageSortDown, "", T_object),
		_DESCRIPTOR(CursorResize, "", T_object),
	_GROUPEND(AdvancedAppearance, 3, 1);
	//================================================================
}
using namespace UITableHeaderNamespace;
//======================================================================================

UITableHeader::UITableHeader () :
UIPage              (),
mTable              (0),
mButtonStyle        (0),
mImageSortUp        (0),
mImageSortDown      (0),
mCursorResize       (0),
mResizerWidth       (4L),
mDividerButtonDown  (false),
mDividerColumnRight (-1),
mLastDragPoint      (),
mPadFront           (false),
mPadBack            (false),
mHasRecreated       (false)
{
	SetBackgroundColor (UIColor (200, 200, 200));	SetBackgroundOpacity (1.0f);
}

//-----------------------------------------------------------------

UITableHeader::~UITableHeader ()
{
	SetTable         (0);
	SetButtonStyle   (0);
	SetImageSortUp   (0);
	SetImageSortDown (0);
	SetCursorResize  (0);
}

//-----------------------------------------------------------------

bool UITableHeader::IsA( const UITypeID Type ) const
{
	return Type == TUITableHeader || UIPage::IsA (Type);
}

//-----------------------------------------------------------------
void UITableHeader::GetLinkPropertyNames( UIPropertyNameVector & in) const
{
	in.push_back (PropertyName::Table);
	in.push_back (PropertyName::ButtonStyle);
	in.push_back (PropertyName::ImageSortUp);
	in.push_back (PropertyName::ImageSortDown);
	in.push_back (PropertyName::CursorResize);

	UIPage::GetLinkPropertyNames (in);
}

//----------------------------------------------------------------------

void UITableHeader::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIPage::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
	GROUP_TEST_AND_PUSH(AdvancedAppearance, category, o_groups);
}

//----------------------------------------------------------------------

void UITableHeader::GetPropertyNames( UIPropertyNameVector & in, bool forCopy) const
{
	in.push_back (PropertyName::Table);
	in.push_back (PropertyName::ButtonStyle);
	in.push_back (PropertyName::ImageSortUp);
	in.push_back (PropertyName::ImageSortDown);
	in.push_back (PropertyName::CursorResize);
	in.push_back (PropertyName::PadFront);
	in.push_back (PropertyName::PadBack);

	UIPage::GetPropertyNames (in, forCopy);
}

//-----------------------------------------------------------------

bool UITableHeader::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::Table)
	{
		UITable * const table = static_cast<UITable *>(GetObjectFromPath(Value, TUITable));
		if( table || Value.empty() )
		{
			SetTable (table);
			return true;
		}
	}
	else if ( Name == PropertyName::ButtonStyle  )
	{
		UIButtonStyle * const style = static_cast<UIButtonStyle *>(GetObjectFromPath(Value, TUIButtonStyle));
		if( style || Value.empty() )
		{
			SetButtonStyle (style);
			return true;
		}
	}
	else if ( Name == PropertyName::ImageSortDown)
	{
		UIImageStyle * const style = static_cast<UIImageStyle *>(GetObjectFromPath(Value, TUIImageStyle));
		if( style || Value.empty() )
		{
			SetImageSortDown (style);
			return true;
		}
	}
	else if ( Name == PropertyName::ImageSortUp  )
	{
		UIImageStyle * const style = static_cast<UIImageStyle *>(GetObjectFromPath(Value, TUIImageStyle));
		if( style || Value.empty() )
		{
			SetImageSortDown (style);
			return true;
		}
	}
	else if ( Name == PropertyName::CursorResize  )
	{
		UICursor * const cursor = static_cast<UICursor *>(GetObjectFromPath(Value, TUICursor));
		if( cursor || Value.empty() )
		{
			SetCursorResize (cursor);
			return true;
		}
	}
	else if ( Name == PropertyName::PadFront  )
	{
		return UIUtils::ParseBoolean (Value, mPadFront);
	}
	else if ( Name == PropertyName::PadBack  )
	{
		return UIUtils::ParseBoolean (Value, mPadBack);
	}
	return UIPage::SetProperty (Name, Value);
}

//-----------------------------------------------------------------

bool UITableHeader::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if ( Name == PropertyName::Table        )
	{
		if (mTable)
		{
			GetPathTo (Value, mTable);
			return true;
		}
	}
	else if ( Name == PropertyName::ButtonStyle  )
	{
		if (mButtonStyle)
		{
			GetPathTo (Value, mButtonStyle);
			return true;
		}
	}
	else if ( Name == PropertyName::ImageSortDown)
	{
		if (mImageSortDown)
		{
			GetPathTo (Value, mImageSortDown);
			return true;
		}
	}
	else if ( Name == PropertyName::ImageSortUp  )
	{
		if (mImageSortUp)
		{
			GetPathTo (Value, mImageSortUp);
			return true;
		}
	}
	else if ( Name == PropertyName::CursorResize  )
	{
		if (mCursorResize)
		{
			GetPathTo (Value, mCursorResize);
			return true;
		}
	}
	else if ( Name == PropertyName::PadFront  )
	{
		return UIUtils::FormatBoolean (Value, mPadFront);
	}
	else if ( Name == PropertyName::PadBack  )
	{
		return UIUtils::FormatBoolean (Value, mPadBack);
	}

	return UIPage::GetProperty (Name, Value);
}

//-----------------------------------------------------------------

void UITableHeader::Render( UICanvas & canvas) const
{
	UIPage::Render (canvas);
}

//-----------------------------------------------------------------

void UITableHeader::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, Code code )
{
	if (NotifyingObject == mTable)
	{
		if (code == UINotification::TableSizeChanged)
		{
			UISize extent;
			SetMinimumScrollExtent (UISize::zero);
			mTable->GetScrollExtent (extent);
			extent.y = 0L;
			SetMinimumScrollExtent (extent);
			SetScrollExtent (extent);
			UpdateButtonLayout ();
		}
		else if (code == UINotification::TableColumnsChanged)
		{
			RecreateButtons (false);
		}

		UI_UNREF (ContextObject);
	}
}

//-----------------------------------------------------------------

void UITableHeader::SetSize( const UISize & size)
{
	const long oldHeight = GetHeight ();

	UIPage::SetSize (size);

	if (oldHeight != size.y)
		UpdateButtonLayout ();
}

//-----------------------------------------------------------------

bool UITableHeader::ProcessMessage( const UIMessage & msg)
{
	if (UIPage::ProcessMessage (msg))
		return true;

	return false;
}

//-----------------------------------------------------------------

void UITableHeader::Link ()
{
	UIPage::Link ();

	if (mButtonStyle)
		mButtonStyle->Link ();
	if (mImageSortUp)
		mImageSortUp->Link ();
	if (mImageSortDown)
		mImageSortDown->Link ();
	if (mCursorResize)
		mCursorResize->Link ();

	RecreateButtons (true);
}

//-----------------------------------------------------------------

void UITableHeader::OnButtonPressed (UIWidget * context)
{
	const int index = findButtonIndex (static_cast<UIButton *>(context), GetChildrenRef ());
	assert (index >= 0);

	if (mTable)
	{
		UITableModel * const model = mTable->GetTableModel ();

		if (model)
			model->sortOnColumn (index, UITableModel::SD_reverse);
	}
}

//-----------------------------------------------------------------

bool UITableHeader::OnMessage( UIWidget *context, const UIMessage & msg )
{
	if (context->IsA (TUIPage) && context->GetParent () == this)
	{
		if (msg.Type == UIMessage::LeftMouseDown)
		{
			mDividerColumnRight = findDividerColumnRight (static_cast<UIPage *>(context), GetChildrenRef ());
			mDividerButtonDown = true;
			mLastDragPoint = msg.MouseCoords;
			return false;
		}
		else if (msg.Type == UIMessage::LeftMouseUp)
		{
			mDividerButtonDown = false;
			return false;
		}
		else if (mDividerButtonDown && msg.Type == UIMessage::MouseMove)
		{
			if (mTable)
			{
				UITableModel * const model = mTable->GetTableModel ();
				if (model)
				{
					if (mDividerColumnRight > 0 && mDividerColumnRight < model->GetColumnCount ())
					{
						mTable->ResizeColumns (mDividerColumnRight, msg.MouseCoords.x - mLastDragPoint.x);
					}
				}
			}

//			mLastDragPoint = msg.MouseCoords;

			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------

void UITableHeader::SetTable (UITable * table)
{
	RemoveProperty (PropertyName::Table);

	if (mTable)
		mTable->StopListening (this);

	const bool retval = AttachMember (mTable, table);

	if (mTable)
		mTable->Listen (this);

	if (retval)
		RecreateButtons (true);
}

//-----------------------------------------------------------------

void UITableHeader::SetButtonStyle   (UIButtonStyle * style)
{
	RemoveProperty (PropertyName::ButtonStyle);

	const bool retval = AttachMember (mButtonStyle, style);

	if (retval)
		RecreateButtons (true);
}

//-----------------------------------------------------------------

void UITableHeader::SetImageSortUp   (UIImageStyle * style)
{
	RemoveProperty (PropertyName::ImageSortUp);

	const bool retval = AttachMember (mImageSortUp, style);

	if (retval)
		RecreateButtons (true);
}

//-----------------------------------------------------------------

void UITableHeader::SetImageSortDown (UIImageStyle * style)
{
	RemoveProperty (PropertyName::ImageSortDown);

	const bool retval = AttachMember (mImageSortDown, style);

	if (retval)
		RecreateButtons (true);
}

//-----------------------------------------------------------------

void UITableHeader::SetCursorResize  (UICursor     * cursor)
{
	RemoveProperty (PropertyName::CursorResize);
	AttachMember (mCursorResize, cursor);
}

//----------------------------------------------------------------------

void UITableHeader::ClearHeaderChildren ()
{
	const UIObjectList & olist = GetChildrenRef ();
	for (UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
	{
		UIBaseObject * const child = *it;
		if (child->IsA (TUIWidget))
			static_cast<UIWidget *>(child)->RemoveCallback (this);
	}
	Clear ();
}

//-----------------------------------------------------------------

void UITableHeader::RecreateButtons (bool force)
{
	UIButtonStyle * const style = mButtonStyle;

	if (!mTable || !style)
	{
		ClearHeaderChildren ();
		return;
	}

	UITableModel * const tableModel = mTable->GetTableModel ();

	if (!tableModel)
	{
		ClearHeaderChildren ();
		return;
	}

	const long numColumns = tableModel->GetColumnCount ();

	const UIObjectList & olist = GetChildrenRef ();
	if (force || !mHasRecreated || numColumns + (numColumns - 1) != static_cast<int>(olist.size ()))
	{
		ClearHeaderChildren ();

		mDividerButtonDown = false;

		for (int i = 0; i < numColumns; ++i)
		{
			if (i != 0)
			{
				UIPage * const resizer = new UIPage;
				resizer->SetMouseCursor (mCursorResize);
				AddChild (resizer);
				resizer->SetName ("th_resizer");
				resizer->Link ();
				resizer->AddCallback  (this);
				resizer->SetTransient (true);
			}

			UIButton * const butt = new UIButton;
			butt->SetStyle  (style);
			AddChild (butt);
			butt->Link ();
			butt->SetName ("th_butt");
			butt->AddCallback  (this);
			butt->SetDragable  (false);
			butt->SetTransient (true);
		}
		mHasRecreated = true;
	}

	UpdateButtonLabels ();
	UpdateButtonLayout ();
}

//-----------------------------------------------------------------

void UITableHeader::UpdateButtonLayout ()
{
	UITableModel * const tableModel = mTable ? mTable->GetTableModel () : 0;

	if (!tableModel)
		return;

	const long numColumns = tableModel->GetColumnCount ();

	if (static_cast<int>(GetChildCount ()) < numColumns)
	{
//		assert (false);
		return;
	}

	const UIObjectList & olist = GetChildrenRef ();

	const long halfResizerWidth = mResizerWidth / 2L;

	int i = 0;
	long x = 0;
	for (UIObjectList::const_iterator it = olist.begin (); it != olist.end () && i < numColumns; ++it)
	{
		if ((*it)->IsA (TUIButton))
		{
			long width = 0;

			UIButton * const butt = static_cast<UIButton *>(*it);

			if (mTable->GetTotalColumnExtents (i, x, width))
			{
				if (i == 0)
				{
					if (mPadFront)
					{
						width  -= mResizerWidth;
						x += halfResizerWidth;
					}
					else
					{
						width  -= halfResizerWidth;
					}
				}
				else if ((i + 1) == numColumns)
				{
					x      += halfResizerWidth;

					if (mPadBack)
						width  -= mResizerWidth;
					else
						width  -= halfResizerWidth;
				}
				else
				{
					x      += halfResizerWidth;
					width  -= mResizerWidth;
				}

				butt->SetLocation (x, 0);
				butt->SetWidth (width);
				butt->SetHeight (GetHeight ());
				butt->SetTransient (true);
			}
			else
			{
				width = 0;
//				assert (false);
			}

			x += width;
			++i;
		}
		else if ((*it)->IsA (TUIPage))
		{
			UIPage * const resizer = static_cast<UIPage *>(*it);
			resizer->SetWidth (mResizerWidth);
			resizer->SetHeight (GetHeight ());
			resizer->SetBackgroundColor (UIColor::red);
			resizer->SetBackgroundOpacity (0.0f);
			resizer->SetLocation (x, 0);
			resizer->SetTransient (true);
		}
	}
}

//-----------------------------------------------------------------

void UITableHeader::UpdateButtonLabels ()
{
	UITableModel * const tableModel = mTable ? mTable->GetTableModel () : 0;

	if (!tableModel)
		return;

	const long numColumns = tableModel->GetColumnCount ();

	if (static_cast<int>(GetChildCount ()) < numColumns)
	{
		assert (false);
		return;
	}

	const UIObjectList & olist = GetChildrenRef ();

	int i = 0;
	for (UIObjectList::const_iterator it = olist.begin (); it != olist.end () && i < numColumns; ++it)
	{
		if ((*it)->IsA (TUIButton))
		{
			UIString str;
			tableModel->GetLocalizedColumnName (i, str);

			UIButton * const button = static_cast<UIButton *>(*it);
			button->SetText    (str);

			UIString tooltipStr;
			if (!tableModel->GetLocalizedColumnTooltip (i, tooltipStr) || tooltipStr.empty ())
				tooltipStr = str;

			button->SetTooltip (tooltipStr);
			++i;
		}
	}
}

// ======================================================================
