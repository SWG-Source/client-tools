// ============================================================================
//
// TextureAttribute.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "FirstParticleEditor.h"
#include "TextureAttribute.h"
#include "TextureAttribute.moc"

#include "clientParticle/ParticleDescriptionQuad.h"
#include "ParticleEditorUtility.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/TreeFile.h"

// ============================================================================
//
// TextureAttribute
//
// ============================================================================

//-----------------------------------------------------------------------------
TextureAttribute::TextureAttribute(QWidget *parent, char const *name)
 : BaseTextureAttribute(parent, name)
 , m_previousFrameStartItem(-1)
 , m_shaderPath("")
{
	connect(m_loadShaderPushButton, SIGNAL(clicked()), this, SLOT(onLoadShaderPushButtonClicked()));
	connect(m_textureVisibleCheckBox, SIGNAL(clicked()), this, SLOT(onShowTextureCheckBoxClicked()));
	connect(m_frameCountComboBox, SIGNAL(activated(int)), this, SLOT(onRebuildParticleDescription()));
	connect(m_frameStartComboBox, SIGNAL(activated(int)), this, SLOT(onRebuildParticleDescription()));
	connect(m_frameEndComboBox, SIGNAL(activated(int)), this, SLOT(onRebuildParticleDescription()));
	connect(m_animationTimingComboBox, SIGNAL(activated(int)), this, SLOT(onRebuildParticleDescription()));
	connect(m_framesPerSecondComboBox, SIGNAL(activated(int)), this, SLOT(onRebuildParticleDescription()));

	// Set initial values

	m_textureVisibleCheckBox->setChecked(true);

	// Build the frames per second combo box

	for (int i = 8; i <= 64; ++i)
	{
		char text[256];
		sprintf(text, "%d", i);
		m_framesPerSecondComboBox->insertItem(text);
	}
}

//-----------------------------------------------------------------------------
void TextureAttribute::onLoadShaderPushButtonClicked()
{
	// Get the last valid path

	QSettings settings;
	settings.insertSearchPath(QSettings::Windows, ParticleEditorUtility::getSearchPath());
	QString result(settings.readEntry("ShaderFileName", "c://"));

	QString shaderPath(QFileDialog::getOpenFileName(result, "Shader files (*.sht)", this, "ShaderFileDialog", "Open Shader"));

	if (!shaderPath.isNull())
	{
		// Save the path

		settings.writeEntry("ShaderFileName", static_cast<char const *>(shaderPath));

		// Save the shader path

		std::string path(shaderPath.latin1());
		FileNameUtils::swapChar(path, '/', '\\');
		setShaderPath(path);

		// Signal the shader changed

		onRebuildParticleDescription();
	}
}

//-----------------------------------------------------------------------------
void TextureAttribute::onShowTextureCheckBoxClicked()
{
	// Rebuild the particle description

	onRebuildParticleDescription();
}

//-----------------------------------------------------------------------------
std::string const &TextureAttribute::getShaderPath() const
{
	return m_shaderPath;
}

//-----------------------------------------------------------------------------
void TextureAttribute::setShaderPath(std::string const &path)
{
	bool result = TreeFile::stripTreeFileSearchPathFromFile(path, m_shaderPath);

	if (!result)
	{
		m_shaderPath = path;
	}

	// Set the filename and extension

	std::string shaderFileName(FileNameUtils::get(m_shaderPath, FileNameUtils::fileName | FileNameUtils::extension));

	m_shaderFileNameLineEdit->setText(shaderFileName.c_str());

	// Set the drive and path

	std::string shaderPath(FileNameUtils::get(m_shaderPath, FileNameUtils::drive | FileNameUtils::directory));

	m_shaderPathLineEdit->setText(shaderPath.c_str());

	// Enable the texture and update the widget

	m_textureVisibleCheckBox->setChecked(true);
}

//-----------------------------------------------------------------------------
void TextureAttribute::onRebuildParticleDescription(int)
{
	validateWidget();
	
	emit shaderChanged();
}

//-----------------------------------------------------------------------------
void TextureAttribute::validateWidget()
{
	m_frameCountComboBox->setEnabled((m_shaderPathLineEdit->text().length() > 0) && m_textureVisibleCheckBox->isChecked());
	m_frameStartComboBox->setEnabled((m_frameCountComboBox->currentItem() > 0) && m_frameCountComboBox->isEnabled());
	m_frameEndComboBox->setEnabled((m_frameCountComboBox->currentItem() > 0) && m_frameCountComboBox->isEnabled());
	m_animationTimingComboBox->setEnabled((m_frameCountComboBox->currentItem() > 0) && m_frameCountComboBox->isEnabled());
	m_framesPerSecondComboBox->setEnabled((m_frameCountComboBox->currentItem() > 0) && (m_animationTimingComboBox->currentItem() > 0) && m_frameCountComboBox->isEnabled());

	// Get the end frame depending on the frame count combo box

	int frameEnd = 1;
	bool forceRebuild = false;

	switch (m_frameCountComboBox->currentItem())
	{
		case 1:
			{
				frameEnd = 4;
			}
			break;
		case 2:
			{
				frameEnd = 16;
			}
			break;
		default:
			{
				DEBUG_FATAL((m_frameCountComboBox->currentItem() != 0), ("TextureAttribute::onRebuildParticleDescription() - Invalid frame count specified."));
			}
			break;
	}

	if (m_previousFrameCountItem != m_frameCountComboBox->currentItem())
	{
		m_previousFrameCountItem = m_frameCountComboBox->currentItem();
		forceRebuild = true;
	}

	// See if we need to rebuild the frame start combo box
	
	if (((m_frameCountComboBox->currentItem() == 0) && (m_frameStartComboBox->count() != 1)) ||
	    ((m_frameCountComboBox->currentItem() == 1) && (m_frameStartComboBox->count() != 4)) ||
	    ((m_frameCountComboBox->currentItem() == 2) && (m_frameStartComboBox->count() != 16)) ||
		(m_frameCountComboBox->currentItem() > 2) ||
		forceRebuild)
	{
		m_frameStartComboBox->clear();

		for (int i = 0; i < frameEnd; ++i)
		{
			char text[256];
			sprintf(text, "%d", i + 1);
			m_frameStartComboBox->insertItem(text);
		}
	}

	// See if we need to rebuild the frame end combo box

	if ((m_previousFrameStartItem != m_frameStartComboBox->currentItem()) || forceRebuild)
	{
		// Store the selected item

		int const currentFrameStart = m_frameStartComboBox->currentItem();
		int const currentFrameEnd = m_previousFrameStartItem + m_frameEndComboBox->currentItem();

		// Remove all the items

		m_frameEndComboBox->clear();

		// Add in the new item count

		for (int i = currentFrameStart; i < frameEnd; ++i)
		{
			char text[256];
			sprintf(text, "%d", i + 1);
			m_frameEndComboBox->insertItem(text);
		}

		// See if we can reset the previous selected frame end item

		int const newCurrentItem = currentFrameEnd - currentFrameStart;

		if ((newCurrentItem >= 0) &&
		    (newCurrentItem < m_frameEndComboBox->count()))
		{
			m_frameEndComboBox->setCurrentItem(newCurrentItem);
		}
		else
		{
			m_frameEndComboBox->setCurrentItem(0);
		}

		// Store the frame start item
		
		if (m_previousFrameStartItem != m_frameStartComboBox->currentItem())
		{
			m_previousFrameStartItem = m_frameStartComboBox->currentItem();
		}
	}
}

//-----------------------------------------------------------------------------
void TextureAttribute::resetWidget()
{
	m_frameCountComboBox->setCurrentItem(0);
	m_frameStartComboBox->setCurrentItem(0);
	m_frameEndComboBox->setCurrentItem(0);
	m_animationTimingComboBox->setCurrentItem(0);
	m_framesPerSecondComboBox->setCurrentItem(0);

	validateWidget();
}

//-----------------------------------------------------------------------------
void TextureAttribute::setWidget(ParticleDescriptionQuad const &particleDescriptionQuad)
{
	setShaderPath(particleDescriptionQuad.getParticleTexture().getShaderPath().getString());

	if (particleDescriptionQuad.getParticleTexture().getShaderPath().isEmpty())
	{
		// If the shader filename is empty, then set the default texture widget states

		resetWidget();
	}
	else
	{
		// Since there is a shader filename, enable set all the texture wiget states

		switch (particleDescriptionQuad.getParticleTexture().getFrameCount())
		{
			case 1:
				{
					m_frameCountComboBox->setCurrentItem(0);
				}
				break;
			case 4:
				{
					m_frameCountComboBox->setCurrentItem(1);
				}
				break;
			case 16:
				{
					m_frameCountComboBox->setCurrentItem(2);
				}
				break;
			default:
			{
			}
		}

		const int frameStart = particleDescriptionQuad.getParticleTexture().getFrameStart();

		m_frameStartComboBox->setCurrentItem(frameStart);

		validateWidget();

		const int frameEnd = particleDescriptionQuad.getParticleTexture().getFrameEnd();

		m_frameEndComboBox->setCurrentItem(frameEnd - frameStart);
	}

	if (particleDescriptionQuad.getParticleTexture().getFramesPerSecond() < 0.0f)
	{
		// Match particle lifetime

		m_animationTimingComboBox->setCurrentItem(0);
		m_framesPerSecondComboBox->setCurrentItem(0);
	}
	else
	{
		// Use the specified frames per second

		m_animationTimingComboBox->setCurrentItem(1);
		int const framesPerSecond = static_cast<int>(particleDescriptionQuad.getParticleTexture().getFramesPerSecond());
		m_framesPerSecondComboBox->setCurrentItem(framesPerSecond - 8);
	}

	validateWidget();
}

// ============================================================================
