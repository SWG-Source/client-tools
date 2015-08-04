//===================================================================
//
// IndexedTriangleListShaderPrimitive.cpp
// copyright 2001, sony online entertainment
//
//===================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/IndexedTriangleListShaderPrimitive.h"

#include "clientGraphics/Graphics.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/DynamicIndexBuffer.h"
#include "clientGraphics/VertexBufferFormat.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "sharedObject/Appearance.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Transform.h"

#include <vector>

//===================================================================

IndexedTriangleListShaderPrimitive::IndexedTriangleListShaderPrimitive (const Appearance& appearance, const IndexedTriangleList* indexedTriangleList, bool bOwnsList, const VectorArgb& color) :
	ShaderPrimitive (),
	m_appearance (appearance),
	m_indexedTriangleList (NON_NULL (indexedTriangleList)),
	m_color (color),
	m_bOwnsList(bOwnsList)
{
}

//-------------------------------------------------------------------
	
IndexedTriangleListShaderPrimitive::~IndexedTriangleListShaderPrimitive ()
{
	if(m_bOwnsList) delete m_indexedTriangleList;
	m_indexedTriangleList = NULL;
}

//-------------------------------------------------------------------

const Vector IndexedTriangleListShaderPrimitive::getPosition_w () const
{
	return m_appearance.getTransform_w ().getPosition_p ();
}
	
//-------------------------------------------------------------------

const StaticShader& IndexedTriangleListShaderPrimitive::prepareToView () const
{
	return ShaderTemplateList::get3dVertexColorZStaticShader ().prepareToView ();
}

//-------------------------------------------------------------------
	
float IndexedTriangleListShaderPrimitive::getDepthSquaredSortKey () const
{
	return m_appearance.getTransform_w ().getPosition_p ().magnitudeBetweenSquared (ShaderPrimitiveSorter::getCurrentCameraPosition ());
}

//-------------------------------------------------------------------
	
int IndexedTriangleListShaderPrimitive::getVertexBufferSortKey () const
{
	return 0;
}

//-------------------------------------------------------------------
	
void IndexedTriangleListShaderPrimitive::prepareToDraw () const
{
	NOT_NULL (m_indexedTriangleList);

	Graphics::setObjectToWorldTransformAndScale (m_appearance.getTransform_w (), Vector::xyz111);

	{
		const std::vector<Vector> vertices = m_indexedTriangleList->getVertices ();
		
		VertexBufferFormat format;
		format.setPosition ();
		format.setColor0 ();
		DynamicVertexBuffer vertexBuffer (format);

		vertexBuffer.lock (vertices.size ());

			VertexBufferWriteIterator v = vertexBuffer.begin ();
			const uint n = vertices.size ();
			uint i;
			for (i = 0; i < n; ++i, ++v)
			{
				v.setPosition (vertices [i]);
				v.setColor0 (m_color);
			}

		vertexBuffer.unlock ();

		Graphics::setVertexBuffer (vertexBuffer);
	}

	{
		const std::vector<int> indices = m_indexedTriangleList->getIndices ();

		DynamicIndexBuffer indexBuffer;

		indexBuffer.lock (indices.size ());

			Index *ii = indexBuffer.begin ();
			const uint n = indices.size ();
			uint i;
			for (i = 0; i < n; ++i, ++ii)
				*ii = static_cast<Index> (indices [i]);

		indexBuffer.unlock ();

		Graphics::setIndexBuffer (indexBuffer);
	}
}

//-------------------------------------------------------------------
	
void IndexedTriangleListShaderPrimitive::draw () const
{
	const GlFillMode fillMode = Graphics::getFillMode ();
	Graphics::setFillMode (GFM_wire);

	const GlCullMode cullMode = Graphics::getCullMode ();
	Graphics::setCullMode (GCM_none);

	Graphics::drawIndexedTriangleList ();

	Graphics::setFillMode (fillMode);
	Graphics::setCullMode (cullMode);
}

// ----------------------------------------------------------------------

IndexedTriangleList const * IndexedTriangleListShaderPrimitive::getMesh( void ) const
{
	return m_indexedTriangleList;
}

//===================================================================


