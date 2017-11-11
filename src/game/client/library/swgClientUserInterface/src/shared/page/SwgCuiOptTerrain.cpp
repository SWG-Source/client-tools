//======================================================================
//
// SwgCuiOptTerrain.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiOptTerrain.h"

#include "UICheckbox.h"
#include "UIData.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "clientObject/ShadowVolume.h"
#include "clientTerrain/ClientProceduralTerrainAppearance.h"
#include "clientTerrain/ConfigClientTerrain.h"
#include "sharedTerrain/TerrainObject.h"

//======================================================================

SwgCuiOptTerrain::SwgCuiOptTerrain (UIPage & page) :
SwgCuiOptBase ("SwgCuiOptTerrain", page)
{
	UISliderbar * slider = 0;
	UICheckbox * checkbox = 0;

	getCodeDataObject (TUISliderbar, slider, "sliderTerrainThresholdHigh");
	registerSlider (*slider, TerrainObject::setHighLevelOfDetailThreshold, TerrainObject::getHighLevelOfDetailThreshold, ConfigClientTerrain::getHighLevelOfDetailThreshold, 1.f, ClientProceduralTerrainAppearance::getMaximumThresholdHigh (), 30.0f);

	getCodeDataObject (TUISliderbar, slider, "sliderTerrainThreshold");
	registerSlider (*slider, TerrainObject::setLevelOfDetailThreshold, TerrainObject::getLevelOfDetailThreshold, ConfigClientTerrain::getThreshold, 1.f, ClientProceduralTerrainAppearance::getMaximumThreshold (), 8.0f);

	getCodeDataObject (TUICheckbox, checkbox, "checkFloraFarRadial");
	registerCheckbox (*checkbox, ClientProceduralTerrainAppearance::setDynamicFarFloraEnabled, ClientProceduralTerrainAppearance::getDynamicFarFloraEnabled, ConfigClientTerrain::getDynamicFarFloraEnabled);

	getCodeDataObject (TUICheckbox, checkbox, "checkFloraNearRadial");
	registerCheckbox (*checkbox, ClientProceduralTerrainAppearance::setDynamicNearFloraEnabled, ClientProceduralTerrainAppearance::getDynamicNearFloraEnabled, ConfigClientTerrain::getDynamicNearFloraEnabled);

	getCodeDataObject (TUICheckbox, checkbox, "checkFloraNonCollidable");
	registerCheckbox (*checkbox, ClientProceduralTerrainAppearance::setStaticNonCollidableFloraEnabled, ClientProceduralTerrainAppearance::getStaticNonCollidableFloraEnabled, ConfigClientTerrain::getStaticNonCollidableFloraEnabled);

	getCodeDataObject (TUISliderbar, slider, "sliderFloraNearRadialDistance");
	registerSlider (*slider, ClientProceduralTerrainAppearance::setDynamicNearFloraDistance, ClientProceduralTerrainAppearance::getDynamicNearFloraDistance, ConfigClientTerrain::getDynamicNearFloraDistanceDefault, 16.f, 64.f);

	getCodeDataObject (TUISliderbar, slider, "sliderFloraNonCollidableDistance");
	registerSlider (*slider, ClientProceduralTerrainAppearance::setStaticNonCollidableFloraDistance, ClientProceduralTerrainAppearance::getStaticNonCollidableFloraDistance, ConfigClientTerrain::getStaticNonCollidableFloraDistanceDefault, 32.f, 128.f);

	getCodeDataObject (TUICheckbox, checkbox, "checkSpecular");
	registerCheckbox (*checkbox, ClientProceduralTerrainAppearance::setDeferredSpecularTerrainEnabled, ClientProceduralTerrainAppearance::getDeferredSpecularTerrainEnabled, ClientProceduralTerrainAppearance::getSpecularTerrainCapable);

	if (!ClientProceduralTerrainAppearance::getSpecularTerrainCapable())
	{
		checkbox->SetEnabled(false);
		checkbox->SetChecked(false);
	}
	else
		checkbox->SetEnabled(true);
}

//======================================================================
