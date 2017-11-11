// ======================================================================
//
// GameWidget.h
// copyright(c) 2001-2005 Sony Online Entertainment
//
// ======================================================================

#ifndef SWOOSHEDITORGAMEWIDGET_H
#define SWOOSHEDITORGAMEWIDGET_H

// ======================================================================

// ======================================================================

#include "swgClientQtWidgets/GameWidget.h"

//-----------------------------------------------------------------------

class SwooshEditorGameWidget : public GameWidget
{
public:

	explicit SwooshEditorGameWidget(QWidget * parent, char const * name, WFlags const flags = 0);
	virtual ~SwooshEditorGameWidget();

private:
	SwooshEditorGameWidget();
	SwooshEditorGameWidget(SwooshEditorGameWidget const & rhs);
	SwooshEditorGameWidget & operator=(SwooshEditorGameWidget const & rhs);
};

// ======================================================================

#endif
