//======================================================================
//
// SwgCuiContainerProviderFilter.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiContainerProviderFilter_H
#define INCLUDED_SwgCuiContainerProviderFilter_H

//======================================================================

class ClientObject;

//----------------------------------------------------------------------

class SwgCuiContainerProviderFilter
{
public:
	virtual bool showObject      (const ClientObject & obj) const = 0;
	virtual ~SwgCuiContainerProviderFilter () = 0 {};
};

//======================================================================

#endif
