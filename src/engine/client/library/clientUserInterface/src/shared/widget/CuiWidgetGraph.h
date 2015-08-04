// ======================================================================
//
// CuiWidgetGraph.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiWidgetGraph_H
#define INCLUDED_CuiWidgetGraph_H

// ======================================================================

#include "UIWidget.h"

/**
* CuiWidgetGraph is a simple histogram display, used for things like network graphs.
*/

class CuiWidgetGraph : public UIWidget
{
public:
	struct PropertyName
	{
		static const UILowerString GraphColor;
		static const UILowerString NumElements;
	};
	
	virtual UIBaseObject *   Clone( void ) const { return 0; }
	virtual	UIStyle *        GetStyle( void ) const;
	
	virtual void             Render( UICanvas & ) const;

	bool                     SetProperty( const UILowerString & Name, const UIString &Value );
	bool                     GetProperty( const UILowerString & Name, UIString &Value ) const;
	void                     GetPropertyNames( UIPropertyNameVector &In, bool forCopy ) const;

	int                      getNumElements () const;
	void                     setNumElements (int n);

	void                     addData (real r);
	void                     clearData ();

	                        ~CuiWidgetGraph ();
private:
	                         CuiWidgetGraph ();
	                         CuiWidgetGraph (const CuiWidgetGraph & rhs);
	CuiWidgetGraph &         operator=    (const CuiWidgetGraph & rhs);

private:

	typedef stddeque <real>::fwd  DataDeque;
	DataDeque *              m_data;

	int                      m_numElements;

	UIColor                  m_graphColor;
};
//-----------------------------------------------------------------

inline UIStyle * CuiWidgetGraph::GetStyle( void ) const
{
	return 0;
}

// ======================================================================

#endif
