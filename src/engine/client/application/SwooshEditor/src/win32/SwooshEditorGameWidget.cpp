// ======================================================================
//
// GameWidget.cpp
// copyright(c) 2001-2005 Sony Online Entertainment
//
// ======================================================================

#include "FirstSwooshEditor.h"

// module includes

#include "SwooshEditorGameWidget.h"
#include "SwooshEditorGameWidget.moc"

// ======================================================================

SwooshEditorGameWidget::SwooshEditorGameWidget(QWidget* theParent, char const * theName, WFlags const flags)
: GameWidget(theParent, theName, "AnimationEditor.cfg", flags)
{
}

// ----------------------------------------------------------------------

SwooshEditorGameWidget::~SwooshEditorGameWidget()
{
}

// ======================================================================
