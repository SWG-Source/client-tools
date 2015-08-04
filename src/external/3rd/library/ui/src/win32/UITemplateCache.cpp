#include "_precompile.h"

#include "UITemplateCache.h"
#include "UIBaseObject.h"
#include "UILoader.h"
#include "UITemplate.h"
#include "UIData.h"
#include "UIWidget.h"

#include <map>
#include <cassert>

//-----------------------------------------------------------------

struct UITemplateCache::DataToTemplateInstanceMap
{
	typedef std::map<UIData *, WidgetVector *> Container;
	Container c;
};

//-----------------------------------------------------------------

UITemplateCache::UITemplateCache () :
UINotification (),
UIEventCallback (),
mCache (new DataToTemplateInstanceMap),
mModifyingData (0)
{
	assert (mCache);
}

//-----------------------------------------------------------------

UITemplateCache::~UITemplateCache( void )
{
	Clear();
	mModifyingData = 0;
	delete mCache;
	mCache = 0;
}

//-----------------------------------------------------------------

void UITemplateCache::Notify( UINotificationServer *NotifyingObject, UIBaseObject *ContextObject, UINotification::Code NotificationCode )
{
	UI_UNREF (NotificationCode);
	UI_UNREF (NotifyingObject);

	if( ContextObject && ContextObject->IsA( TUIData ) )
	{
		if (ContextObject != mModifyingData)
		Remove( reinterpret_cast<UIData *>( ContextObject ) );
	}
}

//-----------------------------------------------------------------

UITemplateCache::WidgetVector *UITemplateCache::Get( const UIData *DataItem ) const
{
	DataToTemplateInstanceMap::Container::const_iterator i = mCache->c.find( const_cast<UIData *>( DataItem ) );

	if( i != mCache->c.end() )
		return i->second;
	else
		return 0;
}

//-----------------------------------------------------------------

void UITemplateCache::Add( UIData *DataItem, UITemplateCache::WidgetVector *InstanceToCache )
{
	Remove(DataItem);
	
	DataItem->Listen( this );
	DataItem->Attach(0);
	mCache->c[DataItem] = InstanceToCache;
}

void UITemplateCache::Add( UIData *DataItem, UITemplate *TemplateToInstanciate, UIBaseObject *ParentObject )
{
	UILoader  		  TemplateLoader;	
	WidgetVector *Instance = new WidgetVector;

	TemplateToInstanciate->Instanciate( TemplateLoader, *(UIBaseObject *)DataItem, Instance );

	for( WidgetVector::iterator i = Instance->begin(); i != Instance->end(); ++i )
	{
		UIWidget *o = *i;
		
		o->SetParent( ParentObject );
		o->Link();
		
		o->AddCallback (this);
		
		const UINarrowString prefix (o->GetName () + ".");
		const size_t prefix_len = prefix.length ();
		
		const UIBaseObject::UIPropertyNameMap * const props = DataItem->GetPropertyMap ();
		
		if (props)
		{
			const UIBaseObject::UIPropertyNameMap::const_iterator pend = props->end ();
			for (UIBaseObject::UIPropertyNameMap::const_iterator pit = props->begin (); pit != pend; ++pit)
			{
				const UILowerString & lname = (*pit).first;
				
				if (!_strnicmp (prefix.c_str (), lname.c_str (), prefix_len))
				{
					if (lname.c_str () [prefix_len] != 0)
					{
						const UINarrowString name (lname.c_str () + prefix_len);
						
						o->SetProperty (UILowerString (name), (*pit).second);
					}
				}
			}
		}
	}	
	
	Add( DataItem, Instance );
}

void UITemplateCache::Remove( UIData *DataItem )
{
	DataToTemplateInstanceMap::Container::iterator i = mCache->c.find( DataItem );

	if( i != mCache->c.end() )
	{
		WidgetVector *CachedInstance = i->second;

		DataItem->StopListening( this );

		for( WidgetVector::iterator CurrentObject = CachedInstance->begin(); CurrentObject != CachedInstance->end(); ++CurrentObject )
		{
			(*CurrentObject)->RemoveCallback (this);
			(*CurrentObject)->Detach(0);
		}

		delete CachedInstance;
		mCache->c.erase( i );

		DataItem->Detach(0);
	}
}

void UITemplateCache::Clear( void )
{
	while( !mCache->c.empty() )
	{
		DataToTemplateInstanceMap::Container::iterator i							 = mCache->c.begin();
		WidgetVector											 *CachedInstance = i->second;

		i->first->StopListening( this );
		i->first->Detach(0);

		for( WidgetVector::iterator CurrentObject = CachedInstance->begin(); CurrentObject != CachedInstance->end(); ++CurrentObject )
		{
			(*CurrentObject)->RemoveCallback (this);
			(*CurrentObject)->Detach(0);
		}

		delete CachedInstance;
		mCache->c.erase( mCache->c.begin() );
	}
}

//-----------------------------------------------------------------

void UITemplateCache::OnWidgetDataChanged (const UIWidget * context, const UINarrowString & property, const UIString & value)
{
	assert (context);

	//-----------------------------------------------------------------
	//-- don't process nameless widgets

	if (context->GetName ().empty ())
		return;

	UIData * dataResult = 0;

	const DataToTemplateInstanceMap::Container::iterator mend = mCache->c.end ();
	for (DataToTemplateInstanceMap::Container::iterator mit = mCache->c.begin (); dataResult == 0 && mit != mend; ++mit)
	{
		const WidgetVector * const wv = (*mit).second;
		
		assert (wv);
		
		const WidgetVector::const_iterator end = wv->end ();
		for (WidgetVector::const_iterator it = wv->begin (); it != end; ++it)
		{
			if (*it == context)
			{
				dataResult = (*mit).first;
				break;
			}
		}
	}

	if (dataResult)
	{

		const Unicode::NarrowString propname ( context->GetName () + "." + property);

		mModifyingData = dataResult;
		dataResult->SetProperty (UILowerString (propname), value);
		mModifyingData = 0;
	}
}

//-----------------------------------------------------------------
