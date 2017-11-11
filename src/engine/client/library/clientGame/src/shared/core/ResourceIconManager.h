//======================================================================
//
// ResourceIconManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ResourceIconManager_H
#define INCLUDED_ResourceIconManager_H

//======================================================================

class AppearanceTemplate;
class Appearance;
class Object;
class NetworkId;
class Texture;

//----------------------------------------------------------------------

class ResourceIconManager
{
public:
	static void                       install                             ();
	static void                       remove                              ();
	static void                       reset                               ();

	static const Texture *            fetchTextureForClass                (const std::string & name);
	static const Texture *            fetchTextureForType                 (const NetworkId & id);
	static const AppearanceTemplate * fetchAppearanceTemplateForClass     (const std::string & name, std::string & resourceClassFound);
	static Appearance *               createAppearanceForClass            (const std::string & name);
	static Object *                   getObjectForClass                   (const std::string & name);
	static Object *                   getObjectForType                    (const NetworkId & id);
};

//======================================================================

#endif
