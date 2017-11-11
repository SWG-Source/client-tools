// ============================================================================
//
// PEAttributeView.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef PEATTRIBUTEVIEW_H
#define PEATTRIBUTEVIEW_H

#include "Attribute.h"

class ColorRamp;
class WaveForm;

//-----------------------------------------------------------------------------
class PEAttributeView : public QScrollView
{
	Q_OBJECT

public:

	PEAttributeView(QWidget *parentWidget, char const *name);

	void       addWaveFormAttributeView(QWidget *parentWidget, WaveFormAttribute **attribute, std::string const &label, ParticleBoundFloat const &bound, bool const followWithLine = true);
	void       addColorRampAttributeView(QWidget *parentWidget, ColorRampAttribute **attribute, std::string const &label, bool const followWithLine = true);
	void       setupAttributeView(QWidget *parentWidget, Attribute *attribute, bool const followWithLine = true);
	void       setAttributeColorRamp(ColorRamp const &colorRamp, WaveForm const &alphaWaveForm);
	void       setAttributeWaveForm(WaveForm const &waveForm);
	void       addLine(QWidget *parentWidget);
	Attribute *getSelectedAttribute() const;

public slots:

	void onAttributeDoubleClicked(Attribute *attribute);
	void onEnsureVisible(int, int, int, int);
	void onAttributeChangedForceRebuild();
	void onAttributeChangedForceRebuild(int);

signals:

	void colorRampAttributeSelected(const ColorRamp &, const WaveForm &);
	void waveFormAttributeSelected(const WaveForm &);
	void attributeChangedForceRebuild();

protected:

	void   selectAttribute(Attribute *attribute, bool const makeSureVisible = true);

	QVBox *m_attributesVBox;

private:

	virtual void setNewCaption(char const *) {}

	Attribute *m_attributeSelected;
};

// ============================================================================

#endif // PEATTRIBUTEVIEW_H