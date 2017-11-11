#ifndef __UITABSET_H__
#define __UITABSET_H__

#include "UIWidget.h"

#include <list>

class UITabSetStyle;
class UITemplate;
class UIPage;

typedef std::list<UIPage *> UIPageList;

class UITabSet : public UIWidget
{
public:

	static const char			 *TypeName;

	class PropertyName
	{
	public:
		static const UILowerString Style;
		static const UILowerString Template;
	};

													UITabSet( void );
	virtual								 ~UITabSet( void );

	virtual bool						IsA( const UITypeID ) const;
	virtual const char		 *GetTypeName( void ) const;
	virtual UIBaseObject	 *Clone( void ) const;

	virtual void                  GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void						GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void                  GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool						SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool						GetProperty( const UILowerString & Name, UIString &Value ) const;

	virtual bool						ProcessMessage( const UIMessage & );
	virtual void						Render( UICanvas & ) const;

	virtual bool						AddChild( UIBaseObject * );
	virtual UIBaseObject	 *GetChild( const char *ChildName ) const;
	virtual bool						RemoveChild( UIBaseObject * );
	virtual void						SelectChild( UIBaseObject * );
	virtual void						GetChildren( UIObjectList & ) const;
	virtual unsigned long		GetChildCount( void ) const;
	virtual bool						CanChildMove( UIBaseObject *, ChildMovementDirection );
	virtual bool						MoveChild( UIBaseObject *, ChildMovementDirection );
	virtual UIWidget			 *GetWidgetFromPoint( const UIPoint & ) const;

					void            SetTemplate( UITemplate * );
					void            SetStyle( UITabSetStyle * );
					UITabSetStyle	 *GetTabSetStyle( void ) const { return mStyle; };
	virtual	UIStyle				 *GetStyle( void ) const;

private:

													UITabSet( UITabSet & );
	UITabSet							 &operator = ( UITabSet & );

	UITabSetStyle					 *mStyle;
	UITemplate						 *mTemplate;
	UIPageList						  mPages;
	UIPage								 *mActivePage;
};

#endif // __UITABSET_H__