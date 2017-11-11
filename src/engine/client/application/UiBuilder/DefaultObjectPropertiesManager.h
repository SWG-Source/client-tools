#ifndef INCLUDED_DefaultObjectPropertiesManager_H
#define INCLUDED_DefaultObjectPropertiesManager_H

#include "UIBaseObject.h"

#include <map>
#include <hash_map>
#include <list>

class DefaultObjectPropertiesManager
{
public:

	// ----------------------------------------------------------

	typedef std::hash_map<UINarrowString, UIString> StringMap;
	
	// ----------------------------------------------------------

	struct DefaultObjectProperties
	{
		UITypeID        TypeID;
		UINarrowString  TypeName;
		StringMap       Properties;
	};

	typedef std::list<DefaultObjectProperties> DefaultObjectPropertiesList;

	// ----------------------------------------------------------

	DefaultObjectPropertiesManager();
	~DefaultObjectPropertiesManager();

	void SaveTo(FILE *);
	void LoadFrom(FILE *);

	void ApplyDefaultPropertiesToObject(UIBaseObject *) const;

	const DefaultObjectPropertiesList &getDefaultProperties() const { return m_defaultObjectProperties; }
	StringMap *                        getSelectedPropertyMap() { return m_selectedPropertyMap; }

	void                               setSelectedPropertyMap(StringMap *map) { m_selectedPropertyMap=map; }

private:

	DefaultObjectPropertiesList    m_defaultObjectProperties;
	StringMap                     *m_selectedPropertyMap;
};

#endif
