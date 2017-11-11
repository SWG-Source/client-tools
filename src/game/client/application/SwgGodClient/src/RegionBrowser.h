// ======================================================================
//
// RegionBrowser.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_RegionBrowser_H
#define INCLUDED_RegionBrowser_H

// ======================================================================

#include "BaseRegionBrowser.h"

#include "RegionRenderer.h"

// ======================================================================

class QHideEvent;
class QTimer;
class QShowEvent;

// ======================================================================

class RegionBrowser : public BaseRegionBrowser, public MessageDispatch::Receiver
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	explicit RegionBrowser(QWidget *theParent=0, const char *theName=0);
	virtual ~RegionBrowser();

	void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

	void showEvent(QShowEvent *e);
	void hideEvent(QHideEvent *e);

	bool isVisible();

	std::map<std::string, RegionRenderer::Region*> getVisibleRegions();

public slots:
	virtual void onMissionCheck(bool);
	virtual void onBuildableCheck(bool);
	virtual void onDifficultyCheck(bool);
	virtual void onGeographicCheck(bool);
	virtual void onMunicipalCheck(bool);
	virtual void onPvPCheck(bool);
	virtual void onSpawnableCheck(bool);

private:
	//disabled
	RegionBrowser(const RegionBrowser& rhs);
	RegionBrowser& operator=(const RegionBrowser& rhs);

private slots:
	virtual void slotTimerTimeOut();

private:
	QTimer*      m_timer;
	bool         m_visible;
};

// ======================================================================

#endif
