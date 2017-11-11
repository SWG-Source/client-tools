// ======================================================================
//
// IncludeHandler.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_IncludeHandler_H
#define INCLUDED_IncludeHandler_H

// ======================================================================

#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9shader.h>

// ======================================================================

class IncludeHandler : public ID3DXInclude
{
public:
	virtual HRESULT STDMETHODCALLTYPE Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
	virtual HRESULT STDMETHODCALLTYPE Close(LPCVOID pData);
};

// ======================================================================

#endif
