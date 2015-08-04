// ======================================================================
//
// CuiGenericRendererCallback.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiGenericRendererCallback_H
#define INCLUDED_CuiGenericRendererCallback_H

class UIWidget;
class UICanvas;

// ======================================================================

class CuiGenericRendererCallback
{
public:

	CuiGenericRendererCallback ();
	virtual ~CuiGenericRendererCallback () = 0;

	virtual void genericRenderCallback (const UIWidget & widget, UICanvas & canvas) const = 0;

protected:
	CuiGenericRendererCallback (const CuiGenericRendererCallback & rhs);
	CuiGenericRendererCallback & operator= (const CuiGenericRendererCallback & rhs);
};

// ======================================================================

#endif
