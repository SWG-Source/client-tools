// ======================================================================
//
// SuiCreatePageCommand.h
// Defines SuiCreatePage_Command (struct + enum)
//
// ======================================================================

#ifndef INCLUDED_SuiCreatePageCommand_H
#define INCLUDED_SuiCreatePageCommand_H

#include "Unicode.h"
#include <vector>

struct SuiCreatePage_Command
{
	enum Type
	{
		CLEAR_DATA_SOURCE,
		ADD_CHILD_WIDGET,
		SET_PROPERTY,
		ADD_DATA_ITEM,
		SUBSCRIBE_TO_PROPERTY
	} m_type;

	std::vector<Unicode::String> m_parameters;
};

#endif