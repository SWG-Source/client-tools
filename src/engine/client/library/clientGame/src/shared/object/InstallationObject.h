//===================================================================
//
// InstallationObject.h
// copyright 2001, Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_InstallationObject_H
#define INCLUDED_InstallationObject_H

//===================================================================

class SharedInstallationObjectTemplate;
class TurretObject;

#include "clientGame/TangibleObject.h"

//===================================================================

class InstallationObject : public TangibleObject
{
public:

	explicit InstallationObject (const SharedInstallationObjectTemplate* newTemplate);
	virtual ~InstallationObject ();

	void                 setHarvesterActive             (Object & player, bool active);
	void                 listenToHarvester              (Object & player, bool listen);
	void                 getHarvesterResourceData       (Object & player);
	void                 selectHarvesterResource        (Object & player, const NetworkId & resourcePoolId);

	float                getPower(void) const;
	float                getPowerRate(void) const;

	void                 setTurretObject                (TurretObject * turretObject);
	TurretObject       * getTurretObject                ();

protected:

	ClientSynchronizedUi * createSynchronizedUi ();

private:

	struct Callbacks
	{
		struct InstallationActive
		{
			void modified (InstallationObject & target, const bool & old, const bool & value, bool local) const;
		};
	};


 	InstallationObject ();
	InstallationObject (const InstallationObject& rhs);
	InstallationObject&	operator= (const InstallationObject& rhs);

private:

	Archive::AutoDeltaVariableCallback<bool, Callbacks::InstallationActive, InstallationObject>  m_activated;
	Archive::AutoDeltaVariable<float>  m_power;             // Amount of power the installation has
	Archive::AutoDeltaVariable<float>  m_powerRate;         // Power used, in units/hour
	TurretObject * m_turretObject;

};


inline float InstallationObject::getPower(void) const
{
	return m_power.get();
}

inline float InstallationObject::getPowerRate(void) const
{
	return m_powerRate.get();
}


//===================================================================

#endif
