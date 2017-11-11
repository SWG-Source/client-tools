// ============================================================================
//
// ParticleAttribute.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "ParticleAttribute.h"
#include "ParticleAttribute.moc"

// ============================================================================
//
// ParticleAttribute
//
// ============================================================================

//-----------------------------------------------------------------------------
ParticleAttribute::ParticleAttribute(QWidget *parent, char const *name)
 : BaseParticleAttribute(parent, name)
{
    connect(m_randomRotationDirectionCheckBox, SIGNAL(clicked()), this, SLOT(slotForceRebuild()));
    connect(m_particleTypeComboBox, SIGNAL(activated(int)), this, SLOT(slotSwitchAttributeView(int)));
}

//-----------------------------------------------------------------------------
void ParticleAttribute::slotForceRebuild()
{
	emit signalForceRebuild();
}

//-----------------------------------------------------------------------------
void ParticleAttribute::slotSwitchAttributeView(int)
{
	switch (m_particleTypeComboBox->currentItem())
	{
		case 0:
			{
				emit signalSwitchAttributeView(PEAttributeViewer::AV_particleQuad);
			}
			break;
		case 1:
			{
				emit signalSwitchAttributeView(PEAttributeViewer::AV_particleMesh);
			}
			break;
		default:
			{
				DEBUG_FATAL(true, ("ParticleAttribute::slotSwitchAttributeView() - Invalid attribute view specified."));
			}
			break;
	}
}

// ============================================================================