// ============================================================================
//
// PEAttributeViewParticleMesh.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "PEAttributeViewParticleMesh.h"
#include "PEAttributeViewParticleMesh.moc"

#include "ParticleAttribute.h"
#include "ParticleNameAttribute.h"
#include "PEAttributeWidgetMesh.h"
#include "TextureAttribute.h"

///////////////////////////////////////////////////////////////////////////////
//
// PEAttributeViewParticleMesh
//
///////////////////////////////////////////////////////////////////////////////

ParticleBoundFloat PEAttributeViewParticleMesh::m_boundScale(128.0f, 0.01f);
ParticleBoundFloat PEAttributeViewParticleMesh::m_boundRotation(4096.0f);
ParticleBoundFloat PEAttributeViewParticleMesh::m_boundAlpha(1.0f, 0.0f);
ParticleBoundFloat PEAttributeViewParticleMesh::m_boundSpeedScale(1024.0f);

//-----------------------------------------------------------------------------
PEAttributeViewParticleMesh::PEAttributeViewParticleMesh(QWidget *parentWidget, char const *name)
 : PEAttributeView(parentWidget, name)
 , m_nameWidget(NULL)
 , m_particleBaseWidget(NULL)
 , m_attributeWidgetMesh(NULL)
 , m_particleScaleWidget(NULL)
 , m_particleRotationXWidget(NULL)
 , m_particleRotationYWidget(NULL)
 , m_particleRotationZWidget(NULL)
 , m_particleColorWidget(NULL)
 , m_particleAlphaWidget(NULL)
 , m_particleSpeedScaleWidget(NULL)
{
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);

	// Name attribute

	m_nameWidget = new ParticleNameAttribute(m_attributesVBox, "ParticleNameAttribute");
	connect(m_nameWidget->m_nameLineEdit, SIGNAL(textChanged(const QString &)), parentWidget, SLOT(onAttributeNameChanged(const QString &)));
	connect(m_nameWidget->m_nameLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onAttributeNameChanged(const QString &)));
	m_nameWidget->m_nameLineEdit->setText("Default");

	// Base particle attributes

	m_particleBaseWidget = new ParticleAttribute(m_attributesVBox, "ParticleAttribute");
	addLine(m_attributesVBox);
	connect(m_particleBaseWidget, SIGNAL(signalForceRebuild()), this, SLOT(onAttributeChangedForceRebuild()));
	connect(m_particleBaseWidget, SIGNAL(signalSwitchAttributeView(const PEAttributeViewer::AttributeView)), parentWidget, SLOT(slotSwitchAttributeView(const PEAttributeViewer::AttributeView)));

	// Mesh attributes

	m_attributeWidgetMesh = new PEAttributeWidgetMesh(m_attributesVBox, "PEAttributeWidgetMesh");
	addLine(m_attributesVBox);
	connect(m_attributeWidgetMesh, SIGNAL(signalMeshChanged()), this, SLOT(onAttributeChangedForceRebuild()));

	WaveForm waveForm;

	// Scale

	addWaveFormAttributeView(m_attributesVBox, &m_particleScaleWidget, "Scale", m_boundScale);
	ParticleDescriptionMesh::setDefaultScale(waveForm);
	m_particleScaleWidget->setDefaultWaveForm(waveForm);

	// Rotation X

	addWaveFormAttributeView(m_attributesVBox, &m_particleRotationXWidget, "Rotation X", m_boundRotation);
	ParticleDescriptionMesh::setDefaultRotationX(waveForm);
	m_particleRotationXWidget->setDefaultWaveForm(waveForm);

	// Rotation Y

	addWaveFormAttributeView(m_attributesVBox, &m_particleRotationYWidget, "Rotation Y", m_boundRotation);
	ParticleDescriptionMesh::setDefaultRotationY(waveForm);
	m_particleRotationYWidget->setDefaultWaveForm(waveForm);

	// Rotation Z

	addWaveFormAttributeView(m_attributesVBox, &m_particleRotationZWidget, "Rotation Z", m_boundRotation);
	ParticleDescriptionMesh::setDefaultRotationZ(waveForm);
	m_particleRotationZWidget->setDefaultWaveForm(waveForm);

	// Color
	
	ColorRamp colorRamp;
	addColorRampAttributeView(m_attributesVBox, &m_particleColorWidget, "Color (RGB)");
	ParticleDescription::setDefaultColor(colorRamp);
	m_particleColorWidget->setDefaultColorRamp(colorRamp);
	m_particleColorWidget->setEnabled(false);
	
	// Alpha
	
	addWaveFormAttributeView(m_attributesVBox, &m_particleAlphaWidget, "Alpha", m_boundAlpha);
	ParticleDescription::setDefaultAlpha(waveForm);
	m_particleAlphaWidget->setDefaultWaveForm(waveForm);
	m_particleAlphaWidget->setEnabled(false);

	// Speed Scale

	addWaveFormAttributeView(m_attributesVBox, &m_particleSpeedScaleWidget, "Speed Scale", m_boundSpeedScale, false);
	ParticleDescription::setDefaultSpeedScale(waveForm);
	m_particleSpeedScaleWidget->setDefaultWaveForm(waveForm);

	addLine(m_attributesVBox);
	addLine(m_attributesVBox);
	addLine(m_attributesVBox);

	// Tooltip information

	//QToolTip::add(m_particleScaleWidget,
	//              "\n"
	//              "");
	//
	//QToolTip::add(m_particleRotationXWidget,
	//              "\n"
	//              "");
	//
	//QToolTip::add(m_particleRotationYWidget,
	//              "\n"
	//              "");
	//
	//QToolTip::add(m_particleRotationZWidget,
	//              "\n"
	//              "");
	//
	//QToolTip::add(m_particleColorWidget,
	//              "\n"
	//              "");
	//
	//QToolTip::add(m_particleAlphaWidget,
	//              "\n"
	//              "");
	//
	//QToolTip::add(m_particleSpeedScaleWidget,
	//              "\n"
	//              "");

	//selectAttribute(m_particleLengthWidget);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleMesh::setNewCaption(char const *caption)
{
	char text[256];
	sprintf(text, "Particle Mesh (%s)", caption);
	(dynamic_cast<QWidget *>(parent()))->setCaption(text);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleMesh::onAttributeNameChanged(const QString &name)
{
	setNewCaption(name);
}

//-----------------------------------------------------------------------------
void PEAttributeViewParticleMesh::setParticleDescriptionMesh(ParticleDescriptionMesh const &particleDescriptionMesh)
{
	// Select mesh in the particle type combo box

	m_particleBaseWidget->m_particleTypeComboBox->setCurrentItem(1);

	m_nameWidget->m_nameLineEdit->setText(particleDescriptionMesh.getName().c_str());
	m_particleBaseWidget->m_randomRotationDirectionCheckBox->setChecked(particleDescriptionMesh.isRandomRotationDirection());
	m_attributeWidgetMesh->setWidget(particleDescriptionMesh);

	// Set the wave form properties

	m_particleScaleWidget->setWaveForm(particleDescriptionMesh.m_scale, false);
	m_particleRotationXWidget->setWaveForm(particleDescriptionMesh.m_rotationX, false);
	m_particleRotationYWidget->setWaveForm(particleDescriptionMesh.m_rotationY, false);
	m_particleRotationZWidget->setWaveForm(particleDescriptionMesh.m_rotationZ, false);
	//m_particleColorWidget->setColorRamp(particleDescriptionMesh.m_color, particleDescriptionMesh.m_alpha, false);
	//m_particleAlphaWidget->setWaveForm(particleDescriptionMesh.m_alpha, false);
	m_particleSpeedScaleWidget->setWaveForm(particleDescriptionMesh.getSpeedScale(), false);

	if (getSelectedAttribute() == NULL)
	{
		selectAttribute(m_particleScaleWidget, false);
	}
	else
	{
		selectAttribute(getSelectedAttribute(), false);
	}
}

//-----------------------------------------------------------------------------
ParticleDescriptionMesh PEAttributeViewParticleMesh::getParticleDescriptionMesh() const
{
	ParticleDescriptionMesh particleDescriptionMesh;

	std::string name = m_nameWidget->m_nameLineEdit->text();
	particleDescriptionMesh.setName(name.c_str());
	particleDescriptionMesh.setMeshPath(m_attributeWidgetMesh->getMeshPath().c_str());
	particleDescriptionMesh.setRandomRotationDirection(m_particleBaseWidget->m_randomRotationDirectionCheckBox->isChecked());
	particleDescriptionMesh.m_scale = m_particleScaleWidget->getWaveForm();
	particleDescriptionMesh.m_rotationX = m_particleRotationXWidget->getWaveForm();
	particleDescriptionMesh.m_rotationY = m_particleRotationYWidget->getWaveForm();
	particleDescriptionMesh.m_rotationZ = m_particleRotationZWidget->getWaveForm();
	particleDescriptionMesh.setColor(m_particleColorWidget->getColorRampDefault());
	particleDescriptionMesh.setAlpha(m_particleAlphaWidget->getWaveFormDefault());
	particleDescriptionMesh.setSpeedScale(m_particleSpeedScaleWidget->getWaveForm());
	//particleDescriptionMesh.m_attachedParticleEffectPath = "appearance/pt_campfire_s01.prt";

	return particleDescriptionMesh;
}

// ============================================================================