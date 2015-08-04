#ifndef __UICLOCK_H__
#define __UICLOCK_H__

#include "UIBaseObject.h"
#include "UINotification.h"

//-----------------------------------------------------------------

class UIClock : public UIBaseObject, public UINotificationServer
{
public:
	
	static const char      *TypeName; //lint !e1516 // data m ember hides inherited member
	
	UIClock  ();
	~UIClock ();
	
	bool                   IsA              (const UITypeID) const;
	const char *           GetTypeName      () const;
	UIBaseObject  *        Clone            () const;
	
	UITime                 GetTime          () const { return mTime; };
	void                   Advance          (UITime AdvanceBy);
	void                   Reset            ();
	UITime                 GetLastFrameTime () const;

	static UIClock *       GetSingleton     ();
	
	// Access the global clock object
	static UIClock &gUIClock ()
	{ 
		if( !gSingleton )
			gSingleton = new UIClock; 
		
		return *gSingleton; 
	};
	
	static void ExplicitDestroy ();
	
private:
	
	UIClock & operator= (const UIClock &);
	UIClock   (const UIClock &);
	
	UITime mTime;
	UITime mLastFrameTime;

	static UIClock *gSingleton;
};

//----------------------------------------------------------------------

inline UITime UIClock::GetLastFrameTime () const
{
	return mLastFrameTime;
}

//----------------------------------------------------------------------

inline UIClock * UIClock::GetSingleton     ()
{
	return gSingleton;
}


//-----------------------------------------------------------------

#endif // __UICLOCK_H__
