#ifndef __UINOTIFICATION_H__
#define __UINOTIFICATION_H__

//-----------------------------------------------------------------

// This is an abstract interface for notification

//-----------------------------------------------------------------

class UIBaseObject;
class UINotificationServer;

//-----------------------------------------------------------------

class UINotification
{
public:

	enum Code
	{
		ObjectChanged,
		ChildAdded,
		ChildRemoved,
		ChildChanged,
		TableSizeChanged,
		TableDataChanged,
		TableSortingChanged,
		TableColumnsChanged
	};

	virtual void Notify (UINotificationServer *notifyingObject, UIBaseObject *contextObject, UINotification::Code notificationCode) = 0;
	virtual ~UINotification () = 0;

	//-- this must be implemented to shut up stupid lint warnings
	UINotification () {}
};

//-----------------------------------------------------------------

class UINotificationServer
{
public:

	UINotificationServer  ();
	void Listen           (UINotification *);
	void ListenPerFrame   (UINotification *);
	void StopListening    (UINotification *);
	void SendNotification (UINotification::Code code, UIBaseObject * context, bool repeating = false);
	
	bool HasListener      (const UINotification &, bool & pendingAdd, bool & pendingRemove) const;
	bool IsNotifying      () const;

	virtual ~UINotificationServer();

private:
	
	typedef ui_stdmap<UINotification *, bool>::fwd UINotificationMap;
	UINotificationMap * mListeners;

	typedef ui_stdvector<UINotification *>::fwd UINotificationVector;
	UINotificationMap *    mListenersPendingAdd;
	UINotificationVector * mListenersPendingRemove;
	
	bool mNotifying;
};

//----------------------------------------------------------------------

inline bool UINotificationServer::IsNotifying      () const
{
	return mNotifying;
}

//-----------------------------------------------------------------

#endif // __UINOTIFICATION_H__
