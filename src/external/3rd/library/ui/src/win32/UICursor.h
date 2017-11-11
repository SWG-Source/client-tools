#ifndef __UICURSOR_H__
#define __UICURSOR_H__

#include "UIBaseObject.h"

class UICanvas;
class UIImageStyle;

class UICursor : public UIBaseObject
{
public:

	class PropertyName
	{	//lint !e578 symbol hides symbol
	public:
		static const UILowerString ImageStyle;
		static const UILowerString HotSpot;
	};

	static const char		  *TypeName; //lint !e1516 // data member hides inherited member

										     UICursor();
	virtual		    		    ~UICursor();

	virtual bool				   IsA( const UITypeID ) const;
	virtual const char    *GetTypeName( void ) const;
	virtual UIBaseObject	*Clone( void ) const;

	virtual void               GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const;
	virtual void				   GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void               GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool				   SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool				   GetProperty( const UILowerString & Name, UIString &Value ) const;

					void					 Render( UICanvas &, const UIPoint & ) const;
					void					 SetImageStyle( UIImageStyle * );

					UIPoint				 GetHotSpot( void ) const { return mHotSpot; };

					UISize				 GetSize( void ) const;
					long					 GetWidth( void ) const;
					long					 GetHeight( void ) const;

	UIImageStyle * GetImageStyle ();

private:

	UIImageStyle					*mImage;
	UIPoint								 mHotSpot;
};

//----------------------------------------------------------------------

inline UIImageStyle * UICursor::GetImageStyle ()
{
	return mImage;
}

//----------------------------------------------------------------------

#endif // __UICURSOR_H__
