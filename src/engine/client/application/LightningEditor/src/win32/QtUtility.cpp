// ============================================================================
//
// QtUtility.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstLightningEditor.h"
#include "QtUtility.h"

#include "sharedFile/FileNameUtils.h"

///////////////////////////////////////////////////////////////////////////////
//
// QtUtility
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
void QtUtility::saveWidget(QWidget const &widget)
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
void QtUtility::loadWidget(QWidget &widget, int const defaultX, int const defaultY, int const defaultWidth, int const defaultHeight)
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
char const *QtUtility::getSearchPath()
{
	return "/SOE/SwgLightningEditor";
}

//-----------------------------------------------------------------------------
float QtUtility::getFloat(QLineEdit *lineEdit)
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
int QtUtility::getInt(QLineEdit *lineEdit)
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
bool QtUtility::isValidIffFile(QWidget *owner, char const *path, Tag const &tag)
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
QString QtUtility::getLastPath()
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());

	QString result(settings.readEntry("LastPath", "c:\\"));

	return result;
}

//-----------------------------------------------------------------------------
void QtUtility::setLastPath(QString const &path)
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());

	settings.writeEntry("LastPath", path);
}

//-----------------------------------------------------------------------------
float QtUtility::validateLineEditFloat(QLineEdit *lineEdit, float const min, float const max, int const precision)
{
	return setLineEditFloat(lineEdit, getFloat(lineEdit), min, max, precision);
}

//-----------------------------------------------------------------------------
int QtUtility::validateLineEditInt(QLineEdit *lineEdit, int const min, int const max)
{
	return setLineEditInt(lineEdit, getInt(lineEdit), min, max);
}

//-----------------------------------------------------------------------------
float QtUtility::setLineEditFloat(QLineEdit *lineEdit, float const value, float const min, float const max, int const precision)
{
	float result = clamp(min, value, max);

	char text[256];
	sprintf(text, "%*.*f", precision + 1, precision, result);
	lineEdit->setText(text);

	return result;
}

//-----------------------------------------------------------------------------
int QtUtility::setLineEditInt(QLineEdit *lineEdit, int const value, int const min, int const max)
{
	int result = clamp(min, value, max);

	char text[256];
	sprintf(text, "%d", result);
	lineEdit->setText(text);

	return result;
}

//=============================================================================
