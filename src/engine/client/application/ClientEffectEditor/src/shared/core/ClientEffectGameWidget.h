// ======================================================================
//
// ClientEffectGameWidget.h
// copyright(c) 2001-2005 Sony Online Entertainment
//
// ======================================================================

#ifndef CLIENTEFFECTGAMEWIDGET_H
#define CLIENTEFFECTGAMEWIDGET_H

// ======================================================================

// ======================================================================

#include "swgClientQtWidgets/GameWidget.h"

//-----------------------------------------------------------------------

class ClientEffectGameWidget : public GameWidget
{
public:

	explicit ClientEffectGameWidget(QWidget * parent, char const * name, WFlags const flags = 0);
	virtual ~ClientEffectGameWidget();

private:
	ClientEffectGameWidget();
	ClientEffectGameWidget(ClientEffectGameWidget const & rhs);
	ClientEffectGameWidget & operator=(ClientEffectGameWidget const & rhs);
};

// ======================================================================

#endif
