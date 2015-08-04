#ifndef INCLUDED_ObjectPropertiesEditor_H
#define INCLUDED_ObjectPropertiesEditor_H

class UIBaseObject;
class ObjectEditor;

#include "UIPropertyCategories.h"
#include "UIPropertyDescriptor.h"
#include "UIBaseObject.h"
#include "EditorMonitor.h"

#include <map>
#include <vector>

class DynamicUIPropertyGroup;

class ObjectPropertiesEditor : public EditorMonitor
{
public:

	// ==========================================================================

	ObjectPropertiesEditor(ObjectEditor &i_editor);
	virtual void destroy()=0;

	virtual void setAcceleratorTable(HACCEL x)=0;
	virtual void saveUserPreferences()=0;
	virtual void setActiveAppearance(bool i_showActive)=0;

	// --------------------------------------------------------------------------
   // lock the dialog to temporarily prevent redraws during periods where numerous 
	// events will be generated.
   virtual void lock(); 

	// --------------------------------------------------------------------------
   // unlock the dialog to allow redraws. lock and unlock are reference-count based 
	// to each call to lock must have a corresponding call to unlock.
   virtual void unlock();

	bool isLocked() const { return m_lockRef>0; }

	virtual void onEditReset();
	virtual void onEditInsertSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling);
	virtual void onEditRemoveSubtree(UIBaseObject &subTree);
	virtual void onEditMoveSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling, UIBaseObject *oldParent);
	virtual void onEditSetObjectProperty(UIBaseObject &object, const char *i_propertyName);
	virtual void onSelect(UIBaseObject &object, bool isSelected);

	// ==========================================================================

public:

	virtual ~ObjectPropertiesEditor();

	// ==========================================================================

	struct pUIPropertyGroupSort
	{
		bool operator()(const UIPropertyGroup *p1, const UIPropertyGroup *p2) const 
		{
			if      (p1->m_groupLevelMajor < p2->m_groupLevelMajor) { return true;  }
			else if (p1->m_groupLevelMajor > p2->m_groupLevelMajor) { return false; }
			else if (p1->m_groupLevelMinor < p2->m_groupLevelMinor) { return true;  }
			else if (p1->m_groupLevelMinor > p2->m_groupLevelMinor) { return false; }
			else
			{
				return strcmp(p1->m_name, p2->m_name)<0;
			}
		}
	};

	// ==========================================================================

	class PropertyCategory;

	typedef UIBaseObject::UIObjectVector         UIObjectVector;

	// ==========================================================================

	class PropertyList
	{
	public:
		PropertyList(const UIPropertyGroup &i_propertyGroup, PropertyCategory &owner);
		virtual ~PropertyList();

		// ----------------------------------------

		bool empty() const { return m_objects.empty(); }

		// ----------------------------------------

		const char *getGroupName()  const { return m_propertyGroup.m_name; }
		int         getPropertyCount() const { return m_propertyGroup.fieldCount; }

		const UIObjectVector &getObjects() const { return m_objects; }

		// ----------------------------------------

		virtual void clear()=0;
		virtual void onPropertyGroupChanged()=0;
		virtual void addObject(UIBaseObject &o)=0;
		virtual bool removeObject(UIBaseObject &o)=0;
		virtual void onSetValue(UIBaseObject &o, const char *i_propertyName)=0;

	protected:

		PropertyCategory       &m_owner;
		const UIPropertyGroup  &m_propertyGroup;
		UIObjectVector          m_objects;
	};

	// ==========================================================================

	class PropertyCategory : public EditorMonitor
	{
	public:

		PropertyCategory(ObjectEditor &i_editor, UIPropertyCategories::Category i_category);
		virtual ~PropertyCategory();

		// ------------------------------------------------------

		typedef std::map<const UIPropertyGroup *, PropertyList *, pUIPropertyGroupSort> PropertyListMap;

		// ------------------------------------------------------

		ObjectEditor &getEditor()                   { return m_editor; }
		int           getSelectionCount()     const { return m_selectionCount; }
		void          getLabelText(char *o_buffer, int bufferSize) const;

		// ------------------------------------------------------

		// ------------------------------------------------------
		// Editor object-related change events are passed here.
		virtual void onEditReset();
		virtual void onEditRemoveSubtree(UIBaseObject &subTree);
		virtual void onEditSetObjectProperty(UIBaseObject &object, const char *i_propertyName);
		virtual void onSelect(UIBaseObject &object, bool isSelected);
		// ------------------------------------------------------

	protected:

		virtual PropertyList *_newPropertyList(
			const UIPropertyGroup &propertyGroup, 
			const PropertyListMap::iterator &insertionPoint
		)=0; 

		virtual void _freePropertyList(
			PropertyList *pl,
			const PropertyListMap::iterator &listIter
		)=0;

		PropertyList *_createUserPropertyList();
		void          _destroyUserPropertyList();
		PropertyList *_getUserPropertyList();

		void _onRemoveAll();
		void _onSelect(UIBaseObject &obj);
		void _onSelectUser(UIBaseObject &obj);
		void _onDeselect(UIBaseObject &obj, bool recurse);
		void _onDeselectUser(UIBaseObject &obj, bool recurse);
		void _r_removeFromList(UIBaseObject &obj, PropertyList &pl);
		void _onSetValue(UIBaseObject &obj, const char *i_propertyName);


		// ------------------------------------------------------

		ObjectEditor                   &m_editor;
		UIPropertyCategories::Category  m_category;
		PropertyListMap                 m_propertyLists; // mapping from UIPropertyGroup to our associated property list class.
		int                             m_selectionCount;
		DynamicUIPropertyGroup         *m_userGroup; // this is only valid if m_category is C_UserDefined
	};

	// ==========================================================================

protected:

	// ==========================================================================

	void              _construct();

	virtual PropertyCategory *_createCategory(int categoryIndex)=0;
	virtual void              _lock()=0;
	virtual void              _unlock()=0;

	// ---------------------------------------------

	ObjectEditor &m_editor;
	int m_selectionCount;
   int m_lockRef;

	PropertyCategory *m_propertyCategories[UIPropertyCategories::C_NUM_CATEGORIES];

	// ==========================================================================

};

#endif
