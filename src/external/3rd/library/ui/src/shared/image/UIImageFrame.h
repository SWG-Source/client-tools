//======================================================================
//
// UIImageFrame.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIImageFrame_H
#define INCLUDED_UIImageFrame_H

//======================================================================

#include "UIImageFragment.h"
//----------------------------------------------------------------------

typedef ui_stdvector<UIImageFragment *>::fwd UIImageFragmentVector;

class UIImageFrame : public UIImageFragment
{
public:

	static const char			*TypeName;//lint !e1516 // data member hides inherited data member

	class PropertyName
	{		//lint !e578 symbol hides symbol
	public:

		static const UILowerString Duration;			
	};

												 UIImageFrame();
	virtual								~UIImageFrame();

	virtual bool					 IsA( const UITypeID ) const;
	virtual const char		*GetTypeName( void ) const;
	virtual UIBaseObject	*Clone( void ) const;

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void					 GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;

	virtual bool					 SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool					 GetProperty( const UILowerString & Name, UIString &Value ) const;

	virtual bool					 AddChild( UIBaseObject * );
	virtual bool					 RemoveChild( UIBaseObject * );
	virtual void					 GetChildren( UIObjectList & ) const;
	virtual unsigned long	 GetChildCount( void ) const;
	
					UITime				 GetDuration( void ) const { return mDuration; };

					void					 Render( UICanvas &) const;
					void					 Render( UICanvas &, const UISize & size) const;

	
private:

	UIImageFrame & operator=      (const UIImageFrame &);
	               UIImageFrame   (const UIImageFrame &);

	UIImageFragmentVector	*mFragments;
	UITime								 mDuration;	
};

//======================================================================

#endif
