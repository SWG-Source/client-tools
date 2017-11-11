// ======================================================================
//
// IncludeHandler.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstShaderBuilder.h"
#include "IncludeHandler.h"

// ======================================================================

#include "sharedFile/TreeFile.h"

// ======================================================================

HRESULT IncludeHandler::Open(D3DXINCLUDE_TYPE, LPCSTR pFileName, LPCVOID, LPCVOID *ppData, UINT *pBytes)
{
	AbstractFile *file = TreeFile::open(pFileName, AbstractFile::PriorityData, true);
	if (!file)
		return STG_E_FILENOTFOUND;
	*pBytes = file->length();
	*ppData = reinterpret_cast<void*>(file->readEntireFileAndClose());
	delete file;
	return 0;
}

HRESULT IncludeHandler::Close(LPCVOID pData)
{
	delete [] const_cast<byte *>(reinterpret_cast<byte const *>(pData));
	return 0;
}

// ======================================================================
