//===================================================================
//
// ResourceContainerObject.cpp
// copyright 2001, Sony Online Entertainment
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/ResourceContainerObject.h"

#include "clientGame/ClientController.h"
#include "clientGame/ClientResourceContainerObjectTemplate.h"
#include "clientGame/ObjectAttributeManager.h"
#include "clientGame/ResourceTypeManager.h"
#include "clientGame/ResourceIconManager.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureList.h"
#include "sharedFoundation/DebugInfoManager.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/Universe.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplateList.h"

//===================================================================

namespace
{
	const std::string default_texture = "texture/ui_res_default.dds";
}

//----------------------------------------------------------------------

ResourceContainerObject::ResourceContainerObject(const SharedResourceContainerObjectTemplate* newTemplate) :
	TangibleObject              (newTemplate),
	m_resourceType              (),
	m_quantity                  (0),
	m_maxQuantity               (0),
	m_resourceName              (),
	m_parentName                (),
	m_resourceNameId            (),
	m_updateResourceTypeManager (true)
{
	m_resourceType.setSourceObject(this);
	m_resourceNameId.setSourceObject(this);
	m_quantity.setSourceObject(this);
	m_maxQuantity.setSourceObject(this);

	addSharedVariable    (m_quantity);
	addSharedVariable    (m_resourceType);

	addSharedVariable_np (m_maxQuantity);
	addSharedVariable_np (m_parentName);
	addSharedVariable_np (m_resourceName);
	addSharedVariable_np (m_resourceNameId);
}

//-------------------------------------------------------------------
	
ResourceContainerObject::~ResourceContainerObject()
{
	if (isInWorld ())
		ResourceContainerObject::removeFromWorld ();
}

//-------------------------------------------------------------------

float ResourceContainerObject::alter (float time)
{
	if (m_updateResourceTypeManager && isInitialized ())
	{
		m_updateResourceTypeManager = false;
		const NetworkId & id = m_resourceType.get ();
		if (id != NetworkId::cms_invalid)
		{
			if (m_resourceName.get ().empty ())
			{
				WARNING (true, ("ResourceContainerObject [%s] invalid resource name. type=[%s] name=[%s] parent=[%s]", getNetworkId ().getValueString ().c_str (), id.getValueString ().c_str (), Unicode::wideToNarrow (m_resourceName.get ()).c_str (), m_parentName.get ().c_str ()));
			}
			else if (m_parentName.get ().empty ())
			{
				WARNING (true, ("ResourceContainerObject [%s] invalid parent.        type=[%s] name=[%s] parent=[%s]", getNetworkId ().getValueString ().c_str (), id.getValueString ().c_str (), Unicode::wideToNarrow (m_resourceName.get ()).c_str (), m_parentName.get ().c_str ()));
			}
			else
			{
				ResourceTypeManager::setTypeInfo (id, m_resourceName.get (), m_parentName.get ());
				
				const Texture * const texture = ResourceIconManager::fetchTextureForType (id);
				
				if (!texture)
					WARNING (true, ("ResourceContainerObject No texture available for [%s]", id.getValueString ().c_str ()));
				else
				{
					Appearance * const app = getAppearance  ();				
					if (app)
						app->setTexture (TAG (R,E,P,0), *texture);
					
					texture->release ();
				}
			}
		}
		else
		{
			const Texture * const texture = TextureList::fetch (default_texture.c_str ());
			
			if (texture)
			{
				Appearance * const app = getAppearance  ();		
				if (app)
					app->setTexture (TAG (R,E,P,0), *texture);
				
				texture->release ();
			}
		}
	}

	return TangibleObject::alter (time);
}

//----------------------------------------------------------------------

const NetworkId &       ResourceContainerObject::getResourceType () const
{
	return m_resourceType.get ();
}

//----------------------------------------------------------------------

const int               ResourceContainerObject::getQuantity     () const
{
	return m_quantity.get ();
}

//----------------------------------------------------------------------

const int               ResourceContainerObject::getMaxQuantity  () const
{
	return m_maxQuantity.get ();
}

//----------------------------------------------------------------------

const Unicode::String & ResourceContainerObject::getResourceName () const
{
	if (!m_resourceNameId.get ().isInvalid())
		return m_displayName;
	else
		return m_resourceName.get ();
}

//----------------------------------------------------------------------

const std::string & ResourceContainerObject::getParentName   () const
{
	return m_parentName.get ();
}

//----------------------------------------------------------------------

const bool              ResourceContainerObject::derivesFromResource (const Unicode::String & name) const
{
	if (m_resourceName.get ().empty ())
		return false;

	if (m_parentName.get ().empty ())
	{
		WARNING (true, ("resource container of [%s] has an empty parent string", Unicode::wideToNarrow (m_resourceName.get ()).c_str ()));
		return false;
	}
	return name == m_resourceName.get () || Universe::getInstance ().derivesFromResource (Unicode::wideToNarrow (name), m_parentName.get ());
}

//----------------------------------------------------------------------

void ResourceContainerObject::setDisplayName(const Unicode::String & name)
{
	m_displayName = name;

	ObjectAttributeManager::setObjectAttributesDirty(getNetworkId());
}

// ----------------------------------------------------------------------

void ResourceContainerObject::getObjectInfo(std::map<std::string, std::map<std::string, Unicode::String> > & propertyMap) const
{
	/**
	When adding a variable to this class, please add it here.  Variable that aren't easily displayable are still listed, for tracking purposes.
	*/

	/**
	Don't compile in production build because this maps human-readable values to data members and makes hacking easier
	*/
#if PRODUCTION == 0
	static std::string const ms_debugInfoSectionName("ResourceContainerObject");

	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "m_resourceType", m_resourceType.get().getValueString());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "m_resourceNameId", m_resourceNameId.get().getDebugString());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "m_quantity", m_quantity.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "m_maxQuantity", m_maxQuantity.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "m_resourceName", m_resourceName.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "m_parentName", m_parentName.get());
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "m_displayName", m_displayName);
	DebugInfoManager::addProperty(propertyMap, ms_debugInfoSectionName, "m_updateResourceTypeManager", (m_updateResourceTypeManager ? "true" : "false"));

	TangibleObject::getObjectInfo(propertyMap);
#else
	UNREF(propertyMap);
#endif
}

//----------------------------------------------------------------------

void ResourceContainerObject::Callbacks::ResourceChange::modified (ResourceContainerObject & target, const NetworkId &, const NetworkId &, bool) const
{
	target.m_updateResourceTypeManager = true;

	ObjectAttributeManager::setObjectAttributesDirty(target.getNetworkId());
}

//----------------------------------------------------------------------

void ResourceContainerObject::Callbacks::ResourceNameChange::modified (ResourceContainerObject & target, const StringId &, const StringId &resourceStringId, bool) const
{
	if (!resourceStringId.isInvalid())
	{
		target.setDisplayName(resourceStringId.localize());
		target.setObjectName(Unicode::emptyString);
		target.setObjectNameStringId(resourceStringId);
	}

	ObjectAttributeManager::setObjectAttributesDirty(target.getNetworkId());
}

//----------------------------------------------------------------------

void ResourceContainerObject::Callbacks::ResourceQuantityChange::modified(ResourceContainerObject & target, const int &, const int &, bool) const
{
	ObjectAttributeManager::setObjectAttributesDirty(target.getNetworkId());
}

//===================================================================
