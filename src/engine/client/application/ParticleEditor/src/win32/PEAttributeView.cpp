// ============================================================================
//
// PEAttributeView.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "PEAttributeView.h"
#include "PEAttributeView.moc"

///////////////////////////////////////////////////////////////////////////////
//
// PEAttributeView
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
PEAttributeView::PEAttributeView(QWidget *parentWidget, char const *name)
 : QScrollView(parentWidget, name)
 , m_attributesVBox(NULL)
 , m_attributeSelected(NULL)
{
	setVScrollBarMode(QScrollView::AlwaysOn);

	m_attributesVBox = new QVBox(viewport());
	addChild(m_attributesVBox);
}

//-----------------------------------------------------------------------------
void PEAttributeView::setAttributeColorRamp(ColorRamp const &colorRamp, WaveForm const &alphaWaveForm)
{
	ColorRampAttribute *attribute = dynamic_cast<ColorRampAttribute *>(m_attributeSelected);

	if (attribute != NULL)
	{
		attribute->setColorRamp(colorRamp, alphaWaveForm, false);
	}
}

//-----------------------------------------------------------------------------
void PEAttributeView::setAttributeWaveForm(WaveForm const &waveForm)
{
	WaveFormAttribute *attribute = dynamic_cast<WaveFormAttribute *>(m_attributeSelected);

	if (attribute != NULL)
	{
		attribute->setWaveForm(waveForm, false);
	}
}

//-----------------------------------------------------------------------------
void PEAttributeView::addLine(QWidget *parent)
{
	QFrame *line = new QFrame(parent);

	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
}

//-----------------------------------------------------------------------------
void PEAttributeView::addWaveFormAttributeView(QWidget *parentWidget, WaveFormAttribute **attribute, std::string const &label, ParticleBoundFloat const &bound, bool followWithLine)
{
	// Create the child widget

	delete (*attribute);
	(*attribute) = new WaveFormAttribute(parentWidget, label.c_str(), bound.m_max, bound.m_min);
	(*attribute)->setText(label.c_str());

	// Setup the rest of the attribute view

	setupAttributeView(parentWidget, (*attribute), followWithLine);
}

//-----------------------------------------------------------------------------
void PEAttributeView::addColorRampAttributeView(QWidget *parentWidget, ColorRampAttribute **attribute, std::string const &label, bool followWithLine)
{
	// Create the child widget

	delete (*attribute);
	(*attribute) = new ColorRampAttribute(parentWidget, label.c_str());
	(*attribute)->setText(label.c_str());

	// Setup the rest of the attribute view

	setupAttributeView(parentWidget, (*attribute), followWithLine);
}

//-----------------------------------------------------------------------------
void PEAttributeView::setupAttributeView(QWidget *parentWidget, Attribute *attribute, bool followWithLine)
{
	// The attribute signals this that it was double clicked
	
	connect(attribute, SIGNAL(attributeDoubleClicked(Attribute *)), this, SLOT(onAttributeDoubleClicked(Attribute *)));

	// The attribute signals this to make sure that it is visible
	
	connect(attribute, SIGNAL(ensureVisible(int, int, int, int)), this, SLOT(onEnsureVisible(int, int, int, int)));

	// The attribue signals that it has changed so rebuild the particle effect

	connect(attribute, SIGNAL(attributeChanged()), this, SLOT(onAttributeChangedForceRebuild()));

	// Whether to add a line after the widget

	if (followWithLine)
	{
		addLine(parentWidget);
	}
}

//-----------------------------------------------------------------------------
void PEAttributeView::onAttributeDoubleClicked(Attribute *attribute)
{
	NOT_NULL(attribute);

	selectAttribute(attribute);
}

//-----------------------------------------------------------------------------
void PEAttributeView::onEnsureVisible(int x, int y, int width, int height)
{
	ensureVisible(x, y, width, height);
}

//-----------------------------------------------------------------------------
void PEAttributeView::onAttributeChangedForceRebuild()
{
	emit attributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeView::onAttributeChangedForceRebuild(int)
{
	emit attributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeView::selectAttribute(Attribute *attribute, bool const makeSureVisible)
{
	if (attribute != NULL)
	{
		// Deselect the currently selected attribute

		if (m_attributeSelected != NULL)
		{
			m_attributeSelected->setSelected(false);
		}

		// Select the new selected attribute

		m_attributeSelected = attribute;
		m_attributeSelected->setSelected(true);

		if (makeSureVisible)
		{
			// Make sure the selected attribute is visible

			ensureVisible(m_attributeSelected->pos().x(), m_attributeSelected->pos().y(), m_attributeSelected->width(), m_attributeSelected->height());
		}

		// Tell the attribute editor to change to the correct attribute

		WaveFormAttribute *waveFormAttribute = dynamic_cast<WaveFormAttribute *>(attribute);

		if (waveFormAttribute != NULL)
		{
			emit waveFormAttributeSelected(waveFormAttribute->getWaveForm());
		}
		else
		{
			ColorRampAttribute *colorRampAttribute = dynamic_cast<ColorRampAttribute *>(attribute);

			if (colorRampAttribute != NULL)
			{
				emit colorRampAttributeSelected(colorRampAttribute->getColorRamp(), colorRampAttribute->getAlphaWaveForm());
			}
		}
	}
}

//-----------------------------------------------------------------------------
Attribute *PEAttributeView::getSelectedAttribute() const
{
	return m_attributeSelected;
}

// ============================================================================