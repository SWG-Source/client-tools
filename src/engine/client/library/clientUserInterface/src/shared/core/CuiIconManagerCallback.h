//======================================================================
//
// CuiIconManagerCallback.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiIconManagerCallback_H
#define INCLUDED_CuiIconManagerCallback_H

//======================================================================

class ClientObject;
class UICursor;
class UIWidget;

//----------------------------------------------------------------------

class CuiIconManagerCallback
{
public:
	                        CuiIconManagerCallback () {};
	virtual                ~CuiIconManagerCallback () = 0  {}
	virtual void            overrideTooltip     (const ClientObject & obj, Unicode::String & str) const = 0;
	virtual void            overrideCursor      (const ClientObject & obj, UICursor *& cursor) const;
	virtual bool            overrideDoubleClick (const UIWidget & viewer) const;
};

//======================================================================

#endif
