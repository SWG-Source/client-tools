#ifndef __UIDROPDOWNBOX_H__
#define __UIDROPDOWNBOX_H__

#include "UINotification.h"
#include "UITemplateCache.h"
#include "UIWidget.h"

class UIDataSource;
class UIDropdownboxStyle;
class UITemplate;
class UIScrollbar;
class UIListbox;

class UIDropdownbox : public UIWidget, public UINotification
{
public:

	static const char            *TypeName;

	class PropertyName
	{
	public:
		static const UILowerString DataSource;
		static const UILowerString OnSelect;
		static const UILowerString SelectedItem;
		static const UILowerString SelectedItemIndex;
		static const UILowerString Style;
		static const UILowerString Template;

		static const UILowerString OpenSound;
		static const UILowerString CloseSound;
	};

	                             UIDropdownbox();
	virtual                      ~UIDropdownbox();

	virtual void                 Notify( UINotificationServer *, UIBaseObject *, UINotification::Code );

	virtual bool                 IsA( const UITypeID ) const;
	virtual const char          *GetTypeName( void ) const;
	virtual UIBaseObject        *Clone( void ) const;

	virtual void                 GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void                 GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void                 GetLinkPropertyNames (UIPropertyNameVector &) const;


	virtual bool                 SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool                 GetProperty( const UILowerString & Name, UIString &Value ) const;

	virtual void                 SetSelected( const bool );

	virtual bool                 ProcessMessage( const UIMessage & );
	virtual void                 ProcessChildNotificationMessage( UIWidget *, const UIMessage & );
	virtual void                 Render( UICanvas & ) const;
	virtual void                 MinimizeResources( void );

	        void                 SetDataSource( UIDataSource * );
	        UIDataSource        *GetDataSource( void ) const { return mDataSource; };

	        void                 SetTemplate( UITemplate * );
	        UITemplate          *GetTemplate( void ) const { return mTemplate; };

	        void                 SetStyle( UIDropdownboxStyle * );
	        UIDropdownboxStyle  *GetDropdownboxStyle( void ) const { return mStyle; };
	virtual UIStyle             *GetStyle( void ) const;

	        void                 SetSelectedItemIndex( long NewSelection );
	        long                 GetSelectedItemIndex( void ) const { return mSelection; };
	        UIData              *GetSelectedItem( void ) const;

private:

	                             UIDropdownbox( UIDropdownbox & );
	                             UIDropdownbox                &operator = ( UIDropdownbox & );

	        void                 CreatePopup( void );
	        void                 DestroyPopup( void );

	UIDropdownboxStyle          *mStyle;
	UIScrollbar                 *mScrollbar;
	UIListbox                   *mListbox;

	UIDataSource                *mDataSource;
	UITemplate                  *mTemplate;
	UITemplateCache              mTemplateCache;

	long                         mSelection;
};

#endif // __UIDROPDOWNBOX_H__