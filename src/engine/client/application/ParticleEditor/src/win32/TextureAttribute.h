// ============================================================================
//
// TextureAttribute.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef TEXTUREATTRIBUTE_H
#define TEXTUREATTRIBUTE_H

#include "BaseTextureAttribute.h"

class ParticleDescriptionQuad;

//-----------------------------------------------------------------------------
class TextureAttribute : public BaseTextureAttribute
{
	Q_OBJECT

public:

	TextureAttribute(QWidget *parent = NULL, char const *name = "");

	std::string const &getShaderPath() const;
	void               setWidget(ParticleDescriptionQuad const &particleDescriptionQuad);
	void               resetWidget();
	void               validateWidget();

signals:

	void shaderChanged();

private slots:

	void onLoadShaderPushButtonClicked();
	void onShowTextureCheckBoxClicked();
	void onRebuildParticleDescription(int dummy = -1);

private:

	void        setShaderPath(std::string const &path);

	int         m_previousFrameCountItem;
	int         m_previousFrameStartItem;
	std::string m_shaderPath;
};

// ============================================================================

#endif // TEXTUREATTRIBUTE_H