#ifndef __UIIMAGE_H__
#define __UIIMAGE_H__

#include "UIWidget.h"

class UIImageStyle;

class UIImage : public UIWidget
{
public:

	static const char      *TypeName;

	class PropertyName
	{
	public:
		static const UILowerString SourceRect;
		static const UILowerString SourceResource;
		static const UILowerString Style;
		static const UILowerString Stretch;
	};

	UIImage();
	virtual                ~UIImage();

	virtual bool           IsA               ( const UITypeID ) const;
	virtual const char    *GetTypeName       () const;
	virtual UIBaseObject  *Clone             () const;

	virtual void           GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void           GetPropertyNames  (UIPropertyNameVector &, bool forCopy  ) const;
	virtual void           GetLinkPropertyNames (UIPropertyNameVector &) const;

	virtual void GetPropertiesInCategory(UIPropertyCategories::Category category, UIPropertyNameVector & In) const;

	virtual bool           SetProperty       (const UILowerString & Name, const UIString &Value );
	virtual bool           GetProperty       (const UILowerString & Name, UIString &Value ) const;

	virtual bool           ProcessMessage    (const UIMessage & );
	virtual void           Render            (UICanvas & ) const;

	void                   SetStretch        (bool Stretch);
	bool                   GetStretch        () const;

	UICanvas const *       GetCanvas         () const;

	void                   SetSourceRect     ( const UIRect * );
	bool                   GetSourceRect     (UIRect & rect) const;

	void                   SetStyle          ( UIImageStyle * );
	UIImageStyle  const *  GetImageStyle     () const;
	virtual  UIStyle *     GetStyle          () const;

	void                   SetCanvas         (UICanvas *NewCanvas);
	bool                   SetSourceResource (const UIString &NewResourceName);

	void                   AutoSetCanvasRect () const;

protected:

	UIImageStyle          *mStyle;

	UICanvas              *mSourceCanvas;
	mutable UIPoint        mSourcePoint;
	mutable UISize         mSourceSize;

	bool                   mStretch : 1;
	mutable bool           mSourceRectSet : 1;
	mutable bool           mCanvasRectDirty : 1;

private:
	
	UIImage( UIImage & );
	UIImage                &operator = ( UIImage & );
};

//-----------------------------------------------------------------

inline bool UIImage::GetSourceRect (UIRect & rect) const
{
	if (mSourceRectSet == false)
		return false;

	rect.left   = mSourcePoint.x;
	rect.top    = mSourcePoint.y;
	rect.right  = rect.left + mSourceSize.x;
	rect.bottom = rect.top  + mSourceSize.y;

	return true;
}

//----------------------------------------------------------------------

inline void  UIImage::SetStretch        (bool Stretch)
{
	mStretch = Stretch;
}

//----------------------------------------------------------------------

inline bool UIImage::GetStretch        () const
{
	return mStretch;
}

//----------------------------------------------------------------------

inline UICanvas const * UIImage::GetCanvas () const
{
	return mSourceCanvas;
}

//----------------------------------------------------------------------

inline UIImageStyle const *  UIImage::GetImageStyle () const
{
	return mStyle;
};

//-----------------------------------------------------------------

#endif // __UIIMAGE_H__
