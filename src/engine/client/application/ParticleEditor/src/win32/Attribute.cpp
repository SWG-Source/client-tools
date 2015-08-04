// ============================================================================
//
// Attribute.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "Attribute.h"
#include "Attribute.moc"

#include "sharedRandom/Random.h"
#include "WaveFormEdit.h"
#include "ColorRampEdit.h"

// ============================================================================
//
// Attribute
//
// ============================================================================

//-----------------------------------------------------------------------------
Attribute::Attribute(QWidget *parent, char const *name)
 : BaseAttribute(parent, name)
 , m_selected(false)
 , m_optionsPopupMenu(NULL)
{
	// Setup the reset popup menu

	m_optionsPopupMenu = new QPopupMenu(this);
	m_optionsPopupMenu->insertItem("Reset", resetItem);
	m_optionsPopupMenu->setCheckable(false);
	m_optionsPopupMenu->insertItem("Copy", copyItem);
	m_optionsPopupMenu->setCheckable(false);
	m_optionsPopupMenu->insertItem("Paste", pasteItem);
	m_optionsPopupMenu->setCheckable(false);

	connect(m_optionsPopupMenu, SIGNAL(activated(int)), SLOT(onOptionsPopupMenuActivated(int)));

	m_modifierCheckBox->hide();

	connect(m_modifierCheckBox, SIGNAL(clicked()), SLOT(slotModifierCheckBoxClicked()));
}

//-----------------------------------------------------------------------------
void Attribute::slotModifierCheckBoxClicked()
{
}

//-----------------------------------------------------------------------------
void Attribute::setModifiable(bool const modifiable, QString const &label)
{
	if (modifiable)
	{
		m_modifierCheckBox->setText(label);
		m_modifierCheckBox->show();
	}
	else
	{
		m_modifierCheckBox->hide();
	}
}

//-----------------------------------------------------------------------------
void Attribute::setText(std::string const &buttonText)
{
	m_attributeLabel->setText(buttonText.c_str());
}

//-----------------------------------------------------------------------------
char const *Attribute::getText() const
{
	return m_attributeLabel->text();
}

//-----------------------------------------------------------------------------
void Attribute::mouseDoubleClickEvent(QMouseEvent *)
{
	emit attributeDoubleClicked(this);

	emit ensureVisible(pos().x(), pos().y(), width(), height());
}

//-----------------------------------------------------------------------------
void Attribute::mousePressEvent(QMouseEvent *mouseEvent)
{
	// Find the control point to be deleted if the right button was pressed

	if (mouseEvent->button() == Qt::RightButton)
	{
		m_optionsPopupMenu->exec(mouseEvent->globalPos());
	}
}

//-----------------------------------------------------------------------------
void Attribute::setSelected(bool const b)
{
	m_selected = b;
	update();

	emit ensureVisible(pos().x(), pos().y(), width(), height());
}

//-----------------------------------------------------------------------------
bool Attribute::isSelected() const
{
	return m_selected;
}

//-----------------------------------------------------------------------------
void Attribute::onOptionsPopupMenuActivated(int index)
{
	switch (index)
	{
		case resetItem:
			{
				reset();
			}
			break;
		case saveItem:
			{
				save();
			}
			break;
		case loadItem:
			{
				load();
			}
			break;
		case copyItem:
			{
				copy();
			}
			break;
		case pasteItem:
			{
				paste();
			}
			break;
		default:
			{
				DEBUG_FATAL(1, ("Attribute::optionsPopupMenuActivated() - Invalid popup menu option selected."));
			}
	}
}

// ============================================================================
//
// WaveFormAttribute
//
// ============================================================================

WaveForm WaveFormAttribute::m_bufferedWaveForm;
bool     WaveFormAttribute::m_waveFormBuffered = false;

//-----------------------------------------------------------------------------
WaveFormAttribute::WaveFormAttribute(QWidget *parent, char const *name, float const maxValue, float const minValue)
 : Attribute(parent, name)
{
	// No flicker plz

	setBackgroundMode(Qt::NoBackground);

	// Set the waveform max and min

#ifdef _DEBUG
	m_waveForm.setName(name);
#endif // _DEBUG

	m_waveForm.setValueMax(maxValue);
	m_waveForm.setValueMin(minValue);

	connect(m_optionsPopupMenu, SIGNAL(aboutToShow()), SLOT(onAboutToShow()));
}

//-----------------------------------------------------------------------------
void WaveFormAttribute::reset()
{
	setWaveForm(m_defaultWaveForm, true);

	emit attributeDoubleClicked(this);
}

//-----------------------------------------------------------------------------
void WaveFormAttribute::load()
{
}

//-----------------------------------------------------------------------------
void WaveFormAttribute::save()
{
}

//-----------------------------------------------------------------------------
void WaveFormAttribute::copy()
{
	m_waveFormBuffered = true;
	m_bufferedWaveForm = m_waveForm;
}

//-----------------------------------------------------------------------------
void WaveFormAttribute::paste()
{
	if (m_waveFormBuffered)
	{
		m_waveForm.copyControlPoints(m_bufferedWaveForm);

		emit attributeDoubleClicked(this);
		emit attributeChanged();
	}
}

//-----------------------------------------------------------------------------
WaveForm const &WaveFormAttribute::getWaveForm() const
{
	return m_waveForm;
}

//-----------------------------------------------------------------------------
WaveForm const &WaveFormAttribute::getWaveFormDefault() const
{
	return m_defaultWaveForm;
}

//-----------------------------------------------------------------------------
void WaveFormAttribute::setWaveForm(WaveForm const &waveForm, bool const sendEmit)
{
	if (m_defaultWaveForm.getControlPointCount() <= 0)
	{
		// Save the new wave form

		setDefaultWaveForm(waveForm);
	}
	else
	{
		// Save the new wave form

		m_waveForm = waveForm;
	}

	// Force a redraw

	update();

	// Make sure this widget is visible

	emit ensureVisible(pos().x(), pos().y(), width(), height());

	// Let anyone know that the waveform changed

	if (sendEmit)
	{
		emit attributeChanged();
	}
}

//-----------------------------------------------------------------------------
void WaveFormAttribute::setDefaultWaveForm(WaveForm const &defaultWaveForm)
{
	m_defaultWaveForm = defaultWaveForm;
	m_waveForm = defaultWaveForm;
}

//-----------------------------------------------------------------------------
void WaveFormAttribute::paintEvent(QPaintEvent *)
{
	QPainter painter;
	QFontMetrics fm(fontMetrics());

	if ((m_pixMap.width() != width()) ||
	    (m_pixMap.height() != height()))
	{
		m_pixMap.resize(width(), height());
	}

	painter.begin(&m_pixMap);

	painter.fillRect(m_pixMap.rect(), QBrush(paletteBackgroundColor()));

	float max;
	float min;
	WaveFormEdit::WaveFormEditControlPoint waveFormEditControlPoint;
	waveFormEditControlPoint.m_controlPointIter = m_waveForm.getIteratorEnd();

	m_waveForm.calculateMinMax(max, min);

	int const space = 8;
	int const waveFormLabelWidth = fm.width("-4096.00");

	QRect viewPortRect(m_attributeLabel->width() + waveFormLabelWidth + space * 2, space, m_pixMap.width() - m_attributeLabel->width() - waveFormLabelWidth - space * 3, height() - space * 2);

	painter.setViewport(viewPortRect);
	painter.setWindow(QRect(0, 0, painter.viewport().width(), painter.viewport().height()));

	WaveFormEdit::drawWaveForm(painter, 1.0f, max, min, m_waveForm, waveFormEditControlPoint, waveFormEditControlPoint, waveFormEditControlPoint, 4);

	// Restore the viewport to the full widget

	painter.setViewport(rect());
	painter.setWindow(QRect(0, 0, painter.viewport().width(), painter.viewport().height()));

	if (isSelected())
	{
		painter.setPen(QColor("red"));
		painter.drawRect(painter.window());
		painter.drawRect(painter.window().left() + 1, painter.window().top() + 1, painter.window().right() - 1, painter.window().bottom() - 1);
	}

	// Draw the buffer to the widget

	bitBlt(this, 0, 0, &m_pixMap);

	painter.end();
}

//-----------------------------------------------------------------------------
void WaveFormAttribute::onAboutToShow()
{
	m_optionsPopupMenu->setItemEnabled(pasteItem, m_waveFormBuffered);
}

// ============================================================================
//
// ColorRampAttribute
//
// ============================================================================

ColorRamp ColorRampAttribute::m_bufferedColorRamp;
bool      ColorRampAttribute::m_colorRampBuffered = false;

//-----------------------------------------------------------------------------
ColorRampAttribute::ColorRampAttribute(QWidget *parent, char const *name)
 : Attribute(parent, name)
{
	// No flicker plz

	setBackgroundMode(Qt::NoBackground);

	connect(m_optionsPopupMenu, SIGNAL(aboutToShow()), SLOT(onAboutToShow()));
}

//-----------------------------------------------------------------------------
void ColorRampAttribute::reset()
{
	setColorRamp(m_defaultColorRamp, m_alphaWaveForm, true);

	emit attributeDoubleClicked(this);
}

//-----------------------------------------------------------------------------
void ColorRampAttribute::load()
{
}

//-----------------------------------------------------------------------------
void ColorRampAttribute::save()
{
}

//-----------------------------------------------------------------------------
void ColorRampAttribute::copy()
{
	m_colorRampBuffered = true;
	m_bufferedColorRamp = m_colorRamp;
}

//-----------------------------------------------------------------------------
void ColorRampAttribute::paste()
{
	if (m_colorRampBuffered)
	{
		m_colorRamp = m_bufferedColorRamp;

		emit attributeDoubleClicked(this);
		emit attributeChanged();
	}
}

//-----------------------------------------------------------------------------
ColorRamp const &ColorRampAttribute::getColorRamp() const
{
	return m_colorRamp;
}

//-----------------------------------------------------------------------------
ColorRamp const &ColorRampAttribute::getColorRampDefault() const
{
	return m_defaultColorRamp;
}

//-----------------------------------------------------------------------------
void ColorRampAttribute::setColorRamp(ColorRamp const &colorRamp, WaveForm const &alphaWaveForm, bool const sendEmit)
{
	if (m_defaultColorRamp.getControlPointCount() <= 0)
	{
		setDefaultColorRamp(colorRamp);
	}

	// Save the new color ramp

	m_colorRamp = colorRamp;

	// Set the alpha wave form

	m_alphaWaveForm = alphaWaveForm;

	// Force a redraw

	update();

	// Make sure this widget is visible

	emit ensureVisible(pos().x(), pos().y(), width(), height());

	// Let anyone know that the colorramp changed

	if (sendEmit)
	{
		emit attributeChanged();
	}
}

//-----------------------------------------------------------------------------
void ColorRampAttribute::setDefaultColorRamp(ColorRamp const &defaultColorRamp)
{
	m_defaultColorRamp = defaultColorRamp;
	m_colorRamp = defaultColorRamp;
}

//-----------------------------------------------------------------------------
void ColorRampAttribute::paintEvent(QPaintEvent *)
{
	QPainter painter;
	QFontMetrics fm(fontMetrics());
	
	if ((m_pixMap.width() != width()) ||
	    (m_pixMap.height() != height()))
	{
		m_pixMap.resize(width(), height());
	}
	
	painter.begin(&m_pixMap);
	
	painter.fillRect(m_pixMap.rect(), QBrush(paletteBackgroundColor()));
	
	int const space = 8;
	int const waveFormLabelWidth = fm.width("-4096.00");

	QRect viewPortRect(m_attributeLabel->width() + waveFormLabelWidth + space * 2, space, m_pixMap.width() - m_attributeLabel->width() - waveFormLabelWidth - space * 3, height() - space * 2);

	// Set the rectangle for just the color ramp

	painter.setViewport(viewPortRect);
	painter.setWindow(QRect(0, 0, painter.viewport().width(), painter.viewport().height()));

	// Draw the color ramp

	ColorRampEdit::drawColorRamp(painter, m_colorRamp, m_alphaWaveForm);
	
	// Restore the viewport to the full widget

	painter.setViewport(rect());
	painter.setWindow(QRect(0, 0, painter.viewport().width(), painter.viewport().height()));

	if (isSelected())
	{
		painter.setPen(QColor("red"));
		painter.drawRect(painter.window());
		painter.drawRect(painter.window().left() + 1, painter.window().top() + 1, painter.window().right() - 1, painter.window().bottom() - 1);
	}
	
	// Draw the buffer to the widget
	
	bitBlt(this, 0, 0, &m_pixMap);
	
	painter.end();
}

//-----------------------------------------------------------------------------
WaveForm const & ColorRampAttribute::getAlphaWaveForm() const
{
	return m_alphaWaveForm;
}

//-----------------------------------------------------------------------------
void ColorRampAttribute::onAboutToShow()
{
	m_optionsPopupMenu->setItemEnabled(pasteItem, m_colorRampBuffered);
}

// ============================================================================