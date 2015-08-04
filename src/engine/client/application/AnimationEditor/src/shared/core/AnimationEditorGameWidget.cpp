// ======================================================================
//
// GameWidget.cpp
// copyright(c) 2001-2005 Sony Online Entertainment
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"

// module includes

#include "AnimationEditorGameWidget.h"
#include "AnimationEditorGameWidget.moc"

// ======================================================================

AnimationEditorGameWidget::AnimationEditorGameWidget(QWidget* theParent, char const * theName, WFlags const flags)
: GameWidget(theParent, theName, "AnimationEditor.cfg", flags)
{
}

// ----------------------------------------------------------------------

AnimationEditorGameWidget::~AnimationEditorGameWidget()
{
}

// ======================================================================
