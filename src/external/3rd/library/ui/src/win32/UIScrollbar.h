#ifndef __UISCROLLBAR_H__
#define __UISCROLLBAR_H__

#include "UISliderbase.h"

//----------------------------------------------------------------------

class UIScrollbar :
public UISliderbase
{
public:

	static const char * const TypeName;

	class PropertyName
	{
	public:

		static const UILowerString AutoVisibility;
		static const UILowerString Control;
	};

	                          UIScrollbar        ();
	virtual                  ~UIScrollbar        ();

	virtual bool              IsA                (const UITypeID) const;
	virtual const char       *GetTypeName        () const;
	virtual UIBaseObject     *Clone              () const;

	virtual void              GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void              GetPropertyNames   (UIPropertyNameVector &, bool forCopy ) const;
	virtual void              GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool              SetProperty        (const UILowerString & Name, const UIString &Value);
	virtual bool              GetProperty        (const UILowerString & Name, UIString &Value) const;

	virtual bool              CanSelect          () const;

	void                      AttachToControl    (UIWidget *);
	UIWidget *                GetAttachedControl ();

protected:

	virtual long              GetDesiredThumbSize          (long range) const;
	virtual long              GetDesiredThumbPosition      (long range) const;
	virtual void              IncrementValue               (bool page, bool upDown, bool notify = true);
	virtual void              UpdateValueFromThumbPosition (long position, long range, bool notify = true);
	virtual void              UpdatePerFrame               ();

private:
	                          UIScrollbar        (const UIScrollbar &);
	UIScrollbar &             operator =        (const UIScrollbar &);

	UIWidget                 *mAttachedControl;


	enum State
	{
		BF_AutoVisibility = 0x0001
	};

	unsigned char              mState;
};

//----------------------------------------------------------------------

inline UIWidget * UIScrollbar::GetAttachedControl ()
{
	return mAttachedControl;
}

//----------------------------------------------------------------------

#endif // __UISCROLLBAR_H__