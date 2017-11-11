// ============================================================================
//
// ParticleEditorUtility.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "ParticleEditorUtility.h"

#include "sharedFile/FileNameUtils.h"
#include "sharedMath/Range.h"

///////////////////////////////////////////////////////////////////////////////
//
// ParticleBoundFloat
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
ParticleBoundFloat::ParticleBoundFloat(float const value)
 : m_max(value)
 , m_min(-value)
{
}

//-----------------------------------------------------------------------------
ParticleBoundFloat::ParticleBoundFloat(float const max, float const min)
 : m_max(max)
 , m_min(min)
{
	DEBUG_FATAL((min > max), ("min(%f) should be <= max(%f)"));
}

//-----------------------------------------------------------------------------
float ParticleBoundFloat::getDifference() const
{
	return m_max - m_min;
}

//-----------------------------------------------------------------------------
float ParticleBoundFloat::getPercent(float const value) const
{
	return (value - m_min) / getDifference();
}

//-----------------------------------------------------------------------------
float ParticleBoundFloat::getValue(float const percent) const
{
	DEBUG_FATAL((percent < 0.0f) || (percent > 1.0f), ("percent(%f) should be >= 0 and <= 1"));

	return m_min + percent * getDifference();
}

///////////////////////////////////////////////////////////////////////////////
//
// ParticleBoundInt
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
ParticleBoundInt::ParticleBoundInt(int const value)
 : m_max(value)
 , m_min(-value)
{
}

//-----------------------------------------------------------------------------
ParticleBoundInt::ParticleBoundInt(int const max, int const min)
 : m_max(max)
 , m_min(min)
{
	DEBUG_FATAL((min > max), ("min(%f) should be <= max(%f)"));
}

//-----------------------------------------------------------------------------
int ParticleBoundInt::getDifference() const
{
	return m_max - m_min;
}

//-----------------------------------------------------------------------------
float ParticleBoundInt::getPercent(float const value) const
{
	return (static_cast<float>(value - m_min)) / (static_cast<float>(getDifference()));
}

//-----------------------------------------------------------------------------
int ParticleBoundInt::getValue(float const percent) const
{
	DEBUG_FATAL((percent < 0.0f) || (percent > 1.0f), ("percent(%f) should be >= 0 and <= 1"));

	return m_min + percent * static_cast<float>(getDifference());
}

///////////////////////////////////////////////////////////////////////////////
//
// ParticleEditorUtility
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
void ParticleEditorUtility::saveWidget(QWidget const &widget)
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
void ParticleEditorUtility::loadWidget(QWidget &widget, int const defaultX, int const defaultY, int const defaultWidth, int const defaultHeight)
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

	widget.move(x, y);

	int w = 100;

	if (defaultWidth >= 0)
	{
		sprintf(text, "%s_Width", widget.name());
		w = settings.readNumEntry(text, defaultWidth);
		w = (w > 10000) ? defaultWidth : w;
	}

	int h = 100;

	if (defaultHeight >= 0)
	{
		sprintf(text, "%s_Height", widget.name());
		h = settings.readNumEntry(text, defaultHeight);
		h = (h > 10000) ? defaultHeight : h;
	}

	if ((defaultWidth >= 0) && (defaultHeight >= 0))
	{
		widget.resize(w, h);
	}

	widget.show();
}

//-----------------------------------------------------------------------------
char const *ParticleEditorUtility::getSearchPath()
{
	return "/SOE/SwgParticleEditor";
}

//-----------------------------------------------------------------------------
float ParticleEditorUtility::validateLineEditFloat(QLineEdit * lineEdit, Range const & range, int const precision)
{
	return setLineEditFloat(lineEdit, getFloat(lineEdit), range, precision);
}

//-----------------------------------------------------------------------------
float ParticleEditorUtility::validateLineEditFloat(QLineEdit *lineEdit, ParticleBoundFloat const &bound, int const precision)
{
	return setLineEditFloat(lineEdit, getFloat(lineEdit), bound, precision);
}

//-----------------------------------------------------------------------------
int ParticleEditorUtility::validateLineEditInt(QLineEdit *lineEdit, ParticleBoundInt const &bound)
{
	return setLineEditInt(lineEdit, getInt(lineEdit), bound);
}

//-----------------------------------------------------------------------------
float ParticleEditorUtility::getRangeControlPercent(QRangeControl const *rangeControl)
{
	DEBUG_FATAL(rangeControl->minValue() > rangeControl->maxValue(), ("invalid min/max"));

	float percent = 0.0f;
	float difference = static_cast<float>(rangeControl->maxValue() - rangeControl->minValue());

	if (difference > 0.0f)
	{
		percent = (rangeControl->value() - rangeControl->minValue()) / difference;
	}

	return percent;
}

//-----------------------------------------------------------------------------
float ParticleEditorUtility::setLineEditFloat(QLineEdit *lineEdit, float const value, Range const & range, int const precision)
{
	float result = value;

	if (result > range.getMax())
	{
		result = range.getMax();
	}
	else if (result < range.getMin())
	{
		result = range.getMin();
	}

	char text[256];
	snprintf(text, sizeof(text), "%*.*f", precision + 1, precision, result);
	text[sizeof(text) - 1] = '\0';
	lineEdit->setText(text);

	return result;
}

//-----------------------------------------------------------------------------
float ParticleEditorUtility::setLineEditFloat(QLineEdit *lineEdit, float const value, ParticleBoundFloat const &bound, int const precision)
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
int ParticleEditorUtility::setLineEditInt(QLineEdit *lineEdit, int const value, ParticleBoundInt const &bound)
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
float ParticleEditorUtility::getFloat(QLineEdit *lineEdit)
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
int ParticleEditorUtility::getInt(QLineEdit *lineEdit)
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
float ParticleEditorUtility::clamp(float const value, float const max, float const min)
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
float ParticleEditorUtility::clampMin(float const value, float const min)
{
	float result = value;

	if (result < min)
	{
		result = min;
	}

	return result;
}

//-----------------------------------------------------------------------------
float ParticleEditorUtility::clampMax(float const value, float const max)
{
	float result = value;

	if (result > max)
	{
		result = max;
	}

	return result;
}

//-----------------------------------------------------------------------------
bool ParticleEditorUtility::isValidIffFile(QWidget *owner, char const *path, Tag const &tag)
{
	bool result = false;

	// Make sure this is an iff file

	if (FileNameUtils::isIff(path))
	{
		// Make sure this is the correct kind of iff

		Iff iff(path);

		if (iff.getCurrentName() == tag)
		{
			result = true;
		}
		else
		{
			char specifiedTagString[256];
			ConvertTagToString(tag, specifiedTagString);
			char currentTagString[256];
			ConvertTagToString(iff.getCurrentName(), currentTagString);
			char text[1024];
			sprintf(text, "The specified file (%s) is not a valid (%s) IFF file. Initial tag is (%s).", path, specifiedTagString, currentTagString);
			QMessageBox::warning(owner, "File Error", text, "OK");
		}
	}
	else
	{
		char text[1024];
		sprintf(text, "The specified file (%s) is not a valid IFF file.", path);
		QMessageBox::warning(owner, "File Error", text, "OK");
	}

	return result;
}

//-----------------------------------------------------------------------------
void ParticleEditorUtility::drawColorText(QPainter *painter, int const x, int const y, char const *text, QColor const &normalTextColor)
{
	QFontMetrics fm(painter->fontMetrics());
	char const *charWalker = &text[0];
	int currentX = x;

	painter->setPen(normalTextColor);

	while (*charWalker != '\0')
	{
		// See if there is a color switch

		if (*charWalker == '~')
		{
			++charWalker;

			if (*charWalker == 'r')
			{
				painter->setPen(QColor("red"));
			}
			else if (*charWalker == 'b')
			{
				painter->setPen(QColor("blue"));
			}
			else if (*charWalker == 'o')
			{
				painter->setPen(QColor("orange"));
			}
			else if (*charWalker == 'p')
			{
				painter->setPen(QColor("purple"));
			}
			else if (*charWalker == 'k')
			{
				painter->setPen(QColor("black"));
			}
			else if (*charWalker == 'g')
			{
				painter->setPen(QColor("darkgreen"));
			}
			else if (*charWalker == 'n')
			{
				painter->setPen(normalTextColor);
			}

			++charWalker;
		}

		char character[2];
		character[0] = *charWalker;
		character[1] = '\0';

		painter->drawText(currentX, y, QString(character));
		currentX += fm.width(*charWalker);

		++charWalker;
	}
}

//-----------------------------------------------------------------------------
QString ParticleEditorUtility::getParticleEffectPath()
{
	// Returns the last used particle effect path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());

	return settings.readEntry("ParticleEffectFilePath", "c:\\");
}

//-----------------------------------------------------------------------------
void ParticleEditorUtility::setParticleEffectPath(QString const &path)
{
	// Saves the last used particle effect path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());

	settings.writeEntry("ParticleEffectFilePath", path);
}

//-----------------------------------------------------------------------------
QString ParticleEditorUtility::getParticleAttachmentPath()
{
	// Returns the last used particle effect path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());

	return settings.readEntry("ParticleAttachmentFilePath", "c:\\");
}

//-----------------------------------------------------------------------------
void ParticleEditorUtility::setParticleAttachmentPath(QString const &path)
{
	// Saves the last used particle attachment path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());

	settings.writeEntry("ParticleAttachmentFilePath", path);
}

//=============================================================================
