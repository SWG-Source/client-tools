// ======================================================================
//
// SnapToGridSettings.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SnapToGridSettings_H
#define INCLUDED_SnapToGridSettings_H

// ======================================================================

#include "BaseSnapToGridSettings.h"

// ======================================================================

class SnapToGridSettings : public BaseSnapToGridSettings
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:

	explicit SnapToGridSettings(QWidget *parent=0, const char *name=0);
	virtual ~SnapToGridSettings();
};

// ======================================================================

#endif
