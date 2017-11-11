// ======================================================================
//
// FilterWindow.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "FilterWindow.h"
#include "FilterWindow.moc"

#include "FilterManager.h"

#include <qcheckbox.h>
#include <qlineedit.h>

// ======================================================================

FilterWindow::FilterWindow(QWidget *theParent, const char *theName)
: BaseFilterWindow (theParent, theName)
{
	IGNORE_RETURN(connect(m_networkIdFilterCheck,     SIGNAL(toggled(bool)),               this, SLOT(OnNetworkIdCheck(bool))));
	IGNORE_RETURN(connect(m_networkIdLowerBoundEdit,  SIGNAL(textChanged(const QString&)), this, SLOT(OnNetworkIdLowerChanged(const QString&))));
	IGNORE_RETURN(connect(m_networkIdUpperBoundEdit,  SIGNAL(textChanged(const QString&)), this, SLOT(OnNetworkIdUpperChanged(const QString&))));

	IGNORE_RETURN(connect(m_radiusFilterCheck,        SIGNAL(toggled(bool)),               this, SLOT(OnRadiusCheck(bool))));
	IGNORE_RETURN(connect(m_minDistanceEdit,          SIGNAL(textChanged(const QString&)), this, SLOT(OnRadiusMinChanged(const QString&))));
	IGNORE_RETURN(connect(m_maxDistanceEdit,          SIGNAL(textChanged(const QString&)), this, SLOT(OnRadiusMaxChanged(const QString&))));
}

// ======================================================================

FilterWindow::~FilterWindow()
{
}

// ======================================================================

void FilterWindow::OnNetworkIdCheck(bool b) const
{
	FilterManager::getNetworkIdFilter().setOn(b);
}

// ======================================================================

void FilterWindow::OnNetworkIdLowerChanged(const QString& str) const
{
	if(str == "")
		FilterManager::getNetworkIdFilter().clearMaxNetworkId();

	bool result;
	unsigned int value = static_cast<unsigned int>(str.toInt(&result));
	if(result)
		FilterManager::getNetworkIdFilter().setMinNetworkId(value);
}

// ======================================================================

void FilterWindow::OnNetworkIdUpperChanged(const QString& str) const
{
	if(str == "")
		FilterManager::getNetworkIdFilter().clearMinNetworkId();

	bool result;
	unsigned int value = static_cast<unsigned int>(str.toInt(&result));
	if(result)
		FilterManager::getNetworkIdFilter().setMaxNetworkId(value);
}

// ======================================================================

void FilterWindow::OnRadiusCheck(bool b) const
{
	FilterManager::getRadiusFilter().setOn(b);
}

// ======================================================================

void FilterWindow::OnRadiusMinChanged(const QString& str) const
{
	if(str == "")
		FilterManager::getRadiusFilter().clearMinThreshold();

	bool result;
	real value = str.toFloat(&result);
	if(result)
		FilterManager::getRadiusFilter().setMinThreshold(value);
}

// ======================================================================

void FilterWindow::OnRadiusMaxChanged(const QString& str) const
{
	if(str == "")
		FilterManager::getRadiusFilter().clearMaxThreshold();

	bool result;
	real value = str.toFloat(&result);
	if(result)
		FilterManager::getRadiusFilter().setMaxThreshold(value);
}

// ======================================================================

