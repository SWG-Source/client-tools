// ======================================================================
//
// ClientEffectGameWidget.cpp
// copyright(c) 2001-2005 Sony Online Entertainment
//
// ======================================================================

#include "ClientEffectEditor/FirstClientEffectEditor.h"

// module includes

#include "ClientEffectGameWidget.h"
#include "ClientEffectGameWidget.moc"

// ======================================================================

ClientEffectGameWidget::ClientEffectGameWidget(QWidget* theParent, char const * theName, WFlags const flags)
: GameWidget(theParent, theName, "ClientEffectEditor.cfg", flags)
{
}

// ----------------------------------------------------------------------

ClientEffectGameWidget::~ClientEffectGameWidget()
{
}

// ======================================================================
