// ======================================================================
//
// SwgCuiLocations.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiLocations_H
#define INCLUDED_SwgCuiLocations_H

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIPage;
class UIButton;
class UIVolumePage;
class UIText;
class CuiLocationManager;

// ======================================================================

class SwgCuiLocations :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                 SwgCuiLocations (UIPage & page);

	//- PS UI support
	virtual void             OnButtonPressed   (UIWidget *context);
	virtual void             OnVolumePageSelectionChanged (UIWidget * context);

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

private:
	virtual                 ~SwgCuiLocations ();
	                         SwgCuiLocations ();
	                         SwgCuiLocations (const SwgCuiLocations & rhs);
	SwgCuiLocations &        operator=    (const SwgCuiLocations & rhs);

private:

	void                     reset ();

	UIButton *               m_okButton;
	UIButton *               m_cancelButton;
	UIVolumePage *           m_planetsVolumePage;
	UIVolumePage *           m_locationsVolumePage;

	UIText *                 m_sample;
	CuiLocationManager *     m_locationManager;

	std::string              m_loadedSnapshotName;
	std::string              m_loadedCameraPathName;
};

// ======================================================================

#endif
