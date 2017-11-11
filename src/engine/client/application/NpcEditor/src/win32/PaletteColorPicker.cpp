// ============================================================================
//
// PaletteColorPicker.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstNpcEditor.h"
#include "PaletteColorPicker.h"
#include "PaletteColorPicker.moc"

// ============================================================================
//
// PaletteColorPicker
//
// ============================================================================

void QColorPushButton::slotReleased()
{
	DEBUG_REPORT_LOG(false, ("QColorPushButton::released() - %d\n", m_color));

	//-- emit a signal here using the color
	emit colorSelected(m_color);
}

// ----------------------------------------------------------------------------

QColorPushButton::QColorPushButton(QWidget *myParent, int color, char const *windowName)
 : QPushButton(myParent, windowName)
 , m_color(color)
{
	IGNORE_RETURN(connect(this, SIGNAL(released()), this, SLOT(slotReleased())));
}

// ----------------------------------------------------------------------------

QColorPushButton::~QColorPushButton()
{
}

// ----------------------------------------------------------------------------

PaletteColorPicker::PaletteColorPicker(QWidget *myParent, QColor const * color, int count, int current, char const *windowName)
 : BasePaletteColorPicker(myParent, windowName)
 , m_selectedColor(current)
{
	int const countX = 16;
	int const countY = 16;

	int const sizeX = 16;
	int const sizeY = 16;

	int const startX = 4;
	int const startY = 4;

	int const padX = 2;
	int const padY = 2;

	QPoint p(startX, startY);
	QSize s(sizeX, sizeY);

	int index = 0;
	int ix, iy;

	for(iy=0; iy<countY; ++iy)
	{
		for(ix=0; ix<countX; ++ix, ++index)
		{
			QColorPushButton * b = new QColorPushButton(this, index);

			b->resize(s);
			b->move(p);
			b->show();

			if(index < count)
			{
				b->setPaletteBackgroundColor(color[index]);
				IGNORE_RETURN(connect(b, SIGNAL(colorSelected(int)), this, SLOT(slotColorSelected(int))));

				if(index == current)
					b->setFocus();
			}
			else
				b->setEnabled(false);

			p.setX(p.x() + sizeX + padX);
		} //lint !e429

		p.setX(startX);
		p.setY(p.y() + sizeY + padY);
	}

	DEBUG_REPORT_LOG(false, ("PaletteColorPicker::PaletteColorPicker() - %d\n", current));
}

// ----------------------------------------------------------------------------

PaletteColorPicker::~PaletteColorPicker()
{
}

// ----------------------------------------------------------------------------

void PaletteColorPicker::slotColorSelected(int c)
{
	DEBUG_REPORT_LOG(false, ("PaletteColorPicker::slotColorSelected() - %d\n", c));

	m_selectedColor = c;

	accept();
}

// ============================================================================
