// ============================================================================
//
// QtUtility.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_QtUtility_H
#define INCLUDED_QtUtility_H

class QColor;
class QDial;
class QLineEdit;
class QPainter;
class QRangeControl;
class QString;
class QWidget;
class QComboBox;

class DataTable;

// ----------------------------------------------------------------------------

class QtUtility
{
public:

	static void saveWidget(QWidget const &widget);
	static void loadWidget(QWidget &widget, int const defaultX, int const defaultY, int const defaultWidth = -1, int const defaultHeight = -1);
	static char const *getSearchPath();
	static QString getLastPath();
	static void setLastPath(QString const &path);
	static void fillComboBoxFromConfigFile(const char * section, const char * keyName, int maxItems, QComboBox * comboBox);
	static void fillComboBoxFromDataTable(DataTable const * dataTable, const char * colName, QComboBox * comboBox);
	static void makeFileNameRelative(QString * filename, char const * section, char const * root);

private: //-- disabled

	QtUtility();
	~QtUtility();
	QtUtility &operator =(QtUtility const &);
};

// ============================================================================

#endif // INCLUDED_QtUtility_H
