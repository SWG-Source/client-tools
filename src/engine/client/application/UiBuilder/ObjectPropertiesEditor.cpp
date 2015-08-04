#include "FirstUiBuilder.h"
#include "ObjectPropertiesEditor.h"
#include "EditUtils.h"
#include "ObjectEditor.h"
#include "DynamicUIPropertyGroup.h"

namespace ObjectPropertiesEditorNamespace 
{
	typedef UIBaseObject::UIPropertyGroupVector  UIPropertyGroupVector;
}
using namespace ObjectPropertiesEditorNamespace;

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

ObjectPropertiesEditor::PropertyList::PropertyList(const UIPropertyGroup &i_propertyCategory, PropertyCategory &owner)
:	  m_owner(owner)
	, m_propertyGroup(i_propertyCategory)
{
}

// ==========================================================================

ObjectPropertiesEditor::PropertyList::~PropertyList()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
// ObjectPropertiesEditor::PropertyCategory
/////////////////////////////////////////////////////////////////////////////////////////////

ObjectPropertiesEditor::PropertyCategory::PropertyCategory(ObjectEditor &i_editor, UIPropertyCategories::Category i_category)
:	 m_editor(i_editor)
	,m_category(i_category)
	,m_selectionCount(0)
	,m_userGroup(0)
{
}

// ==========================================================================

ObjectPropertiesEditor::PropertyCategory::~PropertyCategory()
{
	_destroyUserPropertyList();
}

// ==========================================================================

void ObjectPropertiesEditor::PropertyCategory::getLabelText(char *o_buffer, int bufferSize) const
{
	const char *categoryName = UIPropertyCategories::s_propertyCategoryNames[m_category].c_str();
	o_buffer[bufferSize-1]=0;
	_snprintf(o_buffer, bufferSize-1, "%s (%i)", categoryName, getSelectionCount());
}

// ==========================================================================

void ObjectPropertiesEditor::PropertyCategory::onEditReset()
{
	_onRemoveAll();
}

// ==========================================================================

void ObjectPropertiesEditor::PropertyCategory::onEditRemoveSubtree(UIBaseObject &subTree)
{
	_onDeselect(subTree, true);
}

// ==========================================================================

void ObjectPropertiesEditor::PropertyCategory::onEditSetObjectProperty(UIBaseObject &object, const char *i_propertyName)
{
	_onSetValue(object, i_propertyName);
}

// ==========================================================================

void ObjectPropertiesEditor::PropertyCategory::onSelect(UIBaseObject &object, bool isSelected)
{
	if (isSelected)
	{
		if (m_category!=UIPropertyCategories::C_UserDefined)
		{
			_onSelect(object);
		}
		else
		{
			_onSelectUser(object);
		}
	}
	else
	{
		if (m_category!=UIPropertyCategories::C_UserDefined)
		{
			_onDeselect(object, false);
		}
		else
		{
			_onDeselectUser(object, false);
		}
	}
}

// ==========================================================================

ObjectPropertiesEditor::PropertyList *ObjectPropertiesEditor::PropertyCategory::_createUserPropertyList()
{
	assert(!m_userGroup);
	m_userGroup = new DynamicUIPropertyGroup;
	return _newPropertyList(m_userGroup->getUIPropertyGroup(), m_propertyLists.begin());
}

// ==========================================================================

void ObjectPropertiesEditor::PropertyCategory::_destroyUserPropertyList()
{
	delete m_userGroup;
	m_userGroup=0;
}

// ==========================================================================

ObjectPropertiesEditor::PropertyList *ObjectPropertiesEditor::PropertyCategory::_getUserPropertyList()
{
	if (!m_userGroup)
	{
		return _createUserPropertyList();
	}
	else
	{
		const UIPropertyGroup &pg = m_userGroup->getUIPropertyGroup();
		return m_propertyLists[&pg];
	}
}

// ==========================================================================

void ObjectPropertiesEditor::PropertyCategory::_onRemoveAll()
{
	if (m_category!=UIPropertyCategories::C_UserDefined)
	{
		while (!m_propertyLists.empty())
		{
			PropertyListMap::iterator front = m_propertyLists.begin();
			PropertyList *pl = static_cast<PropertyList *>(front->second);
			m_propertyLists.erase(front);
			delete pl;
		}
	}
	else
	{
		PropertyList *pl = _getUserPropertyList();
		pl->clear();
		m_userGroup->setObject(0);
	}

	m_selectionCount=0;
}

// ==========================================================================

void ObjectPropertiesEditor::PropertyCategory::_onSelect(UIBaseObject &obj)
{
	bool added=false;
	UIPropertyGroupVector tempPropList;

	obj.GetPropertyGroups(tempPropList, m_category);

	// for each group...
	for (UIPropertyGroupVector::iterator pgi=tempPropList.begin();pgi!=tempPropList.end();++pgi)
	{
		const UIPropertyGroup *pg = *pgi;

		// ------------------------------------------------------------------------------
		// Get the property list for this group, creating one if necessary
		PropertyList *pl=0;
		{
			PropertyListMap::iterator pli = m_propertyLists.lower_bound(pg);

			// if the iterator is at end or our group is LT the iterator, we need to add.
			if (pli==m_propertyLists.end() || m_propertyLists.key_comp()(pg, pli->first))
			{
				pl = _newPropertyList(*pg, pli);
			}
			else // the iterator points to an equivalent group (should be the same one, really).
			{
				pl = pli->second;
			}
		}
		// ------------------------------------------------------------------------------

		pl->addObject(obj);
		added=true;
	}

	if (added)
	{
		m_selectionCount++;
	}
}

// ==========================================================================

void ObjectPropertiesEditor::PropertyCategory::_onSelectUser(UIBaseObject &obj)
{
	UIBaseObject *anchor = m_editor.getAnchorSelection(); assert(anchor);
	if (!anchor)
	{
		return;
	}

	PropertyList *pl = _getUserPropertyList();

	// if the anchor has changed, we need to re-build the property group
	if (anchor!=m_userGroup->getObject())
	{
		m_userGroup->setObject(anchor);
		pl->onPropertyGroupChanged();
	}

	pl->addObject(obj);
	m_selectionCount++;
}

// ==========================================================================

void ObjectPropertiesEditor::PropertyCategory::_onDeselect(UIBaseObject &obj, bool recurse)
{
	bool removed=false;

	UIPropertyGroupVector tempPropList;

	obj.GetPropertyGroups(tempPropList, m_category);

	// for each group...
	for (UIPropertyGroupVector::iterator pgi=tempPropList.begin();pgi!=tempPropList.end();++pgi)
	{
		const UIPropertyGroup *pg = *pgi;

		// ------------------------------------------------------------------------------
		// Get the property list for this group
		PropertyListMap::iterator pli = m_propertyLists.find(pg);
		if (pli==m_propertyLists.end())
		{
			continue;
		}
		PropertyList *const pl=pli->second;
		// ------------------------------------------------------------------------------

		if (pl->removeObject(obj))
		{
			removed=true;

			// ------------------------------------------------------------------------------
			// if that was the last one of its kind, remove this property list from the dialog.
			if (pl->getObjects().empty())
			{
				_freePropertyList(pl, pli);
			}
		}
		// TODO - remove this page if no more selections?
	}

	if (removed)
	{
		m_selectionCount--;
	}

	// -----------------------------------

	if (recurse)
	{
		UIBaseObject::UIObjectList children;
		EditUtils::getChildren(children, obj);
		for (UIBaseObject::UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
		{
			UIBaseObject *c = *ci; assert(c);
			_onDeselect(*c, true);
		}
	}
}

// ==========================================================================

void ObjectPropertiesEditor::PropertyCategory::_onDeselectUser(UIBaseObject &obj, bool recurse)
{
	PropertyList *pl = _getUserPropertyList();
	_r_removeFromList(obj, *pl);

	UIBaseObject *anchor = m_editor.getAnchorSelection();

	// if the anchor has changed, we need to re-build the property group
	if (anchor!=m_userGroup->getObject())
	{
		m_userGroup->setObject(anchor);
		pl->onPropertyGroupChanged();
	}
}

// ==========================================================================

void ObjectPropertiesEditor::PropertyCategory::_r_removeFromList(UIBaseObject &obj, PropertyList &pl)
{
	if (pl.removeObject(obj))
	{
		m_selectionCount--;
	}

	UIBaseObject::UIObjectList children;
	EditUtils::getChildren(children, obj);
	for (UIBaseObject::UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
	{
		UIBaseObject *c = *ci; assert(c);
		_r_removeFromList(*c, pl);
	}
}

// ==========================================================================

void ObjectPropertiesEditor::PropertyCategory::_onSetValue(UIBaseObject &obj, const char *i_propertyName)
{
	for (PropertyListMap::iterator pli=m_propertyLists.begin();pli!=m_propertyLists.end();++pli)
	{
		PropertyList *pl = pli->second;
		pl->onSetValue(obj, i_propertyName);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////
// ObjectPropertiesEditor
/////////////////////////////////////////////////////////////////////////////////////////////

ObjectPropertiesEditor::ObjectPropertiesEditor(ObjectEditor &i_editor)
:	 m_editor(i_editor)
	,m_selectionCount(0)
	,m_lockRef(0)
{
	memset(m_propertyCategories, 0, sizeof(m_propertyCategories));
}

// =====================================================================

ObjectPropertiesEditor::~ObjectPropertiesEditor()
{
	for (int i=0;i<UIPropertyCategories::C_NUM_CATEGORIES;i++)
	{
		delete m_propertyCategories[i];
	}
}

// =====================================================================

void ObjectPropertiesEditor::_construct()
{
	for (int i=0;i<UIPropertyCategories::C_NUM_CATEGORIES;i++)
	{
		m_propertyCategories[i] = _createCategory(i);
	}
}

// =====================================================================

// lock the dialog to temporarily prevent redraws during periods where numerous events will be generated.
void ObjectPropertiesEditor::lock()
{
   if (!m_lockRef)
	{
		_lock();
	}
   m_lockRef++;
}

// =====================================================================

// unlock the dialog to allow redraws. lock and unlock are reference-count based to each call to lock
// must have a corresponding call to unlock.
void ObjectPropertiesEditor::unlock()
{
   assert(m_lockRef>0);
   m_lockRef--;

   // if lock ref has returned back to zero, re-enable redraws
   // and invalidate this window.
   if (!m_lockRef)
   {
		_unlock();
   }
}

// =====================================================================

void ObjectPropertiesEditor::onEditReset()
{
	for (int i=0;i<UIPropertyCategories::C_NUM_CATEGORIES;i++)
	{
		m_propertyCategories[i]->onEditReset();
	}
	m_selectionCount=m_editor.getSelections().size();
}

// =====================================================================

void ObjectPropertiesEditor::onEditInsertSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling)
{
	for (int i=0;i<UIPropertyCategories::C_NUM_CATEGORIES;i++)
	{
		m_propertyCategories[i]->onEditInsertSubtree(subTree, previousSibling);
	}
	m_selectionCount=m_editor.getSelections().size();
}

// =====================================================================

void ObjectPropertiesEditor::onEditRemoveSubtree(UIBaseObject &subTree)
{
	for (int i=0;i<UIPropertyCategories::C_NUM_CATEGORIES;i++)
	{
		m_propertyCategories[i]->onEditRemoveSubtree(subTree);
	}
	m_selectionCount=m_editor.getSelections().size();
}

// =====================================================================

void ObjectPropertiesEditor::onEditMoveSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling, UIBaseObject *oldParent)
{
	for (int i=0;i<UIPropertyCategories::C_NUM_CATEGORIES;i++)
	{
		m_propertyCategories[i]->onEditMoveSubtree(subTree, previousSibling, oldParent);
	}
	m_selectionCount=m_editor.getSelections().size();
}

// =====================================================================

void ObjectPropertiesEditor::onEditSetObjectProperty(UIBaseObject &object, const char *i_propertyName)
{
	for (int i=0;i<UIPropertyCategories::C_NUM_CATEGORIES;i++)
	{
		m_propertyCategories[i]->onEditSetObjectProperty(object, i_propertyName);
	}
}

// =====================================================================

void ObjectPropertiesEditor::onSelect(UIBaseObject &object, bool isSelected)
{
	for (int i=0;i<UIPropertyCategories::C_NUM_CATEGORIES;i++)
	{
		m_propertyCategories[i]->onSelect(object, isSelected);
	}
	m_selectionCount=m_editor.getSelections().size();
}

