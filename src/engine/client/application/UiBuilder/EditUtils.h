#ifndef INCLUDED_EditUtils_H
#define INCLUDED_EditUtils_H

#include <UIBaseObject.h>
#include <vector>

class EditUtils
{
public:

	typedef UIBaseObject::UIObjectList        UIObjectList;
	typedef UIBaseObject::UIObjectVector      UIObjectVector;

	typedef std::vector<int>                  IndexPath;

	// ---------------------------------------------------------------------------
	// Test to see if an object is both a UIWidget and has the Transient property.
	static bool isTransientWidget(UIBaseObject &object);
	// ---------------------------------------------------------------------------

	// ---------------------------------------------------------------------------
	// Test to see if an object is a descendant of another object.
	static bool isDescendant(const UIBaseObject &obj, const UIBaseObject &testParent);
	// ---------------------------------------------------------------------------

	// ---------------------------------------------------------------------------
	// Returns a list of the objects children EXCEPT transient widgets (or other
	// non-editable objects).  Should be used instead of UIBaseObject::GetChildren)
	static void getChildren(UIObjectList &o_children, UIBaseObject &object);
	// ---------------------------------------------------------------------------

	static UIBaseObject *getTopLevelObject(UIBaseObject *descendent);

	static void          getSubTree(UIBaseObject &obj, UIObjectList &o_list);
	static void          getSubTree(UIBaseObject &obj, UIObjectVector &o_list);
	static int           getChildIndex(UIBaseObject &object, UIBaseObject *ignoreObject=0);
	static void          getIndexPath(IndexPath &o_path, UIBaseObject &object, UIBaseObject *ignoreObject=0);
	static bool          insertChild(UIBaseObject &object, UIBaseObject &parent, int childIndex, UIBaseObject *&o_previousSibling);
	static UIBaseObject *getChild(int childIndex, UIBaseObject &parent); 
	static UIBaseObject *getObjectFromPath(
		const IndexPath::const_iterator &i_start, 
		const IndexPath::const_iterator &i_stop,
		UIBaseObject *root=0
	);
	static int          getChildCount(UIBaseObject &parent, UIBaseObject *ignoreObject=0);

	static UITypeID getParenthoodType(UIBaseObject &parent);
	static bool     isAdoptionAllowed(UIBaseObject &object, UITypeID parenthoodType);

	// ---------------------------------------------------------------------------
	// returns a list of objects, none of which are descendants of other objects in the list.
	static void removeDescendants(UIObjectVector &io_objects);
	// ---------------------------------------------------------------------------
};

#endif
