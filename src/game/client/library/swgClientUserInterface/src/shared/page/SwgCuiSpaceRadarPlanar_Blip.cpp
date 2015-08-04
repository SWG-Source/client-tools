//======================================================================
//
// SwgCuiSpaceRadarPlanar_Blip.cpp
// copyright(c) 2004 Sony Online Entertainment
//
//======================================================================

#include "swgClientUserInterface/FirstSwgClientUserInterface.h"
#include "swgClientUserInterface/SwgCuiSpaceRadarPlanar_Blip.h"

//======================================================================

SwgCuiSpaceRadarPlanar::Blip::Blip(SwgCuiSpaceRadarPlanar & planarRadar) :
m_radar(planarRadar),
m_decayTimer(0.0f),
m_blipvert(),
m_isDamaging(false),
m_imageHead(planarRadar.newHeadImage()),
m_imageTail(planarRadar.newTailImage())
{
}

//-----------------------------------------------------------------

SwgCuiSpaceRadarPlanar::Blip::~Blip()
{
	m_radar.deleteWidget(m_imageHead);
	m_imageHead = NULL;

	m_radar.deleteWidget(m_imageTail);
	m_imageTail = NULL;
}

//-----------------------------------------------------------------

void SwgCuiSpaceRadarPlanar::Blip::setVisible(bool value)
{
	if (m_imageHead)
	{
		m_imageHead->SetVisible(value);
	}

	if (m_imageTail)
	{
		m_imageTail->SetVisible(value);
	}
}

//======================================================================
