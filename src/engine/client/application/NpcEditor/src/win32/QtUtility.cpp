// ============================================================================
//
// QtUtility.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstNpcEditor.h"
#include "QtUtility.h"

#include "sharedUtility/DataTable.h"
#include "sharedFoundation/ConfigFile.h"

#include <qcombobox.h>
#include <qsettings.h>
#include <qstring.h>
#include <qwidget.h>

// ============================================================================
//
// QtUtility
//
// ============================================================================

// ----------------------------------------------------------------------------

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
	IGNORE_RETURN(settings.writeEntry(text, x));

	sprintf(text, "%s_PositionY", widget.name());
	IGNORE_RETURN(settings.writeEntry(text, y));

	sprintf(text, "%s_Width", widget.name());
	IGNORE_RETURN(settings.writeEntry(text, w));

	sprintf(text, "%s_Height", widget.name());
	IGNORE_RETURN(settings.writeEntry(text, h));
}

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

char const *QtUtility::getSearchPath()
{
	return "/SOE/NpcEditor";
}

// ----------------------------------------------------------------------------

QString QtUtility::getLastPath()
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());

	QString result(settings.readEntry("LastPath", "c:\\"));

	return result;
}

// ----------------------------------------------------------------------------

void QtUtility::setLastPath(QString const &path)
{
	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, getSearchPath());

	IGNORE_RETURN(settings.writeEntry("LastPath", path));
}

// ----------------------------------------------------------------------------

void QtUtility::fillComboBoxFromConfigFile(const char * section, const char * keyName, int maxItems, QComboBox * comboBox)
{
	const char * itemName = 0;
	bool done = false;
	int i = 0;

	comboBox->clear();

	//-- keep looping until no more names are found
	while(!done && i < maxItems)
	{
		itemName = ConfigFile::getKeyString(section, keyName, i++, 0);

		if(itemName)
			comboBox->insertItem(itemName);
		else
			done = true;
	}

	comboBox->setEnabled(comboBox->count() > 0);
}

// ----------------------------------------------------------------------------

void QtUtility::fillComboBoxFromDataTable(DataTable const * dataTable, const char * colName, QComboBox * comboBox)
{
	comboBox->clear();

	for (int row = 0; row < dataTable->getNumRows(); ++row)
	{
		comboBox->insertItem(dataTable->getStringValue(colName, row));
	}

	if (comboBox->count() > 0)
	{
		comboBox->setCurrentItem(0);
	}

}

// ----------------------------------------------------------------------------

void QtUtility::makeFileNameRelative(QString * filename, char const * section, char const * root)
{
	QString defaultRoot = ConfigFile::getKeyString(section, root, 0, 0);

	int index = filename->find(defaultRoot);

	//-- make the path relative
	if(index != -1)
		IGNORE_RETURN(filename->remove(0, static_cast<unsigned int>(index)));
}

// ============================================================================
