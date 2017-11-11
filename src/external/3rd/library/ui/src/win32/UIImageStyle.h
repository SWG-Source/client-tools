#ifndef __UIIMAGESTYLE_H__
#define __UIIMAGESTYLE_H__

#include "UIStyle.h"

class UIImageFrame;
class UiMemoryBlockManager;
class UIImageFragment;

//----------------------------------------------------------------------

typedef ui_stdlist<UIImageFrame *>::fwd UIImageFrameList;

//----------------------------------------------------------------------

class UIImageStyle : public UIStyle
{
public:
	
	static const char * const TypeName;//lint !e1516 // data member hides inherited data member
	
	class PropertyName
	{    //lint !e578 symbol hides symbol
	public:
		
		static const UILowerString Loop;
		static const UILowerString Reversing;
		static const UILowerString PlayTime;
		static const UILowerString OpacityRelativeMin;
	};
	
	                          UIImageStyle ();
	                         ~UIImageStyle    ();
	
	bool                      IsA              (const UITypeID) const;
	const char *              GetTypeName      () const;
	UIBaseObject *            Clone            () const;
	
	bool                      AddChild         (UIBaseObject * );
	bool                      RemoveChild      (UIBaseObject * );
	UIBaseObject *            GetChild         (const char * ) const;
	void                      GetChildren      (UIObjectList & ) const;
	unsigned long             GetChildCount    () const;
	
	bool                      CanChildMove     (UIBaseObject *, ChildMovementDirection );
	bool                      MoveChild        (UIBaseObject *, ChildMovementDirection );
	
	virtual void              GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	void                      GetPropertyNames     (UIPropertyNameVector &, bool forCopy  ) const;
	void                      CopyPropertiesFrom   (const UIBaseObject & rhs);

	bool                      SetProperty      (const UILowerString & Name, const UIString &Value );
	bool                      GetProperty      (const UILowerString & Name, UIString &Value ) const;
	
	void                      Render           (UITime, UICanvas &, const UIPoint &, const UISize & size ) const;
	void                      Render           (UITime, UICanvas &, const UIPoint &) const;
	void                      Render           (UIImageFrame & SourceFrame, UICanvas &DestinationCanvas, const UIPoint &Location, const UISize & size ) const;
	void                      Render           (UIImageFrame & SourceFrame, UICanvas &DestinationCanvas, const UIPoint &Location) const;
	
	const UISize &            GetSize          () const;
	long                      GetWidth         () const;
	long                      GetHeight        () const;      
	
	float                     GetPlayTime      () const { return mPlayTime; };
	void                      SetPlayTime      (const float NewPlayTime ) { mPlayTime = NewPlayTime; };
	
	bool                      GetFrame         (UITime, UIImageFrame *& ) const;
	
	const UICanvas  *         GetSourceCanvas  () const;
	const UIRect              GetSourceRect    () const;

	const UIImageFragment *   GetFragmentSelf  () const;

	float                     GetOpacityRelativeMin () const;

private:
	
	UIImageStyle & operator=      (const UIImageStyle &);
	               UIImageStyle   (const UIImageStyle &);
	
	
	float                     mPlayTime;
	UIImageFrameList *        mFrames;
	UIImageFragment *         mFragmentSelf;
	float                     mOpacityRelativeMin;
	bool                      mReversing : 1;
	bool                      mLooping : 1;
};

//----------------------------------------------------------------------

inline const UIImageFragment * UIImageStyle::GetFragmentSelf  () const
{
	return mFragmentSelf;
}

//----------------------------------------------------------------------

inline float UIImageStyle::GetOpacityRelativeMin () const
{
	return mOpacityRelativeMin;

}

//-----------------------------------------------------------------

#endif // __UIIMAGESTYLE_H__
