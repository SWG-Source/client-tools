#ifndef __UISLIDERBASE_H_
#define __UISLIDERBASE_H_

class UISliderbarStyle;
class UIButton;

#include "UINotification.h"
#include "UIPage.h"


#include <bitset>

//----------------------------------------------------------------------

class UISliderbase :
public UIPage
{
public:

	static const char * const TypeName;

	class PropertyName
	{
	public:
		static const UILowerString  OnSliderChange;
		static const UILowerString  Style;
		static const UILowerString  ValueUpdateContinuous;
	};

	                          UISliderbase ();
	virtual                  ~UISliderbase ();

	virtual bool              IsA               (const UITypeID) const;
	virtual const char *      GetTypeName       () const;
	virtual UIBaseObject *    Clone             () const;

	virtual void              GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void              GetPropertyNames  (UIPropertyNameVector &, bool forCopy ) const;
	virtual void              GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool              SetProperty       (const UILowerString & Name, const UIString &Value);
	virtual bool              GetProperty       (const UILowerString & Name, UIString &Value) const;

	virtual bool              ProcessMessage    (const UIMessage &);
	virtual void              Render            (UICanvas &) const;

	        void              SetStyle          (UISliderbarStyle *);
	UISliderbarStyle *        GetSliderbarStyle () const;
	UIStyle *                 GetStyle          () const;


protected:

	virtual long              GetDesiredThumbSize          (long range) const;
	virtual long              GetDesiredThumbPosition      (long range) const = 0;
	virtual void              IncrementValue               (bool page, bool upDown, bool notify = true) = 0;
	virtual void              UpdateValueFromThumbPosition (long position, long range, bool notify = true) = 0;
	virtual void              UpdatePerFrame               ();

	virtual bool              GetDesiredDisabledTrackPosition (long range, long & pos) const;

	UIButton *                GetThumb                     ();

private:

	virtual void              PrivateOnActivate        (UIWidget *Context);
	virtual bool              PrivateOnMessage         (UIWidget *Context, const UIMessage & msg);
	virtual void              PrivateNotify            (UINotificationServer *notifyingObject, UIBaseObject *contextObject, UINotification::Code notificationCode);

	class PrivateCallback;
	friend class PrivateCallback;

	enum ButtonType
	{
		BT_start,
		BT_end,
		BT_thumb,
		BT_track,
		BT_trackDisabled,
		BT_overlay,
		BT_numButtonTypes
	};

	                           UISliderbase      (const UISliderbase &);
	UISliderbase               &operator =       (const UISliderbase &);

	void                      UpdateFromStyle          ();
	void                      UpdateThumbAndOverlay    () const;
	void                      IncrementValueInternal   (bool page, bool upDown, bool notify = true);

	UISliderbarStyle *        mStyle;

	bool                      mValueUpdateContinuous;

	UIButton *                mButtons [BT_numButtonTypes];

	unsigned long             mLastIncrementTime;

	std::bitset<2> mAutoPress;

	long                      mThumbMouseOffset;

	PrivateCallback *         mPrivateCallback;

};

//----------------------------------------------------------------------

inline UISliderbarStyle  *UISliderbase::GetSliderbarStyle () const
{
	return mStyle;
};

//----------------------------------------------------------------------

inline UIButton * UISliderbase::GetThumb                     ()
{
	return mButtons [static_cast<size_t>(BT_thumb)];
}

//----------------------------------------------------------------------


#endif // __UISLIDERBASE_H_