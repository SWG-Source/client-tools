#ifndef __UITEMPLATE_H__
#define __UITEMPLATE_H__

#include "UIBaseObject.h"
#include "UILoaderToken.h"
#include "UINotification.h"

#include <list>
#include <vector>

class UILoader;
class UIWidget;

//----------------------------------------------------------------------

class UITemplate :
public UIBaseObject,
 public UINotificationServer
{
public:

	typedef std::list<UILoaderToken> UILoaderTokenList;
	typedef std::vector<UIWidget *>  UIWidgetVector;

	static       char      TokenDelimitor;
	static const char			*TypeName; //lint !e1516 // data m ember hides inherited member

	class PropertyName
	{ //lint !e578 // data m ember hides inherited member
	public:

		static const UILowerString Size;
		static const UILowerString Template;
	};

												 UITemplate();
	virtual								~UITemplate();

	virtual bool					 IsA( const UITypeID ) const;
	virtual const char		*GetTypeName( void ) const;
	virtual UIBaseObject	*Clone( void ) const;

	virtual void                GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void					 GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void                GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool					 SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool					 GetProperty( const UILowerString & Name, UIString &Value ) const;

					void					 AddToken( const UILoaderToken & );
					void           GetTokens( UILoaderTokenList & ) const;

					void					 SetSize( const UISize & );
					void					 GetSize( UISize &Out ) const { Out = mSize; };
					UISize				 GetSize( void ) const { return mSize; };
					UIScalar       GetWidth( void ) const { return mSize.x; };
					UIScalar       GetHeight( void ) const { return mSize.y; };

					void					 GetTemplate( UIString & ) const;
					void					 SetTemplate( const UIString & );

					void					 Instanciate( UILoader &, const UIBaseObject &, UIWidgetVector * = 0 ) const;

private:

					void					 FillOutTemplate( const UIBaseObject &, const UIString &, UIString & ) const;

	UISize								 mSize;
	UILoaderTokenList			 mTemplateTokens;
};

#endif // __UITEMPLATE_H__
