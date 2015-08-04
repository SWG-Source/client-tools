// ======================================================================
//
// UIUndo.h
//
// ======================================================================

#ifndef INCLUDED_UIUndo_H
#define INCLUDED_UIUndo_H

// ======================================================================

#include "UILowerString.h"
#include "UISmartPointer.h"
#include "UIString.h"

#include <string>

// ======================================================================

class UIBaseObject;

// ======================================================================

class UIUndo
{
public:
	explicit UIUndo(UIBaseObject * const object);
	UIUndo(UIBaseObject * const object, UILowerString propertyName, UIString oldValue, UIString newValue);
	~UIUndo();

	UIUndo(const UIUndo & rhs);
	UIUndo & operator=(const UIUndo & rhs);

	bool undo(bool &rebuildTree, UIBaseObject **selectedObject); // return true to continue undoing.

	UIBaseObject * getObject();
	UIBaseObject * getSourceObject();

	void replaceSource(UIBaseObject * newSource);

	UILowerString const &getPropertyName() const;
	UIString const &getOldValue() const;

private:
	UIUndo();

	//////////////////////////////////////////////////////////////////////////
	// DO NOT USE mSourceObject.  It is essentially a key.
	UIBaseObject * mSourceObject;
	//////////////////////////////////////////////////////////////////////////


	UISmartPointer<UIBaseObject> mObject;

	std::string mParentPath;
	std::string mChildPath;
	std::string mSiblingPath;

	UILowerString mPropertyName;
	UIString mOldValue;
	UIString mNewValue;
};

// ======================================================================

#endif
