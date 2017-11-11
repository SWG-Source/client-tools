// ============================================================================
//
// Attribute.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "BaseAttribute.h"
#include "clientParticle/ColorRamp.h"
#include "sharedMath/WaveForm.h"

//-----------------------------------------------------------------------------
class Attribute : public BaseAttribute
{
	Q_OBJECT

public:

	Attribute(QWidget *parent, char const *name);

	void            setText(std::string const &buttonText);
	char const *    getText() const;
	void            setSelected(bool const b);
	void            setModifiable(bool const modifiable, QString const &label);

signals:

	void ensureVisible(int, int, int, int);
	void attributeDoubleClicked(Attribute *);
	void attributeChanged();

private slots:

	void onOptionsPopupMenuActivated(int);
	void slotModifierCheckBoxClicked();

protected:

	bool  isSelected() const;

	QPopupMenu *m_optionsPopupMenu;
	QPixmap     m_pixMap;

	enum
	{
		resetItem = 0,
		saveItem,
		loadItem,
		copyItem,
		pasteItem
	};

private:

	virtual void mouseDoubleClickEvent(QMouseEvent *mouseEvent);
	virtual void mousePressEvent(QMouseEvent *mouseEvent);
	virtual void reset() = 0;
	virtual void load() = 0;
	virtual void save() = 0;
	virtual void copy() = 0;
	virtual void paste() = 0;

	bool    m_selected;
};

//-----------------------------------------------------------------------------
class WaveFormAttribute : public Attribute
{
	Q_OBJECT

public:

	WaveFormAttribute(QWidget *parent, char const *name, float const maxValue, float const minValue);

	WaveForm const &getWaveForm() const;
	WaveForm const &getWaveFormDefault() const;
	void            setWaveForm(WaveForm const &waveForm, bool const sendEmit = true);
	void            setDefaultWaveForm(WaveForm const &defaultWaveForm);

private slots:

	void onAboutToShow();

private:

	virtual void paintEvent(QPaintEvent *paintEvent);
	virtual void reset();
	virtual void load();
	virtual void save();
	virtual void copy();
	virtual void paste();

	WaveForm     m_waveForm;
	WaveForm     m_defaultWaveForm;

	static WaveForm m_bufferedWaveForm;
	static bool     m_waveFormBuffered;

private:

	// Disabled

	WaveFormAttribute(const WaveFormAttribute &);
};

//-----------------------------------------------------------------------------
class ColorRampAttribute : public Attribute
{
	Q_OBJECT

public:

	ColorRampAttribute(QWidget *parent, char const *name);

	ColorRamp const &getColorRamp() const;
	ColorRamp const &getColorRampDefault() const;
	void             setColorRamp(ColorRamp const &colorRamp, WaveForm const &alphaWaveForm, bool const sendEmit = true);
	WaveForm const & getAlphaWaveForm() const;
	void             setDefaultColorRamp(ColorRamp const &defaultColorRamp);

private slots:

	void onAboutToShow();

private:

	virtual void paintEvent(QPaintEvent *paintEvent);
	virtual void reset();
	virtual void load();
	virtual void save();
	virtual void copy();
	virtual void paste();

	ColorRamp m_colorRamp;
	ColorRamp m_defaultColorRamp;
	WaveForm  m_alphaWaveForm;

	static ColorRamp m_bufferedColorRamp;
	static bool      m_colorRampBuffered;

private:

	// Disabled

	ColorRampAttribute(const ColorRampAttribute &);
};

// ============================================================================

#endif // ATTRIBUTE_H