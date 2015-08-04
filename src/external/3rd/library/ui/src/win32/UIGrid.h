#ifndef __UIGRID_H__
#define __UIGRID_H__

#include "UIWidget.h"
#include "UINotification.h"
#include "UITemplateCache.h"

#include <set>

class UIGridStyle;
class UIDataSource;

typedef std::set<UITemplate *>  UITemplateSet;

class UIGrid : public UIWidget, public UINotification
{
public:

	class PropertyName
	{
	public:

		static const UILowerString CellCount;
		static const UILowerString CellSize;
		static const UILowerString DataSource;
		static const UILowerString SelectedItem;
		static const UILowerString Style;
	};

	static const char     *TypeName;

	                       UIGrid();
	virtual               ~UIGrid();

	virtual  void          Notify( UINotificationServer *, UIBaseObject *, UINotification::Code );

	virtual bool           IsA( const UITypeID ) const;
	virtual const char    *GetTypeName( void ) const;
	virtual UIBaseObject  *Clone( void ) const;

	virtual void           GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void           GetPropertyNames( UIPropertyNameVector &, bool forCopy) const;
	virtual void           GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool           SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool           GetProperty( const UILowerString & Name, UIString &Value ) const;

	virtual bool           ProcessMessage( const UIMessage & );
	virtual void           Render( UICanvas & ) const;

	virtual UIWidget      *GetWidgetFromPoint( const UIPoint &, bool mustGetInput) const;

	void                   SetStyle( UIGridStyle * );
	        UIGridStyle   *GetGridStyle( void ) const { return mStyle; };
	virtual UIStyle       *GetStyle( void ) const;

	        void           SetDataSource( UIDataSource * );

private:

		                   UIGrid( UIGrid & );
	UIGrid                &operator = ( UIGrid & );

	void                   TranslateTemplateInstanceToCell( UIWidgetVector *theTemplateInstance, const UIPoint &CellLocation ) const;

	UIGridStyle           *mStyle;
	UIData                *mSelectedDataObject;
	UIDataSource          *mDataSource;

	UISize                 mCellCount;
	UISize                 mCellSize;
	bool                  *mCellSelectedState;

	UITemplateSet          mAttachedTemplates;
	UITemplateCache        mTemplateCache;
};

#endif // __UIGRID_H__