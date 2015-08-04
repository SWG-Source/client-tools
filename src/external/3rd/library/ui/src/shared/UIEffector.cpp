//======================================================================
//
// UIEffector.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "_precompile.h"
#include "UIEffector.h"
#include "UILowerString.h"
#include "UIManager.h"
#include "UIPropertyDescriptor.h"

#include <vector>

//======================================================================
const UILowerString UIEffector::PropertyName::SoundOnActivate("SoundOnActivate");
const UILowerString UIEffector::PropertyName::SoundOnDeactivate("SoundOnDeactivate");
const UILowerString UIEffector::PropertyName::SoundOnDelayActivate("SoundOnDelayActivate");

//======================================================================================
#define _TYPENAME UIEffector

namespace UIEffectorNamespace
{
	//================================================================
	// Appearance category
	_GROUPBEGIN(Appearance)
	  _DESCRIPTOR(SoundOnActivate, "", T_string),
	  _DESCRIPTOR(SoundOnDelayActivate, "", T_string),
	  _DESCRIPTOR(SoundOnDeactivate, "", T_string)
	_GROUPEND(Appearance, 1, 0);
	//================================================================
}
using namespace UIEffectorNamespace;

//======================================================================================

//======================================================================

UIEffector::UIEffector() :
UIBaseObject ()
{
}

//======================================================================================

UIEffector::~UIEffector()
{
}

//======================================================================================

bool UIEffector::IsA(const UITypeID Type) const
{
	return (Type == TUIEffector) || UIBaseObject::IsA( Type );
}

//======================================================================================

UIEffector::EffectResult UIEffector::OnCreate( UIBaseObject * )
{
	return Continue;
}

//======================================================================================

void UIEffector::OnDestroy(UIBaseObject *)
{
}

//======================================================================================

void UIEffector::GetPropertyGroups(UIPropertyGroupVector &o_groups, UIPropertyCategories::Category category) const
{
	UIBaseObject::GetPropertyGroups(o_groups, category);
	GROUP_TEST_AND_PUSH(Appearance, category, o_groups);
}

//======================================================================================

void UIEffector::GetPropertyNames(UIPropertyNameVector & In, bool forCopy ) const
{
	UIBaseObject::GetPropertyNames(In, forCopy);
	
	In.push_back(PropertyName::SoundOnActivate);
	In.push_back(PropertyName::SoundOnDelayActivate);
	In.push_back(PropertyName::SoundOnDeactivate); 
}

//======================================================================================

bool UIEffector::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::SoundOnActivate)
	{
		mSoundOnActivate = Value;
		return true;
	}
	else if (Name == PropertyName::SoundOnDeactivate)
	{
		mSoundOnDeactivate = Value;
		return true;
	}
	else if (Name == PropertyName::SoundOnDelayActivate)
	{
		mSoundOnDelayActivate = Value;
		return true;
	}
	else
		return UIBaseObject::SetProperty(Name, Value);

}

//======================================================================================

bool UIEffector::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::SoundOnActivate)
	{
		Value = mSoundOnActivate;
		return true;
	}
	else if (Name == PropertyName::SoundOnDeactivate)
	{
		Value = mSoundOnDeactivate;
		return true;
	}
	else if (Name == PropertyName::SoundOnDelayActivate)
	{
		Value = mSoundOnDelayActivate;
		return true;
	}
	else
		return UIBaseObject::GetProperty(Name, Value);
}

//======================================================================================

void UIEffector::PlaySoundActivate() const
{
	if (!mSoundOnActivate.empty() && UIManager::isUIReady())
	{
		UIManager::gUIManager().PlaySound(UIUnicode::wideToNarrow(mSoundOnActivate).c_str());
	}
}

//======================================================================================

void UIEffector::PlaySoundDeactivate() const
{
	if (!mSoundOnDeactivate.empty() && UIManager::isUIReady())
	{
		UIManager::gUIManager().PlaySound(UIUnicode::wideToNarrow(mSoundOnDeactivate).c_str());
	}
}

//======================================================================================

void UIEffector::PlaySoundDelay() const
{
	if (!mSoundOnDelayActivate.empty() && UIManager::isUIReady())
	{
		UIManager::gUIManager().PlaySound(UIUnicode::wideToNarrow(mSoundOnDelayActivate).c_str());
	}
}


//======================================================================
