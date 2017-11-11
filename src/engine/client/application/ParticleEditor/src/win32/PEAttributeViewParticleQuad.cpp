// ============================================================================
//
// PEAttributeViewParticleQuad.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "PEAttributeViewParticleQuad.h"
#include "PEAttributeViewParticleQuad.moc"

#include "ParticleAttribute.h"
#include "ParticleNameAttribute.h"
#include "TextureAttribute.h"


///////////////////////////////////////////////////////////////////////////////
//
// PEAttributeViewParticleQuad
//
///////////////////////////////////////////////////////////////////////////////

ParticleBoundFloat PEAttributeViewParticleQuad::m_boundLength(32.0f);
ParticleBoundFloat PEAttributeViewParticleQuad::m_boundWidth(32.0f);
ParticleBoundFloat PEAttributeViewParticleQuad::m_boundRotation(4096.0f);
ParticleBoundFloat PEAttributeViewParticleQuad::m_boundAlpha(1.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleQuad::m_boundSpeedScale(1024.0f);
ParticleBoundFloat PEAttributeViewParticleQuad::m_boundParticleRelativeRotationX(1024.0f);
ParticleBoundFloat PEAttributeViewParticleQuad::m_boundParticleRelativeRotationY(1024.0f);
ParticleBoundFloat PEAttributeViewParticleQuad::m_boundParticleRelativeRotationZ(1024.0f);

//-----------------------------------------------------------------------------
PEAttributeViewParticleQuad::PEAttributeViewParticleQuad(QWidget *parentWidget, char const *name)
 : PEAttributeView(parentWidget, name)
 , m_nameWidget(NULL)
 , m_particleBaseWidget(NULL)
 , m_particleTextureWidget(NULL)
 , m_particleLengthWidget(NULL)
 , m_particleWidthWidget(NULL)
 , m_particleRotationWidget(NULL)
 , m_particleColorWidget(NULL)
 , m_particleAlphaWidget(NULL)
 , m_particleSpeedScaleWidget(NULL)
 , m_particleRelativeRotationXWidget(NULL)
 , m_particleRelativeRotationYWidget(NULL)
 , m_particleRelativeRotationZWidget(NULL)
{
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);

	// Name attribute

	m_nameWidget = new ParticleNameAttribute(m_attributesVBox, "ParticleNameAttribute");
	connect(m_nameWidget->m_nameLineEdit, SIGNAL(textChanged(const QString &)), parentWidget, SLOT(onAttributeNameChanged(const QString &)));
	connect(m_nameWidget->m_nameLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onAttributeNameChanged(const QString &)));

	// Base particle attributes

	m_particleBaseWidget = new ParticleAttribute(m_attributesVBox, "ParticleAttribute");
	addLine(m_attributesVBox);
	connect(m_particleBaseWidget, SIGNAL(signalForceRebuild()), this, SLOT(onAttributeChangedForceRebuild()));
	connect(m_particleBaseWidget, SIGNAL(signalSwitchAttributeView(const PEAttributeViewer::AttributeView)), parentWidget, SLOT(slotSwitchAttributeView(const PEAttributeViewer::AttributeView)));

	// Texture attributes

	m_particleTextureWidget = new TextureAttribute(m_attributesVBox);
	addLine(m_attributesVBox);
	connect(m_particleTextureWidget, SIGNAL(shaderChanged()), this, SLOT(onAttributeChangedForceRebuild()));

	WaveForm waveForm;

	// Length

	addWaveFormAttributeView(m_attributesVBox, &m_particleLengthWidget, "Length", m_boundLength);
	ParticleDescriptionQuad::setDefaultLength(waveForm);
	m_particleLengthWidget->setDefaultWaveForm(waveForm);
	m_particleLengthWidget->setModifiable(true, "Link");

	connect(m_particleLengthWidget->m_modifierCheckBox, SIGNAL(clicked()), this, SLOT(slotParticleLengthWidgetModifiableCheckBoxClicked()));

	// Width

	addWaveFormAttributeView(m_attributesVBox, &m_particleWidthWidget, "Width", m_boundWidth);
	ParticleDescriptionQuad::setDefaultWidth(waveForm);
	m_particleWidthWidget->setDefaultWaveForm(waveForm);

	// Rotation

	addWaveFormAttributeView(m_attributesVBox, &m_particleRotationWidget, "Rotation", m_boundRotation);
	ParticleDescriptionQuad::setDefaultRotation(waveForm);
	m_particleRotationWidget->setDefaultWaveForm(waveForm);

	// Color

	ColorRamp colorRamp;
	addColorRampAttributeView(m_attributesVBox, &m_particleColorWidget, "Color (RGB)");
	ParticleDescription::setDefaultColor(colorRamp);
	m_particleColorWidget->setDefaultColorRamp(colorRamp);

	// Alpha

	addWaveFormAttributeView(m_attributesVBox, &m_particleAlphaWidget, "Alpha", m_boundAlpha);
	ParticleDescription::setDefaultAlpha(waveForm);
	m_particleAlphaWidget->setDefaultWaveForm(waveForm);

	// Speed Scale

	addWaveFormAttributeView(m_attributesVBox, &m_particleSpeedScaleWidget, "Speed Scale", m_boundSpeedScale, false);
	ParticleDescription::setDefaultSpeedScale(waveForm);
	m_particleSpeedScaleWidget->setDefaultWaveForm(waveForm);

	// Relative Rotation

	addWaveFormAttributeView(m_attributesVBox, &m_particleRelativeRotationXWidget, "Local Rotation X", m_boundParticleRelativeRotationX, false);
	ParticleDescription::setDefaultParticleRelativeRotationX(waveForm);
	m_particleRelativeRotationXWidget->setDefaultWaveForm(waveForm);

	addWaveFormAttributeView(m_attributesVBox, &m_particleRelativeRotationYWidget, "Local Rotation Y", m_boundParticleRelativeRotationY, false);
	ParticleDescription::setDefaultParticleRelativeRotationY(waveForm);
	m_particleRelativeRotationYWidget->setDefaultWaveForm(waveForm);

	addWaveFormAttributeView(m_attributesVBox, &m_particleRelativeRotationZWidget, "Local Rotation Z", m_boundParticleRelativeRotationZ, false);
	ParticleDescription::setDefaultParticleRelativeRotationZ(waveForm);
	m_particleRelativeRotationZWidget->setDefaultWaveForm(waveForm);

	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);

	// Tooltip information

	QToolTip::add(m_particleLengthWidget,
	              "Length is the body space y dimension of the particle. If using the Orient\n"
	              "With Velocity emitter parameter, the length specifies the dimension that is\n"
	              "pointing with the movement direction of the particle.");

	QToolTip::add(m_particleWidthWidget,
	             "Width is the body space x dimension of the particle.");

	QToolTip::add(m_particleRotationWidget, "Needs a tooltip");

	QToolTip::add(m_particleColorWidget,
	              "Color specifies the Red, Green, and Blue color components.");

	QToolTip::add(m_particleAlphaWidget,
	              "Alpha controls the fading in and out of the particle.");

	QToolTip::add(m_particleSpeedScaleWidget,
	              "Speed Scale allows the modification of the speed of the particle during the\n"
	              "particle's lifetime. It allows you to speed up and slow down the particle as\n"
	              "desired. A negative speed scale moves the particle in the reverse direction.");

	//selectAttribute(m_particleLengthWidget);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleQuad::slotParticleLengthWidgetModifiableCheckBoxClicked()
{
	if (m_particleLengthWidget->m_modifierCheckBox->isChecked())
	{
		m_particleWidthWidget->setEnabled(false);

		selectAttribute(m_particleLengthWidget);
	}
	else
	{
		m_particleWidthWidget->setEnabled(true);
	}

	emit attributeChangedForceRebuild();
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleQuad::setNewCaption(char const *caption)
{
	char text[256];
	sprintf(text, "Particle Quad (%s)", caption);
	(dynamic_cast<QWidget *>(parent()))->setCaption(text);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleQuad::onAttributeNameChanged(const QString &name)
{
	setNewCaption(name);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleQuad::setParticleDescriptionQuad(ParticleDescriptionQuad const &particleDescriptionQuad)
{
	// Select quad in the particle type combo box

	m_particleBaseWidget->m_particleTypeComboBox->setCurrentItem(0);

	m_nameWidget->m_nameLineEdit->setText(particleDescriptionQuad.getName().c_str());
	m_particleBaseWidget->m_randomRotationDirectionCheckBox->setChecked(particleDescriptionQuad.isRandomRotationDirection());

	// Set the texture properties

	m_particleTextureWidget->setWidget(particleDescriptionQuad);

	// Set the wave form properties

	m_particleLengthWidget->setWaveForm(particleDescriptionQuad.getLength(), false);
	m_particleWidthWidget->setWaveForm(particleDescriptionQuad.getWidth(), false);
	m_particleRotationWidget->setWaveForm(particleDescriptionQuad.getRotation(), false);
	m_particleColorWidget->setColorRamp(particleDescriptionQuad.getColor(), particleDescriptionQuad.getAlpha(), false);
	m_particleAlphaWidget->setWaveForm(particleDescriptionQuad.getAlpha(), false);
	m_particleSpeedScaleWidget->setWaveForm(particleDescriptionQuad.getSpeedScale(), false);
	m_particleRelativeRotationXWidget->setWaveForm(particleDescriptionQuad.getParticleRelativeRotationX(), false);
	m_particleRelativeRotationYWidget->setWaveForm(particleDescriptionQuad.getParticleRelativeRotationY(), false);
	m_particleRelativeRotationZWidget->setWaveForm(particleDescriptionQuad.getParticleRelativeRotationZ(), false);
	
	m_particleLengthWidget->m_modifierCheckBox->setChecked(particleDescriptionQuad.isLengthAndWidthLinked());
	slotParticleLengthWidgetModifiableCheckBoxClicked();

	if (getSelectedAttribute() == NULL)
	{
		selectAttribute(m_particleLengthWidget, false);
	}
	else
	{
		selectAttribute(getSelectedAttribute(), false);
	}
}

//-----------------------------------------------------------------------------
ParticleDescriptionQuad PEAttributeViewParticleQuad::getParticleDescriptionQuad() const
{
	ParticleDescriptionQuad particleDescriptionQuad;

	particleDescriptionQuad.setName(m_nameWidget->m_nameLineEdit->text().latin1());
	particleDescriptionQuad.setRandomRotationDirection(m_particleBaseWidget->m_randomRotationDirectionCheckBox->isChecked());
	particleDescriptionQuad.setRotation(m_particleRotationWidget->getWaveForm());
	particleDescriptionQuad.setLength(m_particleLengthWidget->getWaveForm());
	particleDescriptionQuad.setWidth(m_particleWidthWidget->getWaveForm());
	particleDescriptionQuad.setColor(m_particleColorWidget->getColorRamp());
	particleDescriptionQuad.setAlpha(m_particleAlphaWidget->getWaveForm());
	particleDescriptionQuad.setSpeedScale(m_particleSpeedScaleWidget->getWaveForm());
	particleDescriptionQuad.setParticleRelativeRotationX(m_particleRelativeRotationXWidget->getWaveForm());
	particleDescriptionQuad.setParticleRelativeRotationY(m_particleRelativeRotationYWidget->getWaveForm());
	particleDescriptionQuad.setParticleRelativeRotationZ(m_particleRelativeRotationZWidget->getWaveForm());
	particleDescriptionQuad.setLengthAndWidthLinked(m_particleLengthWidget->m_modifierCheckBox->isChecked());

	int frameCount = 1;

	switch (m_particleTextureWidget->m_frameCountComboBox->currentItem())
	{
		case 1:
			{
				frameCount = 4;
			}
			break;
		case 2:
			{
				frameCount = 16;
			}
			break;
		default:
			{
				DEBUG_FATAL((m_particleTextureWidget->m_frameCountComboBox->currentItem() != 0), ("PEAttributeViewParticleQuad::getParticleDescription() - Invalid frameCount specified"));
			}
			break;
	}

	int const frameStart = m_particleTextureWidget->m_frameStartComboBox->currentItem();
	int const frameEnd = frameStart + m_particleTextureWidget->m_frameEndComboBox->currentItem();
	float framesPerSecond = -1.0f;

	if (m_particleTextureWidget->m_animationTimingComboBox->currentItem() > 0)
	{
		framesPerSecond = atof(static_cast<char const *>(m_particleTextureWidget->m_framesPerSecondComboBox->currentText()));
	}

	bool const textureVisible = m_particleTextureWidget->m_textureVisibleCheckBox->isChecked();

	particleDescriptionQuad.setParticleTexture(ParticleTexture(m_particleTextureWidget->getShaderPath().c_str(), frameCount, frameStart, frameEnd, framesPerSecond, textureVisible));
	//particleDescriptionQuad.m_attachedParticleEffectPath = "appearance/pt_campfire_s01.prt";

	return particleDescriptionQuad;
}

// ============================================================================