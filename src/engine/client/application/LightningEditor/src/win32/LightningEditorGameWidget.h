// ======================================================================
//
// GameWidget.h
// copyright(c) 2001-2005 Sony Online Entertainment
//
// ======================================================================

#ifndef LIGHTNINGEDITORGAMEWIDGET_H
#define LIGHTNINGEDITORGAMEWIDGET_H

// ======================================================================

// ======================================================================

#include "swgClientQtWidgets/GameWidget.h"

//-----------------------------------------------------------------------

class LightningEditorGameWidget : public GameWidget
{
public:

	explicit LightningEditorGameWidget(QWidget * parent, char const * name, WFlags const flags = 0);
	virtual ~LightningEditorGameWidget();

private:
	LightningEditorGameWidget();
	LightningEditorGameWidget(LightningEditorGameWidget const & rhs);
	LightningEditorGameWidget & operator=(LightningEditorGameWidget const & rhs);
};

// ======================================================================

#endif
