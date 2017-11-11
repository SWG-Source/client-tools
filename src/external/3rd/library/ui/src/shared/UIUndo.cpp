// ======================================================================
//
// UIUndo.cpp
//
// ======================================================================
#include "_precompile.h"
#include "UIUndo.h"

#include "UIBaseObject.h"
#include "UIManager.h"
#include "UIPage.h"
#include "UIUtils.h"

#include <list>

//-----------------------------------------------------------------

namespace UIUndoNamespace
{
	std::string const s_rootPath("/");
}

using namespace UIUndoNamespace;

//-----------------------------------------------------------------

UIUndo::UIUndo(UIBaseObject * const object) :
mSourceObject(object),
mObject(object->DuplicateObject()),
mParentPath(object->GetParent() ? object->GetParent()->GetFullPath() : s_rootPath),
mChildPath(object->GetFullPath()),
mSiblingPath(),
mPropertyName(),
mOldValue(),
mNewValue()
{
	// Not necessary, but someone may change the logic and introduce an annoying bug.
	mObject->RemoveFromParent(); 

	// Get sibling path.
	UIPage * const parentPage = UI_ASOBJECT(UIPage, object->GetParent());
	if (parentPage) 
	{
		UIBaseObject * siblingObject = parentPage->GetPreviousChild(object);
		if (!siblingObject) 
		{
			siblingObject = parentPage->GetNextChild(object);
		}
		
		if (siblingObject) 
		{
			mSiblingPath = siblingObject->GetFullPath();
		}
	}
}

UIUndo::UIUndo(UIBaseObject * const object, UILowerString propertyName, UIString oldValue, UIString newValue) :
mSourceObject(object),
mObject(object->DuplicateObject()),
mParentPath(object->GetParent()->GetFullPath()),
mChildPath(object->GetFullPath()),
mSiblingPath(),
mPropertyName(propertyName),
mOldValue(oldValue),
mNewValue(newValue)
{
	// Not necessary, but someone may change the logic and introduce an annoying bug.
	mObject->RemoveFromParent(); 

	// Get sibling path.
	UIPage * const parentPage = UI_ASOBJECT(UIPage, object->GetParent());
	if (parentPage) 
	{
		UIBaseObject * siblingObject = parentPage->GetPreviousChild(object);
		if (!siblingObject) 
		{
			siblingObject = parentPage->GetNextChild(object);
		}
		
		if (siblingObject) 
		{
			mSiblingPath = siblingObject->GetFullPath();
		}
	}
}

//-----------------------------------------------------------------

UIUndo::~UIUndo()
{
	mSourceObject = 0;
}

//-----------------------------------------------------------------

UIUndo::UIUndo(const UIUndo & rhs) :
mSourceObject(rhs.mSourceObject),
mObject(rhs.mObject),
mParentPath(rhs.mParentPath),
mChildPath(rhs.mChildPath),
mSiblingPath(rhs.mSiblingPath),
mPropertyName(rhs.mPropertyName),
mOldValue(rhs.mOldValue),
mNewValue(rhs.mNewValue)
{
	// Attach.
	mObject->RemoveFromParent();
}

//-----------------------------------------------------------------

UIUndo & UIUndo::operator=(UIUndo const & rhs)
{
	if (this != &(rhs)) 
	{
		mSourceObject = rhs.mSourceObject;
		
		mObject = rhs.mObject;
		
		mParentPath = rhs.mParentPath;
		mChildPath = rhs.mChildPath;
		mSiblingPath = rhs.mSiblingPath;		
		mPropertyName = rhs.mPropertyName;
		mOldValue = rhs.mOldValue;
		mNewValue = rhs.mNewValue;
	}

	return *this;
}

//-----------------------------------------------------------------

bool UIUndo::undo(bool &rebuildTree, UIBaseObject **selectedObject)
{
	bool shouldContinueToNextUndoObject = false;

	UIBaseObject * parentObject = UIManager::gUIManager().GetObjectFromPath(mParentPath.c_str());
	UIBaseObject * existingChildObject = UIManager::gUIManager().GetObjectFromPath(mChildPath.c_str());
	UIBaseObject * siblingObject = UIManager::gUIManager().GetObjectFromPath(mSiblingPath.c_str());

	rebuildTree = true;

	if(existingChildObject && !mPropertyName.empty())
	{
		existingChildObject->SetProperty(mPropertyName, mOldValue);
		rebuildTree = false;
		if(mPropertyName == UIBaseObject::PropertyName::Name)
		{
			rebuildTree = true;
			(*selectedObject) = parentObject;
		}
		return false;
	}

	if (existingChildObject) 
	{
		existingChildObject->RemoveFromParent();
	}

	if (parentObject)
	{
		bool const parentIsPage = parentObject->IsA(TUIPage);
		UIPage * const parentPage = parentIsPage ? static_cast<UIPage*>(parentObject) : NULL;

		if (parentPage)
		{
			UIBaseObject::UIObjectList lst;
			parentPage->GetChildren(lst);
			bool hasChild = false;
			for(UIBaseObject::UIObjectList::iterator it = lst.begin(); it != lst.end(); ++it)
			{
				if((*it) == siblingObject)
					hasChild = true;
			}
			if(hasChild)
				UI_IGNORE_RETURN(parentPage->InsertChildAfter(mObject, siblingObject));
			else
				UI_IGNORE_RETURN(parentPage->AddChild(mObject));
		}
		else
		{
			UI_IGNORE_RETURN(parentObject->AddChild(mObject));
		}
		
		parentObject->Link();

		if (parentPage) 
		{
			// Pack is necessary.
			parentPage->Pack();
		}
	}
	else
	{
		shouldContinueToNextUndoObject = true;
	}

	return shouldContinueToNextUndoObject;
}

//-----------------------------------------------------------------

UIBaseObject * UIUndo::getObject()
{
	return mObject;
}

//-----------------------------------------------------------------

UIBaseObject * UIUndo::getSourceObject()
{
	return mSourceObject;
}

//-----------------------------------------------------------------

void UIUndo::replaceSource(UIBaseObject * newSource)
{
	mSourceObject = newSource;
}

//-----------------------------------------------------------------

UILowerString const &UIUndo::getPropertyName() const
{
	return mPropertyName;
}

//-----------------------------------------------------------------

UIString const &UIUndo::getOldValue() const
{
	return mOldValue;
}


// ======================================================================
