// ============================================================================
//
// PaletteColorPicker.h
// Copyright 2004, Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_PaletteColorPicker_H
#define INCLUDED_PaletteColorPicker_H

#include "BasePaletteColorPicker.h"

#include <qpushbutton.h>

// ----------------------------------------------------------------------------

class QColorPushButton : public QPushButton
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	explicit QColorPushButton(QWidget *myParent, int color = 0, char const *windowName = 0);
	virtual ~QColorPushButton();

public slots:

	void slotReleased();

signals:

	void colorSelected(int c);

private:

	int m_color;

private: //-- disabled

	QColorPushButton(QColorPushButton const &);
	QColorPushButton &operator =(QColorPushButton const &);
	QColorPushButton();
};

// ----------------------------------------------------------------------------

class PaletteColorPicker : public BasePaletteColorPicker
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	PaletteColorPicker(QWidget *myParent, QColor const * color, int count, int current, char const *windowName = 0);
	virtual ~PaletteColorPicker();

	int getSelectedColor() const;

public slots:

	void slotColorSelected(int c);

private:

	int m_selectedColor;

private: //-- disabled

	PaletteColorPicker(PaletteColorPicker const &);
	PaletteColorPicker &operator =(PaletteColorPicker const &);
	PaletteColorPicker();
};

// ----------------------------------------------------------------------------

inline int PaletteColorPicker::getSelectedColor() const
{
	return m_selectedColor;
}

// ============================================================================

#endif // INCLUDED_PaletteColorPicker_H
