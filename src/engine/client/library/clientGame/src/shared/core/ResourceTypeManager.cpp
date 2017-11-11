//======================================================================
//
// ResourceTypeManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ResourceTypeManager.h"

#include "sharedFoundation/NetworkId.h"
#include "sharedGame/ResourceClassObject.h"
#include "sharedGame/Universe.h"
#include "clientUserInterface/CuiStringVariablesManager.h"
#include "UnicodeUtils.h"
#include "clientGame/ClientStringIds.h"
#include "clientGame/ObjectAttributeManager.h"
#include <map>

//======================================================================

namespace
{
	struct TypeInfo
	{
		NetworkId       id;
		Unicode::String displayName;
		std::string     parent;

		TypeInfo (const NetworkId & _id, const Unicode::String & _displayName, const std::string & _parent) :
			id (_id),
			displayName (_displayName),
			parent      (_parent)
		{
		}
		TypeInfo () {}
	};

	typedef stdmap<NetworkId, TypeInfo>::fwd IdTypeMap;
	IdTypeMap s_idTypeMap;
}

//----------------------------------------------------------------------

void    ResourceTypeManager::reset               ()
{
	s_idTypeMap.clear ();
}

//----------------------------------------------------------------------

const NetworkId & ResourceTypeManager::findTypeByName (const Unicode::String & str)
{
	for (IdTypeMap::const_iterator it = s_idTypeMap.begin (); it != s_idTypeMap.end (); ++it)
	{
		const TypeInfo & tinfo = (*it).second;
		if (tinfo.displayName == str)
			return (*it).first;
	}

	return NetworkId::cms_invalid;
}

//----------------------------------------------------------------------

void ResourceTypeManager::setTypeInfo         (const NetworkId & id, const Unicode::String & displayName, const std::string & parent)
{
	if(id.isValid () && !displayName.empty () && !parent.empty ())
	{
		Unicode::String localDisplayName = displayName;
		if (displayName[0] == '@')
		{
			localDisplayName = StringId(Unicode::wideToNarrow(displayName)).localize();
		}
		s_idTypeMap [id] = TypeInfo (id, localDisplayName, parent);
		ObjectAttributeManager::requestUpdate (id);
	}
}

//----------------------------------------------------------------------

bool ResourceTypeManager::hasTypeInfo(NetworkId const & id)
{
	return id.isValid() && (s_idTypeMap.find(id) != s_idTypeMap.end());
}

//----------------------------------------------------------------------

bool ResourceTypeManager::findTypeParent      (const NetworkId & id, std::string &     parent)
{
	const IdTypeMap::const_iterator it = s_idTypeMap.find (id);
	if (it != s_idTypeMap.end ())
	{
		parent = (*it).second.parent;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

ResourceClassObject *  ResourceTypeManager::findTypeParent         (const NetworkId & id)
{
	std::string parentName;
	if (findTypeParent (id, parentName))
	{
		ResourceClassObject * const parentClass = Universe::getInstance ().getResourceClassByName (parentName);
		return parentClass;
	}

	return 0;
}

//----------------------------------------------------------------------

bool ResourceTypeManager::findTypeParentDisplayName (const NetworkId & id, Unicode::String & name)
{
	const ResourceClassObject * const rc = findTypeParent (id);

	if (rc)
	{
		name = rc->getFriendlyName ().localize();
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool ResourceTypeManager::findTypeDisplayName (const NetworkId & id, Unicode::String & displayName)
{
	const IdTypeMap::const_iterator it = s_idTypeMap.find (id);
	if (it != s_idTypeMap.end ())
	{
		displayName = (*it).second.displayName;
		if (displayName[0] == '@') // This is a string id
		{
			displayName = StringId(Unicode::wideToNarrow(displayName)).localize();
		}
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

bool ResourceTypeManager::createTypeDisplayLabel (const NetworkId & id, Unicode::String & str)
{
	const IdTypeMap::const_iterator it = s_idTypeMap.find (id);
	if (it != s_idTypeMap.end ())
	{
		const std::string & parent   = (*it).second.parent;

		const ResourceClassObject * const parentClass = Universe::getInstance ().getResourceClassByName (parent);

		if (parentClass)
		{
			Unicode::String nameValue = (*it).second.displayName;
			if (nameValue[0] == '@') // This is a string id
				nameValue = StringId(Unicode::wideToNarrow(nameValue)).localize();
			
			CuiStringVariablesManager::process (ClientStringIds::resource_type_label_prose, nameValue, parentClass->getFriendlyName ().localize(), Unicode::emptyString, str);
			return true;
		}
		else
			WARNING (true, ("type with no parent"));

	}
	return false;
}

//======================================================================
