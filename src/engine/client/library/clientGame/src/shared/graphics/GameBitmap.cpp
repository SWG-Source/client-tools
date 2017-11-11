//===================================================================
//
// GameBitmap.cpp
// asommers 6-10-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/GameBitmap.h"

#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Shader.h"
#include "clientGraphics/VertexBuffer.h"

//===================================================================
// PUBLIC GameBitmap
//===================================================================

GameBitmap::GameBitmap (const Shader* shader, float width, float height) :
	m_shader (shader),
	m_width (width),
	m_height (height)
{
	NOT_NULL (m_shader);
	if (m_shader)
		m_shader->fetch ();
}

//-------------------------------------------------------------------

GameBitmap::~GameBitmap ()
{
	if (m_shader)
	{
		m_shader->release();
		m_shader = 0;
	}
}

//-------------------------------------------------------------------

float GameBitmap::getWidth () const
{
	return m_width;
}

//-------------------------------------------------------------------

float GameBitmap::getHeight () const
{
	return m_height;
}

//-------------------------------------------------------------------

const Shader* GameBitmap::getShader () const
{
	return m_shader;
}

//-------------------------------------------------------------------

void GameBitmap::render (const float ulx, const float uly, bool centerOnPoint) const
{
	NOT_NULL (m_shader);
	if (!m_shader)
		return;

	float x = ulx;
	float y = uly;

	VertexBufferFormat format;
	format.setPosition();
	format.setTransformed();
	format.setNumberOfTextureCoordinateSets(1);
	format.setTextureCoordinateSetDimension(0, 2);
	DynamicVertexBuffer vertexBuffer (format);

	if (centerOnPoint)
	{
		x -= (m_width / 2);
		y -= (m_height / 2);
	}

	vertexBuffer.lock (4);

		VertexBufferWriteIterator v = vertexBuffer.begin();

		v.setPosition (Vector (x, y, 0.f));
		v.setOoz (1.f);
		v.setTextureCoordinates (0, 0.f, 0.f);
		++v;

		v.setPosition (Vector (x + m_width, y, 0.f));
		v.setOoz (1.f);
		v.setTextureCoordinates (0, 1.f, 0.f);
		++v;

		v.setPosition (Vector (x + m_width, y + m_height, 0.f));
		v.setOoz (1.f);
		v.setTextureCoordinates (0, 1.f, 1.f);
		++v;

		v.setPosition (Vector (x, y + m_height, 0.f));
		v.setOoz (1.f);
		v.setTextureCoordinates (0, 0.f, 1.f);

	vertexBuffer.unlock ();

	Graphics::setStaticShader (m_shader->prepareToView());
	Graphics::setVertexBuffer (vertexBuffer);
	Graphics::drawTriangleFan ();
};

//===================================================================


