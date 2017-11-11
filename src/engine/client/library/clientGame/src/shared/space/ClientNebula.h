//======================================================================
//
// ClientNebula.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ClientNebula_H
#define INCLUDED_ClientNebula_H

//======================================================================

class AppearanceTemplate;
class ClientEffectTemplate;
class NebulaVisualQuadShaderGroup;
class SoundTemplate;

//----------------------------------------------------------------------

class ClientNebula
{
public:

	explicit ClientNebula(int nebulaId);
	~ClientNebula();

	int getId() const;
	bool isPopulated() const;

	void populate();
	void clear();

	NebulaVisualQuadShaderGroup const * getNebulaVisualQuadShaderGroup() const;

	SoundTemplate const * getLightningSoundTemplate() const;
	ClientEffectTemplate const * getClientHitLightningClientEffectTemplate() const;
	ClientEffectTemplate const * getServerHitLightningClientEffectTemplate() const;
	ClientEffectTemplate const * getEnvironmentalDamageClientEffectTemplate() const;

private:

	void setPopulated(bool populated);

private:

	NebulaVisualQuadShaderGroup * m_nebulaVisualQuadShaderGroup;

	int m_nebulaId;
	bool m_isPopulated;

	AppearanceTemplate const * m_lightningAppearanceTemplate;
	SoundTemplate const * m_lightningSoundTemplate;
	ClientEffectTemplate const * m_clientHitLightningClientEffectTemplate;
	ClientEffectTemplate const * m_serverHitLightningClientEffectTemplate;
	ClientEffectTemplate const * m_environmentalDamageClientEffectTemplate;
};

//----------------------------------------------------------------------

inline int ClientNebula::getId() const
{
	return m_nebulaId;
}

//----------------------------------------------------------------------

inline bool ClientNebula::isPopulated() const
{
	return m_isPopulated;
}

//----------------------------------------------------------------------

inline SoundTemplate const * ClientNebula::getLightningSoundTemplate() const
{
	return m_lightningSoundTemplate;
}

//----------------------------------------------------------------------

inline NebulaVisualQuadShaderGroup const * ClientNebula::getNebulaVisualQuadShaderGroup() const
{
	return m_nebulaVisualQuadShaderGroup;
}

//----------------------------------------------------------------------

inline ClientEffectTemplate const * ClientNebula::getClientHitLightningClientEffectTemplate() const
{
	return m_clientHitLightningClientEffectTemplate;
}

//----------------------------------------------------------------------

inline ClientEffectTemplate const * ClientNebula::getServerHitLightningClientEffectTemplate() const
{
	return m_serverHitLightningClientEffectTemplate;
}

//----------------------------------------------------------------------

inline ClientEffectTemplate const * ClientNebula::getEnvironmentalDamageClientEffectTemplate() const
{
	return m_environmentalDamageClientEffectTemplate;
}

//======================================================================

#endif
