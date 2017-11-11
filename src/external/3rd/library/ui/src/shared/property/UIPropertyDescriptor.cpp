#include "_precompile.h"
#include "UIPropertyDescriptor.h"
#include "UILowerString.h"

#include <algorithm>

///////////////////////////////////////////////////////////////////////////

UIPropertyGroup::UIPropertyGroup(
	 const char                     *i_name
	,UIPropertyCategories::Category  i_category
	,int                             i_groupLevelMajor
	,int                             i_groupLevelMinor
	,int                             i_fieldCount
	,UIPropertyDescriptor           *i_fields
	)
:	 m_name(i_name)
	,category(i_category)
	,m_groupLevelMajor(i_groupLevelMajor)
	,m_groupLevelMinor(i_groupLevelMinor)
	,fieldCount(i_fieldCount)
	,fields(i_fields)
{
	sort();
}

// =====================================================================

UIPropertyGroup &UIPropertyGroup::operator=(const UIPropertyGroup &x)
{
	m_name=x.m_name;
	category=x.category;
	m_groupLevelMajor=x.m_groupLevelMajor;
	m_groupLevelMinor=x.m_groupLevelMinor;
	fieldCount=x.fieldCount;
	fields=x.fields;
	return *this;
}

// =====================================================================

void UIPropertyGroup::sort()
{
	std::stable_sort(fields, fields+fieldCount);
}

// =====================================================================

const UIPropertyDescriptor *UIPropertyGroup::find(const char *i_propertyName) const
{
	if (!fields || !fieldCount || !i_propertyName)
	{
		return 0;
	}

   int lbound=0;
   int ubound=fieldCount;
   int iter=(lbound+ubound)/2;
   while (ubound>lbound)
   {
		const UIPropertyDescriptor &desc = fields[iter];

		int compare = _stricmp(i_propertyName, desc.m_name);

      if      (compare==0)   { return &desc; }
      else if (compare< 0)   ubound=iter;
      else                   lbound=iter+1;

      iter=(ubound+lbound)/2;
   }

   return false;

}
