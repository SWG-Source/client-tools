#ifndef __UITabbedPaneStyle_H__
#define __UITabbedPaneStyle_H__

#include "UIWidgetStyle.h"

class UIImageStyle;
class UIButtonStyle;

//----------------------------------------------------------------------

class UITabbedPaneStyle : public UIWidgetStyle
{
public:

	static const char     * const TypeName;

	struct PropertyName
	{
		static const UILowerString ButtonStyle;
		static const UILowerString ButtonStyleLeft;
		static const UILowerString  ButtonStyleRight;
	};

	enum ButtonStyleType
	{
		BST_left,
		BST_middle,
		BST_right,
		BST_numTypes
	};

	                        UITabbedPaneStyle();
	virtual                ~UITabbedPaneStyle();

	virtual bool            IsA( const UITypeID ) const;
	virtual const char     *GetTypeName( void ) const;
	virtual UIBaseObject   *Clone( void ) const;

	virtual void            Link ();

	virtual void            GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void            GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void            GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool            SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool            GetProperty( const UILowerString & Name, UIString &Value ) const;

	UIButtonStyle *         GetButtonStyle (ButtonStyleType type) const;

	void                    SetButtonStyle (ButtonStyleType type, UIButtonStyle *);

private:

	UIButtonStyle          *mButtonStyle [BST_numTypes];
};

//----------------------------------------------------------------------

inline UIButtonStyle * UITabbedPaneStyle::GetButtonStyle (ButtonStyleType type) const
{
	return mButtonStyle [type];
}

//----------------------------------------------------------------------

#endif // __UITabbedPaneStyle_H__