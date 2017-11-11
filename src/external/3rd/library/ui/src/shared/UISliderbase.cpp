#include "_precompile.h"

#include "UIButton.h"
#include "UIButtonStyle.h"
#include "UICanvas.h"
#include "UIClock.h"
#include "UIEventCallback.h"
#include "UIEventCallback.h"
#include "UIImageStyle.h"
#include "UIMessage.h"
#include "UINotification.h"
#include "UIPropertyDescriptor.h"
#include "UIRectangleStyle.h"
#include "UISliderbarStyle.h"
#include "UISliderbase.h"
#include "UIUtils.h"
#include "UIWidgetRectangleStyles.h"

#include <vector>

class UISliderbase::PrivateCallback :
public UINotification,
public UIEventCallback
{
public:

	explicit PrivateCallback (UISliderbase & sliderbase) :
		UINotification  (),
		UIEventCallback (),
		mSliderbase     (sliderbase),
		mListening      (false)
	{
		mSliderbase.AddCallback (this);
		setListening            (true);
	}

	~PrivateCallback ()
	{
		setListening               (false);
		mSliderbase.RemoveCallback (this);
	}

	void  OnActivate        (UIWidget *Context)
	{
		mSliderbase.PrivateOnActivate (Context);
	}

	bool  OnMessage         (UIWidget *Context, const UIMessage & msg)
	{
		return mSliderbase.PrivateOnMessage (Context, msg);
	}

	void  Notify(UINotificationServer *notifyingObject, UIBaseObject *contextObject, UINotification::Code notificationCode)
	{
		if (mSliderbase.IsEnabled() || mSliderbase.WillDraw()) 
		{
			mSliderbase.PrivateNotify(notifyingObject, contextObject, notificationCode);
		}
	}

	void setListening (bool b)
	{
		if (b != mListening)
		{
			mListening = b;

			if (mListening)
				UIClock::gUIClock ().ListenPerFrame (this);
			else
				UIClock::gUIClock ().StopListening (this);
		}
	}

private:

	PrivateCallback ();
	PrivateCallback & operator= (const PrivateCallback & rhs);
	PrivateCallback (const PrivateCallback & rhs);

	UISliderbase & mSliderbase;

	bool           mListening : 1;
};

//----------------------------------------------------------------------

const char * const UISliderbase::TypeName                 = "Sliderbase";

const UILowerString  UISliderbase::PropertyName::OnSliderChange   = UILowerString ("OnSliderChange");
const UILowerString  UISliderbase::PropertyName::Style      = UILowerString ("Style");
const UILowerString  UISliderbase::PropertyName::ValueUpdateContinuous  = UILowerString ("ValueUpdateContinuous");

//======================================================================================
#define _TYPENAME UISliderbase

namespace UISliderbaseNamespace
{
	const unsigned long INCREMENT_TICK_TIME = 1;
	const unsigned long INCREMENT_TICK_TIME_DELAY = 50;

	//================================================================
	// Basic category
	_GROUPBEGIN(Basic)
		_DESCRIPTOR(Style, "", T_object),
		_DESCRIPTOR(ValueUpdateContinuous, "", T_bool),
	_GROUPEND(Basic, 3, 0);
	//================================================================
}
using namespace UISliderbaseNamespace;
//======================================================================================

UISliderbase::UISliderbase() :
UIPage                 (),
mStyle                 (0),
mValueUpdateContinuous (true),
mLastIncrementTime     (0),
mThumbMouseOffset      (0),
mPrivateCallback       (0)
{
	mPrivateCallback = new PrivateCallback (*this);

	mAutoPress [0] = mAutoPress [1] = false;

	for (int i = 0; i < BT_numButtonTypes; ++i)
	{
		mButtons [i] = new UIButton;
		mButtons [i]->SetTransient (true);
		mButtons [i]->Attach       (this);
		mButtons [i]->SetVisible   (false);
		AddChild (mButtons [i]);
		mButtons [i]->AddCallback (mPrivateCallback);
	}

	MoveChild (mButtons [BT_overlay],       UIBaseObject::Bottom);
	MoveChild (mButtons [BT_trackDisabled], UIBaseObject::Bottom);
	MoveChild (mButtons [BT_track],         UIBaseObject::Bottom);

	mButtons [BT_start]->SetPressedStateSticky (true);
	mButtons [BT_start]->SetName ("start");
	mButtons [BT_end]->SetPressedStateSticky (true);
	mButtons [BT_end]->SetName ("end");
	mButtons [BT_thumb]->SetPressedStateSticky (true);
	mButtons [BT_thumb]->SetName ("thumb");
}

//----------------------------------------------------------------------

UISliderbase::~UISliderbase()
{
	for (int i = 0; i < BT_numButtonTypes; ++i)
	{
		mButtons [i]->RemoveCallback (mPrivateCallback);
		RemoveChild (mButtons [i]);
		mButtons [i]->Detach (this);
		mButtons [i] = 0;
	}

	SetStyle(0);

	delete mPrivateCallback;
	mPrivateCallback = 0;
}

//----------------------------------------------------------------------

bool UISliderbase::IsA( const UITypeID Type ) const
{
	return (Type == TUISliderbase) || UIPage::IsA( Type );
}

//----------------------------------------------------------------------

const char *UISliderbase::GetTypeName() const
{
	return TypeName;
}

//----------------------------------------------------------------------

UIBaseObject *UISliderbase::Clone() const
{
	return 0;
}

//----------------------------------------------------------------------

bool UISliderbase::ProcessMessage( const UIMessage &msg )
{
	if( UIPage::ProcessMessage( msg ) )
		return true;

	if( !mStyle )
		return false;

	switch( msg.Type )
	{
	case UIMessage::LeftMouseDown:
	case UIMessage::RightMouseDown:
	case UIMessage::MiddleMouseDown:
		break;

	case UIMessage::LeftMouseUp:
	case UIMessage::RightMouseUp:
	case UIMessage::MiddleMouseUp:
		break;

	case UIMessage::MouseMove:
		break;

	case UIMessage::MouseWheel:
		{
			if (msg.Data > 0)
			{
				for (int i = 0; i < msg.Data; ++i)
					IncrementValueInternal (false, true);
			}
			else
			{
				for (int i = 0; i < -msg.Data; ++i)
					IncrementValueInternal (false, false);
			}
		}
		return true;
	}

	return false;
}


//----------------------------------------------------------------------

void UISliderbase::Render( UICanvas &DestinationCanvas ) const
{
	//-- render the background

	if (mStyle)
	{
		const UIRectangleStyle * const bgstyle = mStyle->GetRectangleStyles ().GetAppropriateStyle (*this, 0);

		if (bgstyle)
			bgstyle->Render (GetAnimationState (), DestinationCanvas, GetSize ());

		UpdateThumbAndOverlay ();
	}

	//-- now render all the children, possibly overriding the background

	UIPage::Render (DestinationCanvas);
}

//----------------------------------------------------------------------

void UISliderbase::UpdateThumbAndOverlay () const
{
	if (mButtons [BT_thumb]->IsVisible ())
	{
		const UIRect & thumbMargin = mStyle->GetButtonMargin (UISliderbarStyle::BT_thumb);

		if (mStyle->GetLayout () == UIStyle::L_horizontal)
		{
			const long range = GetWidth () - thumbMargin.left - thumbMargin.right;

			mButtons [BT_thumb]->SetWidth (GetDesiredThumbSize (range));

			const long position = thumbMargin.left + GetDesiredThumbPosition (range - mButtons [BT_thumb]->GetWidth ());

			mButtons [BT_thumb]->SetLocation ( position, mButtons [BT_thumb]->GetLocation ().y);

			if (mButtons [BT_overlay]->IsVisible ())
			{
				const UIRect & overlayMargin = mStyle->GetButtonMargin (UISliderbarStyle::BT_overlay);
				const long length = position - mButtons [BT_overlay]->GetLocation ().x - overlayMargin.right;
				mButtons [BT_overlay]->SetWidth (length);
			}

			if (mButtons [BT_trackDisabled]->IsVisible ())
			{
				const UIRect & margin = mStyle->GetButtonMargin (UISliderbarStyle::BT_track);

				long disabledTrackPosition = 0L;
				if (GetDesiredDisabledTrackPosition (range - mButtons [BT_thumb]->GetWidth (), disabledTrackPosition))
				{
					const UIPoint & currentPos = mButtons [BT_trackDisabled]->GetLocation ();

					disabledTrackPosition += margin.left + mButtons [BT_thumb]->GetWidth ();

					const long length = GetWidth () - margin.right - disabledTrackPosition;
					mButtons [BT_trackDisabled]->SetLocation (UIPoint (disabledTrackPosition, currentPos.y));
					mButtons [BT_trackDisabled]->SetWidth    (length);
				}
			}
		}
		else
		{
			const long range = GetHeight () - thumbMargin.top - thumbMargin.bottom;

			mButtons [BT_thumb]->SetHeight (GetDesiredThumbSize (range));

			const long position = GetDesiredThumbPosition (range - mButtons [BT_thumb]->GetHeight ());

			const long thumbPosition = GetHeight () - (thumbMargin.bottom + position);

			mButtons [BT_thumb]->SetLocation (mButtons [BT_thumb]->GetLocation ().x, thumbPosition - mButtons [BT_thumb]->GetHeight ());

			if (mButtons [BT_overlay]->IsVisible ())
			{
				const UIRect & overlayMargin = mStyle->GetButtonMargin (UISliderbarStyle::BT_overlay);

				const long length = GetHeight () - overlayMargin.bottom - overlayMargin.top - thumbPosition;

				mButtons [BT_overlay]->SetHeight (length);
				mButtons [BT_overlay]->SetLocation (mButtons [BT_overlay]->GetLocation ().x, thumbPosition + overlayMargin.top);
			}
		}
	}
}

//----------------------------------------------------------------------

void UISliderbase::SetStyle( UISliderbarStyle *NewStyle )
{
	if (mStyle)
		mStyle->StopListening (mPrivateCallback);

	AttachMember (mStyle, NewStyle);

	UpdateFromStyle ();

	if (mStyle)
		mStyle->Listen (mPrivateCallback);
}

//----------------------------------------------------------------------

void UISliderbase::GetLinkPropertyNames( UIPropertyNameVector &In) const
{
	In.push_back( PropertyName::Style );

	UIPage::GetLinkPropertyNames( In );
}

//----------------------------------------------------------------------

void UISliderbase::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIPage::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Basic, category, o_groups);
}

//----------------------------------------------------------------------

void UISliderbase::GetPropertyNames( UIPropertyNameVector &In, bool forCopy  ) const
{
	In.push_back( PropertyName::OnSliderChange );
	In.push_back( PropertyName::Style );
	In.push_back( PropertyName::ValueUpdateContinuous );

	UIPage::GetPropertyNames( In, forCopy );
}

//----------------------------------------------------------------------

bool UISliderbase::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if( Name == PropertyName::Style )
	{
		UIBaseObject *NewStyle = GetObjectFromPath( Value, TUISliderbarStyle );

		if( NewStyle || Value.empty() )
		{
			SetStyle( reinterpret_cast<UISliderbarStyle *>( NewStyle ) );
			return true;
		}
	}
	else if ( Name == PropertyName::ValueUpdateContinuous )
		return UIUtils::ParseBoolean( Value, mValueUpdateContinuous);

	return UIPage::SetProperty( Name, Value );
}

//----------------------------------------------------------------------

bool UISliderbase::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if( Name == PropertyName::Style )
	{
		if( mStyle )
		{
			GetPathTo( Value, mStyle );
			return true;
		}
	}
	else if ( Name == PropertyName::ValueUpdateContinuous )
		return UIUtils::FormatBoolean( Value, mValueUpdateContinuous);

	return UIPage::GetProperty( Name, Value );
}

//----------------------------------------------------------------------

UIStyle *UISliderbase::GetStyle() const
{
	return mStyle;
};


//----------------------------------------------------------------------

void UISliderbase::PrivateNotify (UINotificationServer *notifyingObject, UIBaseObject *, UINotification::Code)
{
	if (notifyingObject == mStyle)
	{
		UpdateFromStyle ();
	}
	//-- clock
	else
	{
		unsigned long const currentTime = UIClock::gUIClock().GetTime();
		if (mLastIncrementTime < currentTime)
		{
			if (mButtons [BT_start]->IsVisible () && mButtons [BT_start]->IsPressed ())
			{
				mAutoPress [0] = true;
				IncrementValueInternal (false, false, true);
			}
			else if (mButtons [BT_end]->IsVisible () && mButtons [BT_end]->IsPressed ())
			{
				mAutoPress [1] = true;
				IncrementValueInternal (false, true, true);
			}
		}

		UpdatePerFrame();
	}
}

//----------------------------------------------------------------------

void UISliderbase::UpdatePerFrame               ()
{
}

//----------------------------------------------------------------------

void UISliderbase::UpdateFromStyle ()
{
	if (!mStyle)
	{
		for (int i = 0; i < BT_numButtonTypes; ++i)
		{
			if (mButtons [i])
				mButtons [i]->SetVisible (false);
		}
		return;
	}

	for (int i = 0; i < BT_numButtonTypes; ++i)
	{
		UIButtonStyle * const bstyle = mStyle->GetButtonStyle (static_cast<UISliderbarStyle::ButtonType>(i));
		mButtons [i]->SetStyle (bstyle);
		mButtons [i]->SetVisible (bstyle != 0);
	}

	//-- horizontal

	if (mButtons [BT_start]->IsVisible ())
	{
		const UIRect & margin = mStyle->GetButtonMargin (UISliderbarStyle::BT_start);
		const long length     = mStyle->GetButtonLength (UISliderbarStyle::BT_start);

		if (mStyle->GetLayout () == UIStyle::L_horizontal)
		{
			mButtons [BT_start]->SetLocation       (margin.Location ());
			mButtons [BT_start]->SetSize           (UISize (length, GetHeight () - margin.top - margin.bottom));
			mButtons [BT_start]->SetPropertyNarrow (UIWidget::PropertyName::PackSize, "f,a");
			mButtons [BT_start]->SetPropertyNarrow (UIWidget::PropertyName::PackLocation, "nfn,nfn");
		}
		else
		{
			mButtons [BT_start]->SetLocation       (UIPoint (margin.left, GetHeight () - margin.bottom - length));
			mButtons [BT_start]->SetSize           (UISize (GetWidth () - margin.left - margin.right, length));
			mButtons [BT_start]->SetPropertyNarrow (UIWidget::PropertyName::PackSize, "a,f");
			mButtons [BT_start]->SetPropertyNarrow (UIWidget::PropertyName::PackLocation, "nfn,fff");
		}
	}

	if (mButtons [BT_end]->IsVisible ())
	{
		const UIRect & margin = mStyle->GetButtonMargin (UISliderbarStyle::BT_end);
		const long length     = mStyle->GetButtonLength (UISliderbarStyle::BT_end);

		if (mStyle->GetLayout () == UIStyle::L_horizontal)
		{
			mButtons [BT_end]->SetLocation       (UISize (GetWidth () - margin.right - length, margin.top));
			mButtons [BT_end]->SetSize           (UISize (length, GetHeight () - margin.top - margin.bottom));
			mButtons [BT_end]->SetPropertyNarrow (UIWidget::PropertyName::PackSize,     "f,a");
			mButtons [BT_end]->SetPropertyNarrow (UIWidget::PropertyName::PackLocation, "fff,nfn");
		}
		else
		{
			mButtons [BT_end]->SetLocation       (margin.Location ());
			mButtons [BT_end]->SetSize           (UISize (GetWidth () - margin.left - margin.right, length));
			mButtons [BT_end]->SetPropertyNarrow (UIWidget::PropertyName::PackSize,     "a,f");
			mButtons [BT_end]->SetPropertyNarrow (UIWidget::PropertyName::PackLocation, "nfn,nfn");
		}
	}

	//----------------------------------------------------------------------

	if (mButtons [BT_track]->IsVisible ())
	{
		const UIRect & margin = mStyle->GetButtonMargin (UISliderbarStyle::BT_track);

		mButtons [BT_track]->SetLocation       (margin.Location ());
		mButtons [BT_track]->SetSize           (UISize (GetWidth () - margin.left - margin.right, GetHeight () - margin.top - margin.bottom));
		mButtons [BT_track]->SetPropertyNarrow (UIWidget::PropertyName::PackSize,     "a,a");
	}

	//----------------------------------------------------------------------

	if (mButtons [BT_trackDisabled]->IsVisible ())
	{
		mButtons [BT_trackDisabled]->SetEnabled (false);

		const UIRect & margin = mStyle->GetButtonMargin (UISliderbarStyle::BT_trackDisabled);

//		mButtons [BT_trackDisabled]->SetLocation       (margin.Location ());
		if (mStyle->GetLayout () == UIStyle::L_horizontal)
		{
			mButtons [BT_trackDisabled]->SetSize           (UISize (0L, GetHeight () - margin.top - margin.bottom));
			mButtons [BT_trackDisabled]->SetLocation       (UISize (0L, margin.top));
			mButtons [BT_trackDisabled]->SetPropertyNarrow (UIWidget::PropertyName::PackSize,     "p,a");
			mButtons [BT_trackDisabled]->SetPropertyNarrow (UIWidget::PropertyName::PackLocation, "fff,nfn");
		}
		else
		{
			mButtons [BT_trackDisabled]->SetSize           (UISize (GetWidth () - margin.left - margin.right, 0L));
			mButtons [BT_trackDisabled]->SetLocation       (UISize (margin.left, 0L));
			mButtons [BT_trackDisabled]->SetPropertyNarrow (UIWidget::PropertyName::PackSize,     "a,p");
			mButtons [BT_trackDisabled]->SetPropertyNarrow (UIWidget::PropertyName::PackLocation, "nfn,nfn");
		}
	}

	//----------------------------------------------------------------------

	if (mButtons [BT_overlay]->IsVisible ())
	{
		const UIRect & margin = mStyle->GetButtonMargin (UISliderbarStyle::BT_overlay);

		if (mStyle->GetLayout () == UIStyle::L_horizontal)
		{
			mButtons [BT_overlay]->SetLocation       (margin.Location ());
			mButtons [BT_overlay]->SetHeight         (GetHeight () - margin.top - margin.bottom);
			mButtons [BT_overlay]->SetPropertyNarrow (UIWidget::PropertyName::PackSize,     "p,a");
			mButtons [BT_overlay]->SetPropertyNarrow (UIWidget::PropertyName::PackLocation, "nfn,nfn");
		}
		else
		{
			mButtons [BT_overlay]->SetLocation       (margin.left, GetHeight () - margin.bottom);
			mButtons [BT_overlay]->SetHeight         (0L);
			mButtons [BT_overlay]->SetWidth          (GetWidth () - margin.left - margin.right);
			mButtons [BT_overlay]->SetPropertyNarrow (UIWidget::PropertyName::PackSize,     "a,p");
			mButtons [BT_overlay]->SetPropertyNarrow (UIWidget::PropertyName::PackLocation, "nfn,fff");
		}
	}

	if (mButtons [BT_thumb]->IsVisible ())
	{
		const UIRect & margin = mStyle->GetButtonMargin (UISliderbarStyle::BT_thumb);
		const long length     = mStyle->GetButtonLength (UISliderbarStyle::BT_thumb);

		if (mStyle->GetLayout () == UIStyle::L_horizontal)
		{
			mButtons [BT_thumb]->SetLocation       (margin.Location ());
			mButtons [BT_thumb]->SetHeight         (GetHeight () - margin.top - margin.bottom);
			mButtons [BT_thumb]->SetMinimumSize    (UISize (length, 0L));
			mButtons [BT_thumb]->SetWidth          (length);
			mButtons [BT_thumb]->SetPropertyNarrow (UIWidget::PropertyName::PackSize,     "p,a");
			mButtons [BT_thumb]->SetPropertyNarrow (UIWidget::PropertyName::PackLocation, "nfn,nfn");
		}
		else
		{
			mButtons [BT_thumb]->SetLocation       (UISize (margin.left, GetHeight () - length - margin.bottom));
			mButtons [BT_thumb]->SetMinimumSize    (UISize (0L, length));
			mButtons [BT_thumb]->SetWidth          (GetWidth () - margin.left - margin.right);
			mButtons [BT_thumb]->SetHeight         (length);
			mButtons [BT_thumb]->SetPropertyNarrow (UIWidget::PropertyName::PackSize,     "a,p");
			mButtons [BT_thumb]->SetPropertyNarrow (UIWidget::PropertyName::PackLocation, "nfn,fff");
		}
	}
}

//----------------------------------------------------------------------

long UISliderbase::GetDesiredThumbSize (long) const
{
	return 0;
}

//----------------------------------------------------------------------

void UISliderbase::PrivateOnActivate (UIWidget * context)
{
	if (context == mButtons [BT_start])
	{
		mAutoPress [0] = false;
		IncrementValueInternal (false, false, true);
	}
	else if (context == mButtons [BT_end])
	{
		mAutoPress [1] = false;
		IncrementValueInternal (false, true, true);
	}
}

//----------------------------------------------------------------------

bool UISliderbase::PrivateOnMessage(UIWidget *context, const UIMessage & msg)
{
	if (!mStyle)
		return true;

	if (context == this && mButtons [BT_thumb]->IsVisible () && msg.Type == UIMessage::LeftMouseDown)
	{
		const UIRect & thumbMargin = mStyle->GetButtonMargin (UISliderbarStyle::BT_thumb);
		if (msg.MouseCoords.x >= thumbMargin.left && msg.MouseCoords.x <= GetWidth () - thumbMargin.right)
		{
			const UIWidget * const underMouse = GetWidgetFromPoint (msg.MouseCoords, true);

			if (underMouse != mButtons [BT_thumb] &&
				underMouse != mButtons [BT_start] &&
				underMouse != mButtons [BT_end])
			{
				long range = 0L;
				long position = 0L;

				if (mStyle->GetLayout () == UIStyle::L_horizontal)
				{
					range    = GetWidth () - thumbMargin.left - thumbMargin.right - mButtons [BT_thumb]->GetWidth ();
					position = msg.MouseCoords.x - thumbMargin.left - mButtons [BT_thumb]->GetWidth () / 2L;
				}
				else
				{
					range    = GetHeight () - thumbMargin.top - thumbMargin.bottom - mButtons [BT_thumb]->GetHeight ();
					position = GetHeight () - msg.MouseCoords.y - thumbMargin.bottom - mButtons [BT_thumb]->GetHeight () / 2L;
				}

				UpdateValueFromThumbPosition (position, range, true);
				UpdateThumbAndOverlay ();

				//-- force the thumb to go into the pressed state
				if (mButtons [BT_thumb])
				{
					UIMessage buttonMessage = msg;
					buttonMessage.MouseCoords = UIPoint::zero;
					mButtons [BT_thumb]->ProcessMessage (buttonMessage);
					SetWidgetWithMouseLock (mButtons [BT_thumb]);
				}

				return false;
			}
		}
	}

	else if (context == mButtons [BT_thumb] && mButtons [BT_thumb]->IsVisible ())
	{
		if (msg.Type == UIMessage::LeftMouseDown)
		{
			if (mStyle->GetLayout () == UIStyle::L_horizontal)
				mThumbMouseOffset = msg.MouseCoords.x;
			else
				mThumbMouseOffset = msg.MouseCoords.y;

			return true;
		}
		else if (msg.Type == UIMessage::MouseMove && mButtons [BT_thumb]->IsPressedByMouse ())
		{
			const UIRect & thumbMargin = mStyle->GetButtonMargin (UISliderbarStyle::BT_thumb);

			long range = 0L;
			long position = 0L;

			if (mStyle->GetLayout () == UIStyle::L_horizontal)
			{
				range    = GetWidth () - thumbMargin.left - thumbMargin.right - mButtons [BT_thumb]->GetWidth ();
				position = mButtons [BT_thumb]->GetLocation ().x - mThumbMouseOffset + msg.MouseCoords.x - thumbMargin.left;
			}
			else
			{
				range    = GetHeight () - thumbMargin.top - thumbMargin.bottom - mButtons [BT_thumb]->GetHeight ();
				position = GetHeight () - thumbMargin.bottom - mButtons [BT_thumb]->GetLocation ().y - mButtons [BT_thumb]->GetHeight () + mThumbMouseOffset - msg.MouseCoords.y;
			}

			UpdateValueFromThumbPosition (position, range, true);
		}
	}

	return true;
}

//----------------------------------------------------------------------

void UISliderbase::IncrementValueInternal(bool page, bool upDown, bool notify)
{
	unsigned long const currentUpdateTime = UIClock::gUIClock ().GetTime() + INCREMENT_TICK_TIME_DELAY;
	if (currentUpdateTime != mLastIncrementTime) 
	{
		mLastIncrementTime = currentUpdateTime;
		IncrementValue (page, upDown, notify);
	}
}

//----------------------------------------------------------------------

bool UISliderbase::GetDesiredDisabledTrackPosition (long , long & ) const
{
	return false;
}

//----------------------------------------------------------------------

