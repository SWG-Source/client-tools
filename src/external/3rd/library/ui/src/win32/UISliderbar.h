#ifndef __UISLIDERBAR_H__
#define __UISLIDERBAR_H__

#include "UISliderbase.h"

//----------------------------------------------------------------------

class UISliderbar : 
public UISliderbase
{
public:

	static const char * const TypeName;

	class PropertyName
	{
	public:
		static const UILowerString  LowerLimit;
		static const UILowerString  UpperLimit;
		static const UILowerString  UpperLimitAllowed;
		static const UILowerString  UpperLimitAllowedEnabled;
		static const UILowerString  Value;
	};
	
	                          UISliderbar();
	virtual                  ~UISliderbar();
	
	virtual bool              IsA               (const UITypeID) const;
	virtual const char *      GetTypeName       () const;
	virtual UIBaseObject *    Clone             () const;

	virtual void              GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void              GetPropertyNames  (UIPropertyNameVector &, bool forCopy ) const;
	
	virtual bool              SetProperty       (const UILowerString & Name, const UIString &Value);
	virtual bool              GetProperty       (const UILowerString & Name, UIString &Value) const;
			
	       void               SetValue          (const long, bool notify);
	       long               GetValue          () const;
	
	       long               GetUpperLimit        () const;
	       void               SetUpperLimit        (long, bool notify = true);
	       long               GetUpperLimitAllowed () const;
	       void               SetUpperLimitAllowed (long, bool notify = true);
	
	       long               GetLowerLimit     () const;
	       void               SetLowerLimit     (const long, bool notify = true);

		   void               SetUpperLimitAllowedEnabled (bool b);
		   bool               GetUpperLimitAllowedEnabled () const;

		   void               SetIncrementValue (const long increment);
           long               GetIncrementValue () const;
		
protected:

	virtual long              GetDesiredThumbSize          (long range) const;
	virtual long              GetDesiredThumbPosition      (long range) const;
	virtual void              IncrementValue               (bool page, bool upDown, bool notify = true);
	virtual void              UpdateValueFromThumbPosition (long position, long range, bool notify = true);

	virtual bool              GetDesiredDisabledTrackPosition (long range, long & pos) const;

private:
	
	enum ButtonType
	{
		BT_start,
		BT_end,
		BT_thumb,
		BT_track,
		BT_overlay,
		BT_numButtonTypes
	};

	                           UISliderbar      (const UISliderbar &);
	UISliderbar               &operator =       (const UISliderbar &);
	
	void                      UpdateFromStyle          ();
	void                      UpdateThumbAndOverlay    () const;
	
	long                      mLowerLimit;
	long                      mUpperLimit;
	long                      mUpperLimitAllowed;
	long                      mValue;
	bool                      mUpperLimitAllowedEnabled;
	long                      mIncrementValue;
};

//----------------------------------------------------------------------

inline long               UISliderbar::GetValue          () const 
{ 
	return mValue; 
};

//----------------------------------------------------------------------

inline long               UISliderbar::GetUpperLimit     () const 
{ 
	return mUpperLimit; 
};

//----------------------------------------------------------------------

inline long               UISliderbar::GetLowerLimit     () const 
{ 
	return mLowerLimit; 
};

//----------------------------------------------------------------------

inline long UISliderbar::GetUpperLimitAllowed () const
{
	return mUpperLimitAllowed;
}

//----------------------------------------------------------------------

inline bool UISliderbar::GetUpperLimitAllowedEnabled () const
{
	return mUpperLimitAllowedEnabled;
}

//----------------------------------------------------------------------

inline long UISliderbar::GetIncrementValue() const
{
	return mIncrementValue;
}

//----------------------------------------------------------------------


#endif // __UISLIDERBAR_H__