// ============================================================================
//
// SoundEditorUtility.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstSoundEditor.h"
#include "SoundEditorUtility.h"

// ============================================================================
//
// SoundBoundFloat
//
// ============================================================================

//-----------------------------------------------------------------------------
SoundBoundFloat::SoundBoundFloat(float const value)
 : m_max(value)
 , m_min(-value)
{
}

//-----------------------------------------------------------------------------
SoundBoundFloat::SoundBoundFloat(float const min, float const max)
 : m_min(min)
 , m_max(max)
{
	assert(min <= max);
}

//-----------------------------------------------------------------------------
float SoundBoundFloat::getDifference() const
{
	return m_max - m_min;
}

//-----------------------------------------------------------------------------
float SoundBoundFloat::getPercent(float const value) const
{
	return (value - m_min) / getDifference();
}

//-----------------------------------------------------------------------------
float SoundBoundFloat::getValue(float const percent) const
{
	assert(percent >= 0.0f);
	assert(percent <= 1.0f);

	return m_min + percent * getDifference();
}

// ============================================================================
//
// SoundBoundInt
//
// ============================================================================

//-----------------------------------------------------------------------------
SoundBoundInt::SoundBoundInt(int const value)
 : m_max(value)
 , m_min(-value)
{
}

//-----------------------------------------------------------------------------
SoundBoundInt::SoundBoundInt(int const min, int const max)
 : m_min(min)
 , m_max(max)
{
	assert(min <= max);
}

//-----------------------------------------------------------------------------
int SoundBoundInt::getDifference() const
{
	return m_max - m_min;
}

//-----------------------------------------------------------------------------
float SoundBoundInt::getPercent(float const value) const
{
	return (static_cast<float>(value - m_min)) / (static_cast<float>(getDifference()));
}

//-----------------------------------------------------------------------------
int SoundBoundInt::getValue(float const percent) const
{
	assert(percent >= 0.0f);
	assert(percent <= 1.0f);

	return m_min + percent * static_cast<float>(getDifference());
}

// ============================================================================
//
// SoundEditorUtility
//
// ============================================================================

QTextEdit *SoundEditorUtility::m_outputWindow = NULL;

//-----------------------------------------------------------------------------
void SoundEditorUtility::setOutputWindow(QTextEdit &outputWindow)
{
	m_outputWindow = &outputWindow;
}

//-----------------------------------------------------------------------------
void SoundEditorUtility::report(QString const &text)
{
	assert(m_outputWindow != NULL);

	if ((m_outputWindow != NULL) && !text.isEmpty())
	{
		m_outputWindow->append(text);
		m_outputWindow->scrollToBottom();

	}
}

//-----------------------------------------------------------------------------
void SoundEditorUtility::saveWidget(QWidget const &widget)
{
   QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());
   
	QWidget const *parent = (widget.parentWidget() == NULL) ? &widget : widget.parentWidget();

   int const x = parent->pos().x();
   int const y = parent->pos().y();
   int const w = widget.width();
   int const h = widget.height();

   char text[256];
   sprintf(text, "%s_PositionX", widget.name());
   settings.writeEntry(text, x);

   sprintf(text, "%s_PositionY", widget.name());
   settings.writeEntry(text, y);

   sprintf(text, "%s_Width", widget.name());
   settings.writeEntry(text, w);

   sprintf(text, "%s_Height", widget.name());
   settings.writeEntry(text, h);
}

//-----------------------------------------------------------------------------
void SoundEditorUtility::loadWidget(QWidget &widget, int const defaultX, int const defaultY, int const defaultWidth, int const defaultHeight)
{
   QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());

   char text[256];
   sprintf(text, "%s_PositionX", widget.name());
   int x = settings.readNumEntry(text, defaultX);
	x = (x > 10000) ? defaultX : x;

   sprintf(text, "%s_PositionY", widget.name());
   int y = settings.readNumEntry(text, defaultY);
	y = (y > 10000) ? defaultY : y;

   sprintf(text, "%s_Width", widget.name());
   int w = settings.readNumEntry(text, defaultWidth);
	w = (w > 10000) ? defaultWidth : w;

   sprintf(text, "%s_Height", widget.name());
   int h = settings.readNumEntry(text, defaultHeight);
	h = (h > 10000) ? defaultHeight : h;

   widget.move(x, y);
   widget.resize(w, h);
	widget.show();
}

//-----------------------------------------------------------------------------
char const *SoundEditorUtility::getSearchPath()
{
	return "/SOE/SwgSoundEditor";
}

//-----------------------------------------------------------------------------
float SoundEditorUtility::validateLineEditFloat(QLineEdit *lineEdit, SoundBoundFloat const &bound, int const precision)
{
	return setLineEditFloat(lineEdit, getFloat(lineEdit), bound, precision);
}

//-----------------------------------------------------------------------------
int SoundEditorUtility::validateLineEditInt(QLineEdit *lineEdit, SoundBoundInt const &bound)
{
	return setLineEditInt(lineEdit, getInt(lineEdit), bound);
}

//-----------------------------------------------------------------------------
void SoundEditorUtility::checkMinMax(QLineEdit *a, QLineEdit *b)
{
	float const aValue = getFloat(a);
	float const bValue = getFloat(b);

	if (aValue > bValue)
	{
		// Swap

		QString temp = a->text();
		a->setText(b->text());
		b->setText(temp);
	}
}

//-----------------------------------------------------------------------------
float SoundEditorUtility::getRangeControlPercent(QRangeControl const *rangeControl)
{
	assert(rangeControl->minValue() <= rangeControl->maxValue());

	float percent = 0.0f;
	float difference = static_cast<float>(rangeControl->maxValue() - rangeControl->minValue());

	if (difference > 0.0f)
	{
		percent = (rangeControl->value() - rangeControl->minValue()) / difference;
	}

	return percent;
}

//-----------------------------------------------------------------------------
float SoundEditorUtility::setLineEditFloat(QLineEdit *lineEdit, float const value, SoundBoundFloat const &bound, int const precision)
{
	float result = value;

	if (result > bound.m_max)
	{
		result = bound.m_max;
	}
	else if (result < bound.m_min)
	{
		result = bound.m_min;
	}

	char text[256];
	sprintf(text, "%*.*f", precision + 1, precision, result);
	lineEdit->setText(text);

	return result;
}

//-----------------------------------------------------------------------------
int SoundEditorUtility::setLineEditInt(QLineEdit *lineEdit, int const value, SoundBoundInt const &bound)
{
	int result = value;

	if (result > bound.m_max)
	{
		result = bound.m_max;
	}
	else if (result < bound.m_min)
	{
		result = bound.m_min;
	}

	char text[256];
	sprintf(text, "%d", result);
	lineEdit->setText(text);

	return result;
}

//-----------------------------------------------------------------------------
float SoundEditorUtility::getFloat(QLineEdit *lineEdit)
{
	bool valid = true;

	float result = lineEdit->text().toFloat(&valid);

	// Just slam the value so we don't crash

	if (!valid)
	{
		result = 0.0f;
	}

	return result;
}

//-----------------------------------------------------------------------------
int SoundEditorUtility::getInt(QLineEdit *lineEdit)
{
	bool valid = true;

	int result = lineEdit->text().toInt(&valid);

	// Just slam the value so we don't crash

	if (!valid)
	{
		result = 0;
	}

	return result;
}

//-----------------------------------------------------------------------------
float SoundEditorUtility::clamp(float const value, float const min, float const max)
{
	float result = value;

	if (result > max)
	{
		result = max;
	}
	else if (result < min)
	{
		result = min;
	}

	return result;
}

//-----------------------------------------------------------------------------
float SoundEditorUtility::clampMin(float const value, float const min)
{
	float result = value;

	if (result < min)
	{
		result = min;
	}

	return result;
}

//-----------------------------------------------------------------------------
float SoundEditorUtility::clampMax(float const value, float const max)
{
	float result = value;

	if (result > max)
	{
		result = max;
	}

	return result;
}

// ============================================================================