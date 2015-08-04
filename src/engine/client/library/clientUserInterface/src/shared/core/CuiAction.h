// ======================================================================
//
// CuiAction.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiAction_H
#define INCLUDED_CuiAction_H

class Object;

// ======================================================================
class CuiAction
{
public:

	CuiAction ();
	virtual      ~CuiAction () = 0;
	virtual bool  performAction (const std::string & id, const Unicode::String & params) const = 0;

	typedef stdvector<Unicode::String>::fwd StringVector;

	static void     parseParams              (const Unicode::String & params, StringVector & sv);
	static Object * findObjectFromParam      (const Unicode::String & param);
	static Object * findObjectFromFirstParam (const Unicode::String & params, bool useLookAt, bool mustExist = false, const std::string & id = std::string ());

protected:

	CuiAction (const CuiAction & rhs);
	CuiAction & operator= (const CuiAction & rhs);
};

// ======================================================================

#endif
