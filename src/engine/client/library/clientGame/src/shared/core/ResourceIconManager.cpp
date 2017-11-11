//======================================================================
//
// ResourceIconManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ResourceIconManager.h"

#include "clientGame/ResourceTypeManager.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/Texture.h"
#include "clientObject/SpriteAppearance.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/Watcher.h"
#include "sharedGame/ResourceClassObject.h"
#include "sharedGame/Universe.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include <algorithm>
#include <map>
#include <set>

//======================================================================

namespace ResourceIconManagerNamespace
{
	typedef Watcher<Object> ObjectWatcher;
	struct AppearanceInfo
	{
		std::string   appearanceTemplateName;
		std::string   resourceClass;
		ObjectWatcher object;
	};

	typedef stdmap<std::string, AppearanceInfo>::fwd ResourceIconMap;
	ResourceIconMap s_icons;

}

using namespace ResourceIconManagerNamespace;

//----------------------------------------------------------------------

void ResourceIconManager::install ()
{
	InstallTimer const installTimer("ResourceIconManager::install");

}

//----------------------------------------------------------------------

void ResourceIconManager::remove  ()
{
	reset ();
}

//----------------------------------------------------------------------

/**
* Delete any outstanding resource icon objects
*/

void ResourceIconManager::reset ()
{
	typedef stdset<Object *>::fwd ObjectSet;
	ObjectSet os;

	for (ResourceIconMap::iterator it = s_icons.begin (); it != s_icons.end (); ++it)
	{
		AppearanceInfo & appInfo = (*it).second;
		ObjectWatcher & watcher  = appInfo.object;
		os.insert (watcher.getPointer ());
		watcher = 0;
	}

	std::for_each (os.begin (), os.end (), PointerDeleter ());
	os.clear ();
}

//----------------------------------------------------------------------

const AppearanceTemplate * ResourceIconManager::fetchAppearanceTemplateForClass     (const std::string & name, std::string & resourceClassFound)
{
	resourceClassFound = name;

	std::string filename;
	const ResourceIconMap::const_iterator it = s_icons.find (name);
	if (it != s_icons.end ())
	{
		const AppearanceInfo & appInfo = (*it).second;
		filename            = appInfo.appearanceTemplateName;
		resourceClassFound  = appInfo.resourceClass;
	}
	else
	{
		filename = "appearance/ui_res_" + name + ".spr";
	}

	const AppearanceTemplate * at = 0;

	if (TreeFile::exists (filename.c_str ()))
		at = AppearanceTemplateList::fetch (filename.c_str ());

	if (at == 0)
	{
		const ResourceClassObject * const rclass = Universe::getInstance ().getResourceClassByName (name);
		if (!rclass)
		{
			WARNING (true, ("ResourceIconManager request for resouce class [%s] does not exist, using inorganic", name.c_str ()));

			at = ResourceIconManager::fetchAppearanceTemplateForClass ("inorganic", resourceClassFound);
			if (at)
			{
				AppearanceInfo appInfo;
				appInfo.appearanceTemplateName = at->getName ();
				appInfo.resourceClass          = resourceClassFound;
				s_icons [name]                 = appInfo;
			}
		}
		else
		{
			const ResourceClassObject * const parentClass = rclass->getParent ();

			if (parentClass)
			{
				at = ResourceIconManager::fetchAppearanceTemplateForClass (parentClass->getResourceClassName (), resourceClassFound);
				if (at)
				{
					AppearanceInfo appInfo;
					appInfo.appearanceTemplateName = at->getName ();
					appInfo.resourceClass          = resourceClassFound;
					s_icons [name]                 = appInfo;
				}
			}
		}
	}
	else if (it == s_icons.end ())
	{
		AppearanceInfo appInfo;
		appInfo.appearanceTemplateName = at->getName ();
		appInfo.resourceClass          = name;
		s_icons [name]                 = appInfo;
	}

	WARNING (!at, ("ResourceIconManager failed to load icon for [%s]", name.c_str ()));
	return at;
}

//----------------------------------------------------------------------

Appearance * ResourceIconManager::createAppearanceForClass            (const std::string & name)
{
	std::string resourceClassFound;
	const AppearanceTemplate * const at = fetchAppearanceTemplateForClass (name, resourceClassFound);
	if (at)
	{
		const bool asynchronousLoaderEnabled = AsynchronousLoader::isEnabled ();
		AsynchronousLoader::disable ();

		Appearance * const ap = at->createAppearance ();
		AppearanceTemplateList::release (at);

		if (asynchronousLoaderEnabled)
			AsynchronousLoader::enable ();

		return ap;
	}

	return 0;
}

//----------------------------------------------------------------------

/**
* Objects returned from this function are owned and managed by the ResourceIconManager.
* Do not delete them!
*/

Object * ResourceIconManager::getObjectForClass                (const std::string & name)
{
	ResourceIconMap::iterator it = s_icons.find (name);
	if (it != s_icons.end ())
	{
		const AppearanceInfo & appInfo = (*it).second;
		Object * obj = appInfo.object.getPointer ();
		if (obj)
			return obj;
	}

	const AppearanceTemplate * at = 0;

	//-- this will cause an entry to be put into the s_icons map for [name],
	//-- if it does not already exist

	std::string resourceClassFound;
	at = fetchAppearanceTemplateForClass (name, resourceClassFound);

	Object * obj = 0;
	if (at)
	{
		if (name != resourceClassFound)
			obj = getObjectForClass (resourceClassFound);

		if (!obj)
		{
			const bool asynchronousLoaderEnabled = AsynchronousLoader::isEnabled ();
			AsynchronousLoader::disable ();

			obj = new Object;
			obj->setAppearance (at->createAppearance ());

			if (asynchronousLoaderEnabled)
				AsynchronousLoader::enable ();
		}

		AppearanceTemplateList::release (at);
	}

	if (it == s_icons.end ())
		it = s_icons.find (name);

	DEBUG_FATAL (it == s_icons.end (), ("bad data"));

	if (it != s_icons.end ())
	{
		AppearanceInfo & appInfo = (*it).second;
		appInfo.object = obj;
	}

	return obj;
}

//----------------------------------------------------------------------

/**
* Objects returned from this function are owned and managed by the ResourceIconManager.
* Do not delete them!
*/

Object * ResourceIconManager::getObjectForType  (const NetworkId & id)
{
	std::string parent;
	if (ResourceTypeManager::findTypeParent (id, parent))
	{
		return getObjectForClass (parent);
	}
	else
		WARNING (true, ("ResourceIconManager::createObjectForType for invalid type [%s]", id.getValueString ().c_str ()));

	return 0;
}

//----------------------------------------------------------------------

const Texture * ResourceIconManager::fetchTextureForClass  (const std::string & name)
{
	Appearance * const app = createAppearanceForClass (name);
	const Texture * texture = 0;

	if (app)
	{
		NOT_NULL (app->getAppearanceTemplate ());
		const SpriteAppearance * const sprite = dynamic_cast<SpriteAppearance *>(app);
		if (!sprite)
			WARNING (true, ("ResourceIconManager::fetchTextureForClass [%s] is not a sprite", app->getAppearanceTemplate ()->getName ()));
		else
		{
			const Shader * const shader = sprite->getShader ();
			if (!shader)
				WARNING (true, ("ResourceIconManager::fetchTextureForClass [%s] has no shader", app->getAppearanceTemplate ()->getName ()));
			else
			{
				const StaticShader * const staticShader = dynamic_cast<const StaticShader *>(shader);

				if (staticShader)
				{
					if (staticShader->getTexture (TAG (M,A,I,N), texture))
					{
						texture->fetch ();
					}
					else
						WARNING (true, ("ResourceIconManager::fetchTextureForClass [%s] shader has no MAIN texture", app->getAppearanceTemplate ()->getName ()));
				}
			}
		}

		delete app;
	}

	return texture;
}

//----------------------------------------------------------------------

const Texture * ResourceIconManager::fetchTextureForType  (const NetworkId & id)
{
	std::string parent;
	if (ResourceTypeManager::findTypeParent (id, parent))
	{
		return fetchTextureForClass (parent);
	}
	else
		WARNING (true, ("ResourceIconManager::fetchTextureForType for invalid type [%s]", id.getValueString ().c_str ()));

	return 0;
}

//======================================================================
