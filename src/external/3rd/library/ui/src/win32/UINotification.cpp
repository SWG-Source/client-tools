#include "_precompile.h"

#include "UIClock.h"
#include "UINotification.h"
#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

//-----------------------------------------------------------------

UINotificationServer::UINotificationServer () :
mListeners              (0),
mListenersPendingAdd    (0),
mListenersPendingRemove (0),
mNotifying     (false)
{
}

//-----------------------------------------------------------------

void UINotificationServer::Listen (UINotification *NewListener)
{
	assert (NewListener);
	if (!NewListener)
		return;

	if (!mListeners)
		mListeners = new UINotificationMap;

	if (mNotifying)
	{
		if (mListenersPendingRemove)
		{
			const UINotificationVector::iterator it = std::find (mListenersPendingRemove->begin (), mListenersPendingRemove->end (), NewListener);
			if (it != mListenersPendingRemove->end ())
			{
				mListenersPendingRemove->erase (it);
				return;
			}
		}

		if (!mListenersPendingAdd)
			mListenersPendingAdd = new UINotificationMap;

		(*mListenersPendingAdd) [NewListener] = false;
	}
	else
		(*mListeners) [NewListener] = false;
}

//-----------------------------------------------------------------

void UINotificationServer::ListenPerFrame( UINotification *NewListener )
{
	assert (NewListener);
	if (!NewListener)
		return;

	if (!mListeners)
		mListeners = new UINotificationMap;

	if (mNotifying)
	{
		if (mListenersPendingRemove)
		{
			const UINotificationVector::iterator it = std::find (mListenersPendingRemove->begin (), mListenersPendingRemove->end (), NewListener);
			if (it != mListenersPendingRemove->end ())
			{
				mListenersPendingRemove->erase (it);
				return;
			}
		}

		if (!mListenersPendingAdd)
			mListenersPendingAdd = new UINotificationMap;

		(*mListenersPendingAdd) [NewListener] = true;
	}

	(*mListeners) [NewListener] = true;
}

//-----------------------------------------------------------------

void UINotificationServer::StopListening( UINotification *ListenerToRemove )
{
	assert (ListenerToRemove);
	if (!ListenerToRemove)
		return;

	if (mNotifying)
	{
		if (!mListenersPendingRemove)
			mListenersPendingRemove = new UINotificationVector;

		UINotificationVector::iterator upper = std::upper_bound (mListenersPendingRemove->begin (), mListenersPendingRemove->end (), ListenerToRemove);

		if (upper == mListenersPendingRemove->end ())
			mListenersPendingRemove->push_back (ListenerToRemove);
		else
			mListenersPendingRemove->insert (upper, ListenerToRemove);
	}
	else
	{
		if (mListeners)
			mListeners->erase(ListenerToRemove);
	}
}

//-----------------------------------------------------------------

void UINotificationServer::SendNotification( UINotification::Code NotificationCode, UIBaseObject *Context, bool repeating )
{	
	if (mListeners && !mNotifying)
	{
		if (mListeners->empty ())
			return;

		mNotifying = true;
				
		{
			for (UINotificationMap::iterator it = mListeners->begin (); it != mListeners->end (); ++it)
			{
				const bool perFrame = (*it).second;
				//-- only put pointers on the vectors if they want to be updated again this frame
				if (!perFrame || !repeating)
				{
					UINotification * const notification = (*it).first;

					assert (notification);

					if (!notification)
						continue;

					//-- this one is scheduled for remove
					if (mListenersPendingRemove && !mListenersPendingRemove->empty ())
					{
						if (std::binary_search (mListenersPendingRemove->begin (), mListenersPendingRemove->end (), notification))
							continue;
					}

					notification->Notify (this, Context, NotificationCode);
				}
			}
		}
		
		{
			if (mListenersPendingRemove && !mListenersPendingRemove->empty ())
			{
				for (UINotificationVector::const_iterator it = mListenersPendingRemove->begin (); it != mListenersPendingRemove->end (); ++it)
				{
					mListeners->erase (*it);
				}
				delete mListenersPendingRemove;
				mListenersPendingRemove = 0;
			}
		}
		
		{
			if (mListenersPendingAdd && !mListenersPendingAdd->empty ())
			{
				for (UINotificationMap::iterator it = mListenersPendingAdd->begin (); it != mListenersPendingAdd->end (); ++it)
				{
					const bool perFrame                 = (*it).second;
					UINotification * const notification = (*it).first;
					(*mListeners) [notification] = perFrame;
				}
				delete mListenersPendingAdd;
				mListenersPendingAdd = 0;
			}
		}
		
		mNotifying = false;
	}
}

//----------------------------------------------------------------------

bool UINotificationServer::HasListener      (const UINotification & listener, bool & pendingAdd, bool & pendingRemove) const
{
	const bool primary = mListeners &&              mListeners->find           (const_cast<UINotification*>(&listener))  != mListeners->end ();
	pendingAdd         = mListenersPendingAdd &&    mListenersPendingAdd->find (const_cast<UINotification*>(&listener)) != mListenersPendingAdd->end ();
	pendingRemove      = mListenersPendingRemove && std::find (mListenersPendingRemove->begin (), mListenersPendingRemove->end (), &listener) != mListenersPendingRemove->end ();
	return primary;
}

//-----------------------------------------------------------------

UINotification::~UINotification ()
{
#if _DEBUG
	bool pendingAdd = false;
	bool pendingRemove = false;

	UIClock * const clock = UIClock::GetSingleton ();

	if (clock && clock->HasListener      (*this, pendingAdd, pendingRemove))
	{
		if (!pendingRemove)
		{
			const bool isNotifying = UIClock::gUIClock ().IsNotifying ();
			UI_UNREF (isNotifying);
			
			assert (!(pendingAdd && isNotifying));     // assert if notification is pending add and isnotifying
			assert (!pendingAdd);                      // assert if notification is pending add
			assert (!isNotifying);                     // assert if notification is on primary list and isnotifying
			assert (false);                            // assert if notification is on primary list
		}
	}
#endif
}

//-----------------------------------------------------------------

UINotificationServer::~UINotificationServer ()
{
	delete mListeners;
	mListeners = 0;

	delete mListenersPendingAdd;
	delete mListenersPendingRemove;
	mListenersPendingRemove = 0;
	mListenersPendingAdd = 0;
}

//-----------------------------------------------------------------
