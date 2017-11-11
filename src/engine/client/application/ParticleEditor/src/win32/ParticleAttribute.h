// ============================================================================
//
// ParticleAttribute.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef PARTICLEATTRIBUTE_H
#define PARTICLEATTRIBUTE_H

#include "BaseParticleAttribute.h"
#include "PEAttributeViewer.h"

//-----------------------------------------------------------------------------
class ParticleAttribute : public BaseParticleAttribute
{
	Q_OBJECT

public:

	ParticleAttribute(QWidget *parent, char const *name);

signals:

	void signalForceRebuild();
	void signalSwitchAttributeView(const PEAttributeViewer::AttributeView);

private slots:

	void slotForceRebuild();
	void slotSwitchAttributeView(int = 0);
};

// ============================================================================

#endif // PARTICLEATTRIBUTE_H
