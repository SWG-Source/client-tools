// ======================================================================
//
// GameWidget.h
// copyright(c) 2001-2005 Sony Online Entertainment
//
// ======================================================================

#ifndef ANIMATIONEDITORGAMEWIDGET_H
#define ANIMATIONEDITORGAMEWIDGET_H

// ======================================================================

// ======================================================================

#include "swgClientQtWidgets/GameWidget.h"

//-----------------------------------------------------------------------

class AnimationEditorGameWidget : public GameWidget
{
public:

	explicit AnimationEditorGameWidget(QWidget * parent, char const * name, WFlags const flags = 0);
	virtual ~AnimationEditorGameWidget();

private:
	AnimationEditorGameWidget();
	AnimationEditorGameWidget(AnimationEditorGameWidget const & rhs);
	AnimationEditorGameWidget & operator=(AnimationEditorGameWidget const & rhs);
};

// ======================================================================

#endif
