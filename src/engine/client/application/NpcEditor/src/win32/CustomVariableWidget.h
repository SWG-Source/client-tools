// ============================================================================
//
// CustomVariableWidget.h
// Copyright 2004, Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_CustomVariableWidget_H
#define INCLUDED_CustomVariableWidget_H

#include "BaseCustomVariableWidget.h"

#include <vector>

class PaletteArgb;

// ----------------------------------------------------------------------------

class CustomVariableWidget : public BaseCustomVariableWidget
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	CustomVariableWidget(QWidget *myparent, char const * variableName, int minValue,
		int maxValue, int currentValue, PaletteArgb const * colorPalette = 0, char const *windowName = 0);
	virtual ~CustomVariableWidget();

	void setValue(int value);
	void setColors(PaletteArgb const * pal);

public slots:

	void slotValueChanged(int value);
	void slotColorButtonReleased();

signals:

	void variableChanged(int, const char *);

private:

	std::vector<QColor> m_colors;

	int m_value;
	int m_minValue;
	int m_maxValue;

	// Disabled
	CustomVariableWidget(CustomVariableWidget const &);
	CustomVariableWidget &operator =(CustomVariableWidget const &);
	CustomVariableWidget();
};

// ============================================================================

#endif // INCLUDED_CustomVariableWidget_H
