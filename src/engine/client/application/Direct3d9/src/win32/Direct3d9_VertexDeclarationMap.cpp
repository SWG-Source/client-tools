// ======================================================================
//
// Direct3d9_VertexDeclarationMap.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstDirect3d9.h"
#include "Direct3d9_VertexDeclarationMap.h"

#include "Direct3d9.h"
#include "Direct3d9_VertexBufferDescriptorMap.h"

#include "clientGraphics/VertexBufferFormat.h"
#include "clientGraphics/VertexBufferDescriptor.h"

#include <map>

// ======================================================================

namespace Direct3d9_VertexDeclarationMapNamespace
{
	class Key
	{
	public:

		Key(VertexBufferFormat const * const * vertexBufferFormat, int count);
		bool operator < (const Key &rhs) const;

	private:
	
		enum
		{
			MAX_VERTEX_BUFFERS = 2
		};

		uint32 m_key[MAX_VERTEX_BUFFERS];
	};

	typedef std::map<Key, IDirect3DVertexDeclaration9 *> VertexDeclarationMap;

	VertexDeclarationMap *ms_vertexDeclarationMap;
	D3DVERTEXELEMENT9     ms_vertexElements[32];
	D3DVERTEXELEMENT9     ms_vertexElementsEnd = D3DDECL_END();
}
using namespace Direct3d9_VertexDeclarationMapNamespace;

// ======================================================================

Direct3d9_VertexDeclarationMapNamespace::Key::Key(VertexBufferFormat const * const * vertexBufferFormat, int count)
{
	DEBUG_FATAL(count > MAX_VERTEX_BUFFERS, ("Too many VB's in a VBVector %d/%d", count, MAX_VERTEX_BUFFERS));

	int i = 0;
	for (i = 0; i < count; ++i)
		m_key[i] = vertexBufferFormat[i]->getFlags();
	for ( ; i < MAX_VERTEX_BUFFERS; ++i)
		m_key[i] = 0;
}

// ----------------------------------------------------------------------

bool Direct3d9_VertexDeclarationMapNamespace::Key::operator < (const Key &rhs) const
{
	for (int i = 0; i < MAX_VERTEX_BUFFERS-1; ++i)
	{
		if (m_key[i] < rhs.m_key[i])
			return true;
		if (m_key[i] > rhs.m_key[i])
			return false;
	}

	return m_key[MAX_VERTEX_BUFFERS-1] < rhs.m_key[MAX_VERTEX_BUFFERS-1];
}

// ======================================================================

void Direct3d9_VertexDeclarationMap::install()
{
	ms_vertexDeclarationMap = new VertexDeclarationMap;
}

// ----------------------------------------------------------------------

void Direct3d9_VertexDeclarationMap::remove()
{
	if (ms_vertexDeclarationMap)
	{
		while (!ms_vertexDeclarationMap->empty())
		{
			ms_vertexDeclarationMap->begin()->second->Release();
			ms_vertexDeclarationMap->erase(ms_vertexDeclarationMap->begin());
		}

		delete ms_vertexDeclarationMap;
		ms_vertexDeclarationMap = 0;
	}
}

// ----------------------------------------------------------------------

IDirect3DVertexDeclaration9 *Direct3d9_VertexDeclarationMap::fetchVertexDeclaration(VertexBufferFormat const * const * vertexBufferFormats, int count)
{
	Key key(vertexBufferFormats, count);
	
	// search for the key
	VertexDeclarationMap::iterator f = ms_vertexDeclarationMap->find(key);

	// return it if found
	if (f != ms_vertexDeclarationMap->end())
	{
		IDirect3DVertexDeclaration9 *vertexDeclaration = f->second;
		vertexDeclaration->AddRef();
		return vertexDeclaration;
	}
	
	int vertexElement = 0;
	int textureCoordinate = 0;
	for (int i = 0; i < count; ++i)
	{
		VertexBufferFormat const & vertexBufferFormat = *vertexBufferFormats[i];
		VertexBufferDescriptor const & descriptor = Direct3d9_VertexBufferDescriptorMap::getDescriptor(vertexBufferFormat);

		if (vertexBufferFormat.isTransformed())
		{
				ms_vertexElements[vertexElement].Stream     = static_cast<BYTE>(i);
				ms_vertexElements[vertexElement].Offset     = static_cast<BYTE>(descriptor.offsetPosition);
				ms_vertexElements[vertexElement].Type       = D3DDECLTYPE_FLOAT4;
				ms_vertexElements[vertexElement].Method     = D3DDECLMETHOD_DEFAULT;
#ifdef FFP
				ms_vertexElements[vertexElement].Usage      = D3DDECLUSAGE_POSITIONT;
#else
				ms_vertexElements[vertexElement].Usage      = D3DDECLUSAGE_POSITION;
#endif

				ms_vertexElements[vertexElement].UsageIndex = 0;
				++vertexElement;
		}
		else
			if (vertexBufferFormat.hasPosition())
			{
				ms_vertexElements[vertexElement].Stream     = static_cast<BYTE>(i);
				ms_vertexElements[vertexElement].Offset     = static_cast<BYTE>(descriptor.offsetPosition);
				ms_vertexElements[vertexElement].Type       = D3DDECLTYPE_FLOAT3;
				ms_vertexElements[vertexElement].Method     = D3DDECLMETHOD_DEFAULT;
				ms_vertexElements[vertexElement].Usage      = D3DDECLUSAGE_POSITION;
				ms_vertexElements[vertexElement].UsageIndex = 0;
				++vertexElement;
			}	

		if (vertexBufferFormat.hasNormal())
		{
			ms_vertexElements[vertexElement].Stream     = static_cast<BYTE>(i);
			ms_vertexElements[vertexElement].Offset     = static_cast<BYTE>(descriptor.offsetNormal);
			ms_vertexElements[vertexElement].Type       = D3DDECLTYPE_FLOAT3;
			ms_vertexElements[vertexElement].Method     = D3DDECLMETHOD_DEFAULT;
			ms_vertexElements[vertexElement].Usage      = D3DDECLUSAGE_NORMAL;
			ms_vertexElements[vertexElement].UsageIndex = 0;
			++vertexElement;
		}

		if (vertexBufferFormat.hasPointSize())
		{
			ms_vertexElements[vertexElement].Stream     = static_cast<BYTE>(i);
			ms_vertexElements[vertexElement].Offset     = static_cast<BYTE>(descriptor.offsetPointSize);
			ms_vertexElements[vertexElement].Type       = D3DDECLTYPE_FLOAT1;
			ms_vertexElements[vertexElement].Method     = D3DDECLMETHOD_DEFAULT;
			ms_vertexElements[vertexElement].Usage      = D3DDECLUSAGE_PSIZE;
			ms_vertexElements[vertexElement].UsageIndex = 0;
			++vertexElement;
		}

		if (vertexBufferFormat.hasColor0())
		{
			ms_vertexElements[vertexElement].Stream     = static_cast<BYTE>(i);
			ms_vertexElements[vertexElement].Offset     = static_cast<BYTE>(descriptor.offsetColor0);
			ms_vertexElements[vertexElement].Type       = D3DDECLTYPE_D3DCOLOR;
			ms_vertexElements[vertexElement].Method     = D3DDECLMETHOD_DEFAULT;
			ms_vertexElements[vertexElement].Usage      = D3DDECLUSAGE_COLOR;
			ms_vertexElements[vertexElement].UsageIndex = 0;
			++vertexElement;
		}

		if (vertexBufferFormat.hasColor1())
		{
			ms_vertexElements[vertexElement].Stream     = static_cast<BYTE>(i);
			ms_vertexElements[vertexElement].Offset     = static_cast<BYTE>(descriptor.offsetColor1);
			ms_vertexElements[vertexElement].Type       = D3DDECLTYPE_D3DCOLOR;
			ms_vertexElements[vertexElement].Method     = D3DDECLMETHOD_DEFAULT;
			ms_vertexElements[vertexElement].Usage      = D3DDECLUSAGE_COLOR;
			ms_vertexElements[vertexElement].UsageIndex = 1;
			++vertexElement;
		}

		const int numberOfTextureCoordinateSets = vertexBufferFormat.getNumberOfTextureCoordinateSets();
		for (int j = 0; j < numberOfTextureCoordinateSets; ++j)
		{
			ms_vertexElements[vertexElement].Stream     = static_cast<BYTE>(i);
			ms_vertexElements[vertexElement].Offset     = static_cast<BYTE>(descriptor.offsetTextureCoordinateSet[j]);

			switch (vertexBufferFormat.getTextureCoordinateSetDimension(j))
			{
				case 1:
					ms_vertexElements[vertexElement].Type   = D3DDECLTYPE_FLOAT1;
					break;

				case 2:
					ms_vertexElements[vertexElement].Type   = D3DDECLTYPE_FLOAT2;
					break;

				case 3:
					ms_vertexElements[vertexElement].Type   = D3DDECLTYPE_FLOAT3;
					break;

				case 4:
					ms_vertexElements[vertexElement].Type   = D3DDECLTYPE_FLOAT4;
					break;
			}

			ms_vertexElements[vertexElement].Method     = D3DDECLMETHOD_DEFAULT;
			ms_vertexElements[vertexElement].Usage      = D3DDECLUSAGE_TEXCOORD;
			ms_vertexElements[vertexElement].UsageIndex = static_cast<BYTE>(textureCoordinate);
			++textureCoordinate;
			++vertexElement;
		}
	}

	DEBUG_FATAL(vertexElement == 0, ("No vertex elements defined"));
	ms_vertexElements[vertexElement++] = ms_vertexElementsEnd;

	IDirect3DVertexDeclaration9 *vertexDeclaration = 0;
	HRESULT hresult = Direct3d9::getDevice()->CreateVertexDeclaration(ms_vertexElements, &vertexDeclaration);
	FATAL_DX_HR("CreateVertexDeclaration failed %s", hresult);
	NOT_NULL(vertexDeclaration);

	const bool inserted = ms_vertexDeclarationMap->insert(VertexDeclarationMap::value_type(key, vertexDeclaration)).second;
	UNREF(inserted);
	DEBUG_FATAL(!inserted, ("item already existed in map"));

	vertexDeclaration->AddRef();
	return vertexDeclaration;
}

// ----------------------------------------------------------------------

IDirect3DVertexDeclaration9 *Direct3d9_VertexDeclarationMap::fetchVertexDeclaration(VertexBufferFormat const &vertexBufferFormat)
{
	VertexBufferFormat const * const format[1] = { &vertexBufferFormat };
	return fetchVertexDeclaration(format, 1);
}

// ======================================================================
