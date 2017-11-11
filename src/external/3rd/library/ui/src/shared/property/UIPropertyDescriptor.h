//======================================================================
//
// UIPropertyDescriptor.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_UIPropertyDescriptor_H
#define INCLUDED_UIPropertyDescriptor_H

//======================================================================

#include "UIPropertyTypes.h"
#include "UIPropertyCategories.h"

class UIPropertyDescriptor
{
public:

	enum Flag {
		 F_READ_ONLY    = (1<<0)
		,F_SUB_PROPERTY = (1<<1) // indicates this is a descriptor for a property that is part of some other property.
		,F_ERROR        = (1<<2)
	};

   UIPropertyDescriptor() :
      m_name(0),
      m_description(0),
      m_type(UIPropertyTypes::T_int),
		m_flags(0)
   {
   }

   explicit UIPropertyDescriptor(
      const char          * i_name, 
      const char          * i_description, 
      UIPropertyTypes::Type i_type,
		int                   i_flags=0
      )
      :
      m_name(i_name), 
		m_description(i_description),
      m_type(i_type),
		m_flags(i_flags)
	{}

	bool operator<(const UIPropertyDescriptor &x) const { return _stricmp(m_name, x.m_name)<0; }

	bool isReadOnly()                             const { return (m_flags&F_READ_ONLY)!=0; }
	bool isSubProperty()                          const { return (m_flags&F_SUB_PROPERTY)!=0; }
	bool isError()                                const { return (m_flags&F_ERROR)!=0; }

   const char            *m_name;
   const char            *m_description;
   UIPropertyTypes::Type  m_type;
	unsigned               m_flags;
};

//======================================================================

class UIPropertyGroup
{
public:
	UIPropertyGroup() {}
	UIPropertyGroup(
		 const char                     *i_name
		,UIPropertyCategories::Category  i_category
		,int                             i_groupLevelMajor
		,int                             i_groupLevelMinor
		,int                             i_fieldCount
		,UIPropertyDescriptor           *i_fields
	);

	UIPropertyGroup &operator=(const UIPropertyGroup &x);
	void sort();

	const UIPropertyDescriptor *find(const char *i_propertyName) const;

	const char                      *m_name;
	UIPropertyCategories::Category   category;
	int                              m_groupLevelMajor;
	int                              m_groupLevelMinor;
   int                              fieldCount;
   UIPropertyDescriptor            *fields;
};

//======================================================================

#define BASIC_PROPERTY_DESCRIPTOR(_class, _name, _desc, _type)\
UIPropertyDescriptor(_class::PropertyName::_name.c_str(),_desc,UIPropertyTypes::_type)

#define BASIC_PROPERTY_DESCRIPTOR_FLAGS(_class, _name, _desc, _type, _flags)\
UIPropertyDescriptor(_class::PropertyName::_name.c_str(),_desc,UIPropertyTypes::_type,UIPropertyDescriptor::_flags)

#define PROPERTY_GROUP_BEGIN(_category) UIPropertyDescriptor s_##_category##PropertyDescriptors[] = {

#define PROPERTY_GROUP_END(_type, _category, _groupMajor, _groupMinor) };\
UIPropertyGroup s_##_category##PropertyGroup(\
	_type::TypeName,\
	UIPropertyCategories::C_##_category,\
	_groupMajor, _groupMinor,\
	sizeof(s_##_category##PropertyDescriptors) / sizeof(*s_##_category##PropertyDescriptors),\
	s_##_category##PropertyDescriptors)

#define GROUP_TEST_AND_PUSH(_have_category, _want_category, _container)\
if (  _want_category==UIPropertyCategories::C_ALL_CATEGORIES\
	|| _want_category==UIPropertyCategories::C_##_have_category\
	)\
{\
	_container.push_back(&s_##_have_category##PropertyGroup);\
}

// define _TYPENAME to be your classes name to use these
#define _DESCRIPTOR(_name, _desc, _type)               BASIC_PROPERTY_DESCRIPTOR(_TYPENAME, _name, _desc, _type)
#define _DESCRIPTOR_FLAGS(_name, _desc, _type, _flags) BASIC_PROPERTY_DESCRIPTOR_FLAGS(_TYPENAME, _name, _desc, _type, _flags)
#define _GROUPBEGIN                                    PROPERTY_GROUP_BEGIN
#define _GROUPEND(_category, _groupMajor, _groupMinor) PROPERTY_GROUP_END(_TYPENAME, _category, _groupMajor, _groupMinor)

//======================================================================

#endif
