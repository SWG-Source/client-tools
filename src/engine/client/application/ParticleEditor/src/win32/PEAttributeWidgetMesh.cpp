// ============================================================================
//
// PEAttributeWidgetMesh.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "PEAttributeWidgetMesh.h"
#include "PEAttributeWidgetMesh.moc"

#include "clientAudio/Audio.h"
#include "clientParticle/ParticleDescriptionMesh.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/TreeFile.h"


///////////////////////////////////////////////////////////////////////////////
//
// PEAttributeWidgetMesh
//
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
PEAttributeWidgetMesh::PEAttributeWidgetMesh(QWidget *parentWidget, char const *name)
 : PEBaseAttributeWidgetMesh(parentWidget, name)
{
	connect(m_loadMeshPushButton, SIGNAL(clicked()), this, SLOT(slotLoadMeshPushButtonClicked()));

	setMeshPath("appearance/defaultappearance.msh");
}

//-----------------------------------------------------------------------------
void PEAttributeWidgetMesh::slotLoadMeshPushButtonClicked()
{
	// Get the last valid path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, ParticleEditorUtility::getSearchPath());
	QString result(settings.readEntry("MeshFileName", "c://"));

	QString meshPath(QFileDialog::getOpenFileName(result, "Any Appearance File (*.msh *.apt *.sat)", this, "MeshFileDialog", "Open Mesh"));

	if (!meshPath.isNull())
	{
		// Save the path

		settings.writeEntry("MeshFileName", static_cast<char const *>(meshPath));

		// Save the shader path

		std::string path(meshPath.latin1());
		FileNameUtils::swapChar(path, '/', '\\');
		setMeshPath(path);

		// Signal the shader changed

		slotRebuildParticleDescription();
	}
}

//-----------------------------------------------------------------------------
std::string const &PEAttributeWidgetMesh::getMeshPath() const
{
	return m_meshPath;
}

//-----------------------------------------------------------------------------
void PEAttributeWidgetMesh::setMeshPath(std::string const &path)
{
	bool result = TreeFile::stripTreeFileSearchPathFromFile(path, m_meshPath);

	if (!result)
	{
		m_meshPath = path;
	}

	// Set the filename and extension

	m_meshFileNameLineEdit->setText(FileNameUtils::get(m_meshPath, FileNameUtils::fileName | FileNameUtils::extension).c_str());

	// Set the drive and path

	m_meshPathLineEdit->setText(FileNameUtils::get(m_meshPath, FileNameUtils::drive | FileNameUtils::directory).c_str());
}

//-----------------------------------------------------------------------------
void PEAttributeWidgetMesh::slotRebuildParticleDescription()
{
	emit signalMeshChanged();
}

//-----------------------------------------------------------------------------
void PEAttributeWidgetMesh::setWidget(ParticleDescriptionMesh const &particleDescriptionMesh)
{
	setMeshPath(particleDescriptionMesh.getMeshPath().getString());
}

// ============================================================================
