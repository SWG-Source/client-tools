//===================================================================
//
// EnvironmentBlockManager.cpp
// asommers
//
// copyright 2002, sony online entertainment
// 
//===================================================================

#include "clientTerrain/FirstClientTerrain.h"
#include "clientTerrain/EnvironmentBlockManager.h"

#include "clientTerrain/EnvironmentBlock.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedTerrain/EnvironmentGroup.h"
#include "sharedUtility/DataTable.h"

#include <algorithm>
#include <map>

//===================================================================

namespace EnvironmentBlockManagerNamespace
{
	enum ColumnData
	{
		CD_name,
		CD_weatherIndex,
		CD_gradientSkyTextureName,
		CD_cloudLayerBottomShaderTemplateName,
		CD_cloudLayerBottomShaderSize,
		CD_cloudLayerBottomSpeed,
		CD_cloudLayerTopShaderTemplateName,
		CD_cloudLayerTopShaderSize,
		CD_cloudLayerTopSpeed,
		CD_colorRampFileName,
		CD_shadowsEnabled,
		CD_fogEnabled,
		CD_minimumFogDensity,
		CD_maximumFogDensity,
		CD_cameraAppearanceTemplateName,
		CD_dayEnvironmentTextureName,
		CD_nightEnvironmentTextureName,
		CD_day1AmbientSoundTemplateName,
		CD_day2AmbientSoundTemplateName,
		CD_night1AmbientSoundTemplateName,
		CD_night2AmbientSoundTemplateName,
		CD_firstMusicSoundTemplateName,
		CD_sunriseMusicSoundTemplateName,
		CD_sunsetMusicSoundTemplateName,
		CD_windSpeedScale
	};
}

using namespace EnvironmentBlockManagerNamespace;

//===================================================================
// PUBLIC EnvironmentBlockManager
//===================================================================

EnvironmentBlockManager::EnvironmentBlockManager (const EnvironmentGroup* const environmentGroup, const char* const fileName) :
	m_environmentBlockMap (new EnvironmentBlockMap),
	m_defaultEnvironmentBlock (new EnvironmentBlock ())
{
	load (environmentGroup, fileName);

	//--
	EnvironmentBlockData data;
	data.m_name = "_default";
	m_defaultEnvironmentBlock->setData (data);
}

//-------------------------------------------------------------------

EnvironmentBlockManager::~EnvironmentBlockManager ()
{
	std::for_each (m_environmentBlockMap->begin (), m_environmentBlockMap->end (), PointerDeleterPairSecond ());
	delete m_environmentBlockMap;

	delete m_defaultEnvironmentBlock;
}

//-------------------------------------------------------------------

const EnvironmentBlock* EnvironmentBlockManager::getEnvironmentBlock (const int familyId, const int weatherIndex) const
{
	const int key = familyId << 16 | weatherIndex;
	EnvironmentBlockMap::iterator iter = m_environmentBlockMap->find (key);
	if (iter != m_environmentBlockMap->end ())
		return iter->second;

	//-- if not found, return environment block 0
	if (weatherIndex != 0)
		return getEnvironmentBlock (familyId, 0);

	return m_defaultEnvironmentBlock;
}

//-------------------------------------------------------------------

const EnvironmentBlock* EnvironmentBlockManager::getDefaultEnvironmentBlock () const
{
	return m_defaultEnvironmentBlock;
}

//===================================================================
// PRIVATE EnvironmentBlockManager
//===================================================================

void EnvironmentBlockManager::load (const EnvironmentGroup* const environmentGroup, const char* fileName)
{
	Iff iff;
	if (iff.open (fileName, true))
	{
		DataTable dataTable;
		dataTable.load (iff);

		const int numberOfRows = dataTable.getNumRows ();
		int row;
		for (row = 0; row < numberOfRows; ++row)
		{
			EnvironmentBlockData data;
			data.m_name                               = dataTable.getStringValue (CD_name, row);
			data.m_weatherIndex                       = dataTable.getIntValue    (CD_weatherIndex, row);
			data.m_gradientSkyTextureName             = dataTable.getStringValue (CD_gradientSkyTextureName, row);
			data.m_cloudLayerBottomShaderTemplateName = dataTable.getStringValue (CD_cloudLayerBottomShaderTemplateName, row);
			data.m_cloudLayerBottomShaderSize         = dataTable.getFloatValue  (CD_cloudLayerBottomShaderSize, row);
			data.m_cloudLayerBottomSpeed              = dataTable.getFloatValue  (CD_cloudLayerBottomSpeed, row);
			data.m_cloudLayerTopShaderTemplateName    = dataTable.getStringValue (CD_cloudLayerTopShaderTemplateName, row);
			data.m_cloudLayerTopShaderSize            = dataTable.getFloatValue  (CD_cloudLayerTopShaderSize, row);
			data.m_cloudLayerTopSpeed                 = dataTable.getFloatValue  (CD_cloudLayerTopSpeed, row);
			data.m_colorRampFileName                  = dataTable.getStringValue (CD_colorRampFileName, row);
			data.m_shadowsEnabled                     = dataTable.getIntValue    (CD_shadowsEnabled, row) != 0;
			data.m_fogEnabled                         = dataTable.getIntValue    (CD_fogEnabled, row) != 0;
			data.m_minimumFogDensity                  = dataTable.getFloatValue  (CD_minimumFogDensity, row);
			data.m_maximumFogDensity                  = dataTable.getFloatValue  (CD_maximumFogDensity, row);
			data.m_cameraAppearanceTemplateName       = dataTable.getStringValue (CD_cameraAppearanceTemplateName, row);
			data.m_dayEnvironmentTextureName          = dataTable.getStringValue (CD_dayEnvironmentTextureName, row);
			data.m_nightEnvironmentTextureName        = dataTable.getStringValue (CD_nightEnvironmentTextureName, row);
			data.m_day1AmbientSoundTemplateName       = dataTable.getStringValue (CD_day1AmbientSoundTemplateName, row);
			data.m_day2AmbientSoundTemplateName       = dataTable.getStringValue (CD_day2AmbientSoundTemplateName, row);
			data.m_night1AmbientSoundTemplateName     = dataTable.getStringValue (CD_night1AmbientSoundTemplateName, row);
			data.m_night2AmbientSoundTemplateName     = dataTable.getStringValue (CD_night2AmbientSoundTemplateName, row);
			data.m_firstMusicSoundTemplateName        = dataTable.getStringValue (CD_firstMusicSoundTemplateName, row);
			data.m_sunriseMusicSoundTemplateName      = dataTable.getStringValue (CD_sunriseMusicSoundTemplateName, row);
			data.m_sunsetMusicSoundTemplateName       = dataTable.getStringValue (CD_sunsetMusicSoundTemplateName, row);
			data.m_windSpeedScale                     = dataTable.getFloatValue  (CD_windSpeedScale, row);

			if (environmentGroup->hasFamily (data.m_name))
			{
				const int familyId = environmentGroup->getFamilyId (data.m_name);
				data.m_familyId = familyId;

				const int key      = familyId << 16 | data.m_weatherIndex;
				EnvironmentBlock* const environmentBlock = new EnvironmentBlock ();
				environmentBlock->setData (data);

				m_environmentBlockMap->insert (std::make_pair (key, environmentBlock));
			}
			else
				DEBUG_WARNING (true, ("EnvironmentBlockManager::load: environment block file %s specifies family %s not found within terrain file", fileName, data.m_name));
		}
	}
}

//===================================================================
