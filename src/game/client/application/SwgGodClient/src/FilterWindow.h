// ======================================================================
//
// FilterWindow.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FilterWindow_H
#define INCLUDED_FilterWindow_H

// ======================================================================

#include "BaseFilterWindow.h"

// ======================================================================

class FilterManager;
class QString;

// ======================================================================

class FilterWindow : public BaseFilterWindow
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:

	explicit FilterWindow(QWidget *theParent=0, const char *theName=0);
	virtual ~FilterWindow();
private:
	//disabled
	FilterWindow (const FilterWindow & rhs);
	FilterWindow & operator= (const FilterWindow & rhs);

private slots:
	void OnNetworkIdCheck       (bool) const;
	void OnNetworkIdLowerChanged(const QString&) const;
	void OnNetworkIdUpperChanged(const QString&) const;
	void OnRadiusCheck          (bool) const;
	void OnRadiusMinChanged     (const QString&) const;
	void OnRadiusMaxChanged     (const QString&) const;
};

// ======================================================================

#endif
