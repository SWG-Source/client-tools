#ifndef INCLUDED_ObjectFactory_H
#define INCLUDED_ObjectFactory_H

class UIBaseObject;
class DefaultObjectPropertiesManager;

class ObjectFactory
{
public:
	ObjectFactory(const DefaultObjectPropertiesManager &defaultProperties);

	UIBaseObject *newObject(unsigned menuID, UIBaseObject *selectedParent) const;

	const DefaultObjectPropertiesManager &getDefaultPropertiesManager() const { return m_defaults; }

protected:

	const DefaultObjectPropertiesManager &m_defaults;
};

#endif

