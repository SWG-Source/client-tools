//===================================================================
//
// ClientDataFile_DestructionSequence.h
// jwatson
//
// copyright 2004, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ClientDataFile_DestructionSequence_H
#define INCLUDED_ClientDataFile_DestructionSequence_H

class AppearanceTemplate;
class Breakpoint;
class ClientEffectTemplate;
class Iff;
class Object;
class ObjectTemplate;
class SoundTemplate;

//----------------------------------------------------------------------

class DestructionSequence
{
public:

	class DestructionChildObject
	{
	public:
		DestructionChildObject();
		void load(Iff & iff);
		void preload();
		void unload();
		void apply(Object & parent) const;
	private:
		std::string m_objectTemplateName;
		std::string m_appearanceTemplateName;
		std::string m_hardpointName;
		ObjectTemplate const * m_objectTemplate;
		AppearanceTemplate const * m_appearanceTemplate;
	};

	class ClientEffectData
	{
	public:
		void load(Iff & iff);
		void preload();
		void unload();
	public:
		std::string m_clientEffectName;
		ClientEffectTemplate const * m_clientEffectTemplate;
		float m_nominalSize;
	};

	class AmbientSoundData
	{
	public:
		void load(Iff & iff);
		void preload();
		void unload();
	public:
		std::string m_soundTemplateName;
		SoundTemplate const * m_soundTemplate;
		float m_distanceFactor;
	};

	typedef stdvector<Breakpoint>::fwd BreakpointVector;
	typedef stdvector<ClientEffectData>::fwd ClientEffectDataVector;
	typedef stdvector<AmbientSoundData>::fwd AmbientSoundDataVector;
	typedef stdvector<DestructionChildObject>::fwd DestructionChildObjectVector;

public:

	void load(Iff & iff);
	DestructionSequence();
	~DestructionSequence();

	void preload();
	void unload();

	BreakpointVector const & getBreakpoints() const;
	ClientEffectDataVector const & getSequenceExplosions() const;
	AmbientSoundDataVector const & getAmbientSounds() const;
	float getSequenceTimeLength() const;
	ClientEffectData const & getClientEffectDataSplit() const;
	ClientEffectData const & getClientEffectDataFinale() const;

	void applyChildObjects(Object & parent) const;

private:

	DestructionSequence const & operator=(DestructionSequence const & rhs);
	DestructionSequence(DestructionSequence const & rhs);

private:

	float m_sequenceTimeLength;
	ClientEffectData m_clientEffectSplit;
	ClientEffectData m_clientEffectFinale;

	BreakpointVector * m_breakpoints;
	ClientEffectDataVector * m_sequenceExplosions;
	AmbientSoundDataVector * m_ambientSounds;
	DestructionChildObjectVector * m_destructionChildObjects;
};

#endif

//===================================================================
