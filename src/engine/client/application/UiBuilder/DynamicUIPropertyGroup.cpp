#include "FirstUiBuilder.h"
#include "DynamicUIPropertyGroup.h"

#include "UIBaseObject.h"
#include "UIString.h"

#include <map>

/////////////////////////////////////////////////////////////////////////////////////

namespace DynamicUIPropertyGroupNamespace
{
	typedef bool (*pf_parser)(const char *&, const char *const);

	// ============================================================================================

	static bool _fork(pf_parser parser, const char *&iter, const char *const i_string_end)
	{
		const char *savedIter=iter;
		if (parser(iter, i_string_end))
		{
			return true;
		}
		else
		{
			iter=savedIter;
			return false;
		}
	}

	// ============================================================================================

	static bool _optional(pf_parser parser, const char *&iter, const char *const i_string_end)
	{
		_fork(parser, iter, i_string_end);
		return true;
	}

	// ============================================================================================

	static bool _lineEnd(const char *&iter, const char *const i_string_end)
	{
		return iter==i_string_end;
	}

	// ============================================================================================

	static bool _isSpace(const char *&iter, const char *const i_string_end)
	{
		if (  !_lineEnd(iter, i_string_end)
			&& isspace(*iter))
		{
			iter++;
			return true;
		}
		return false;
	}

	// ============================================================================================

	static bool _zeroOrMoreSpace(const char *&iter, const char *const i_string_end)
	{
		do
		{
			if (_lineEnd(iter, i_string_end))
			{
				return true;
			}
			if (!isspace(*iter))
			{
				return false;
			}
			iter++;
		} while (true);
	}

	// ============================================================================================

	static bool _zeroOrMoreNumerics(const char *&iter, const char *const i_string_end)
	{
		do
		{
			if (_lineEnd(iter, i_string_end))
			{
				return true;
			}
			if (!isdigit(*iter))
			{
				return false;
			}
			iter++;
		} while (true);
	}

	// ============================================================================================

	static bool _oneOrMoreNumerics(const char *&iter, const char *const i_string_end)
	{
		if (  !_lineEnd(iter, i_string_end)
			&& isdigit(*iter)
			)
		{
			iter++;
			return _zeroOrMoreNumerics(iter, i_string_end);
		}
		return false;
	}

	// ============================================================================================

	static bool _plusOrMinus(const char *&iter, const char *const i_string_end)
	{
		if (  !_lineEnd(iter, i_string_end)
			&& (*iter=='+' || *iter=='-')
			)
		{
			iter++;
			return true;
		}
		return false;
	}

	// ============================================================================================

	static bool _dot(const char *&iter, const char *const i_string_end)
	{
		if (  !_lineEnd(iter, i_string_end)
			&& *iter=='.'
			)
		{
			return true;
		}
		return false;
	}

	// ============================================================================================

	static bool _slash(const char *&iter, const char *const i_string_end)
	{
		if (  !_lineEnd(iter, i_string_end)
			&& *iter=='/'
			)
		{
			return true;
		}
		return false;
	}

	// ============================================================================================

	static bool _pound(const char *&iter, const char *const i_string_end)
	{
		if (  !_lineEnd(iter, i_string_end)
			&& *iter=='#'
			)
		{
			return true;
		}
		return false;
	}

	// ============================================================================================

	static bool _floatingPoint(const char *&iter, const char *i_string_end)
	{
		// optional + or -
		if (!_optional(_plusOrMinus, iter, i_string_end))
		{
			return false;
		}

		if (_fork(_dot, iter, i_string_end)) // dot numeric+
		{
			if (!_oneOrMoreNumerics(iter, i_string_end))
			{
				return false;
			}
		}
		else // numeric+ [dot numeric*]
		{
			if (!_oneOrMoreNumerics(iter, i_string_end))
			{
				return false;
			}
			if (_fork(_dot, iter, i_string_end))
			{
				if (!_optional(_zeroOrMoreNumerics, iter, i_string_end))
				{
					return false;
				}
			}
		}

		return true;
	}

	// ============================================================================================

	static bool _integer(const char *&iter, const char *const i_string_end)
	{
		// optional + or -
		if (!_optional(_plusOrMinus, iter, i_string_end))
		{
			return false;
		}

		return _oneOrMoreNumerics(iter, i_string_end);
	}

	// ============================================================================================

	static bool _boolean(const char *&iter, const char *const i_string_end)
	{
		if (  i_string_end-iter>=4
			&& strcmp(iter, "true")==0
			)
		{
			iter+=4;
			return true;
		}
		else if (
			   i_string_end-iter>=5
			&& strcmp(iter, "false")==0
			)
		{
			iter+=5;
			return true;
		}
		return false;
	}

	// ============================================================================================

	static UIPropertyTypes::Type _deducePropertyType(const UILowerString &name, const UIString &i_wideValue)
	{
		const UINarrowString value = Unicode::wideToNarrow(i_wideValue);
		UIPropertyTypes::Type returnValue = UIPropertyTypes::T_string;

		if (!value.empty())
		{
			const char *iter=value.c_str();
			const char *const stop = iter + value.size();

			if (_fork(_slash, iter, stop))
			{
				returnValue=UIPropertyTypes::T_object;
			}
			else if (_fork(_pound, iter, stop))
			{
				returnValue=UIPropertyTypes::T_color;
			}
			else if (_fork(_boolean, iter, stop))
			{
				returnValue=UIPropertyTypes::T_bool;
			}
		}

		return returnValue;
	}

	// ============================================================================================
}
using namespace DynamicUIPropertyGroupNamespace;

/////////////////////////////////////////////////////////////////////////////////////

DynamicUIPropertyGroup::DynamicUIPropertyGroup()
: m_object(0)
{
	_initGroup();
}

// ================================================================================

DynamicUIPropertyGroup::~DynamicUIPropertyGroup()
{
	clear();
}

// ================================================================================

void DynamicUIPropertyGroup::clear()
{
	_initGroup();
	m_object=0;
	m_fields.clear();
}

// ================================================================================

void DynamicUIPropertyGroup::setObject(const UIBaseObject *o)
{
	if (!empty())
	{
		clear();
	}

	m_object=o;
	if (!m_object)
	{
		return;
	}

	typedef UIBaseObject::UIPropertyNameMap UIPropertyNameMap;

	const UIPropertyNameMap *propMap = m_object->GetPropertyMap();
	if (!propMap || propMap->empty())
	{
		return;
	}

	UIBaseObject::UIPropertyGroupVector groups;
	o->GetPropertyGroups(groups);

	const int numProperties = propMap->size();
	m_fields.reserve(numProperties);

	UIPropertyNameMap::const_iterator pi;
	int i=0;
	for (pi=propMap->begin();pi!=propMap->end();++pi,++i)
	{
		const UIPropertyNameMap::value_type &pm = *pi;
		const UILowerString                 &propertyName = pm.first;
		const UIString                      &propertyValue = pm.second;

		// ---------------------------------------------
		bool stockProperty=false;
		for (unsigned j=0;j<groups.size();++j)
		{
			const UIPropertyGroup *g=groups[j];
			if (g->find(propertyName.c_str()))
			{
				stockProperty=true;
				break;
			}
		}
		// ---------------------------------------------

		if (!stockProperty)
		{
			m_fields.push_back();
			UIPropertyDescriptor &field = m_fields.back();

			field.m_name = propertyName.c_str();
			field.m_description = "";
			field.m_type = _deducePropertyType(propertyName, propertyValue);
		}

		/*
		if (stockProperty)
		{
			field.m_flags|=UIPropertyDescriptor::F_ERROR;
		}
		*/
	}

	m_group.fieldCount=m_fields.size();
	m_group.fields=&m_fields[0];
	m_group.sort();
}

// ================================================================================

void DynamicUIPropertyGroup::_initGroup()
{
	memset(&m_group, 0, sizeof(m_group));
	m_group.m_name = "";
}

// ================================================================================
