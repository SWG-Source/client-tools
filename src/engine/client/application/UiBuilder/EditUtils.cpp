#include "FirstUiBuilder.h"
#include "EditUtils.h"
#include "EditorMonitor.h"
#include "UIManager.h"
#include "UIPage.h"
#include "UIUtils.h"
#include "UIWidget.h"

#include <list>
#include <set>

// ==========================================================

bool EditUtils::isTransientWidget(UIBaseObject &object)
{
	return object.IsA(TUIWidget) && UI_ASOBJECT(UIWidget, &object)->IsTransient();
}

// ==========================================================

bool EditUtils::isDescendant(const UIBaseObject &obj, const UIBaseObject &testParent)
{
	const UIBaseObject *iter=&obj;
	do
	{
		const UIBaseObject *const parent=iter->GetParent();
		if (!parent)
		{
			return false;
		}
		if (parent==&testParent)
		{
			return true;
		}
		iter=parent;
	} while (true);
}

// ==========================================================

void EditUtils::getChildren(UIObjectList &o_children, UIBaseObject &object)
{
	UIObjectList children;
	object.GetChildren(children);
	for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
	{
		UIBaseObject *o=*ci;
		if (!isTransientWidget(*o))
		{
			o_children.push_back(o);
		}
	}
}

// ==========================================================

UIBaseObject *EditUtils::getTopLevelObject(UIBaseObject *o)
{
	const UIBaseObject *const root = UIManager::gUIManager().GetRootPage();
	UIBaseObject *iter;
	UIBaseObject *parent=o;
	do
	{
		iter = parent;
		parent = iter->GetParent();
	} while (parent && parent!=root);

	return iter;
}

// ==========================================================

void EditUtils::getSubTree(UIBaseObject &obj, UIObjectList &o_list)
{
	o_list.push_back(&obj);

	UIObjectList children;

	EditUtils::getChildren(children, obj);
	for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
	{
		getSubTree(*(*ci), o_list);
	}
}

// ==========================================================

void EditUtils::getSubTree(UIBaseObject &obj, UIObjectVector &o_list)
{
	o_list.push_back(&obj);

	UIObjectList children;
	EditUtils::getChildren(children, obj);
	for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
	{
		getSubTree(*(*ci), o_list);
	}
}

// ==========================================================

int EditUtils::getChildIndex(UIBaseObject &object, UIBaseObject *ignoreObject)
{
	UIBaseObject *parent = object.GetParent();
	if (!parent)
	{
		return -1;
	}
	UIObjectList children;
	parent->GetChildren(children);
	int index=0;
	for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
	{
		UIBaseObject *c=*ci;
		if (EditUtils::isTransientWidget(*c))
		{
			continue;
		}
		if (c==ignoreObject)
		{
			continue;
		}

		if (c==&object)
		{
			return index;
		}

		index++;
	}
	return -1;
}

// ==========================================================

void EditUtils::getIndexPath(IndexPath &o_path, UIBaseObject &object, UIBaseObject *ignoreObject)
{
	UIBaseObject *parent = object.GetParent();
	if (!parent)
	{
		return;
	}
	getIndexPath(o_path, *parent, ignoreObject);

	int index = getChildIndex(object, ignoreObject); 
	assert(index>=0);
	o_path.push_back(index);
}

// ==========================================================

bool EditUtils::insertChild(UIBaseObject &object, UIBaseObject &parent, int childIndex, UIBaseObject *&o_previousSibling)
{
	UIObjectList children;
	parent.GetChildren(children);

	UIObjectList::iterator ci;
	for (ci=children.begin();ci!=children.end();)
	{
		UIBaseObject *c=*ci;
		c->Attach(0);
		if (!parent.RemoveChild(c))
		{
			c->Detach(0);
			ci = children.erase(ci);
		}
		else
		{
			++ci;
		}
	}

	bool added=false;
	bool success=false;
	int index=0;
	o_previousSibling = EM_INSERT_FIRST;
	for (ci=children.begin();ci!=children.end();++ci, ++index)
	{
		if (childIndex==index)
		{
			added=true;
			success = parent.AddChild(&object);
			index++;
		}

		UIBaseObject *c=*ci;
		parent.AddChild(c);
		c->Detach(0);

		if (!added)
		{
			o_previousSibling = c;
		}
	}

	if (!added)
	{
		added=true;
		success = parent.AddChild(&object);
		o_previousSibling = EM_INSERT_LAST;
	}

	return success;
}

// ==========================================================

UIBaseObject *EditUtils::getChild(int childIndex, UIBaseObject &parent)
{
	UIObjectList children;
	parent.GetChildren(children);
	int index=0;
	for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
	{
		UIBaseObject *c=*ci;
		if (EditUtils::isTransientWidget(*c))
		{
			continue;
		}
		if (index==childIndex)
		{
			return c;
		}
		index++;
	}
	return 0;
}

// ==========================================================

UIBaseObject *EditUtils::getObjectFromPath(
	const IndexPath::const_iterator &i_start, 
	const IndexPath::const_iterator &i_stop,
	UIBaseObject *root
)
{
	IndexPath::const_iterator pi;

	UIBaseObject *returnValue = (root) ? root : UIManager::gUIManager().GetRootPage();
	 
	for (pi=i_start;returnValue && pi!=i_stop;++pi)
	{
		const int childIndex = *pi;
		returnValue = getChild(childIndex, *returnValue);
	}

	return returnValue;
}

// ==========================================================

int EditUtils::getChildCount(UIBaseObject &parent, UIBaseObject *ignoreObject)
{
	int returnValue=0;
	UIObjectList children;
	parent.GetChildren(children);
	for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
	{
		UIBaseObject *o=*ci;
		if (  o!=ignoreObject
			&& !EditUtils::isTransientWidget(*o)
			)
		{
			returnValue++;
		}
	}
	return returnValue;
}

// ==========================================================

UITypeID EditUtils::getParenthoodType(UIBaseObject &parent)
{
	if (parent.IsA(TUIComboBox))
	{
		//* ComboBox - ComboBox is a type of Page and directly calls Page::AddChild.  However, any Widgets are automatically
		//marked as "Transient".  This is just dodgy IMO. 
		//	- Insertion of child widgets from within UiBuilder should be dis-allowed.
		//	- Dis-allow all insertion of children from within UiBuilder?
		return TUINumTypes; // can't be a parent.
	}
	else if (parent.IsA(TUISliderplane))
	{
		//Sliderplane - Sliderplane is a type of Widget and can accept a single Button child.  This one is even worse as it 
		//breaks the AddChild/RemoveChild semantics.  When a second button is inserted the first button is automatically detached.
		//tsk tsk.
		//	- Remove this class from the UiBuilder until it is fixed?
		//	- Allow only Button children with special handling if there is already a Button inserted.
		return TUINumTypes; // can't be a parent.
	}
	else if (parent.IsA(TUITabSet))
	{
		//TabSet - TabSet is a type of Widget that accepts only Page children.  This appears to be for implementing PropertySheets.
		return TUITabSet;
	}
	else if (parent.IsA(TUIPage))
	{
		return TUIPage;
	}
	else if (parent.IsA(TUITextStyle))
	{
		// TextStyle only accepts FontCharacter children
		return TUITextStyle;
	}
	else if (parent.IsA(TUIImageStyle))
	{
		// ImageStyle only accepts ImageFrame children
		return TUIImageStyle;
	}
	else if (parent.IsA(TUIDataSourceContainer))
	{
		// DataSourceContainer only accepts DataSourceBase children
		return TUIDataSourceContainer;
	}
	else if (parent.IsA(TUIDataSource))
	{
		// DataSource only accepts Data children
		return TUIDataSource;
	}
	else if (parent.IsA(TUINamespace))
	{
		// Namespace accepts anything but widget children
		return TUINamespace;
	}

	return TUINumTypes;
}

// ==========================================================

bool EditUtils::isAdoptionAllowed(UIBaseObject &object, UITypeID parenthoodType)
{
	if (EditUtils::isTransientWidget(object))
	{
		return false;
	}

	switch (parenthoodType)
	{
	case TUITabSet:
		return object.IsA(TUIPage);

	case TUIPage:
		return true;

	case TUITextStyle:
		return object.IsA(TUIFontCharacter);

	case TUIImageStyle:
		return object.IsA(TUIImageFrame);

	case TUIDataSourceContainer:
		return object.IsA(TUIDataSourceBase);

	case TUIDataSource:
		return object.IsA(TUIData);

	case TUINamespace:
		return !object.IsA(TUIWidget);

	case TUINumTypes:
	default:
		return false;
	}
}

// ==========================================================

void EditUtils::removeDescendants(UIObjectVector &io_objects)
{
	const int numObjects = io_objects.size();
	UIBaseObject **origList = new UIBaseObject *[numObjects];
	memcpy(origList, &io_objects[0], numObjects*sizeof(*origList));

	int i;

	std::set<UIBaseObject *> objects;
	for (i=0;i<numObjects;i++)
	{
		objects.insert(origList[i]);
	}


	io_objects.clear();
	for (i=0;i<numObjects;i++)
	{
		UIBaseObject *const o=origList[i];

		// ------------------------------
		for (UIBaseObject *p = o->GetParent();p;p=p->GetParent())
		{
			if (objects.find(p)!=objects.end()) // this object has a parent in the list
			{
				goto skip;
			}
		}

		io_objects.push_back(o); // this object had no parents in the list.
		// ------------------------------

		skip:;
	}
}

