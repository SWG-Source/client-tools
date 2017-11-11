// ============================================================================
//
// PEAttributeWidgetMesh.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef PEATTRIBUTEWIDGETMESH_H
#define PEATTRIBUTEWIDGETMESH_H

#include "PEBaseAttributeWidgetMesh.h"

#include <string>

class ParticleDescriptionMesh;

//-----------------------------------------------------------------------------
class PEAttributeWidgetMesh : public PEBaseAttributeWidgetMesh
{
	Q_OBJECT

public:

	PEAttributeWidgetMesh(QWidget *parentWidget, char const *name);

	std::string const &getMeshPath() const;
	void               setMeshPath(std::string const &path);
	void               setWidget(ParticleDescriptionMesh const &particleDescriptionMesh);

public slots:

	void slotLoadMeshPushButtonClicked();
	void slotRebuildParticleDescription();

signals:

	void signalMeshChanged();

protected:

private:

	std::string m_meshPath;
};

// ============================================================================

#endif // PEATTRIBUTEWIDGETMESH_H