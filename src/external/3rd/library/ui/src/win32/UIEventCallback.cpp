#include "_precompile.h"

#include "UIEventCallback.h"
#include "UIManager.h"
#include "UIPage.h"

#include <cassert>
#include <list>
#include <vector>

bool UIEventCallback::OnMessage( UIWidget *, const UIMessage & )
{
	return true;
}

void UIEventCallback::OnShow( UIWidget * ) {}

void UIEventCallback::OnHide( UIWidget * ) {}

void UIEventCallback::OnHoverIn( UIWidget * ) {}

void UIEventCallback::OnHoverOut( UIWidget * ) {}

void UIEventCallback::OnEnable( UIWidget * ) {}

void UIEventCallback::OnDisable( UIWidget * ) {}

void UIEventCallback::OnActivate( UIWidget * )   {}

void UIEventCallback::OnDeactivate( UIWidget * ) {}

void UIEventCallback::OnDrop( UIWidget * ) {}

void UIEventCallback::OnWidgetDataChanged (const UIWidget * , const UINarrowString & , const UIString & ) {}

void UIEventCallback::OnButtonPressed( UIWidget * ) {}

void UIEventCallback::OnCheckboxSet( UIWidget * ) {}

void UIEventCallback::OnCheckboxUnset( UIWidget * ) {}

void UIEventCallback::OnDropdownboxSelectionChanged( UIWidget * ) {}

void UIEventCallback::OnListboxSelectionChanged( UIWidget * ) {}

void UIEventCallback::OnListboxDataSourceChanged( UIWidget * ) {}

void UIEventCallback::OnListboxDoubleClicked( UIWidget * ) {}

void UIEventCallback::OnSliderbarChanged( UIWidget * ) {}

void UIEventCallback::OnSliderplaneChanged( UIWidget * ) {}

void UIEventCallback::OnTextboxChanged( UIWidget * ) {}

void UIEventCallback::OnTextboxOverflow( UIWidget * ) {}

void UIEventCallback::OnTabbedPaneChanged (UIWidget * ) {}

void UIEventCallback::OnPopupMenuSelection (UIWidget * ) {}

void UIEventCallback::OnVolumePageSelectionChanged (UIWidget * ) {}
 
void UIEventCallback::OnWidgetRectChanging (UIWidget * , UIRect & ) {}

void UIEventCallback::OnWidgetRectChanged  (UIWidget * ) {}

void UIEventCallback::OnGenericSelectionChanged (UIWidget *) {}

void UIEventCallback::OnRunScript (UIWidget *) {}

void UIEventCallback::OnSizeChanged(UIWidget *) {}

void UIEventCallback::OnTreeRowExpansionToggled(UIWidget * , int ) {}

//----------------------------------------------------------------------

UIEventCallback::~UIEventCallback () 
{
	// RLS TODO: Find out why this is so slow when enabled.

#ifndef __SLOWLY_CHECK_DANGLING_CALLBACKS
	#if PRODUCTION == 0
		#define __SLOWLY_CHECK_DANGLING_CALLBACKS 0
	#else
		#define __SLOWLY_CHECK_DANGLING_CALLBACKS 0
	#endif
#endif

#if __SLOWLY_CHECK_DANGLING_CALLBACKS
	
	typedef std::vector<UIBaseObject *> ObjectVector;
	ObjectVector objects;
	objects.reserve (100);
	
	UIPage * const root = UIManager::gUIManager ().GetRootPage ();
	
	if (root)
	{
		objects.push_back (root);
		
		while (!objects.empty())
		{
			UIBaseObject * const obj = objects.back ();
			objects.pop_back ();
			
			if (obj->IsA (TUIWidget))
			{
				UIBaseObject::UIObjectList olist;
				obj->GetChildren (olist);
				
				objects.reserve (objects.size () + olist.size ());
				for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end (); ++it)
				{
					UIBaseObject * const child = *it;
					if (child->IsA (TUIWidget))
						objects.push_back (child);
				}
				
				if (obj->IsA (TUIWidget))
				{
					UIWidget * const widget = static_cast<UIWidget *>(obj);
					if (widget->HasCallback (this))
					{
						assert (false);
					}
				}
			}
		}
	}
	
#endif
}

//----------------------------------------------------------------------
