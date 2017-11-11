#ifndef __UILISTBOX_H__
#define __UILISTBOX_H__

#include "UINotification.h"
#include "UITemplateCache.h"
#include "UIWidget.h"

class UIDataSource;
class UITemplate;
class UIListboxStyle;
class UIData;

class UIListbox : public UIWidget, public UINotification
{
public:

	static const char						*TypeName;
															
	class PropertyName					
	{														
	public:											
		static const UILowerString AutoScrollSpeed;
		static const UILowerString DataSource;
		static const UILowerString OnSelect;
		static const UILowerString OnDoubleClick;
		static const UILowerString SelectedItem;
		static const UILowerString SelectedItemIndex;
		static const UILowerString Style;
		static const UILowerString Template;
	};													
															
															 UIListbox();
	virtual											~UIListbox();
														
	virtual bool								 IsA( const UITypeID ) const;
	virtual const char					*GetTypeName( void ) const;
	virtual UIBaseObject				*Clone( void ) const;
	
	virtual	void								 Notify( UINotificationServer *, UIBaseObject *, UINotification::Code );

	virtual void                         GetPropertyGroups(UIPropertyGroupVector &, UIPropertyCategories::Category = UIPropertyCategories::C_ALL_CATEGORIES) const;
	virtual void								 GetPropertyNames( UIPropertyNameVector &, bool forCopy ) const;
	virtual void                         GetLinkPropertyNames( UIPropertyNameVector &In ) const;
															
	virtual bool								 SetProperty( const UILowerString & Name, const UIString &Value );
	virtual bool								 GetProperty( const UILowerString & Name, UIString &Value ) const;

	virtual void								 GetScrollExtent( UISize & ) const;
															
	virtual bool								 ProcessMessage( const UIMessage & );
	virtual void								 Render( UICanvas & ) const;
	virtual void                 MinimizeResources( void );
															
					void								 SetDataSource( UIDataSource * );
					UIDataSource				*GetDataSource( void ) const { return mDataSource; };
																				
					void								 SetTemplate( UITemplate * );
					UITemplate					*GetTemplate( void ) const { return mTemplate; };
															
					void								 SetStyle( UIListboxStyle * );
					UIListboxStyle			*GetListboxStyle( void ) const { return mStyle; };
	virtual	UIStyle							*GetStyle( void ) const;
																				
					void								 SetSelectionIndex( long );
					long								 GetSelectionIndex( void ) const { return mSelection; };					

					UIData				      *GetSelectedDataItem( void ) { return mSelectedDataObject; };
					const UIData				*GetSelectedDataItem( void ) const { return mSelectedDataObject; };

					void								 ScrollTo( long );

					UIData              *GetSelectedItem( void );

	virtual void            GetScrollSizes( UISize &PageSize, UISize &LineSize ) const;

private:

															 UIListbox( UIListbox & );
	UIListbox										&operator = ( UIListbox & );

					void								 UpdateSelectionDataItemFromIndex( void );
					void								 UpdateSelectionIndexFromDataItem( void );

	UIListboxStyle							*mStyle;
	UIDataSource								*mDataSource;
	UITemplate									*mTemplate;
	
	long												 mSelection;
	UIData                      *mSelectedDataObject;
	
	long               mAutoScroll;
	float              mAutoScrollSpeed;

	UIPoint            mLastMousePosition;
	UITemplateCache    mTemplateCache;

	mutable long       mSelectedItemsAllocatedSize;
	bool               mMouseDown;
	mutable bool       *mSelectedItems;
};


#endif // __UILISTBOX_H__