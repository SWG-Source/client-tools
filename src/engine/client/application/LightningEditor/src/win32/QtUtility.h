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

//-----------------------------------------------------------------------------
class QtUtility
{
public:

	static void        saveWidget(QWidget const &widget);
	static void        loadWidget(QWidget &widget, int const defaultX, int const defaultY, int const defaultWidth = -1, int const defaultHeight = -1);
	static char const *getSearchPath();
	static float       validateLineEditFloat(QLineEdit *lineEdit, float const min, float const max, int const precision);
	static int         validateLineEditInt(QLineEdit *lineEdit, int const min, int const max);
	static float       setLineEditFloat(QLineEdit *lineEdit, float const value, float const min, float const max, int const precision);
	static int         setLineEditInt(QLineEdit *lineEdit, int const value, int const min, int const max);
	static float       getFloat(QLineEdit *lineEdit);
	static int         getInt(QLineEdit *lineEdit);
	static bool        isValidIffFile(QWidget *owner, char const *path, Tag const &tag);
	static QString     getLastPath();
	static void        setLastPath(QString const &path);

private:

	// Disabled

	QtUtility();
	~QtUtility();
	QtUtility &operator =(QtUtility const &);
};

// ============================================================================

#endif // INCLUDED_QtUtility_H
