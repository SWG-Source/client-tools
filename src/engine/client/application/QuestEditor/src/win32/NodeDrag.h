// ======================================================================
//
// NodeDrage.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_NodeDrag_H
#define INCLUDED_NodeDrag_H

// ----------------------------------------------------------------------

#include <qdom.h>
#include <qdragobject.h>

// ----------------------------------------------------------------------

class NodeDrag : public QDragObject
{
	Q_OBJECT

public:
	explicit NodeDrag(QDomNode node, QWidget * nodeDragSource = 0, char const * const nodeDragName = 0);
	~NodeDrag();

protected:

private:
	QDomNode m_node;

private: //-- disabled
	NodeDrag();
	NodeDrag(NodeDrag const &);
	NodeDrag &operator=(NodeDrag const &);
};

// ======================================================================

#endif // INCLUDED_NodeDrag_H
