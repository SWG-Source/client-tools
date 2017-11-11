// ======================================================================
//
// CuiWidgetGraph.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiWidgetGraph.h"
#include "UIUtils.h"

#include <deque>
#include <set>
#include <vector>

// ======================================================================

const UILowerString  CuiWidgetGraph::PropertyName::GraphColor = UILowerString ("GraphColor");
const UILowerString  CuiWidgetGraph::PropertyName::NumElements = UILowerString ("NumElements");

//-----------------------------------------------------------------
CuiWidgetGraph::CuiWidgetGraph () :
UIWidget (),
m_data (new DataDeque),
m_numElements (0),
m_graphColor ()
{
	NOT_NULL (m_data);
}

//----------------------------------------------------------------------

CuiWidgetGraph::~CuiWidgetGraph ()
{
	delete m_data;
	m_data = 0;
}

//-----------------------------------------------------------------

void CuiWidgetGraph::Render( UICanvas & ) const
{
	// @todo: make it render
}

//-----------------------------------------------------------------

bool CuiWidgetGraph::SetProperty( const UILowerString & Name, const UIString &Value )
{
	if (Name == PropertyName::GraphColor)
	{
		return UIUtils::ParseColor (Value, m_graphColor);
	}
	else if (Name == PropertyName::NumElements)
	{
		long dummy;
		if (UIUtils::ParseLong (Value, dummy))
		{
			m_numElements = dummy;
			return true;
		}
		return false;
	}

	return UIWidget::SetProperty (Name, Value);
}

//-----------------------------------------------------------------

bool CuiWidgetGraph::GetProperty( const UILowerString & Name, UIString &Value ) const
{
	if (Name == PropertyName::GraphColor)
	{
		return UIUtils::FormatColor (Value, m_graphColor);
	}
	else if (Name == PropertyName::NumElements)
	{
		return UIUtils::FormatInteger (Value, static_cast<long>(m_numElements));
	}
	
	return UIWidget::GetProperty (Name, Value);
}

//-----------------------------------------------------------------

void CuiWidgetGraph::GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const
{
	In.push_back ( PropertyName::GraphColor );
	In.push_back ( PropertyName::NumElements );
	UIWidget::GetPropertyNames (In, forCopy);
}

//-----------------------------------------------------------------

int CuiWidgetGraph::getNumElements () const
{
	return m_numElements;
}

//-----------------------------------------------------------------

void CuiWidgetGraph::setNumElements (int n)
{
	if (n < 0)
	{
		DEBUG_FATAL (true, (""));
		return; //lint !e527
	}

	m_numElements = n;

	while (m_data->size () >= static_cast<size_t>(m_numElements))
		m_data->pop_front ();
}

//-----------------------------------------------------------------

void CuiWidgetGraph::addData (real r)
{
	m_data->push_back (r);

	if (m_data->size () >= static_cast<size_t>(m_numElements))
		m_data->pop_front ();
}

//-----------------------------------------------------------------

void CuiWidgetGraph::clearData ()
{
	m_data->clear ();
}

// ======================================================================
