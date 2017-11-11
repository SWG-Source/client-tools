// ======================================================================
//
// CuiGenericRenderer.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiGenericRenderer.h"
#include "clientUserInterface/CuiGenericRendererCallback.h"

// ======================================================================

const char * const CuiGenericRenderer::TypeName	= "CuiGenericRenderer";

//-----------------------------------------------------------------

CuiGenericRenderer::CuiGenericRenderer () :
UIWidget (),
m_callback (0)
{
}

//-----------------------------------------------------------------

CuiGenericRenderer::~CuiGenericRenderer ()
{
}

//-----------------------------------------------------------------

const char * CuiGenericRenderer::GetTypeName () const
{
	return TypeName;
}

//-----------------------------------------------------------------

UIBaseObject   *CuiGenericRenderer::Clone       () const
{
	return new CuiGenericRenderer;
}

//-----------------------------------------------------------------

void CuiGenericRenderer::Render      (UICanvas & canvas) const
{
	UIWidget::Render (canvas);
	if (m_callback)
		m_callback->genericRenderCallback (*this, canvas);
}

//-----------------------------------------------------------------

void CuiGenericRenderer::setCallback (CuiGenericRendererCallback * callback)
{
	m_callback = callback;
}

// ======================================================================
