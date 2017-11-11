// ======================================================================
//
// ObjectTransformWindow.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectTransformWindow_H
#define INCLUDED_ObjectTransformWindow_H

// ======================================================================

#include "BaseObjectTransformWindow.h"

#include <qdialog.h>

// ======================================================================

class ObjectTransformWindow : public BaseObjectTransformWindow
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:

	explicit ObjectTransformWindow(QWidget* paren = 0, const char* name = 0, bool modal = false, WFlags f = 0);
	virtual ~ObjectTransformWindow();

private:
	//disabled
	ObjectTransformWindow(const ObjectTransformWindow & rhs);
	ObjectTransformWindow& operator=(const ObjectTransformWindow& rhs);
};

// ======================================================================

#endif
