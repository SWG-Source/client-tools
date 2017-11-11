// ======================================================================
//
// CuiGenericRenderer.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiGenericRenderer_H
#define INCLUDED_CuiGenericRenderer_H

#include "UIWidget.h"

class CuiGenericRendererCallback;

// ======================================================================

class CuiGenericRenderer : public UIWidget
{
public:

	static const char * const TypeName;
	
	CuiGenericRenderer ();
	~CuiGenericRenderer ();
	
	const char     *GetTypeName () const;
	UIBaseObject   *Clone       () const;
	
	void            Render      (UICanvas &) const;
		
	UIStyle        *GetStyle       () const { return 0; };
	
	void            setCallback (CuiGenericRendererCallback * callback);

private:
	CuiGenericRenderer (const CuiGenericRenderer & rhs);
	CuiGenericRenderer & operator= (const CuiGenericRenderer & rhs);

	CuiGenericRendererCallback * m_callback;
};

// ======================================================================

#endif
