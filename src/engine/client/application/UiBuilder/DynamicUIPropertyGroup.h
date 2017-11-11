#ifndef INCLUDED_DynamicUIPropertyGroup_H
#define INCLUDED_DynamicUIPropertyGroup_H

//////////////////////////////////////////////////////////////////////////////////////////

#include "UIPropertyDescriptor.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////

class UIBaseObject;

//////////////////////////////////////////////////////////////////////////////////////////

class DynamicUIPropertyGroup
{
public:

	DynamicUIPropertyGroup();
	~DynamicUIPropertyGroup();

	// ========================================================================================

	bool                   empty()                   const { return m_group.fieldCount==0; }
	const UIBaseObject    *getObject()               const { return m_object; }
	const UIPropertyGroup &getUIPropertyGroup()      const { return m_group; }

	// ========================================================================================

	void clear();
	void setObject(const UIBaseObject *o);

	// ========================================================================================

private:

	void _initGroup();

	// ========================================================================================

	const UIBaseObject                 *m_object;
	UIPropertyGroup                     m_group;
	std::vector<UIPropertyDescriptor>   m_fields;
};

//////////////////////////////////////////////////////////////////////////////////////////

#endif
