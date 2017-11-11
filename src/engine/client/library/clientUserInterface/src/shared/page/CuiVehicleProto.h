//======================================================================
//
// CuiVehicleProto.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiVehicleProto_H
#define INCLUDED_CuiVehicleProto_H

#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "UIEventCallback.h"

class UISliderbar;
class UITextbox;

//======================================================================

class CuiVehicleProto :
public CuiMediator,
public UIEventCallback
{
public:
	explicit CuiVehicleProto (UIPage & page);

	enum Type	
	{
		T_moveScale,
		T_accelScale,
		T_turnScale,
		T_dampRoll,
		T_dampPitch,
		T_dampHeight,
		T_glide,
		T_pitchAccel,
		T_pitchDecel,
		T_rollTurn,
		T_autoLevel,
		T_hoverHeight,
		T_numTypes
	};

	enum 
	{
		TypePartitionVehicle = T_dampRoll
	};

	void  OnSliderbarChanged ( UIWidget *Context );
	bool  OnMessage          ( UIWidget *Context, const UIMessage & msg );

protected:

	virtual void performActivate ();
	virtual void performDeactivate ();

private:

	virtual ~CuiVehicleProto ();

	CuiVehicleProto ();
	CuiVehicleProto (const CuiVehicleProto& rhs);
	CuiVehicleProto& operator= (const CuiVehicleProto& rhs);

	Type findTypeBySlider  (const UISliderbar & bar) const;
	Type findTypeByTextBox (const UITextbox & tb) const;

private:

	UISliderbar * m_sliders   [T_numTypes];
	UITextbox *   m_textBoxes [T_numTypes];

	NetworkId m_networkId;
};



//======================================================================

#endif
