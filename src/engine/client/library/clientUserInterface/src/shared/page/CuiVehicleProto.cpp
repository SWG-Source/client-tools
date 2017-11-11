//======================================================================
//
// CuiVehicleProto.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "clientUserInterface/FirstClientUserInterface.h"
#include "clientUserInterface/CuiVehicleProto.h"

#include "UIMessage.h"
#include "UIPage.h"
#include "UISliderbar.h"
#include "UITextbox.h"
#include "UIUtils.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGame/VehicleHoverDynamicsClient.h"
#include "clientUserInterface/CuiMediatorTypes.h"
#include "clientUserInterface/CuiSharedPageManager.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/NetworkIdManager.h"

//======================================================================

namespace
{
	const char * const s_pageNames [CuiVehicleProto::T_numTypes] =
	{
		"moveScale",
		"accelScale",
		"turnScale",
		"dampRoll",
		"dampPitch",
		"dampHeight",
		"glide",
		"pitchAccel",
		"pitchDecel",
		"rollTurn",
		"autoLevel",
		"hoverHeight"
	};

	float convert (long value, float a, float b)
	{
		return (static_cast<float>(value) * 0.001f) * (b - a) + a;
	}

	long convert (float value, float a, float b)
	{
		return static_cast<long>((value - a) * 1000.0f / (b - a));
	}

	const float s_typeRanges [CuiVehicleProto::T_numTypes][2] =
	{
		{0.1f, 5.0f },   //moveScale
		{0.1f, 5.0f },   //accelScale
		{0.1f, 2.0f },   //turnScale
		{0.5f, 10.0f },  //dampRoll
		{0.5f, 10.0f },  //dampPitch
		{0.5f, 10.0f },  //dampHeight
		{0.5f, 10.0f },  //glide
		{-PI_OVER_4, PI_OVER_4 }, //pitchAccel
		{-PI_OVER_4, PI_OVER_4 }, //pitchDecel
		{-PI_OVER_2, PI_OVER_2}, //rollTurn
		{0.0f, 1.0f },   //autoLevel
		{0.0f, 20.0f },  //hoverHeight
	};

	typedef void (CreatureObject::* SetterCreature) (float f);
	typedef float (CreatureObject::* GetterCreature) (void) const;

	typedef void (VehicleHoverDynamics::* SetterVehicle) (float f);
	typedef float (VehicleHoverDynamics::* GetterVehicle) () const;

	const SetterVehicle s_settersVehicle [CuiVehicleProto::T_numTypes] =
	{
		0,
		0,
		0,
		&VehicleHoverDynamics::setDampRoll,
		&VehicleHoverDynamics::setDampPitch,
		&VehicleHoverDynamics::setDampHeight,
		&VehicleHoverDynamics::setGlide,
		&VehicleHoverDynamics::setPitchAccel,
		&VehicleHoverDynamics::setPitchDecel,
		&VehicleHoverDynamics::setRollTurn,
		&VehicleHoverDynamics::setAutoLevel,
		&VehicleHoverDynamics::setHoverHeight,
	};

	const GetterVehicle s_gettersVehicle [CuiVehicleProto::T_numTypes] =
	{
		0,
		0,
		0,
		&VehicleHoverDynamics::getDampRoll,
		&VehicleHoverDynamics::getDampPitch,
		&VehicleHoverDynamics::getDampHeight,
		&VehicleHoverDynamics::getGlide,
		&VehicleHoverDynamics::getPitchAccel,
		&VehicleHoverDynamics::getPitchDecel,
		&VehicleHoverDynamics::getRollTurn,
		&VehicleHoverDynamics::getAutoLevel,
		&VehicleHoverDynamics::getHoverHeight,
	};

	const SetterCreature s_settersCreature [CuiVehicleProto::TypePartitionVehicle] =
	{
		&CreatureObject::clientSetMovementScale,
		&CreatureObject::clientSetAccelScale,
		&CreatureObject::clientSetTurnPercent
	};

	const GetterCreature s_gettersCreature [CuiVehicleProto::TypePartitionVehicle] =
	{
		&CreatureObject::getMovementScale,
		&CreatureObject::getAccelScale,
		&CreatureObject::getTurnPercent,
	};

	float updateFromSlider (const UISliderbar & bar, UITextbox & tb, CuiVehicleProto::Type type)
	{
		const float f = convert (bar.GetValue (), s_typeRanges [type][0], s_typeRanges [type][1]);
		Unicode::String s;
		UIUtils::FormatFloat (s, f);
		tb.SetLocalText (s);
		return f;
	}

	float updateFromTextbox (UISliderbar & bar, const UITextbox & tb, CuiVehicleProto::Type type)
	{
		const float f = tb.GetNumericFloatValue ();
		const long val = convert (f, s_typeRanges [type][0], s_typeRanges [type][1]); 
		bar.SetValue (val, false);
		return f;
	}

	void setVehicleProperty (VehicleHoverDynamics & dynamics, CuiVehicleProto::Type type, float value)
	{
		(dynamics.*s_settersVehicle [type]) (value);
	}

	float getVehicleProperty (const VehicleHoverDynamics & dynamics, CuiVehicleProto::Type type)
	{
		return (dynamics.*s_gettersVehicle [type]) ();
	}

	void setCreatureProperty (CreatureObject & creature, CuiVehicleProto::Type type, float value)
	{
		(creature.*s_settersCreature [type]) (value);
	}

	float getCreatureProperty (const CreatureObject & creature, CuiVehicleProto::Type type)
	{
		return (creature.*s_gettersCreature [type]) ();
	}

	float getProperty (const CreatureObject & creature, const VehicleHoverDynamics & dynamics, CuiVehicleProto::Type type)
	{
		if (type < CuiVehicleProto::TypePartitionVehicle)
			return getCreatureProperty (creature, type);
		else
			return getVehicleProperty (dynamics, type);
	}

	void setProperty (CreatureObject & creature, VehicleHoverDynamics & dynamics, CuiVehicleProto::Type type, float value)
	{
		if (type < CuiVehicleProto::TypePartitionVehicle)
			setCreatureProperty (creature, type, value);
		else
			setVehicleProperty (dynamics, type, value);
	}
	
	
	bool getMotorAndVehicle (const NetworkId & id, CreatureObject * &motor, Object *& vehicle)
	{
		if (id.isValid ())
			motor = dynamic_cast<CreatureObject *>(NetworkIdManager::getObjectById (id));
		else
		{
			motor = Game::getPlayerCreature ();
			const CachedNetworkId & target = motor->getLookAtTarget ();
			if (target.isValid ())
				motor = dynamic_cast<CreatureObject *>(target.getObject ());
		}

		if (!motor)
			return false;

		if (motor->getMountedCreature ())
			motor = motor->getMountedCreature ();
		
		if (!motor)
			return false;
		
		const int numChildren = motor->getNumberOfChildObjects ();

		for (int i = 0; i < numChildren; ++i)
		{
			Object * const obj = motor->getChildObject (i);
			if (obj && dynamic_cast<VehicleHoverDynamics *>(obj->getDynamics ()))
			{
				vehicle = obj;
				return true;
			}
		}
		
		return false;
	}
}

//----------------------------------------------------------------------

CuiVehicleProto::CuiVehicleProto (UIPage & page) :
CuiMediator ("CuiVehicleProto", page),
m_networkId ()
{
	UIPage * const composite = safe_cast<UIPage *>(page.GetObjectFromPath ("comp", TUIPage));
	DEBUG_FATAL (!composite, ("CuiVehicleProto failed to find composite [comp]"));

	for (int i = 0; i < T_numTypes; ++i)
	{
		const char * const pageName = s_pageNames [i];
		const UIPage * const propPage = safe_cast<UIPage *>(composite->GetObjectFromPath (pageName, TUIPage));
		DEBUG_FATAL (!propPage, ("CuiVehicleProto failed to find [%s]", pageName));

		m_sliders [i] = safe_cast<UISliderbar *>(propPage->GetObjectFromPath ("slider", TUISliderbar));
		NOT_NULL (m_sliders [i]);
		m_sliders [i]->SetLowerLimit (0);
		m_sliders [i]->SetUpperLimit (1000);

		m_textBoxes [i] = safe_cast<UITextbox *>(propPage->GetObjectFromPath ("tb", TUITextbox));
		NOT_NULL (m_sliders [i]);

		registerMediatorObject (*m_sliders [i], true);
		registerMediatorObject (*m_textBoxes [i], true);
	}

	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	NOT_NULL (parent);
	CuiSharedPageManager::registerMediatorType (parent->GetName (), CuiMediatorTypes::VehicleProto);

}

//----------------------------------------------------------------------

CuiVehicleProto::~CuiVehicleProto ()
{
}

//----------------------------------------------------------------------

void CuiVehicleProto::performActivate ()
{
	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	if (parent)
		CuiSharedPageManager::checkPageVisibility (parent->GetName (), *parent, &getPage (), CuiMediatorTypes::VehicleProto, true);

	CreatureObject * creature = 0;
	Object * vehicle = 0;
	if (!getMotorAndVehicle (NetworkId::cms_invalid, creature, vehicle))
		return;

	m_networkId = creature->getNetworkId ();

	VehicleHoverDynamics * const dynamics = dynamic_cast<VehicleHoverDynamics *>(vehicle->getDynamics ());
	if (!dynamics)
		return;

	for (int i = 0; i < T_numTypes; ++i)
	{
		const CuiVehicleProto::Type type = static_cast<CuiVehicleProto::Type>(i);

		float value = getProperty (*creature, *dynamics, type);
		const long sliderValue = convert (value, s_typeRanges [i][0], s_typeRanges [i][1]);
		m_sliders [i]->SetValue (sliderValue, false);
		updateFromSlider (*m_sliders [i], *m_textBoxes [i], type);
	}
}

//----------------------------------------------------------------------

void CuiVehicleProto::performDeactivate ()
{
	UIPage * const parent = safe_cast<UIPage *>(getPage ().GetParent ());
	if (parent)
		CuiSharedPageManager::checkPageVisibility (parent->GetName (), *parent, &getPage (), CuiMediatorTypes::VehicleProto, false);
}

//----------------------------------------------------------------------

CuiVehicleProto::Type CuiVehicleProto::findTypeBySlider  (const UISliderbar & bar) const
{
	for (int i = 0; i < T_numTypes; ++i)
	{
		if (m_sliders [i] == &bar)
			return static_cast<CuiVehicleProto::Type>(i);
	}
	
	return T_numTypes;
}

//----------------------------------------------------------------------

CuiVehicleProto::Type CuiVehicleProto::findTypeByTextBox (const UITextbox & tb) const
{
	for (int i = 0; i < T_numTypes; ++i)
	{
		if (m_textBoxes [i] == &tb)
			return static_cast<CuiVehicleProto::Type>(i);
	}

	return T_numTypes;
}

//----------------------------------------------------------------------

void  CuiVehicleProto::OnSliderbarChanged ( UIWidget *context )
{
	CreatureObject * creature = 0;
	Object * vehicle = 0;
	if (!getMotorAndVehicle (m_networkId, creature, vehicle))
		return;
	
	VehicleHoverDynamics * const dynamics = dynamic_cast<VehicleHoverDynamics *>(vehicle->getDynamics ());
	if (!dynamics)
		return;

	UISliderbar * const bar = safe_cast<UISliderbar *>(context);
	const CuiVehicleProto::Type type = findTypeBySlider (*bar);
	float f = updateFromSlider (*bar, *m_textBoxes [type], type);
	setProperty (*creature, *dynamics, type, f);
}

//----------------------------------------------------------------------

bool  CuiVehicleProto::OnMessage          ( UIWidget *context, const UIMessage & msg )
{
	if (context->IsA (TUITextbox) && msg.Type == UIMessage::KeyDown && msg.Keystroke == UIMessage::Enter)
	{
		CreatureObject * creature = 0;
		Object * vehicle = 0;
		if (!getMotorAndVehicle (m_networkId, creature, vehicle))
			return false;
		
		VehicleHoverDynamics * const dynamics = dynamic_cast<VehicleHoverDynamics *>(vehicle->getDynamics ());
		if (!dynamics)
			return true;
		
		UITextbox * const tb = safe_cast<UITextbox *>(context);
		const CuiVehicleProto::Type type = findTypeByTextBox (*tb);
		const float value = updateFromTextbox (*m_sliders [type], *tb, type);
		setProperty (*creature, *dynamics, type, value);
		
		return false;
	}
	
	return true;
}

//======================================================================
