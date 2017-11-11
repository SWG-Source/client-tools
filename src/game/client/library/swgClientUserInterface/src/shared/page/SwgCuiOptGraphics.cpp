//======================================================================
//
// SwgCuiOptGraphics.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptGraphics.h"

#include "UICheckbox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "clientAnimation/ConfigClientAnimation.h"
#include "clientGame/Bloom.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/FreeChaseCamera.h"
#include "clientGame/Game.h"
#include "clientGame/GroundScene.h"
#include "clientGame/NebulaManagerClient.h"
#include "clientGame/WorldSnapshot.h"
#include "clientGraphics/ConfigClientGraphics.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/PostProcessingEffectsManager.h"
#include "clientGraphics/ShaderCapability.h"
#include "clientGraphics/ShaderPrimitiveSorter.h"
#include "clientObject/ConfigClientObject.h"
#include "clientObject/DetailAppearance.h"
#include "clientObject/ShadowManager.h"
#include "clientObject/ShadowVolume.h"
#include "clientParticle/ConfigClientParticle.h"
#include "clientParticle/ParticleEffectAppearance.h"
#include "clientSkeletalAnimation/CharacterLodManager.h"
#include "clientSkeletalAnimation/ConfigClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/ConfigClientUserInterface.h"
#include "clientUserInterface/CuiPreferences.h"


//======================================================================

namespace SwgCuiOptGraphicsNamespace
{

	void setScreenShotTypeBmp (bool screenShotTypeBmp)
	{
		if (screenShotTypeBmp)
			Graphics::setScreenShotFormat (GSSF_bmp);
	}

	bool getScreenShotTypeBmp ()
	{
		return Graphics::getScreenShotFormat () == GSSF_bmp;
	}

	void setScreenShotTypeTga (bool screenShotTypeTga)
	{
		if (screenShotTypeTga)
			Graphics::setScreenShotFormat (GSSF_tga);
	}

	bool getScreenShotTypeTga ()
	{
		return Graphics::getScreenShotFormat () == GSSF_tga;
	}

	void setScreenShotTypeJpg (bool screenShotTypeJpg)
	{
		if (screenShotTypeJpg)
			Graphics::setScreenShotFormat (GSSF_jpg);
	}

	bool getScreenShotTypeJpg ()
	{
		return Graphics::getScreenShotFormat () == GSSF_jpg;
	}

	int getDefaultScreenShotQuality ()
	{
		return 100;
	}

	void setEnableBatchRenderer (bool enableBatchRenderer)
	{
		bool enabled = true;
		float screenFraction = 0.f;
		SkeletalAppearance2::getBatchRenderScreenFraction (enabled, screenFraction);
		SkeletalAppearance2::setBatchRenderScreenFraction(enableBatchRenderer, screenFraction);
	}

	bool getEnableBatchRenderer ()
	{
		bool enabled = true;
		float screenFraction = 0.f;
		SkeletalAppearance2::getBatchRenderScreenFraction (enabled, screenFraction);

		return enabled;
	}

	void setEnableCharacterLodManager (bool enableCharacterLodManager)
	{
		bool enabled = ConfigClientSkeletalAnimation::getLodManagerEnable ();
		float firstLodCount = ConfigClientSkeletalAnimation::getLodManagerFirstLodCount ();
		int everyOtherFrameSkinningCharacterCount = ConfigClientSkeletalAnimation::getLodManagerEveryOtherFrameSkinningCharacterCount ();
		int hardSkinningCharacterCount = ConfigClientSkeletalAnimation::getLodManagerHardSkinningCharacterCount ();
		CharacterLodManager::getConfiguration (enabled, firstLodCount, everyOtherFrameSkinningCharacterCount, hardSkinningCharacterCount);
		CharacterLodManager::setConfiguration (enableCharacterLodManager, firstLodCount, everyOtherFrameSkinningCharacterCount, hardSkinningCharacterCount);
	}

	bool getEnableCharacterLodManager ()
	{
		bool enabled = ConfigClientSkeletalAnimation::getLodManagerEnable ();
		float firstLodCount = ConfigClientSkeletalAnimation::getLodManagerFirstLodCount ();
		int everyOtherFrameSkinningCharacterCount = ConfigClientSkeletalAnimation::getLodManagerEveryOtherFrameSkinningCharacterCount ();
		int hardSkinningCharacterCount = ConfigClientSkeletalAnimation::getLodManagerHardSkinningCharacterCount ();
		CharacterLodManager::getConfiguration (enabled, firstLodCount, everyOtherFrameSkinningCharacterCount, hardSkinningCharacterCount);

		return enabled;
	}

	void setParticleLodBias (float particleLodBias)
	{
		ParticleEffectAppearance::setGlobalLodBias (particleLodBias);
	}
	
	float getParticleLodBias ()
	{
		return ParticleEffectAppearance::getGlobalLodBias ();
	}
	
	void setNebulaDensity(float density)
	{
		CuiPreferences::setGlobalNebulaDensity(density);
		NebulaManagerClient::Config::setRegenerate(true);
	}

	void setParticleUserLimit(int userLimit)
	{
		ParticleEffectAppearance::setGlobalUserLimit(userLimit);
	}

	int getParticleUserLimit()
	{
		return ParticleEffectAppearance::getGlobalUserLimit();
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace SwgCuiOptGraphicsNamespace;

//======================================================================

SwgCuiOptGraphics::SwgCuiOptGraphics (UIPage & page) :
SwgCuiOptBase      ("SwgCuiOptGraphics", page),
m_sliderBrightness (0),
m_sliderContrast   (0),
m_sliderGamma      (0)
{
	UISliderbar * slider = 0;
	UICheckbox * checkbox = 0;
	getCodeDataObject (TUISliderbar, m_sliderBrightness, "brightnessSlider");
	registerSlider (*m_sliderBrightness, Game::setBrightness, Game::getBrightness, Game::getDefaultBrightness, 0.5f, 1.5f);

	getCodeDataObject (TUISliderbar, m_sliderContrast, "contrastSlider");
	registerSlider (*m_sliderContrast, Game::setContrast, Game::getContrast, Game::getDefaultContrast, 0.5f, 1.5f);

	getCodeDataObject (TUISliderbar, m_sliderGamma, "gammaSlider");
	registerSlider (*m_sliderGamma, Game::setGamma, Game::getGamma, Game::getDefaultGamma, 0.5f, 1.5f);

	getCodeDataObject (TUISliderbar, slider, "sliderFov");
	registerSlider (*slider, GroundScene::setCameraFieldOfViewDegrees, GroundScene::getCameraFieldOfViewDegrees, ConfigClientGame::getCameraFieldOfView, 60.0f, 120.0f);

	getCodeDataObject (TUISliderbar, slider, "sliderFarPlane");
	registerSlider (*slider, GroundScene::setCameraFarPlane, GroundScene::getCameraFarPlane, ConfigClientGame::getCameraFarPlane, 1024.0f, 4096.0f);

	getCodeDataObject (TUISliderbar, slider, "sliderStaticLodBias");
	registerSlider (*slider, DetailAppearance::setDetailLevelBias, DetailAppearance::getDetailLevelBias, ConfigClientObject::getDetailLevelBias, 0.5f, 2.0f);

	getCodeDataObject (TUISliderbar, slider, "sliderCharacterLodBias");
	registerSlider (*slider, SkeletalAppearance2::setDetailLevelBias, SkeletalAppearance2::getDetailLevelBias, ConfigClientObject::getDetailLevelBias, 0.5f, 2.0f);

	getCodeDataObject (TUISliderbar, slider, "sliderParticleLodBias");
	registerSlider (*slider, setParticleLodBias, getParticleLodBias, ConfigClientObject::getDetailLevelBias, 0.25f, 1.0f);

	getCodeDataObject (TUISliderbar, slider, "sliderNebulaDensity");
	registerSlider (*slider, SwgCuiOptGraphicsNamespace::setNebulaDensity, CuiPreferences::getGlobalNebulaDensity, CuiPreferences::getGlobalNebulaDensityDefault, 1.0f, 120.0f);

	getCodeDataObject (TUICheckbox, checkbox, "checkRenderShadows");
	registerCheckbox (*checkbox, ShadowManager::setEnabled, ShadowManager::getEnabled, ShadowManager::getEnabledDefault);

	getCodeDataObject (TUICheckbox, checkbox, "radioShadowsCharNone");
	registerCheckbox (*checkbox, ShadowManager::setSkeletalShadowsNone, ShadowManager::getSkeletalShadowsNone, ShadowManager::getSkeletalShadowsNoneDefault);

	getCodeDataObject (TUICheckbox, checkbox, "radioShadowsCharSimple");
	registerCheckbox (*checkbox, ShadowManager::setSkeletalShadowsSimple, ShadowManager::getSkeletalShadowsSimple, ShadowManager::getSkeletalShadowsSimpleDefault);

	getCodeDataObject (TUICheckbox, checkbox, "radioShadowsCharVolume");
	registerCheckbox (*checkbox, ShadowManager::setSkeletalShadowsVolumetric, ShadowManager::getSkeletalShadowsVolumetric, ShadowManager::getSkeletalShadowsVolumetricDefault);

	getCodeDataObject (TUICheckbox, checkbox, "radioShadowsNonCharNone");
	registerCheckbox (*checkbox, ShadowManager::setMeshShadowsNone, ShadowManager::getMeshShadowsNone, ShadowManager::getMeshShadowsNoneDefault);

	getCodeDataObject (TUICheckbox, checkbox, "radioShadowsNonCharVolume");
	registerCheckbox (*checkbox, ShadowManager::setMeshShadowsVolumetric, ShadowManager::getMeshShadowsVolumetric, ShadowManager::getMeshShadowsVolumetricDefault);

	getCodeDataObject (TUISliderbar, slider, "sliderShadowDetail");
	registerSlider (*slider, ShadowManager::setShadowDetailLevel, ShadowManager::getShadowDetailLevel, ShadowManager::getShadowDetailLevelDefault, 0.f, 1.f);

	getCodeDataObject (TUICheckbox, checkbox, "checkBatchRenderer");
	registerCheckbox (*checkbox, setEnableBatchRenderer, getEnableBatchRenderer, SwgCuiOptBase::getFalse);

	getCodeDataObject (TUICheckbox, checkbox, "checkCharacterLodManager");
	registerCheckbox (*checkbox, setEnableCharacterLodManager, getEnableCharacterLodManager, ConfigClientSkeletalAnimation::getLodManagerEnable);

	getCodeDataObject (TUICheckbox, checkbox, "checkSimpleCameraCollision");
	registerCheckbox (*checkbox, FreeChaseCamera::setCameraSimpleCollision, FreeChaseCamera::getCameraSimpleCollision, FreeChaseCamera::getCameraSimpleCollisionDefault);

	getCodeDataObject (TUICheckbox, checkbox, "checkScreenshotTypeBmp");
	registerCheckbox (*checkbox, setScreenShotTypeBmp, getScreenShotTypeBmp, SwgCuiOptBase::getFalse);

	getCodeDataObject (TUICheckbox, checkbox, "checkScreenshotTypeTga");
	registerCheckbox (*checkbox, setScreenShotTypeTga, getScreenShotTypeTga, SwgCuiOptBase::getFalse);

	getCodeDataObject (TUICheckbox, checkbox, "checkScreenshotTypeJpg");
	registerCheckbox (*checkbox, setScreenShotTypeJpg, getScreenShotTypeJpg, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUISliderbar, slider, "sliderScreenShotQuality");
	registerSlider (*slider, Graphics::setScreenShotQuality, Graphics::getScreenShotQuality, getDefaultScreenShotQuality, 1, 100);

	getCodeDataObject (TUICheckbox, checkbox, "checkFadeObjects");
	registerCheckbox (*checkbox, DetailAppearance::setFadeInEnabled, DetailAppearance::getFadeInEnabled, SwgCuiOptBase::getTrue);
	if(Graphics::getShaderCapability() < ShaderCapability(1, 1))
	{
		checkbox->SetEnabled(false);
	}

	getCodeDataObject (TUICheckbox, checkbox, "checkCrossFadeDetailLevels");
	registerCheckbox (*checkbox, DetailAppearance::setGlobalCrossFadeEnabled, DetailAppearance::getGlobalCrossFadeEnabled, SwgCuiOptBase::getFalse);
	if(Graphics::getShaderCapability() < ShaderCapability(1, 1))
	{
		checkbox->SetEnabled(false);
	}

	bool const canBloom = PostProcessingEffectsManager::isSupported() && Bloom::isSupported();

	getCodeDataObject(TUICheckbox, checkbox, "checkEnableBloom");
	registerCheckbox (*checkbox, Bloom::setEnabled, Bloom::isEnabled, canBloom ? SwgCuiOptBase::getTrue : SwgCuiOptBase::getFalse);
	if (!canBloom)
	{
		checkbox->SetChecked(false);
	}

	getCodeDataObject(TUICheckbox, checkbox, "checkEnableHeatShimmer");
	registerCheckbox (*checkbox, ShaderPrimitiveSorter::setHeatShadersEnabled, ShaderPrimitiveSorter::getHeatShadersEnabled, SwgCuiOptBase::getFalse);
	if (!PostProcessingEffectsManager::isSupported() || !ShaderPrimitiveSorter::getHeatShadersCapable())
	{
		checkbox->SetChecked(false);
	}

	DEBUG_FATAL(PostProcessingEffectsManager::isEnabled() && PostProcessingEffectsManager::getAntialiasEnabled(),
		("FATAL: both post processing and antialiasing are enabled at the same time!"));
	//Just in case
	if(PostProcessingEffectsManager::isEnabled() && PostProcessingEffectsManager::getAntialiasEnabled())
		PostProcessingEffectsManager::setAntialiasEnabled(false);

	bool const canPostProcess = PostProcessingEffectsManager::isSupported();
	getCodeDataObject(TUICheckbox, checkbox, "checkEnablePostProcess");
	registerCheckbox (*checkbox, PostProcessingEffectsManager::setEnabled, PostProcessingEffectsManager::isEnabled, canPostProcess ? SwgCuiOptBase::getTrue : SwgCuiOptBase::getFalse);
	if (!canPostProcess)
	{
		checkbox->SetProperty(UILowerString("OnSet"), Unicode::emptyString);
		checkbox->SetProperty(UILowerString("OnUnset"), Unicode::emptyString);
		checkbox->SetChecked(false);
		checkbox->SetEnabled(false);
	}
	UICheckbox *postProcessCheckbox = checkbox;

	bool const canAntiAlias = Graphics::supportsAntialias();
	if (canAntiAlias)
	{
		getCodeDataObject(TUICheckbox, checkbox, "checkEnableAntialias");
		checkbox->SetEnabled(true);
		registerCheckbox (*checkbox, PostProcessingEffectsManager::setAntialiasEnabled, PostProcessingEffectsManager::getAntialiasEnabled, SwgCuiOptBase::getFalse);
	}
	else
	{
		getCodeDataObject(TUICheckbox, checkbox, "checkEnableAntialias");
		checkbox->SetProperty(UILowerString("OnSet"), Unicode::emptyString);
		checkbox->SetProperty(UILowerString("OnUnset"), Unicode::emptyString);
		checkbox->SetChecked(false);
		checkbox->SetEnabled(false);
		postProcessCheckbox->SetProperty(UILowerString("OnSet"), Unicode::emptyString);
		postProcessCheckbox->SetProperty(UILowerString("OnUnset"), Unicode::emptyString);
	}
	if(!canPostProcess)
	{
		checkbox->SetProperty(UILowerString("OnSet"), Unicode::emptyString);
		checkbox->SetProperty(UILowerString("OnUnset"), Unicode::emptyString);
	}
	if(PostProcessingEffectsManager::isEnabled())
	{
		checkbox->SetChecked(false);
		checkbox->SetEnabled(false);
	}

	getCodeDataObject (TUICheckbox, checkbox, "checkEnableGlare");
	registerCheckbox (*checkbox, GameCamera::setUserEnableGlare, GameCamera::getUserEnableGlare, SwgCuiOptBase::getTrue);

	getCodeDataObject (TUICheckbox, checkbox, "checkDisableAnimationPriorities");
	registerCheckbox (*checkbox, CuiPreferences::setDisableAnimationPriorities, CuiPreferences::getDisableAnimationPriorities, ConfigClientAnimation::getEnableCombatTrumping);

	getCodeDataObject (TUISliderbar, slider, "sliderParticleLimit");
	registerSlider (*slider, setParticleUserLimit, getParticleUserLimit, ConfigClientParticle::getParticleUserLimit, 0, 2048);

	getCodeDataObject (TUISliderbar, slider, "sliderCameraHeight");
	registerSlider (*slider, FreeChaseCamera::setCameraHeight, FreeChaseCamera::getCameraHeight, FreeChaseCamera::getCameraHeightDefault, 1.4f, 3.5f);

	//-- camera offset
	getCodeDataObject (TUICheckbox, checkbox, "checkOffsetCamera");
	registerCheckbox (*checkbox, FreeChaseCamera::setOffsetCamera, FreeChaseCamera::getOffsetCamera, ConfigClientUserInterface::getOffsetCamera);


}

//----------------------------------------------------------------------

void SwgCuiOptGraphics::performActivate ()
{
	SwgCuiOptBase::performActivate ();
	const bool isWindowed = Graphics::isWindowed ();

	m_sliderBrightness->SetEnabled (!isWindowed);
	m_sliderContrast->SetEnabled   (!isWindowed);
	m_sliderGamma->SetEnabled      (!isWindowed);
}


//======================================================================
