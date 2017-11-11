// ============================================================================
//
// Plotter.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/Plotter.h"

#if PRODUCTION == 0

#include "clientGraphics/Graphics.h"
#include "clientGraphics/ShaderTemplateList.h"
#include "clientGraphics/StaticShader.h"
#include <vector>

// ============================================================================
//
// Plotter
//
// ============================================================================

//-----------------------------------------------------------------------------
Plotter::Plotter()
 : m_x(0)
 , m_y(0)
 , m_width(0)
 , m_height(0)
 , m_minValue(0.0f)
 , m_maxValue(1.0f)
 , m_color(VectorArgb::solidWhite)
 , m_index(0)
 , m_valueList(NULL)
{
}

//-----------------------------------------------------------------------------
Plotter::~Plotter()
{
	delete m_valueList;
	m_valueList = NULL;
}

//-----------------------------------------------------------------------------
void Plotter::setFrame(int const x, int const y, int const width, int const height)
{
	DEBUG_FATAL((width <= 2), ("width must be > 2"));

	if (width > 2)
	{
		if (m_width != width)
		{
			delete m_valueList;
			m_valueList = NULL;
			m_valueList = new ValueList;
			m_valueList->reserve(width);

			// Reset the values

			for (int index = 0; index < width; ++index)
			{
				m_valueList->push_back(0.0f);
			}

			m_index = 0;
		}

		m_x = x;
		m_y = y;
		m_width = width;
		m_height = height;
	}
}

//-----------------------------------------------------------------------------
void Plotter::setFramePosition(int const x, int const y)
{
	m_x = x;
	m_y = y;
}

//-----------------------------------------------------------------------------
void Plotter::setColor(VectorArgb const &color)
{
	m_color = color;
}

//-----------------------------------------------------------------------------
void Plotter::setRange(float const min, float const max)
{
	DEBUG_FATAL((min >= max), ("min (%f) >= max (%f)", min, max));

	m_minValue = min;
	m_maxValue = max;

	if (m_minValue > m_maxValue)
	{
		std::swap(m_minValue, m_maxValue);
	}
}

//-----------------------------------------------------------------------------
void Plotter::addValue(float const value)
{
	if (m_valueList != NULL)
	{
		(*m_valueList)[m_index] = value;

		// Handle the index wrap

		if (++m_index >= m_valueList->size())
		{
			m_index = 0;
		}
	}
}

//-----------------------------------------------------------------------------
void Plotter::draw()
{
	if (m_valueList != NULL)
	{
		Graphics::setStaticShader(ShaderTemplateList::get2dVertexColorAStaticShader());
		Graphics::drawRectangle(m_x, m_y, m_x + m_width, m_y + m_height, VectorArgb(0.75f, 1.0f, 1.0f, 1.0f));

		float const valueDifference = m_maxValue - m_minValue;
		float currentValue = (*m_valueList)[m_index];
		float x = static_cast<float>(m_x);

		for (unsigned int index = m_index + 1; index < m_valueList->size() + m_index; ++index)
		{
			x += 1.0f;

			currentValue = (*m_valueList)[(index % m_valueList->size())];

			float const percent = ((clamp(m_minValue, currentValue - m_minValue, m_maxValue)) / valueDifference);
			float const y = m_y + m_height - (static_cast<float>(m_height - 1) * percent);

			Graphics::drawLine(x, static_cast<float>(m_y + m_height), x, y, m_color);
		}
	}
}

#endif // PRODUCTION == 0

// ============================================================================
