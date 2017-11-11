// ============================================================================
//
// ParticleEditorUtility.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ParticleEditorUtility_H
#define INCLUDED_ParticleEditorUtility_H

class QColor;
class QDial;
class QLineEdit;
class QPainter;
class QRangeControl;
class QString;
class QWidget;
class Range;

//-----------------------------------------------------------------------------
class ParticleBoundFloat
{
public:

	ParticleBoundFloat(float const value);
	ParticleBoundFloat(float const max, float const min);

	float getDifference() const;
	float getPercent(float const value) const; // [0...1]
	float getValue(float const percent) const;

	float m_max;
	float m_min;
};

//-----------------------------------------------------------------------------
class ParticleBoundInt
{
public:

	ParticleBoundInt(int const value);
	ParticleBoundInt(int const max, int const min);

	int   getDifference() const;
	float getPercent(float const value) const; // [0...1]
	int   getValue(float const percent) const;

	int m_max;
	int m_min;
};

//-----------------------------------------------------------------------------
class ParticleEditorUtility
{
public:

	static void        saveWidget(QWidget const &widget);
	static void        loadWidget(QWidget &widget, int const defaultX, int const defaultY, int const defaultWidth = -1, int const defaultHeight = -1);
	static char const *getSearchPath();
	static float       validateLineEditFloat(QLineEdit *lineEdit, Range const & range, int const precision = 2);
	static float       validateLineEditFloat(QLineEdit *lineEdit, ParticleBoundFloat const &bound, int const precision);
	static int         validateLineEditInt(QLineEdit *lineEdit, ParticleBoundInt const &bound);
	static float       setLineEditFloat(QLineEdit *lineEdit, float const value, Range const & range, int const precision = 2);
	static float       setLineEditFloat(QLineEdit *lineEdit, float const value, ParticleBoundFloat const &bound, int const precision);
	static int         setLineEditInt(QLineEdit *lineEdit, int const value, ParticleBoundInt const &bound);
	static float       getRangeControlPercent(QRangeControl const *rangeControl);
	static float       getFloat(QLineEdit *lineEdit);
	static int         getInt(QLineEdit *lineEdit);
	static float       clamp(float const value, float const max, float const min);
	static float       clampMin(float const value, float const min);
	static float       clampMax(float const value, float const max);
	static bool        isValidIffFile(QWidget *owner, char const *path, Tag const &tag);
	static void        drawColorText(QPainter *painter, int const x, int const y, char const *text, QColor const &normalTextColor);
	static QString     getParticleEffectPath();
	static void        setParticleEffectPath(QString const &path);
	static QString     getParticleAttachmentPath();
	static void        setParticleAttachmentPath(QString const &path);

private:

	// Disabled

	ParticleEditorUtility();
	~ParticleEditorUtility();
};

// ============================================================================

#endif // INCLUDED_ParticleEditorUtility_H
