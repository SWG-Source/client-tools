//===================================================================
//
// ResourceContainerObject.h
// copyright 2001, Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_ResourceContainerObject_H
#define INCLUDED_ResourceContainerObject_H

//===================================================================

class SharedResourceContainerObjectTemplate;

#include "clientGame/TangibleObject.h"

//===================================================================

class ResourceContainerObject : public TangibleObject
{
public:

	typedef stdvector<Unicode::String>::fwd StringVector;

	explicit ResourceContainerObject (const SharedResourceContainerObjectTemplate* newTemplate);
	virtual ~ResourceContainerObject ();

	virtual float alter (float time);

	const NetworkId &       getResourceType () const;
	const int               getQuantity     () const;
	const int               getMaxQuantity  () const;
	const Unicode::String & getResourceName () const;
	const std::string &     getParentName   () const;
	const bool              derivesFromResource (const Unicode::String & name) const;

	void setDisplayName(const Unicode::String & name);

	virtual void getObjectInfo(stdmap<std::string, stdmap<std::string, Unicode::String>::fwd >::fwd & propertyMap) const;

	struct Callbacks
	{
		template <typename U> struct DefaultCallback
		{
			void modified (ResourceContainerObject & target, const U & old, const U & value, bool isLocal) const;
		};

		typedef DefaultCallback<NetworkId>  ResourceChange;
		typedef DefaultCallback<StringId>   ResourceNameChange;
		typedef DefaultCallback<int>        ResourceQuantityChange;
	};

	friend Callbacks::ResourceChange;

private:
 	ResourceContainerObject ();
	ResourceContainerObject (const ResourceContainerObject& rhs);
	ResourceContainerObject &operator=(const ResourceContainerObject& rhs);

private:
	Archive::AutoDeltaVariableCallback<NetworkId,Callbacks::ResourceChange, ResourceContainerObject>              
		m_resourceType;
	Archive::AutoDeltaVariableCallback<StringId,Callbacks::ResourceNameChange, ResourceContainerObject>              
		m_resourceNameId;
	Archive::AutoDeltaVariableCallback<int, Callbacks::ResourceQuantityChange, ResourceContainerObject>
		m_quantity;
	Archive::AutoDeltaVariableCallback<int, Callbacks::ResourceQuantityChange, ResourceContainerObject>
		m_maxQuantity;

	Archive::AutoDeltaVariable<Unicode::String> m_resourceName;
	Archive::AutoDeltaVariable<std::string>     m_parentName;

	Unicode::String                             m_displayName;

	bool                                        m_updateResourceTypeManager;

};

//===================================================================

#endif
