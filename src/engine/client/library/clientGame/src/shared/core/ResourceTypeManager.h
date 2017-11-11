//======================================================================
//
// ResourceTypeManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ResourceTypeManager_H
#define INCLUDED_ResourceTypeManager_H

//======================================================================

class NetworkId;
class ResourceClassObject;

//----------------------------------------------------------------------

class ResourceTypeManager
{
public:

	static const NetworkId &      findTypeByName            (const Unicode::String & str);
	static void                   setTypeInfo               (const NetworkId & id, const Unicode::String & displayName, const std::string & parent);
	static bool hasTypeInfo(NetworkId const & id);
	static bool                   findTypeParent            (const NetworkId & id, std::string &     parent);
	static bool                   findTypeDisplayName       (const NetworkId & id, Unicode::String & name);
	static bool                   createTypeDisplayLabel    (const NetworkId & id, Unicode::String & str);
	static ResourceClassObject *  findTypeParent            (const NetworkId & id);
	static bool                   findTypeParentDisplayName (const NetworkId & id, Unicode::String & name);

	static void    reset               ();
};

//======================================================================

#endif
