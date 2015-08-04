#ifndef INCLUDED_EditorMonitor_H
#define INCLUDED_EditorMonitor_H

class UIBaseObject;

class EditorMonitor
{
public:

	// ---------------------------------------
	// Object chenge notifications
	enum {
		 OC_SETROOT
		,OC_INSERT
		,OC_REMOVE
		,OC_REMOVEALL
		,OC_HIERARCHY
		,OC_SELECT
		,OC_PROPERTY
	};

   // lock the dialog to temporarily prevent redraws during periods where numerous events will be generated.
   virtual void lock(); 

   // unlock the dialog to allow redraws. lock and unlock are reference-count based to each call to lock
   // must have a corresponding call to unlock.
   virtual void unlock();

	virtual void onEditReset();
	virtual void onEditInsertSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling);
	virtual void onEditRemoveSubtree(UIBaseObject &subTree);
	virtual void onEditMoveSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling, UIBaseObject *oldParent);
	virtual void onEditSetObjectProperty(UIBaseObject &object, const char *i_propertyName);
	virtual void onSelect(UIBaseObject &object, bool isSelected);
};

#define EM_INSERT_FIRST (UIBaseObject *)(-1)
#define EM_INSERT_LAST  (UIBaseObject *)(0)

#endif
