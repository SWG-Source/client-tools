#ifndef __UI3DVIEW_H__
#define __UI3DVIEW_H__

#include "UIWidget.h"

class UI3DView : public UIWidget
{
public:

	class PropertyName
	{
	public:
		static const UILowerString ModelName;
		static const UILowerString Viewpoint;
		static const UILowerString ViewVector;
	};

	static const char		*TypeName;

											 UI3DView();
	virtual							~UI3DView();
													
	virtual bool				 IsA( const UITypeID Type ) const { return (Type == TUIWidget) || UIBaseObject::IsA( Type ); };
	virtual const char	*GetTypeName( void ) const;
													
	virtual void             GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void				 GetPropertyNames( UIPropertyNameVector & ) const;
												
	virtual bool				 SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool				 GetProperty( const UILowerString & Name, UIString &Value ) const;

	virtual void				 Render( UICanvas & ) const;

private:

											 UI3DView( UI3DView & );
	UI3DView						&operator = ( UI3DView & );
};

#endif // __UI3DVIEW_H__