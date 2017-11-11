// ======================================================================
//
// RegionBrowser.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"

#include "RegionBrowser.h"
#include "RegionBrowser.moc"

#include "UnicodeUtils.h"
#include "unicodeArchive/UnicodeArchive.h"

#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"

#include "sharedNetworkMessages/MessageRegionListCircleResponse.h"
#include "sharedNetworkMessages/MessageRegionListRectResponse.h"

#include "RegionRenderer.h"
#include "ServerCommander.h"

#include <qtimer.h>

// ======================================================================

RegionBrowser::RegionBrowser(QWidget *theParent, const char *theName)
: BaseRegionBrowser (theParent, theName),
  MessageDispatch::Receiver (),
  m_timer(NULL),
  m_visible(false)
{
	connectToMessage ("MessageRegionListCircleResponse");
	connectToMessage ("MessageRegionListRectResponse");

	m_timer = new QTimer(this, "RegionRendererTimer");
	connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimerTimeOut()));
}

// ======================================================================

RegionBrowser::~RegionBrowser()
{
}

//-----------------------------------------------------------------------

void RegionBrowser::receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message)
{
	UNREF(source);

	if (message.isType ("MessageRegionListCircleResponse"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const MessageRegionListCircleResponse circlersp(ri);
		m_regionRenderer->updateCircleRegion(circlersp.getWorldX(), circlersp.getWorldZ(),  circlersp.getRadius(),  Unicode::wideToNarrow(circlersp.getName()), circlersp.getPlanet(), circlersp.getPvP(), circlersp.getBuildable(), circlersp.getSpawnable(), circlersp.getMunicipal(), circlersp.getGeographical(), circlersp.getMinDifficulty(), circlersp.getMaxDifficulty(), circlersp.getMission());
	}
	else if (message.isType ("MessageRegionListRectResponse"))
	{
		Archive::ReadIterator ri = NON_NULL (safe_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		const MessageRegionListRectResponse rectrsp(ri);
		m_regionRenderer->updateRectRegion(rectrsp.getWorldX(),  rectrsp.getWorldZ(),  rectrsp.getURWorldX(),  rectrsp.getURWorldZ(),  Unicode::wideToNarrow(rectrsp.getName()), rectrsp.getPlanet(), rectrsp.getPvP(), rectrsp.getBuildable(), rectrsp.getSpawnable(), rectrsp.getMunicipal(), rectrsp.getGeographical(), rectrsp.getMinDifficulty(), rectrsp.getMaxDifficulty(), rectrsp.getMission());
	}
}

//-----------------------------------------------------------------------

void RegionBrowser::showEvent(QShowEvent *e)
{
    BaseRegionBrowser::showEvent(e);
		m_timer->start(1);
		m_visible = true;
}

//-----------------------------------------------------------------------

void RegionBrowser::hideEvent(QHideEvent *e)
{
    BaseRegionBrowser::hideEvent(e);
		m_timer->stop();
		m_visible = false;
}

//-----------------------------------------------------------------------

bool RegionBrowser::isVisible()
{
	return m_visible;
}

//-----------------------------------------------------------------------

void RegionBrowser::slotTimerTimeOut()
{
	ServerCommander::getInstance().getRegionsList();
	m_timer->start(5000);
}

//-----------------------------------------------------------------------

void RegionBrowser::onMissionCheck(bool checked)
{
	m_regionRenderer->filterOnMission(checked);
}

//-----------------------------------------------------------------------

void RegionBrowser::onBuildableCheck(bool checked)
{
	m_regionRenderer->filterOnBuildable(checked);
}

//-----------------------------------------------------------------------

void RegionBrowser::onDifficultyCheck(bool checked)
{
	m_regionRenderer->filterOnDifficulty(checked);
}

//-----------------------------------------------------------------------

void RegionBrowser::onGeographicCheck(bool checked)
{
	m_regionRenderer->filterOnGeographical(checked);
}

//-----------------------------------------------------------------------

void RegionBrowser::onMunicipalCheck(bool checked)
{
	m_regionRenderer->filterOnMunicipal(checked);
}

//-----------------------------------------------------------------------

void RegionBrowser::onPvPCheck(bool checked)
{
	m_regionRenderer->filterOnPvP(checked);
}

//-----------------------------------------------------------------------

void RegionBrowser::onSpawnableCheck(bool checked)
{
	m_regionRenderer->filterOnSpawnable(checked);
}

//-----------------------------------------------------------------------

std::map<std::string, RegionRenderer::Region*> RegionBrowser::getVisibleRegions()
{
	return m_regionRenderer->getVisibleRegions();
}

// ======================================================================
