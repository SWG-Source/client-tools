// ============================================================================
//
// SoundEditorUtility.h
// copyright Sony Online Entertainment
//
// ============================================================================

#ifndef SOUNDEDITORUTILITY_H
#define SOUNDEDITORUTILITY_H

class QWidget;

class QDial;
class QLineEdit;
class QRangeControl;
class QTextEdit;
class QWidget;

//-----------------------------------------------------------------------------
class SoundBoundFloat
{
public:

	SoundBoundFloat(float const value);
	SoundBoundFloat(float const min, float const max);

	float getDifference() const;
	float getPercent(float const value) const; // [0...1]
	float getValue(float const percent) const;

	float m_min;
	float m_max;
};

//-----------------------------------------------------------------------------
class SoundBoundInt
{
public:

	SoundBoundInt(int const value);
	SoundBoundInt(int const min, int const max);

	int   getDifference() const;
	float getPercent(float const value) const; // [0...1]
	int   getValue(float const percent) const;

	int m_min;
	int m_max;
};

//-----------------------------------------------------------------------------
class SoundEditorUtility
{
public:

	static void        setOutputWindow(QTextEdit &outputWindow);
	static void        report(QString const &text);
	static void        saveWidget(QWidget const &widget);
	static void        loadWidget(QWidget &widget, int const defaultX, int const defaultY, int const defaultWidth = 0, int const defaultHeight = 0);
	static char const *getSearchPath();
	static float       validateLineEditFloat(QLineEdit *lineEdit, SoundBoundFloat const &bound, int const precision);
	static int         validateLineEditInt(QLineEdit *lineEdit, SoundBoundInt const &bound);
	static void        checkMinMax(QLineEdit *a, QLineEdit *b);
	static float       setLineEditFloat(QLineEdit *lineEdit, float const value, SoundBoundFloat const &bound, int const precision);
	static int         setLineEditInt(QLineEdit *lineEdit, int const value, SoundBoundInt const &bound);
	static float       getRangeControlPercent(QRangeControl const *rangeControl);
	static float       getFloat(QLineEdit *lineEdit);
	static int         getInt(QLineEdit *lineEdit);
	static float       clamp(float const value, float const min, float const max);
	static float       clampMin(float const value, float const min);
	static float       clampMax(float const value, float const max);

private:

	static QTextEdit *m_outputWindow;

private:

	// Disabled

	SoundEditorUtility();
	~SoundEditorUtility();
};

// ============================================================================

#endif // SOUNDEDITORUTILITY_H