// ======================================================================
//
// GameWidget.cpp
// copyright(c) 2001-2005 Sony Online Entertainment
//
// ======================================================================

#include "FirstLightningEditor.h"

// module includes

#include "LightningEditorGameWidget.h"
#include "LightningEditorGameWidget.moc"

// ======================================================================

LightningEditorGameWidget::LightningEditorGameWidget(QWidget* theParent, char const * theName, WFlags const flags)
: GameWidget(theParent, theName, "LightningEditor.cfg", flags)
{
}

// ----------------------------------------------------------------------

LightningEditorGameWidget::~LightningEditorGameWidget()
{
}

// ======================================================================
