// ======================================================================
//
// ClientCombatPlaybackManager.cpp
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGame/FirstClientGame.h"
#include "ClientCombatPlaybackManager.h"

#include "clientAnimation/PlaybackScript.h"
#include "clientAnimation/PlaybackScriptManager.h"
#include "clientAnimation/PlaybackScriptTemplate.h"
#include "clientAnimation/PlaybackScriptTemplateList.h"
#include "clientAnimation/PriorityPlaybackScriptManager.h"
#include "clientGame/CellObject.h"
#include "clientGame/ClientCombatActionInfo.h"
#include "clientGame/CombatEffectsManager.h"
#include "clientGame/CombatSpecialMoveVisualData.h"
#include "clientGame/ConfigClientGame.h"
#include "clientGame/CreatureController.h"
#include "clientGame/CreatureInfo.h"
#include "clientGame/CreatureObject.h"
#include "clientGame/Game.h"
#include "clientGraphics/RenderWorld.h"
#include "clientObject/TimerObject.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplate.h"
#include "clientSkeletalAnimation/SkeletalAnimationTemplateList.h"
#include "clientSkeletalAnimation/SkeletalAppearance2.h"
#include "clientUserInterface/CuiCombatManager.h"
#include "sharedDebug/debugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/VoidMemberFunction.h"
#include "sharedGame/GameScheduler.h"
#include "sharedGame/SharedWeaponObjectTemplate.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedRandom/Random.h"
#include "swgSharedUtility/Postures.def"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

// ======================================================================

bool                                                       ClientCombatPlaybackManager::ms_installed;

ClientCombatPlaybackManager::ScriptSelectorMap             ClientCombatPlaybackManager::ms_scriptSelectorMap;
ClientCombatPlaybackManager::VariableFillerBindingMap      ClientCombatPlaybackManager::ms_variableFillerBindingMap;

ClientCombatPlaybackManager::CrcLowerStringSet             ClientCombatPlaybackManager::ms_playbackScriptTemplateNames;
ClientCombatPlaybackManager::PlaybackScriptTemplateVector  ClientCombatPlaybackManager::ms_preloadedPlaybackScriptTemplates;

#ifdef _DEBUG
ClientCombatPlaybackManager::CrcStringMap                  ClientCombatPlaybackManager::ms_crcLookupMap;
#endif

bool                                                       ClientCombatPlaybackManager::ms_logCombatManagerActivity;

// ======================================================================

namespace ClientCombatPlaybackManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const Tag TAG_AEPS = TAG(A,E,P,S);
	const Tag TAG_ATRG = TAG(A,T,R,G);
	const Tag TAG_ATYP = TAG(A,T,Y,P);
	const Tag TAG_AWPN = TAG(A,W,P,N);
	const Tag TAG_CBTM = TAG(C,B,T,M);
	const Tag TAG_DAMG = TAG(D,A,M,G);
	const Tag TAG_DDFS = TAG(D,D,F,S);
	const Tag TAG_DDSP = TAG(D,D,S,P);
	const Tag TAG_DEPS = TAG(D,E,P,S);
	const Tag TAG_DISP = TAG(D,I,S,P);
	const Tag TAG_DTYP = TAG(D,T,Y,P);
	const Tag TAG_FLOT = TAG(F,L,O,T);
	const Tag TAG_INT  = TAG3(I,N,T);
	const Tag TAG_KEY  = TAG3(K,E,Y);
	const Tag TAG_POST = TAG(P,O,S,T);
	const Tag TAG_PROB = TAG(P,R,O,B);
	const Tag TAG_RAND = TAG(R,A,N,D);
	const Tag TAG_SNGL = TAG(S,N,G,L);
	const Tag TAG_STRN = TAG(S,T,R,N);
	const Tag TAG_TRBF = TAG(T,R,B,F);
	const Tag TAG_TYPE = TAG(T,Y,P,E);
	const Tag TAG_VARS = TAG(V,A,R,S);
	const Tag TAG_DHLC = TAG(D,H,L,C);
	const Tag TAG_AAID = TAG(A,A,I,D);
	const Tag TAG_AANC = TAG(A,A,N,C);
	const Tag TAG_TLCX = TAG(T,L,C,X);
	const Tag TAG_TLCY = TAG(T,L,C,Y);
	const Tag TAG_TLCZ = TAG(T,L,C,Z);
	const Tag TAG_TLCC = TAG(T,L,C,C);

	const float cs_specialMoveEffectDelay = 0.75f;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class DeferredSpecialMoveEffect
	{
	public:

		DeferredSpecialMoveEffect(Object &targetObject, int effectId);

		Object *getTargetObject();
		int     getEffectId() const;

	private:

		// Disabled.
		DeferredSpecialMoveEffect();
		DeferredSpecialMoveEffect(const DeferredSpecialMoveEffect&); //lint -esym(754, DeferredSpecialMoveEffect::DeferredSpecialMoveEffect) // Local structure member not referenced. // Defensive hiding.
		DeferredSpecialMoveEffect &operator =(const DeferredSpecialMoveEffect&);

	private:

		Watcher<Object> m_targetObjectWatcher;
		int             m_effectId;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void queueSpecialMoveEffect(Object &targetObject, int effectId);
	void setupSpecialMoveEffect(const void *context);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	PlaybackScriptManager::ConstPlaybackScriptVector  s_playbackScriptVector;

#ifdef _DEBUG
	char                                              s_currentlyLoadingKeyName[256];
#endif
}

using namespace ClientCombatPlaybackManagerNamespace;

// ======================================================================

class ClientCombatPlaybackManager::BaseStringVariableFiller: public ClientCombatPlaybackManager::VariableFiller
{
public:

	virtual void fillPlaybackScriptVariable(PlaybackScript &playbackScript) const;

protected:

	BaseStringVariableFiller(const Tag &variableTag, const char *value);

private:

	// disabled
	BaseStringVariableFiller();
	BaseStringVariableFiller(const BaseStringVariableFiller&);             //lint -esym(754, BaseStringVariableFiller::BaseStringVariableFiller) // not referenced // true, for prevention
	BaseStringVariableFiller &operator =(const BaseStringVariableFiller&); //lint -esym(754, BaseStringVariableFiller::operator=) // not referenced // true, for prevention

private:

	std::string  m_value;

};

// ======================================================================

class ClientCombatPlaybackManager::StringVariableFiller: public ClientCombatPlaybackManager::BaseStringVariableFiller
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();
	static void remove();

private:

	static VariableFiller *create(Iff &iff);

private:

	StringVariableFiller(const Tag &variableTag, const char *value);

	// disabled
	StringVariableFiller();
	StringVariableFiller(const StringVariableFiller&);             //lint -esym(754, StringVariableFiller::StringVariableFiller) // not referenced // true, for prevention
	StringVariableFiller &operator =(const StringVariableFiller&); //lint -esym(754, StringVariableFiller::operator=) // not referenced // true, for prevention

};

// ======================================================================

class ClientCombatPlaybackManager::FloatVariableFiller: public ClientCombatPlaybackManager::VariableFiller
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();
	static void remove();

public:

	virtual void fillPlaybackScriptVariable(PlaybackScript &playbackScript) const;

private:

	static VariableFiller *create(Iff &iff);

private:

	FloatVariableFiller(const Tag &variableTag, float value);

	// disabled
	FloatVariableFiller();
	FloatVariableFiller(const FloatVariableFiller&);             //lint -esym(754, FloatVariableFiller::FloatVariableFiller) // not referenced // true, for prevention
	FloatVariableFiller &operator =(const FloatVariableFiller&); //lint -esym(754, FloatVariableFiller::operator=)           // not referenced // true, for prevention

private:

	float m_value;

};

// ======================================================================

class ClientCombatPlaybackManager::IntVariableFiller: public ClientCombatPlaybackManager::VariableFiller
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

public:

	static void install();
	static void remove();

public:

	virtual void fillPlaybackScriptVariable(PlaybackScript &playbackScript) const;

private:

	static VariableFiller *create(Iff &iff);

private:

	IntVariableFiller(const Tag &variableTag, int value);

	// disabled
	IntVariableFiller();
	IntVariableFiller(const IntVariableFiller&);             //lint -esym(754, IntVariableFiller::IntVariableFiller)   // not referenced // true, for prevention
	IntVariableFiller &operator =(const IntVariableFiller&); //lint -esym(754, IntVariableFiller::operator=)           // not referenced // true, for prevention

private:

	int m_value;

};

// ======================================================================

struct ClientCombatPlaybackManager::VariableFillerBinding
{
public:

	explicit VariableFillerBinding(VariableFillerCreateFunction createFunction);
	VariableFillerBinding(VariableFillerCreateFunction createFunction, VariableFillerPostLoadFunction postLoadFunction);

public:

	VariableFillerCreateFunction    m_createFunction;
	VariableFillerPostLoadFunction  m_postLoadFunction;

private:

	// disabled
	VariableFillerBinding();

};

// ======================================================================

class ClientCombatPlaybackManager::ScriptSelector
{
public:

	static ScriptSelector *create_generic_0002(Iff &iff);

public:

	virtual ~ScriptSelector();

	virtual void  addPlaybackScriptNames(CrcLowerStringSet &scriptNames) const = 0;
	virtual void  createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const = 0;

protected:

	ScriptSelector();

};

// ======================================================================

class ClientCombatPlaybackManager::SingleScriptSelector: public ClientCombatPlaybackManager::ScriptSelector
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	static ScriptSelector *create_0002(Iff &iff);

public:

	SingleScriptSelector(const char *playbackScriptTemplateName, int scriptPriority);
	virtual ~SingleScriptSelector();

	void addVariableFiller(VariableFiller *variableFiller);

	virtual void  addPlaybackScriptNames(CrcLowerStringSet &scriptNames) const;
	virtual void  createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const;

private:

	typedef stdvector<VariableFiller*>::fwd  VariableFillerVector;

private:

	SingleScriptSelector();

private:

	CrcLowerString        m_playbackScriptTemplateName;
	VariableFillerVector  m_variableFillers;
	int                   m_scriptPriority;

};

// ======================================================================

class ClientCombatPlaybackManager::RandomScriptSelector: public ClientCombatPlaybackManager::ScriptSelector
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	typedef std::vector<ScriptSelector*>  ScriptSelectorMap;
	typedef std::vector<float>            FloatVector;

public:

	static ScriptSelector *create_0002(Iff &iff);

public:

	RandomScriptSelector(const ScriptSelectorMap &scriptSelectors, const FloatVector &cumulativeProbability);
	virtual ~RandomScriptSelector();

	virtual void  addPlaybackScriptNames(CrcLowerStringSet &scriptNames) const;
	virtual void  createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const;

private:

	RandomScriptSelector();

private:

	ScriptSelectorMap  m_scriptSelectors;
	FloatVector        m_cumulativeProbability;

};

// ======================================================================

class ClientCombatPlaybackManager::BaseTypeScriptSelector: public ClientCombatPlaybackManager::ScriptSelector
{
public:

	typedef std::vector<ScriptSelector*>  ScriptSelectorVector;

public:

	static void load_0002(Iff &iff, Tag formTag, ScriptSelectorVector &scriptSelectors, int &defaultTypeIndex);

public:

	BaseTypeScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultTypeIndex);
	virtual ~BaseTypeScriptSelector();

	int           getDefaultTypeIndex() const;

	virtual void  addPlaybackScriptNames(CrcLowerStringSet &scriptNames) const;
	void          createPlaybackScriptByTypeIndex(int typeIndex, const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const;

private:

	BaseTypeScriptSelector();

private:

	ScriptSelectorVector  m_scriptSelectors;
	int                   m_defaultTypeIndex;

};

// ======================================================================

class ClientCombatPlaybackManager::AttackerTypeScriptSelector: public ClientCombatPlaybackManager::BaseTypeScriptSelector
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	static ScriptSelector *create_0002(Iff &iff);

public:

	AttackerTypeScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultTypeIndex);

	virtual void  createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const;

private:

	AttackerTypeScriptSelector();

};

// ======================================================================

class ClientCombatPlaybackManager::DefenderTypeScriptSelector: public ClientCombatPlaybackManager::BaseTypeScriptSelector
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	static ScriptSelector *create_0002(Iff &iff);

public:

	DefenderTypeScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultTypeIndex);

	virtual void  createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const;

private:

	DefenderTypeScriptSelector();

};

// ======================================================================

class ClientCombatPlaybackManager::BasePostureScriptSelector: public ClientCombatPlaybackManager::ScriptSelector
{
public:

	typedef std::vector<ScriptSelector*>  ScriptSelectorVector;

public:

	static void load_0002(Iff &iff, Tag formTag, ScriptSelectorVector &scriptSelectors, int &defaultPostureIndex);

public:

	BasePostureScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultPostureIndex);
	virtual ~BasePostureScriptSelector();

	int           getDefaultPostureIndex() const;

	virtual void  addPlaybackScriptNames(CrcLowerStringSet &scriptNames) const;
	void          createPlaybackScriptByPostureIndex(int postureIndex, const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const;

private:

	BasePostureScriptSelector();

private:

	ScriptSelectorVector  m_scriptSelectors;
	int                   m_defaultPostureIndex;

};

// ======================================================================

class ClientCombatPlaybackManager::AttackerEndPostureScriptSelector: public ClientCombatPlaybackManager::BasePostureScriptSelector
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	static ScriptSelector *create_0002(Iff &iff);

public:

	AttackerEndPostureScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultPostureIndex);

	virtual void  createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const;

private:

	AttackerEndPostureScriptSelector();

};

// ======================================================================

class ClientCombatPlaybackManager::DefenderEndPostureScriptSelector: public ClientCombatPlaybackManager::BasePostureScriptSelector
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	static ScriptSelector *create_0002(Iff &iff);

public:

	DefenderEndPostureScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultPostureIndex);

	virtual void  createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const;

private:

	DefenderEndPostureScriptSelector();

};

// ======================================================================

class ClientCombatPlaybackManager::DefenderDispositionScriptSelector: public ClientCombatPlaybackManager::ScriptSelector
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	typedef std::vector<ScriptSelector*>  ScriptSelectorVector;

public:

	static ScriptSelector *create_0002(Iff &iff);

public:

	DefenderDispositionScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultDispositionIndex);
	virtual ~DefenderDispositionScriptSelector();

	virtual void  addPlaybackScriptNames(CrcLowerStringSet &scriptNames) const;
	virtual void  createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const;

private:

	DefenderDispositionScriptSelector();

private:

	ScriptSelectorVector  m_scriptSelectors;
	int                   m_defaultDispositionIndex;

};

// ======================================================================
// namespace ClientCombatPlaybackManagerNamespace
// ======================================================================

ClientCombatPlaybackManagerNamespace::DeferredSpecialMoveEffect::DeferredSpecialMoveEffect(Object &targetObject, int effectId) :
	m_targetObjectWatcher(&targetObject),
	m_effectId(effectId)
{
}

// ----------------------------------------------------------------------

inline Object *ClientCombatPlaybackManagerNamespace::DeferredSpecialMoveEffect::getTargetObject()
{
	return m_targetObjectWatcher.getPointer();
} //lint !e1762 // (Info -- Member function 'DeferredSpecialMoveEffect::getTargetObject(void)' could be made const) // This is a logically non-const operation.

// ----------------------------------------------------------------------

inline int ClientCombatPlaybackManagerNamespace::DeferredSpecialMoveEffect::getEffectId() const
{
	return m_effectId;
}

// ======================================================================

void ClientCombatPlaybackManagerNamespace::queueSpecialMoveEffect(Object &targetObject, int effectId)
{
	GameScheduler::addCallback(setupSpecialMoveEffect, new DeferredSpecialMoveEffect(targetObject, effectId), cs_specialMoveEffectDelay);
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManagerNamespace::setupSpecialMoveEffect(const void *context)
{
	//-- Unpack the context.
	NOT_NULL(context);
	DeferredSpecialMoveEffect *effectInfo = static_cast<DeferredSpecialMoveEffect*>(const_cast<void*>(context));

	Object *const targetObject = effectInfo->getTargetObject();
	if (!targetObject)
	{
		DEBUG_REPORT_LOG(true, ("ClientCombatPlaybackManagerNamespace::setupSpecialMoveEffect dropping special effect because targetObject doesn't exist on client."));
		return;
	}

	const int effectId = effectInfo->getEffectId();

	//-- Delete the effect info.
	delete effectInfo;
	effectInfo = 0;

	const CrcString *appearanceName = 0;
	float            timeToLive     = 0.0f;

	//-- Lookup the appearance name for the move's special effect.
	const bool success = CombatSpecialMoveVisualData::lookupVisualData(effectId, appearanceName, timeToLive);
	if (!success || !appearanceName || (timeToLive <= 0.0f))
	{
		DEBUG_WARNING(true, ("ClientCombatPlaybackManager: there is no valid client special move visual mapped to effect id [%d].", effectId));
		return;
	}

	//-- Create the appearance.
	// @todo fix this conversion from CrcString to character and (probably) back to CrcString once
	//       AppearanceTemplateList takes a CrcString for a filename.
	Appearance *const appearance = AppearanceTemplateList::createAppearance(appearanceName->getString());
	if (!appearance)
	{
		DEBUG_WARNING(true, ("ClientCombatPlaybackManager: failed to create special effect appearance for filename [%s].", appearanceName->getString()));
		return;
	}

	//-- Create the object.
	TimerObject *const childObject = new TimerObject(timeToLive);
	RenderWorld::addObjectNotifications(*childObject);

	childObject->setAppearance(appearance);

	//-- Attach child object to target.
	childObject->attachToObject_p(targetObject, true);
}

// ======================================================================
// class ClientCombatPlaybackManager::VariableFiller
// ======================================================================

ClientCombatPlaybackManager::VariableFiller::~VariableFiller()
{
}

// ======================================================================

ClientCombatPlaybackManager::VariableFiller::VariableFiller(const Tag &variableTag)
:	m_variableTag(variableTag)
{
}

// ======================================================================
// struct ClientCombatPlaybackManager::VariableFillerBinding
// ======================================================================

ClientCombatPlaybackManager::VariableFillerBinding::VariableFillerBinding(VariableFillerCreateFunction createFunction)
:	m_createFunction(createFunction),
	m_postLoadFunction(NULL)
{
}

// ----------------------------------------------------------------------

ClientCombatPlaybackManager::VariableFillerBinding::VariableFillerBinding(VariableFillerCreateFunction createFunction, VariableFillerPostLoadFunction postLoadFunction)
:	m_createFunction(createFunction),
	m_postLoadFunction(postLoadFunction)
{
}

// ======================================================================
// class ClientCombatPlaybackManager::BaseStringVariableFiller
// ======================================================================

void ClientCombatPlaybackManager::BaseStringVariableFiller::fillPlaybackScriptVariable(PlaybackScript &playbackScript) const
{
	const bool setResult = playbackScript.setStringVariable(getVariableTag(), m_value);

#ifdef _DEBUG
	if (!setResult)
	{
		char buffer[5];

		ConvertTagToString(getVariableTag(), buffer);
		DEBUG_REPORT_LOG(getLogCombatManagerActivity(), ("failed to set variable TAG_%s to [%s]", buffer, m_value.c_str()));
	}
#endif

	UNREF(setResult);
}

// ======================================================================

ClientCombatPlaybackManager::BaseStringVariableFiller::BaseStringVariableFiller(const Tag &variableTag, const char *value)
:	VariableFiller(variableTag),
	m_value(value)
{
}

// ======================================================================
// class ClientCombatPlaybackManager::StringVariableFiller
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ClientCombatPlaybackManager::StringVariableFiller, true, 0, 0, 0);

// ======================================================================

void ClientCombatPlaybackManager::StringVariableFiller::install()
{
	InstallTimer const installTimer("ClientCombatPlaybackManager::StringVariableFiller");

	registerVariableFiller(TAG_STRN, create);
	installMemoryBlockManager();
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::StringVariableFiller::remove()
{
	deregisterVariableFiller(TAG_STRN);
	removeMemoryBlockManager();
}

// ======================================================================

ClientCombatPlaybackManager::VariableFiller *ClientCombatPlaybackManager::StringVariableFiller::create(Iff &iff)
{
	char buffer[1024];

	iff.enterChunk(TAG_STRN);

		const Tag variableTag(iff.read_uint32());
		iff.read_string(buffer, sizeof(buffer)-1);

	iff.exitChunk(TAG_STRN);

	return new StringVariableFiller(variableTag, buffer);
}

// ======================================================================

ClientCombatPlaybackManager::StringVariableFiller::StringVariableFiller(const Tag &variableTag, const char *value)
:	BaseStringVariableFiller(variableTag, value)
{
}

// ======================================================================
// class ClientCombatPlaybackManager::FloatVariableFiller
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ClientCombatPlaybackManager::FloatVariableFiller, true, 0, 0, 0);

// ======================================================================

void ClientCombatPlaybackManager::FloatVariableFiller::install()
{
	InstallTimer const installTimer("ClientCombatPlaybackManager::FloatVariableFiller");

	registerVariableFiller(TAG_FLOT, create);
	installMemoryBlockManager();
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::FloatVariableFiller::remove()
{
	deregisterVariableFiller(TAG_FLOT);
	removeMemoryBlockManager();
}

// ======================================================================

ClientCombatPlaybackManager::VariableFiller *ClientCombatPlaybackManager::FloatVariableFiller::create(Iff &iff)
{
	iff.enterChunk(TAG_FLOT);

		const Tag variableTag(iff.read_uint32());
		const float value = iff.read_float();

	iff.exitChunk(TAG_FLOT);

	return new FloatVariableFiller(variableTag, value);
}

// ======================================================================

void ClientCombatPlaybackManager::FloatVariableFiller::fillPlaybackScriptVariable(PlaybackScript &playbackScript) const
{
	const bool setResult = playbackScript.setFloatVariable(getVariableTag(), m_value);

#ifdef _DEBUG
	if (!setResult)
	{
		char buffer[5];

		ConvertTagToString(getVariableTag(), buffer);
		DEBUG_REPORT_LOG(getLogCombatManagerActivity(), ("failed to set float variable TAG_%s to [%g]", buffer, m_value));
	}
#endif

	UNREF(setResult);
}

// ======================================================================

ClientCombatPlaybackManager::FloatVariableFiller::FloatVariableFiller(const Tag &variableTag, float value) :
	VariableFiller(variableTag),
	m_value(value)
{
}

// ======================================================================
// class ClientCombatPlaybackManager::IntVariableFiller
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(ClientCombatPlaybackManager::IntVariableFiller, true, 0, 0, 0);

// ======================================================================

void ClientCombatPlaybackManager::IntVariableFiller::install()
{
	InstallTimer const installTimer("ClientCombatPlaybackManager::IntVariableFiller");

	registerVariableFiller(TAG_INT, create);
	installMemoryBlockManager();
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::IntVariableFiller::remove()
{
	deregisterVariableFiller(TAG_INT);
	removeMemoryBlockManager();
}

// ======================================================================

ClientCombatPlaybackManager::VariableFiller *ClientCombatPlaybackManager::IntVariableFiller::create(Iff &iff)
{
	iff.enterChunk(TAG_INT);

		const Tag variableTag(iff.read_uint32());
		const int value = iff.read_int32();

	iff.exitChunk(TAG_INT);

	return new IntVariableFiller(variableTag, value);
}

// ======================================================================

void ClientCombatPlaybackManager::IntVariableFiller::fillPlaybackScriptVariable(PlaybackScript &playbackScript) const
{
	const bool setResult = playbackScript.setIntVariable(getVariableTag(), m_value);

#ifdef _DEBUG
	if (!setResult)
	{
		char buffer[5];

		ConvertTagToString(getVariableTag(), buffer);
		DEBUG_REPORT_LOG(getLogCombatManagerActivity(), ("failed to set int variable TAG_%s to [%d]", buffer, m_value));
	}
#endif

	UNREF(setResult);
}

// ======================================================================

ClientCombatPlaybackManager::IntVariableFiller::IntVariableFiller(const Tag &variableTag, int value) :
	VariableFiller(variableTag),
	m_value(value)
{
}

// ======================================================================
// class ClientCombatPlaybackManager::ScriptSelector
// ======================================================================

ClientCombatPlaybackManager::ScriptSelector *ClientCombatPlaybackManager::ScriptSelector::create_generic_0002(Iff &iff)
{
	const Tag currentTag = iff.getCurrentName();
	switch (currentTag)
	{
		case TAG_SNGL:
			return SingleScriptSelector::create_0002(iff);

		case TAG_RAND:
			return RandomScriptSelector::create_0002(iff);

		case TAG_ATYP:
			return AttackerTypeScriptSelector::create_0002(iff);

		case TAG_DTYP:
			return DefenderTypeScriptSelector::create_0002(iff);

		case TAG_AEPS:
			return AttackerEndPostureScriptSelector::create_0002(iff);

		case TAG_DEPS:
			return DefenderEndPostureScriptSelector::create_0002(iff);

		case TAG_DDSP:
			return DefenderDispositionScriptSelector::create_0002(iff);

		default:
			{
				char name[5];

				ConvertTagToString(currentTag, name);
				DEBUG_FATAL(true, ("unsupported ScriptSelector type [%s] for key [%s]", name, s_currentlyLoadingKeyName));
				return 0; //lint !e527 // Unreachable code. // It is reachable in release mode.
			}
	}
}

// ======================================================================

ClientCombatPlaybackManager::ScriptSelector::~ScriptSelector()
{
}

// ======================================================================

inline ClientCombatPlaybackManager::ScriptSelector::ScriptSelector()
{
}

// ======================================================================
// class ClientCombatPlaybackManager::SingleScriptSelector
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ClientCombatPlaybackManager::SingleScriptSelector, true, 0, 0, 0);

// ======================================================================

ClientCombatPlaybackManager::ScriptSelector *ClientCombatPlaybackManager::SingleScriptSelector::create_0002(Iff &iff)
{
	char name[2048];

	iff.enterForm(TAG_SNGL);

		//-- load playback script template name
		iff.enterChunk(TAG_NAME);

			iff.read_string(name, sizeof(name) - 1);
			const int priority = static_cast<int>(iff.read_int8());

		iff.exitChunk(TAG_NAME);

		//-- create the selector
		SingleScriptSelector *const scriptSelector = new SingleScriptSelector(name, priority);

		//-- load variable fillers
		if (iff.enterForm(TAG_VARS, true))
		{
			while (!iff.atEndOfForm())
			{
				//-- determine variable filler to load based on tag
				const Tag variableFillerTag                     = iff.getCurrentName();
				const VariableFillerBindingMap::iterator findIt = ms_variableFillerBindingMap.find(variableFillerTag);
				DEBUG_FATAL(findIt == ms_variableFillerBindingMap.end(), ("failed to find variable filler create function under key [%s].", s_currentlyLoadingKeyName));

				//-- create the variable filler
				VariableFillerCreateFunction createFunction = findIt->second.m_createFunction;
				NOT_NULL(createFunction);

				scriptSelector->addVariableFiller((*createFunction)(iff));
			}

			iff.exitForm(TAG_VARS);
		}

	iff.exitForm(TAG_SNGL);

	return scriptSelector;
}

// ======================================================================

ClientCombatPlaybackManager::SingleScriptSelector::SingleScriptSelector(const char *playbackScriptTemplateName, int priority) :
	ScriptSelector(),
	m_playbackScriptTemplateName(playbackScriptTemplateName),
	m_variableFillers(),
	m_scriptPriority(priority)
{
}

// ----------------------------------------------------------------------

ClientCombatPlaybackManager::SingleScriptSelector::~SingleScriptSelector()
{
	std::for_each(m_variableFillers.begin(), m_variableFillers.end(), PointerDeleter());
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::SingleScriptSelector::addVariableFiller(VariableFiller *variableFiller)
{
	m_variableFillers.push_back(variableFiller);
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::SingleScriptSelector::addPlaybackScriptNames(CrcLowerStringSet &scriptNames) const
{
	IGNORE_RETURN(scriptNames.insert(m_playbackScriptTemplateName));
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::SingleScriptSelector::createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const
{
	playbackScript = 0;

	PlaybackScriptTemplateList::ObjectVector  actorObjects;

	//-- Add all actors.  For now there are always two, later there will be potentially more.
	Object *const attackerObject = info.getAttacker();
	actorObjects.push_back(attackerObject);

	// @todo support adding all defenders when ClientCombatActionInfo supports more.
	Object *const defenderObject = info.getDefender();
	if (defenderObject)
		actorObjects.push_back(defenderObject);

	//-- Bump up playback script priority if this client's player is the attacker or defender.
	int effectiveScriptPriority = m_scriptPriority;

	Object *const playerObject = Game::getPlayer();
	if (playerObject)
	{
		if ((attackerObject == playerObject) && (info.getDefenderDefense() == ClientCombatActionInfo::DD_hit))
		{
			// The player is the attacker and the player successfully hits, so bump up the priority of the visuals.
			effectiveScriptPriority += ConfigClientGame::getPlayerAttackerCombatPriorityBoost();
		}
		else if ((defenderObject == playerObject) && (info.getDefenderDefense() == ClientCombatActionInfo::DD_hit))
		{
			// The player is the defender and the player gets hit, so bump up the priority of the visuals.
			effectiveScriptPriority += ConfigClientGame::getPlayerDefenderCombatPriorityBoost();
		}

		if (defenderObject == playerObject)
		{
			CreatureObject * const playerCreatureObject = CreatureObject::asCreatureObject(playerObject);

			if (playerCreatureObject->isInCombat())
			{
				if (playerCreatureObject->getLookAtTarget() == CachedNetworkId::cms_invalid)
				{
					CreatureObject const * const attackerCreatureObject = CreatureObject::asCreatureObject(attackerObject);
					if (attackerCreatureObject && attackerCreatureObject->isTargettable())
						playerCreatureObject->setLookAtTarget(attackerObject->getNetworkId());
				}
			}
		}
	}

	//-- Create the playback script, return to caller.
	// Fetch the PlaybackScriptTemplate.
	const PlaybackScriptTemplate *const psTemplate = PlaybackScriptTemplateList::fetch(m_playbackScriptTemplateName);
	if (!psTemplate)
	{
		DEBUG_WARNING(true, ("failed to fetch PlaybackScriptTemplate [%s].", m_playbackScriptTemplateName.getString()));
		return;
	}

	// Check if there are any missing actor objects --- can occur due to network out-of-range conditions.
	if (psTemplate->getSupportedActorCount() != static_cast<int>(actorObjects.size()))
		DEBUG_REPORT_LOG(ConfigClientGame::getLogCombatPlaybackSelection(), ("CCPM:SSS: script [%s] expecting [%d] actors, [%d] available, playback script may malfunction.\n", m_playbackScriptTemplateName.getString(), psTemplate->getSupportedActorCount(), static_cast<int>(actorObjects.size())));

	// Create the PlaybackScript.
	playbackScript = psTemplate->createPlaybackScript(effectiveScriptPriority, actorObjects);
	NOT_NULL(playbackScript);

	// Release local references.
	psTemplate->release();

	//-- Set variables for playback script.
	// @todo turn into a std::for once we create a pointer-to-reference conversion function.
	const VariableFillerVector::const_iterator endIt = m_variableFillers.end();
	for (VariableFillerVector::const_iterator it = m_variableFillers.begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);
		(*it)->fillPlaybackScriptVariable(*playbackScript);
	}

	//-- Provide damage for defender if defender is hit.
	// @todo this mechanism must change when we move to multiple defender support.
	//       At that time I think I should expose some kind of state (maybe a string hash)
	//       that lets me add arbitrary data like defender disposition/defense to the PlaybackScript.
	//       this should be exposed to the PlaybackScript in such a way that it supports multiple defenders.
	if (info.getDefenderDefense() == ClientCombatActionInfo::DD_hit)
	{
		//-- Set damage.
		IGNORE_RETURN(playbackScript->setIntVariable(TAG_DAMG, 25));
	}

	//-- If it's a location based attack, set that up here
	bool useLocation = info.getAttackerUseLocation();
	if(useLocation)
	{
		const Vector &targetLocation = info.getAttackerTargetLocation();
		playbackScript->setFloatVariable(TAG_TLCX, targetLocation.x);
		playbackScript->setFloatVariable(TAG_TLCY, targetLocation.y);
		playbackScript->setFloatVariable(TAG_TLCZ, targetLocation.z);
		const NetworkId &targetCell = info.getAttackerTargetCell();
		CellObject *cellObject = safe_cast<CellObject *>(NetworkIdManager::getObjectById(targetCell));
		playbackScript->setObjectVariable(TAG_TLCC, cellObject);
	}

	//-- Save the defender defense/disposition, force it to create the variable if it doesn't exist.
	IGNORE_RETURN(playbackScript->setIntVariable(TAG_DDFS, static_cast<int>(info.getDefenderDefense()), true));

	//-- Set end posture for attacker and defender.
	//   Only set these if the end posture is different than the start posture.  This allows us
	//   to set projectile-firing actions to defender not active if there is no posture change.
	//   There may be a collision on add-style animations, but shouldn't be a big deal.
	int playbackAttackerEndPosture = -1;
	int playbackDefenderEndPosture = -1;

	// Handle attacker end posture
	if (info.getAttackerPostureEnd() >= 0)
	{
		ClientObject   *const attackerAsClient   = (attackerObject   ? attackerObject->asClientObject()     : NULL);
		CreatureObject *const attackerAsCreature = (attackerAsClient ? attackerAsClient->asCreatureObject() : NULL);

		if (attackerAsCreature)
		{
			if (attackerAsCreature->getVisualPosture() != info.getAttackerPostureEnd())
				playbackAttackerEndPosture = info.getAttackerPostureEnd();
		}
	}

	// Handle defender end posture
	if (info.getDefenderPostureEnd() >= 0)
	{
		ClientObject   *const defenderAsClient   = (defenderObject   ? defenderObject->asClientObject()     : NULL);
		CreatureObject *const defenderAsCreature = (defenderAsClient ? defenderAsClient->asCreatureObject() : NULL);

		if (defenderAsCreature)
		{
			if (defenderAsCreature->getVisualPosture() != info.getDefenderPostureEnd())
				playbackDefenderEndPosture = info.getDefenderPostureEnd();
		}
	}

	// Set playback script end posture variables.
	IGNORE_RETURN(playbackScript->setIntVariable(TAG_AEPS, playbackAttackerEndPosture));
	IGNORE_RETURN(playbackScript->setIntVariable(TAG_DEPS, playbackDefenderEndPosture));

	//-- Set the PlaybackScript's concept of the attacker target to the defender.
	if (defenderObject)
		playbackScript->setObjectVariable(TAG_ATRG, defenderObject);

	//-- Set the attacker's weapon for the PlaybackScript.
	playbackScript->setObjectVariable(TAG_AWPN, info.getAttackerWeapon());

	//-- Set the attacker's trail bit fields for the attack.
	IGNORE_RETURN(playbackScript->setIntVariable(TAG_TRBF, info.getAttackerTrailBits()));

	//-- Set the defender's hit location - create var if it doesn't exist
	IGNORE_RETURN(playbackScript->setIntVariable(TAG_DHLC, info.getDefenderHitLocation(), true));

	// Set the attacker's attack ID - create var if it doesn't exist
	IGNORE_RETURN(playbackScript->setIntVariable(TAG_AAID, info.getAttackerActionId(), true));

	// Set the attacker's action name crc - create var if it doesn't exist
	IGNORE_RETURN(playbackScript->setIntVariable(TAG_AANC, info.getAttackerActionNameCrc(), true));

#ifdef _DEBUG
	//-- Log debug info.
	if (ConfigClientGame::getLogCombatPlaybackSelection())
	{
		DEBUG_REPORT_LOG(true, ("CCPM:SSS: selected script [%s].\n", m_playbackScriptTemplateName.getString()));
		DEBUG_REPORT_LOG(true, ("CCPM:SSS: dumping script variables.\n"));
		playbackScript->debugDumpVariables();
	}
#endif
}

// ======================================================================
// class ClientCombatPlaybackManager::RandomScriptSelector
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ClientCombatPlaybackManager::RandomScriptSelector, true, 0, 0, 0);

// ======================================================================

ClientCombatPlaybackManager::ScriptSelector *ClientCombatPlaybackManager::RandomScriptSelector::create_0002(Iff &iff)
{
	//-- read the raw data
	ScriptSelectorMap  scriptSelectors;
	FloatVector           rawWeights;
	float                 totalWeight = 0.0f;

	iff.enterForm(TAG_RAND);

		while (!iff.atEndOfForm())
		{
			iff.enterForm(TAG_ENTR);

				iff.enterChunk(TAG_PROB);

					const float rawWeight = static_cast<float>(iff.read_uint8());

					rawWeights.push_back(rawWeight);
					totalWeight += rawWeight;

				iff.exitChunk(TAG_PROB);

				scriptSelectors.push_back(NON_NULL(ScriptSelector::create_generic_0002(iff)));

			iff.exitForm(TAG_ENTR);
		}

	iff.exitForm(TAG_RAND);

	//-- build cumulative probability table
	DEBUG_FATAL(!(totalWeight > 0.0), ("bad total weight %g when processing key [%s]", totalWeight, s_currentlyLoadingKeyName));

	const size_t entryCount = rawWeights.size();
	FloatVector  cumulativeProbability(entryCount);

	float cumulativeWeight = 0.0f;
	for (size_t i = 0; i < entryCount; ++i)
	{
		cumulativeWeight += rawWeights[i];
		cumulativeProbability[i] = (cumulativeWeight/totalWeight) * 100.00f;
	}

	return new RandomScriptSelector(scriptSelectors, cumulativeProbability);
}

// ======================================================================

ClientCombatPlaybackManager::RandomScriptSelector::RandomScriptSelector(const ScriptSelectorMap &scriptSelectors, const FloatVector &cumulativeProbability)
:	ScriptSelector(),
	m_scriptSelectors(scriptSelectors),
	m_cumulativeProbability(cumulativeProbability)
{
}

// ----------------------------------------------------------------------

ClientCombatPlaybackManager::RandomScriptSelector::~RandomScriptSelector()
{
	std::for_each(m_scriptSelectors.begin(), m_scriptSelectors.end(), PointerDeleter());
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::RandomScriptSelector::addPlaybackScriptNames(CrcLowerStringSet &scriptNames) const
{
	const ScriptSelectorMap::const_iterator endIt = m_scriptSelectors.end();
	for (ScriptSelectorMap::const_iterator it = m_scriptSelectors.begin(); it != endIt; ++it)
	{
		if (*it)
			(*it)->addPlaybackScriptNames(scriptNames);
	}
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::RandomScriptSelector::createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const
{
	DEBUG_FATAL(m_scriptSelectors.size() != m_cumulativeProbability.size(), ("m_scriptSelectors and m_cumulativeProbability out of sync, should be same size (%u/%u) while loading key [%s].", m_scriptSelectors.size(), m_cumulativeProbability.size(), s_currentlyLoadingKeyName));

	//-- Roll the die.
	const float roll = Random::randomReal(0.0f, 100.0f);

	//-- Find the ScriptSelector matching the specified probability.
	ScriptSelector *randomScriptSelector = 0;

	const size_t selectorCount = m_scriptSelectors.size();
	for (size_t i = 0; i < selectorCount; ++i)
	{
		if (roll < m_cumulativeProbability[i])
		{
			randomScriptSelector = m_scriptSelectors[i];
			break;
		}
	}

	if (!randomScriptSelector)
	{
#ifdef _DEBUG
		WARNING_STRICT_FATAL(true, ("failed to resolve roll of %g %% to one of the %u script selectors, no playback script will be returned when loading key [%s].", roll, selectorCount, s_currentlyLoadingKeyName));
#else
		WARNING_STRICT_FATAL(true, ("failed to resolve roll of %g %% to one of the %u script selectors, no playback script will be returned.", roll, selectorCount));
#endif
		playbackScript = 0;
		return;
	}

	//-- Forward the create call to the randomly chosen ScriptSelector instance.
	randomScriptSelector->createPlaybackScript(info, playbackScript);
}

// ======================================================================
// class ClientCombatPlaybackManager::BaseTypeScriptSelector
// ======================================================================

void ClientCombatPlaybackManager::BaseTypeScriptSelector::load_0002(Iff &iff, Tag formTag, ScriptSelectorVector &scriptSelectors, int &defaultTypeIndex)
{
	//-- Resize and initialize scriptSelectors.
	scriptSelectors.resize(static_cast<int>(ClientCombatPlaybackManager::AT_count));
	for (ScriptSelectorVector::size_type i = 0; i < static_cast<int>(ClientCombatPlaybackManager::AT_count); ++i)
		scriptSelectors[i] = 0;

	defaultTypeIndex = 0;

	//-- Read the raw data.
	iff.enterForm(formTag);

		while (!iff.atEndOfForm())
		{
			iff.enterForm(TAG_ENTR);

				//-- Load type.
				iff.enterChunk(TAG_TYPE);

					// Get the type index.
					const int typeIndex = static_cast<int>(iff.read_int16());
					VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, typeIndex, static_cast<int>(ClientCombatPlaybackManager::AT_count));

					// Setup default type index to the last entry in the list.
					defaultTypeIndex = typeIndex;

				iff.exitChunk(TAG_TYPE);

				//-- Load the script selector associated with type.
				scriptSelectors[static_cast<ScriptSelectorVector::size_type>(typeIndex)] = ScriptSelector::create_generic_0002(iff);

			iff.exitForm(TAG_ENTR);
		}

	iff.exitForm(formTag);
}

// ======================================================================

ClientCombatPlaybackManager::BaseTypeScriptSelector::BaseTypeScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultTypeIndex) :
	ScriptSelector(),
	m_scriptSelectors(scriptSelectors),
	m_defaultTypeIndex(defaultTypeIndex)
{
	//-- Validate the default index.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_defaultTypeIndex, static_cast<int>(m_scriptSelectors.size()));
}

// ----------------------------------------------------------------------

ClientCombatPlaybackManager::BaseTypeScriptSelector::~BaseTypeScriptSelector()
{
	std::for_each(m_scriptSelectors.begin(), m_scriptSelectors.end(), PointerDeleter());
}

// ----------------------------------------------------------------------

inline int ClientCombatPlaybackManager::BaseTypeScriptSelector::getDefaultTypeIndex() const
{
	return m_defaultTypeIndex;
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::BaseTypeScriptSelector::addPlaybackScriptNames(CrcLowerStringSet &scriptNames) const
{
	const ScriptSelectorVector::const_iterator endIt = m_scriptSelectors.end();
	for (ScriptSelectorVector::const_iterator it = m_scriptSelectors.begin(); it != endIt; ++it)
	{
		if (*it)
			(*it)->addPlaybackScriptNames(scriptNames);
	}
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::BaseTypeScriptSelector::createPlaybackScriptByTypeIndex(int typeIndex, const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const
{
	//-- Find the ScriptSelector matching the specified probability.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, typeIndex, static_cast<int>(m_scriptSelectors.size()));

	ScriptSelector *scriptSelector = m_scriptSelectors[static_cast<ScriptSelectorVector::size_type>(typeIndex)];
	if (!scriptSelector)
	{
		//-- No script selector for the specified type, try the default type index.
		scriptSelector = m_scriptSelectors[static_cast<ScriptSelectorVector::size_type>(m_defaultTypeIndex)];
	}

	if (!scriptSelector)
	{
#ifdef _DEBUG
		WARNING_STRICT_FATAL(true, ("failed to resolve BaseTypeScriptSelector for combatant type [%d] while loading key [%s].", typeIndex, s_currentlyLoadingKeyName));
#else
		WARNING_STRICT_FATAL(true, ("failed to resolve BaseTypeScriptSelector for combatant type [%d].", typeIndex));
#endif
		playbackScript = 0;
		return;
	}

	//-- Forward the create call to the type-specific ScriptSelector instance.
	scriptSelector->createPlaybackScript(info, playbackScript);
}

// ======================================================================
// class ClientCombatPlaybackManager::AttackerTypeScriptSelector
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ClientCombatPlaybackManager::AttackerTypeScriptSelector, true, 0, 0, 0);

// ======================================================================

ClientCombatPlaybackManager::ScriptSelector *ClientCombatPlaybackManager::AttackerTypeScriptSelector::create_0002(Iff &iff)
{
	ScriptSelectorVector  scriptSelectors(static_cast<ScriptSelectorVector::size_type>(ClientCombatPlaybackManager::AT_count));
	int                   defaultTypeIndex = 0;

	//-- Load the type data.
	load_0002(iff, TAG_ATYP, scriptSelectors, defaultTypeIndex);

	//-- Create the ScriptSelector.
	return new AttackerTypeScriptSelector(scriptSelectors, defaultTypeIndex);
}

// ======================================================================

ClientCombatPlaybackManager::AttackerTypeScriptSelector::AttackerTypeScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultTypeIndex) :
	BaseTypeScriptSelector(scriptSelectors, defaultTypeIndex)
{
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::AttackerTypeScriptSelector::createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const
{
	//-- Find type associated with attacker.
	const Object *const object    = info.getAttacker();
	const int           typeIndex = (object ? getAppearanceTypeIndex(*object) : getDefaultTypeIndex());

	//-- Forward the create call to the ScriptSelector associated with the specified type index.
	createPlaybackScriptByTypeIndex(typeIndex, info, playbackScript);
}

// ======================================================================
// class ClientCombatPlaybackManager::DefenderTypeScriptSelector
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ClientCombatPlaybackManager::DefenderTypeScriptSelector, true, 0, 0, 0);

// ======================================================================

ClientCombatPlaybackManager::ScriptSelector *ClientCombatPlaybackManager::DefenderTypeScriptSelector::create_0002(Iff &iff)
{
	ScriptSelectorVector  scriptSelectors(static_cast<ScriptSelectorVector::size_type>(ClientCombatPlaybackManager::AT_count));
	int                   defaultTypeIndex = 0;

	//-- Load the type data.
	load_0002(iff, TAG_DTYP, scriptSelectors, defaultTypeIndex);

	//-- Create the ScriptSelector.
	return new DefenderTypeScriptSelector(scriptSelectors, defaultTypeIndex);
}

// ======================================================================

ClientCombatPlaybackManager::DefenderTypeScriptSelector::DefenderTypeScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultTypeIndex) :
	BaseTypeScriptSelector(scriptSelectors, defaultTypeIndex)
{
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::DefenderTypeScriptSelector::createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const
{
	//-- Find type associated with defender.
	// @todo update to handle multiple defenders when multi-defender support added to info.
	const Object *const object    = info.getDefender();
	const int           typeIndex = (object ? getAppearanceTypeIndex(*object) : getDefaultTypeIndex());

	//-- Forward the create call to the ScriptSelector associated with the specified type index.
	createPlaybackScriptByTypeIndex(typeIndex, info, playbackScript);
}

// ======================================================================
// class ClientCombatPlaybackManager::BasePostureScriptSelector
// ======================================================================

void ClientCombatPlaybackManager::BasePostureScriptSelector::load_0002(Iff &iff, Tag formTag, ScriptSelectorVector &scriptSelectors, int &defaultPostureIndex)
{
	//-- Resize and initialize scriptSelectors.
	scriptSelectors.resize(static_cast<ScriptSelectorVector::size_type>(ClientCombatActionInfo::getPostureCount()));
	for (ScriptSelectorVector::size_type i = 0; i < static_cast<ScriptSelectorVector::size_type>(ClientCombatPlaybackManager::AT_count); ++i)
		scriptSelectors[i] = 0;

	defaultPostureIndex = 0;

	//-- Read the raw data.
	iff.enterForm(formTag);

		while (!iff.atEndOfForm())
		{
			iff.enterForm(TAG_ENTR);

				//-- Load type.
				iff.enterChunk(TAG_POST);

					// Get the type index.
					int postureIndex = static_cast<int>(iff.read_int16());

					if (postureIndex < 0)
					{
						// When index is < 0, this means the entry is a default.
						// Default entries must come at the end.  This default implementation
						// will fail if every posture has been specified directly (note: in this
						// case, a default makes no sense).
						postureIndex = findNullScriptSelectorIndex(scriptSelectors);
					}

					VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, postureIndex, ClientCombatActionInfo::getPostureCount());

					// Setup default type index to the last entry in the list.
					defaultPostureIndex = postureIndex;

				iff.exitChunk(TAG_POST);

				//-- Load the script selector associated with type.
				scriptSelectors[static_cast<ScriptSelectorVector::size_type>(postureIndex)] = ScriptSelector::create_generic_0002(iff);

			iff.exitForm(TAG_ENTR);
		}

	iff.exitForm(formTag);
}

// ======================================================================

ClientCombatPlaybackManager::BasePostureScriptSelector::BasePostureScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultPostureIndex) :
	ScriptSelector(),
	m_scriptSelectors(scriptSelectors),
	m_defaultPostureIndex(defaultPostureIndex)
{
	//-- Validate the default index.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_defaultPostureIndex, static_cast<int>(m_scriptSelectors.size()));
}

// ----------------------------------------------------------------------

ClientCombatPlaybackManager::BasePostureScriptSelector::~BasePostureScriptSelector()
{
	std::for_each(m_scriptSelectors.begin(), m_scriptSelectors.end(), PointerDeleter());
}

// ----------------------------------------------------------------------

inline int ClientCombatPlaybackManager::BasePostureScriptSelector::getDefaultPostureIndex() const
{
	return m_defaultPostureIndex;
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::BasePostureScriptSelector::addPlaybackScriptNames(CrcLowerStringSet &scriptNames) const
{
	const ScriptSelectorVector::const_iterator endIt = m_scriptSelectors.end();
	for (ScriptSelectorVector::const_iterator it = m_scriptSelectors.begin(); it != endIt; ++it)
	{
		if (*it)
			(*it)->addPlaybackScriptNames(scriptNames);
	}
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::BasePostureScriptSelector::createPlaybackScriptByPostureIndex(int postureIndex, const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const
{
	//-- Find the ScriptSelector matching the specified probability.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, postureIndex, static_cast<int>(m_scriptSelectors.size()));

	ScriptSelector *scriptSelector = m_scriptSelectors[static_cast<ScriptSelectorVector::size_type>(postureIndex)];

	//-- If no script selector for DEAD posture, try one for INCAPACITATED posture.
	//   This is a hack to handle the fact that the dead and incapacitated postures are
	//   treated exactly the same by the client combat playback system.
	if (!scriptSelector && (postureIndex == Postures::Dead))
		scriptSelector = m_scriptSelectors[static_cast<ScriptSelectorVector::size_type>(Postures::Incapacitated)];

	//-- If no script selector for the specified type, try the default type index.
	if (!scriptSelector)
		scriptSelector = m_scriptSelectors[static_cast<ScriptSelectorVector::size_type>(m_defaultPostureIndex)];

	if (!scriptSelector)
	{
#ifdef _DEBUG
		WARNING_STRICT_FATAL(true, ("failed to resolve BasePostureScriptSelector for posture id [%d] while loading key [%s].", postureIndex, s_currentlyLoadingKeyName));
#else
		WARNING_STRICT_FATAL(true, ("failed to resolve BasePostureScriptSelector for posture id [%d].", postureIndex));
#endif
		playbackScript = 0;
		return;
	}

	//-- Forward the create call to the type-specific ScriptSelector instance.
	scriptSelector->createPlaybackScript(info, playbackScript);
}

// ======================================================================
// class ClientCombatPlaybackManager::AttackerEndPostureScriptSelector
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ClientCombatPlaybackManager::AttackerEndPostureScriptSelector, true, 0, 0, 0);

// ======================================================================

ClientCombatPlaybackManager::ScriptSelector *ClientCombatPlaybackManager::AttackerEndPostureScriptSelector::create_0002(Iff &iff)
{
	ScriptSelectorVector  scriptSelectors(static_cast<ScriptSelectorVector::size_type>(ClientCombatPlaybackManager::AT_count));
	int                   defaultPostureIndex = 0;

	//-- Load the type data.
	load_0002(iff, TAG_AEPS, scriptSelectors, defaultPostureIndex);

	//-- Create the ScriptSelector.
	return new AttackerEndPostureScriptSelector(scriptSelectors, defaultPostureIndex);
}

// ======================================================================

ClientCombatPlaybackManager::AttackerEndPostureScriptSelector::AttackerEndPostureScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultPostureIndex) :
	BasePostureScriptSelector(scriptSelectors, defaultPostureIndex)
{
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::AttackerEndPostureScriptSelector::createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const
{
	//-- Get end posture index associated with attacker.
	const Object *const object       = info.getAttacker();
	const int           postureIndex = (object ? info.getAttackerPostureEnd() : getDefaultPostureIndex());

	//-- Forward the create call to the ScriptSelector associated with the specified posture index.
	createPlaybackScriptByPostureIndex(postureIndex, info, playbackScript);
}

// ======================================================================
// class ClientCombatPlaybackManager::DefenderEndPostureScriptSelector
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ClientCombatPlaybackManager::DefenderEndPostureScriptSelector, true, 0, 0, 0);

// ======================================================================

ClientCombatPlaybackManager::ScriptSelector *ClientCombatPlaybackManager::DefenderEndPostureScriptSelector::create_0002(Iff &iff)
{
	ScriptSelectorVector  scriptSelectors(static_cast<ScriptSelectorVector::size_type>(ClientCombatPlaybackManager::AT_count));
	int                   defaultPostureIndex = 0;

	//-- Load the type data.
	load_0002(iff, TAG_DEPS, scriptSelectors, defaultPostureIndex);

	//-- Create the ScriptSelector.
	return new DefenderEndPostureScriptSelector(scriptSelectors, defaultPostureIndex);
}

// ======================================================================

ClientCombatPlaybackManager::DefenderEndPostureScriptSelector::DefenderEndPostureScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultPostureIndex) :
	BasePostureScriptSelector(scriptSelectors, defaultPostureIndex)
{
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::DefenderEndPostureScriptSelector::createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const
{
	//-- Get end posture index associated with defender.
	const Object *const object       = info.getDefender();
	const int           postureIndex = (object ? info.getDefenderPostureEnd() : getDefaultPostureIndex());

	//-- Forward the create call to the ScriptSelector associated with the specified posture index.
	createPlaybackScriptByPostureIndex(postureIndex, info, playbackScript);
}

// ======================================================================
// class ClientCombatPlaybackManager::DefenderDispositionScriptSelector
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(ClientCombatPlaybackManager::DefenderDispositionScriptSelector, true, 0, 0, 0);

// ======================================================================

ClientCombatPlaybackManager::ScriptSelector *ClientCombatPlaybackManager::DefenderDispositionScriptSelector::create_0002(Iff &iff)
{
	ScriptSelectorVector  scriptSelectors(static_cast<ScriptSelectorVector::size_type>(ClientCombatActionInfo::DD_count));
	int                   defaultDispositionIndex = 0;

	//-- Read the raw data.
	iff.enterForm(TAG_DDSP);

		while (!iff.atEndOfForm())
		{
			iff.enterForm(TAG_ENTR);

				//-- Load type.
				iff.enterChunk(TAG_DISP);

					// Get the type index.
					int dispositionIndex = static_cast<int>(iff.read_int16());

					if (dispositionIndex < 0)
					{
						// When index is < 0, this means the entry is a default.
						// Default entries must come at the end.  This default implementation
						// will fail if every disposition has been specified directly (note: in this
						// case, a default makes no sense).
						dispositionIndex = findNullScriptSelectorIndex(scriptSelectors);
					}

					VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, dispositionIndex, static_cast<int>(ClientCombatActionInfo::DD_count));

					// Setup default type index to the last entry in the list.
					defaultDispositionIndex = dispositionIndex;

				iff.exitChunk(TAG_DISP);

				//-- Load the script selector associated with type.
				scriptSelectors[static_cast<ScriptSelectorVector::size_type>(dispositionIndex)] = ScriptSelector::create_generic_0002(iff);

			iff.exitForm(TAG_ENTR);
		}

	iff.exitForm(TAG_DDSP);

	//-- Create the ScriptSelector.
	return new DefenderDispositionScriptSelector(scriptSelectors, defaultDispositionIndex);
}

// ======================================================================

ClientCombatPlaybackManager::DefenderDispositionScriptSelector::DefenderDispositionScriptSelector(const ScriptSelectorVector &scriptSelectors, int defaultDispositionIndex) :
	ScriptSelector(),
	m_scriptSelectors(scriptSelectors),
	m_defaultDispositionIndex(defaultDispositionIndex)
{
	//-- Validate the default index.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, m_defaultDispositionIndex, static_cast<int>(m_scriptSelectors.size()));
}

// ----------------------------------------------------------------------

ClientCombatPlaybackManager::DefenderDispositionScriptSelector::~DefenderDispositionScriptSelector()
{
	std::for_each(m_scriptSelectors.begin(), m_scriptSelectors.end(), PointerDeleter());
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::DefenderDispositionScriptSelector::addPlaybackScriptNames(CrcLowerStringSet &scriptNames) const
{
	const ScriptSelectorVector::const_iterator endIt = m_scriptSelectors.end();
	for (ScriptSelectorVector::const_iterator it = m_scriptSelectors.begin(); it != endIt; ++it)
	{
		if (*it)
			(*it)->addPlaybackScriptNames(scriptNames);
	}
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::DefenderDispositionScriptSelector::createPlaybackScript(const ClientCombatActionInfo &info, PlaybackScript *&playbackScript) const
{
	//-- Get the defender disposition.
	const int dispositionIndex = static_cast<int>(info.getDefenderDefense());

	//-- Find the ScriptSelector matching the specified probability.
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, dispositionIndex, static_cast<int>(m_scriptSelectors.size()));

	ScriptSelector *scriptSelector = m_scriptSelectors[static_cast<ScriptSelectorVector::size_type>(dispositionIndex)];
	if (!scriptSelector)
	{
		//-- No script selector for the specified type, try the default type index.
		scriptSelector = m_scriptSelectors[static_cast<ScriptSelectorVector::size_type>(m_defaultDispositionIndex)];
	}

	if (!scriptSelector)
	{
#ifdef _DEBUG
		WARNING_STRICT_FATAL(true, ("failed to resolve DefenderDispositionScriptSelector for posture id [%d] while loading key [%s].", dispositionIndex, s_currentlyLoadingKeyName));
#else
		WARNING_STRICT_FATAL(true, ("failed to resolve DefenderDispositionScriptSelector for posture id [%d].", dispositionIndex));
#endif
		playbackScript = 0;
		return;
	}

	//-- Forward the create call to the type-specific ScriptSelector instance.
	scriptSelector->createPlaybackScript(info, playbackScript);
}

// ======================================================================
// class ClientCombatPlaybackManager: public static member functions
// ======================================================================

void ClientCombatPlaybackManager::install(const std::string &combatManagerFilename)
{
	InstallTimer const installTimer("ClientCombatPlaybackManager::install");

	DEBUG_FATAL(ms_installed, ("ClientCombatPlaybackManager already installed"));

	//-- mark as installed
	ms_installed = true;

	//-- install locally-defined variable fillers
	StringVariableFiller::install();
	FloatVariableFiller::install();
	IntVariableFiller::install();

	//-- Installs that go on the exit chain.  These don't need remove called.
	SingleScriptSelector::install();
	RandomScriptSelector::install();
	AttackerTypeScriptSelector::install();
	DefenderTypeScriptSelector::install();
	AttackerEndPostureScriptSelector::install();
	DefenderEndPostureScriptSelector::install();
	DefenderDispositionScriptSelector::install();

	//-- load initialization data
	load(combatManagerFilename);

	//-- Register debug flags.
	DebugFlags::registerFlag(ms_logCombatManagerActivity, "ClientGame/Character", "logCombatManager");

	//-- add removal to exit chain
	ExitChain::add(remove, "ClientCombatPlaybackManager");
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::registerVariableFiller(const Tag &fillerTag, VariableFillerCreateFunction createFunction, VariableFillerPostLoadFunction postLoadFunction)
{
	DEBUG_FATAL(!ms_installed, ("ClientCombatPlaybackManager not installed"));
	NOT_NULL(createFunction);

	//-- check if tag already registered
	const VariableFillerBindingMap::iterator lowerBoundResult = ms_variableFillerBindingMap.lower_bound(fillerTag);
	if ((lowerBoundResult != ms_variableFillerBindingMap.end()) && !ms_variableFillerBindingMap.key_comp()(fillerTag, lowerBoundResult->first))
	{
		WARNING(true, ("variable filler [0x%08x] already registered\n", static_cast<unsigned int>(fillerTag)));
		DEBUG_FATAL(true, ("see warning above"));
		return; //lint !e527 // unreachable // false in release
	}

	//-- add creation function
	IGNORE_RETURN(ms_variableFillerBindingMap.insert(lowerBoundResult, VariableFillerBindingMap::value_type(fillerTag, VariableFillerBinding(createFunction, postLoadFunction))));
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::deregisterVariableFiller(const Tag &fillerTag)
{
	DEBUG_FATAL(!ms_installed, ("ClientCombatPlaybackManager not installed"));

	//-- find map entry
	const VariableFillerBindingMap::iterator lowerBoundResult = ms_variableFillerBindingMap.lower_bound(fillerTag);
	if ((lowerBoundResult == ms_variableFillerBindingMap.end()) || ms_variableFillerBindingMap.key_comp()(fillerTag, lowerBoundResult->first))
	{
		WARNING(true, ("variable filler [0x%08x] not registered\n", static_cast<unsigned int>(fillerTag)));
		DEBUG_FATAL(true, ("see warning above"));
		return; //lint !e527 // unreachable // false in release
	}

	//-- remove map entry
	ms_variableFillerBindingMap.erase(lowerBoundResult);
}

// ----------------------------------------------------------------------
/**
 * Stop all playback scripts in which a given object is participating; however,
 * ensure that all other participants' playback script posture changes are preserved.
 */

void ClientCombatPlaybackManager::stopAllPlaybackScriptsForObject(Object const &object, int8 newPosture)
{
	s_playbackScriptVector.clear();
	PlaybackScriptManager::getPlaybackScriptsForActor(&object, s_playbackScriptVector);

	int const count = static_cast<int>(s_playbackScriptVector.size());
	for (int i = 0; i < count; ++i)
	{
		bool killScript = false;
		PlaybackScript *const script = const_cast<PlaybackScript*>(s_playbackScriptVector[static_cast<size_t>(i)]);
		if (script)
		{
			int const actorCount = script->getSupportedActorCount();

			// We only want to actually kill the script if the script cares about the posture for this object AND the script and input posture are different.
			for (int actorIndex = 0; actorIndex < actorCount; ++actorIndex)
			{
				int endPosture = -1;
				Object *const actor = script->getActor(actorIndex);
				if (actor && (actor == &object) && actorIndex == 0)
					killScript = killScript || (script->getIntVariable(TAG_AEPS, endPosture) && endPosture >= 0 && endPosture != newPosture);
				if (actor && (actor == &object) && actorIndex == 1)
					killScript = killScript || (script->getIntVariable(TAG_DEPS, endPosture) && endPosture >= 0 && endPosture != newPosture);
				if (actor && (actor == &object) && actorIndex > 1)
					killScript = true;
			}

			if (!killScript)
				continue;

			for (int actorIndex = 0; actorIndex < actorCount; ++actorIndex)
			{
				Object *const actor = script->getActor(actorIndex);
				if (actor && (actor != &object))
				{
					// Identified another actor in this script that is not the player for which we are killing the script.
					// If there is a posture change for this actor in this script, make sure it still happens.
					bool knowEndPosture = false;
					int  endPosture     = -1;

					if (actorIndex == 0)
						knowEndPosture = script->getIntVariable(TAG_AEPS, endPosture) && (endPosture >= 0);
					else if (actorIndex == 1)
						knowEndPosture = script->getIntVariable(TAG_DEPS, endPosture) && (endPosture >= 0);

					if (knowEndPosture)
					{
						CreatureObject *const creatureActor = dynamic_cast<CreatureObject*>(actor);
						if (creatureActor)
							creatureActor->setVisualPosture(static_cast<Postures::Enumerator>(endPosture));
					}
				}
			}

			PlaybackScriptManager::killPlaybackScript(script);
		}
	}
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::handleCombatAction(const ClientCombatActionInfo &info)
{
	//-- Find the ScriptSelector associated with the attacker's combat action id.
#ifdef _DEBUG
	std::string const *scriptName = NULL;
	{
		CrcStringMap::iterator const findIt = ms_crcLookupMap.find(info.getAttackerActionId());
		if (findIt != ms_crcLookupMap.end())
			scriptName = &findIt->second;
	}
#endif

	Object         *const attackerObject         = info.getAttacker();
//	CreatureObject *const attackerCreatureObject = dynamic_cast<CreatureObject*>(attackerObject);

	Object         *const defenderObject         = info.getDefender();
	CreatureObject *const defenderCreatureObject = dynamic_cast<CreatureObject*>(defenderObject);

	const int damageAmount = info.getDamageAmount();
	if (attackerObject && defenderObject && (damageAmount != 0))
	{
		CuiCombatManager::addDeferredCombatActionDamage(attackerObject->getNetworkId(), defenderObject->getNetworkId(), damageAmount);
	}

	//-- Handle client effects.
	Object *const playerObject = Game::getPlayer();

	if (attackerObject)
	{
		const int clientEffectId = info.getAttackerClientEffectId();
		if ((clientEffectId != 0) && !ConfigClientGame::getDisableCombatSpecialMoveEffects())
		{
			// Only handle client effects if the attacker or defender is the player.
			if ((attackerObject == playerObject) || (defenderObject == playerObject))
				queueSpecialMoveEffect(*attackerObject, clientEffectId);
		}
	}

	if (defenderObject)
	{
		const int clientEffectId = info.getDefenderClientEffectId();
		if ((clientEffectId != 0) && !ConfigClientGame::getDisableCombatSpecialMoveEffects())
		{
			// Only handle client effects if the attacker or defender is the player.
			if ((attackerObject == playerObject) || (defenderObject == playerObject))
				queueSpecialMoveEffect(*defenderObject, clientEffectId);
		}
	}

	//-- Handle the case where attacker and/or defender go incapacitated or dead.  We don't want to play combat actions multiple times for these.
	//const int attackerEndPosture = info.getAttackerPostureEnd();
	const int defenderEndPosture = info.getDefenderPostureEnd();

	//const int attackerCurrentPosture = attackerCreatureObject ? attackerCreatureObject->getVisualPosture() : 0;
	const int defenderCurrentPosture = defenderCreatureObject ? defenderCreatureObject->getVisualPosture() : 0;

	//bool const attackerEndsUpIncapOrDead  = attackerCreatureObject && ((attackerEndPosture == Postures::Dead) || (attackerEndPosture == Postures::Incapacitated));
	bool const defenderEndsUpIncapOrDead  = defenderCreatureObject && ((defenderEndPosture == Postures::Dead) || (defenderEndPosture == Postures::Incapacitated));

	//bool const attackerAlreadyGoingToIncapOrDead = attackerCreatureObject && (attackerCreatureObject->isPlaybackScriptAssignedToIncapacitateMe() || (attackerCurrentPosture == Postures::Incapacitated) || (attackerCurrentPosture == Postures::Dead));
	bool const defenderAlreadyGoingToIncapOrDead = defenderCreatureObject && (defenderCreatureObject->isPlaybackScriptAssignedToIncapacitateMe() || (defenderCurrentPosture == Postures::Incapacitated) || (defenderCurrentPosture == Postures::Dead));
	UNREF(defenderAlreadyGoingToIncapOrDead);

	if (defenderEndsUpIncapOrDead)
	{
		NOT_NULL(defenderCreatureObject);
		// @NOTE: TPERRY - commented out 
		// why do we need to stop all scripts? this interrupts any playing animation before the attack completes
		//		stopAllPlaybackScriptsForObject(*defenderCreatureObject);
		defenderCreatureObject->setPlaybackScriptIsAssignedToIncapacitateMe(true);
	}

	//-- Find the ScriptSelector associated with the attacker's combat action id.
	const uint32                attackerActionId = info.getAttackerActionId();
	const ScriptSelector *const scriptSelector   = lookupScriptSelector(attackerActionId);

	if (scriptSelector)
	{
		//-- Create and initialize a PlaybackScript instance for this combat action.

		DEBUG_REPORT_LOG(ConfigClientGame::getLogCombatPlaybackSelection(), ("CCPM: processing entry [%s].\n", scriptName != NULL ? scriptName->c_str() : "<invalid script>"));

		//   NOTE: this instance could be destroyed if priority dictates this PlaybackScript
		//   is lower priority than an in-progress script.  I could have engineered out
		//   the unecessary creation and setup of the PlaybackScript in the case where
		//   priority would toss it, but that would complicate this section.  I am ignoring
		//   this inefficiency unless it becomes a hotspot.
		PlaybackScript *playbackScript = 0;

		scriptSelector->createPlaybackScript(info, playbackScript);
		if (!playbackScript)
		{
			DEBUG_WARNING(true, ("failed to create PlaybackScript instance, ignoring."));
			return;
		}

		//-- Get the attacker's weapon type.  If ranged or somebody is going dead/incap, tell the priority system to play the attack in all cases.
		bool  alwaysPlayScript  = defenderEndsUpIncapOrDead;
		bool  attackerHasWeapon = false;
		bool  heavyWeaponForcePlay = false;

		Object const *const attackerWeaponObject = info.getAttackerWeapon();
		if (attackerWeaponObject)
		{
			SharedWeaponObjectTemplate const *const objectTemplate = dynamic_cast<SharedWeaponObjectTemplate const*>(attackerWeaponObject->getObjectTemplate());
			if (objectTemplate)
			{
				// If object is a ranged weapon, always play the script regardless of trumping rules.
				// alwaysPlayScript  = (objectTemplate->getAttackType() == SharedWeaponObjectTemplate::AT_ranged);
				attackerHasWeapon = true;
				heavyWeaponForcePlay = ( objectTemplate->getGameObjectType() >= SharedWeaponObjectTemplate::GOT_weapon_heavy_mine && objectTemplate->getGameObjectType() <= SharedWeaponObjectTemplate::GOT_weapon_heavy_special );
			}
		}
		DEBUG_REPORT_LOG(ConfigClientGame::getLogCombatPlaybackSelection(), ("CCPM: attackerHasWeapon [%s], alwaysPlayScript = [%s].\n", attackerHasWeapon ? "true" : "false", alwaysPlayScript ? "true" : "false"));

		//-- Submit the playbackScript.  The script will play if nothing of higher priority
		//   trumps it.
		if(heavyWeaponForcePlay)
			alwaysPlayScript = true;

		const bool scriptAccepted = PriorityPlaybackScriptManager::submitPlaybackScript(playbackScript, alwaysPlayScript);
		if (scriptAccepted)
		{
			//-- Override player target until next alter when things have settled down.
			// @todo revisit this.
			Object *const targetObject = playbackScript->getObjectVariable(TAG_ATRG);
			if (targetObject)
			{
				Object *const attacker = playbackScript->getActor(0);
				if (attacker)
				{
					int hardpointLocation = 0;
					playbackScript->getIntVariable(TAG_DHLC, hardpointLocation);
					CrcLowerString targetHardpoint = CombatEffectsManager::getHitLocationHardPointName(*targetObject, hardpointLocation);

					CreatureController *const controller = dynamic_cast<CreatureController*>(attacker->getController());
					if (controller)
						controller->overrideAnimationTarget(targetObject, true, targetHardpoint);
				}
			}
		}
	}
	else
	{
		//-- Collect some info regarding the combat action.
		const char *const attackerName = (attackerObject ? attackerObject->getDebugName() : 0);
		const char *const defenderName = (defenderObject ? defenderObject->getDebugName() : 0);

		//-- Display debug info regarding the combat action.
		WARNING(true, ("failed to find a ScriptSelector instance mapped to attacker action id [%d], action details follow.", attackerActionId));
		REPORT_LOG(true, ("-| attacker action id [%u].\n", attackerActionId));
		REPORT_LOG(true, ("-| attacker debugName [%s].\n", attackerName ? attackerName : "<no name>"));
		REPORT_LOG(true, ("-| defender debugName [%s].\n", defenderName ? defenderName : "<no name>"));
		REPORT_LOG(true, ("-| attacker type [%s].\n",      attackerObject ? getAppearanceTypeName(getAppearanceTypeIndex(*attackerObject)) : "<no type>"));
		REPORT_LOG(true, ("-| defender type [%s].\n",      defenderObject ? getAppearanceTypeName(getAppearanceTypeIndex(*defenderObject)) : "<no type>"));

		//-- Slam in the proper posture state for the defender
		if(defenderObject && defenderEndsUpIncapOrDead)
		{
			CreatureObject *defenderAsCreature = dynamic_cast<CreatureObject *>(defenderObject);
			if(defenderAsCreature)
				defenderAsCreature->setVisualPosture(static_cast<int8>(defenderEndPosture));
		}
		const int attackerEndPosture = info.getAttackerPostureEnd();
		bool const attackerEndsUpIncapOrDead  = ((attackerEndPosture == Postures::Dead) || (attackerEndPosture == Postures::Incapacitated));
		if(attackerObject && attackerEndsUpIncapOrDead)
		{
			CreatureObject *attackerAsCreature = dynamic_cast<CreatureObject *>(attackerObject);
			if(attackerAsCreature)
				attackerAsCreature->setVisualPosture(static_cast<int8>(attackerEndPosture));
		}
	}
}

// ----------------------------------------------------------------------

int ClientCombatPlaybackManager::getCombatActionKeyCount()
{
#ifdef _DEBUG
	return static_cast<int>(ms_crcLookupMap.size());
#else
	return 0;
#endif
}

// ----------------------------------------------------------------------

std::string &ClientCombatPlaybackManager::getCombatActionKeyString(int index)
{
#ifdef _DEBUG

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getCombatActionKeyCount());

	CrcStringMap::iterator it = ms_crcLookupMap.begin();
	std::advance(it, index);

	return it->second;

#else

	UNREF(index);

	static std::string empty;
	return empty;

#endif
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void ClientCombatPlaybackManager::lookupActionName(uint32 actionId, char *buffer, int bufferLength)
{
	DEBUG_FATAL(bufferLength < 1, ("must provide a buffer with bufferlength of at least 1, length = [%d].", bufferLength));
	NOT_NULL(buffer);

	CrcStringMap::iterator const findIt = ms_crcLookupMap.find(actionId);
	if (findIt == ms_crcLookupMap.end())
		IGNORE_RETURN(snprintf(buffer, static_cast<size_t>(bufferLength - 1), "<actionId 0x%08x not found>", actionId));
	else
		strncpy(buffer, findIt->second.c_str(), static_cast<size_t>(bufferLength - 1));

	buffer[bufferLength - 1] = '\0';
}

#endif

// ======================================================================
// private static member functions
// ======================================================================

void ClientCombatPlaybackManager::remove()
{
	DEBUG_FATAL(!ms_installed, ("ClientCombatPlaybackManager not installed"));

	//-- release data vectors
	std::for_each(ms_scriptSelectorMap.begin(), ms_scriptSelectorMap.end(), PointerDeleterPairSecond());
	ms_scriptSelectorMap.clear();

	//-- remove locally-defined variable fillers
	IntVariableFiller::remove();
	FloatVariableFiller::remove();
	StringVariableFiller::remove();

	ms_variableFillerBindingMap.clear();

	//-- release preloaded PlaybackScriptTemplate instances
	std::for_each(ms_preloadedPlaybackScriptTemplates.begin(), ms_preloadedPlaybackScriptTemplates.end(), VoidMemberFunction(&PlaybackScriptTemplate::release));
	PlaybackScriptTemplateVector().swap(ms_preloadedPlaybackScriptTemplates);

	ms_playbackScriptTemplateNames.clear();

#ifdef _DEBUG
	ms_crcLookupMap.clear();
#endif

	ms_installed = false;
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::load(const std::string &combatManagerFilename)
{
#ifdef _DEBUG
	s_currentlyLoadingKeyName[0] = '\0';
#endif

	//-- load the iff
	Iff iff(combatManagerFilename.c_str());

	iff.enterForm(TAG_CBTM);

		const Tag version = iff.getCurrentName();
		switch (version)
		{
			case TAG_0000:
			case TAG_0001:
				WARNING_STRICT_FATAL(true, ("ClientCombatPlaybackManager: file [%s] out of date, version no longer supported.", combatManagerFilename.c_str()));
				iff.exitForm(true);
				return;

			case TAG_0002:
				load_0002(iff);
				break;

			default:
				{
					char name[5];
					ConvertTagToString(version, name);
					WARNING_STRICT_FATAL(true, ("unsupported combat manager version [%s]", name));

					iff.exitForm(true);
					return;
				}
		}

	iff.exitForm(TAG_CBTM);

#ifdef _DEBUG
	s_currentlyLoadingKeyName[0] = '\0';
#endif

	//-- preload all PlaybackScriptTemplate instances
	preloadPlaybackScriptTemplates(ms_playbackScriptTemplateNames);
	ms_playbackScriptTemplateNames.clear();

	//-- run VariableFiller post-load functions
	executePostLoadFunctions();
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::load_0002(Iff &iff)
{
	char  buffer[1024];

	iff.enterForm(TAG_0002);

		//-- Load all entries.
		while (!iff.atEndOfForm())
		{
			//-- Load the entry.
			iff.enterForm(TAG_ENTR);

				// Load the key.
				iff.enterChunk(TAG_KEY);

					iff.read_string(buffer, sizeof(buffer) - 1);
					const uint32 key = Crc::normalizeAndCalculate(buffer);

#ifdef _DEBUG
					//-- Save the key - string mapping.
					IGNORE_RETURN(ms_crcLookupMap.insert(CrcStringMap::value_type(key, buffer)));

					//-- Remember the curretn key so we can report other errors with a reference to the bad key entry.
					strncpy(s_currentlyLoadingKeyName, buffer, sizeof(s_currentlyLoadingKeyName) - 1);
					s_currentlyLoadingKeyName[sizeof(s_currentlyLoadingKeyName) - 1] = '\0';
#endif

				iff.exitChunk(TAG_KEY);

				// Load the ScriptSelector.
				ScriptSelector *const scriptSelector = ScriptSelector::create_generic_0002(iff);

			iff.exitForm(TAG_ENTR);

			//-- Track the entry.
			if (scriptSelector)
			{
				// Track the script names made available by this script selector.
				// This is used to preload the .pst files.
				scriptSelector->addPlaybackScriptNames(ms_playbackScriptTemplateNames);

				// Add to ScriptSelector map.
				ScriptSelectorMap::iterator lowerBoundResult = ms_scriptSelectorMap.lower_bound(key);
				if ((lowerBoundResult != ms_scriptSelectorMap.end()) && !ms_scriptSelectorMap.key_comp()(key, lowerBoundResult->first))
				{
					// Report error: entry already maps to the given key.
					WARNING(true, ("ClientCombatPlaybackManager: multiple entries map to same value key=[%d], string=[%s], dropping new entry.", key, buffer));
					delete scriptSelector;
				}
				else
				{
					// Add entry to map.
					IGNORE_RETURN(ms_scriptSelectorMap.insert(lowerBoundResult, ScriptSelectorMap::value_type(key, scriptSelector)));
				}
			}
		}

	iff.exitForm(TAG_0002);
}

// ----------------------------------------------------------------------
/**
 * Return an appearance type index based on the type of appearance
 * class associated with the given object.
 *
 * @param object  the object for which an appearance type index will
 *                be determined.  The object's appearance is used
 *                for this determination.
 *
 * @return        the appearance type index for the object's appearance.
 *                If the object has no appearance or if the appearance
 *                is not handled, returns static_cast<int>(AT_none).
 */

int ClientCombatPlaybackManager::getAppearanceTypeIndex(const Object &object)
{
	const Appearance *const baseAppearance = object.getAppearance();
	if (!baseAppearance)
	{
		return static_cast<int>(AT_none);
	}

	if (dynamic_cast<const SkeletalAppearance2*>(baseAppearance))
	{
		return static_cast<int>(AT_skeletal);
	}
	else
	{
		return static_cast<int>(AT_none);
	}
}

// ----------------------------------------------------------------------

const char *const ClientCombatPlaybackManager::getAppearanceTypeName(int typeIndex)
{
	switch (typeIndex)
	{
		case AT_skeletal:
			return "skeletal";

		case AT_none:
		default:
			return "non-skeletal";
	}
}

// ----------------------------------------------------------------------
/**
 * Execute all the post-load functions for registered VariableFillers.
 */

void ClientCombatPlaybackManager::executePostLoadFunctions()
{
	const VariableFillerBindingMap::iterator endIt = ms_variableFillerBindingMap.end();
	for (VariableFillerBindingMap::iterator it = ms_variableFillerBindingMap.begin(); it != endIt; ++it)
	{
		const VariableFillerBinding &binding = it->second;

		if (binding.m_postLoadFunction)
		{
			(*binding.m_postLoadFunction)();
		}
	}
}

// ----------------------------------------------------------------------

void ClientCombatPlaybackManager::preloadPlaybackScriptTemplates(const CrcLowerStringSet &names)
{
	DEBUG_FATAL(!ms_preloadedPlaybackScriptTemplates.empty(), ("expecting no preloaded templates at this time"));

	//-- Size up the container to store all preloaded PlaybackScriptTemplate instances.  These will need
	//   to be released upon shutdown.
	ms_preloadedPlaybackScriptTemplates.reserve(names.size());

	//-- Fetch a PlaybackScriptTemplate for every name in the list.
	const CrcLowerStringSet::const_iterator endIt = names.end();
	for (CrcLowerStringSet::const_iterator it = names.begin(); it != endIt; ++it)
	{
		// Fetch the template.
		const PlaybackScriptTemplate *const psTemplate = PlaybackScriptTemplateList::fetch(*it);

		if (psTemplate)
		{
			// Only add the template to the prefetch list if it is non-null.
			ms_preloadedPlaybackScriptTemplates.push_back(psTemplate);
		}
	}
}

// ----------------------------------------------------------------------

const ClientCombatPlaybackManager::ScriptSelector *ClientCombatPlaybackManager::lookupScriptSelector(uint32 attackerActionId)
{
	//-- Search for the ScriptSelector instance with the given attacker action id.
	const ScriptSelectorMap::iterator findIt = ms_scriptSelectorMap.find(attackerActionId);

	//-- Return it if found; otherwise, return NULL.
	if (findIt != ms_scriptSelectorMap.end())
		return findIt->second;
	else
		return 0;
}

// ----------------------------------------------------------------------

int ClientCombatPlaybackManager::findNullScriptSelectorIndex(const ScriptSelectorVector &ssVector)
{
	const ScriptSelectorVector::size_type  size = ssVector.size();
	for (ScriptSelectorVector::size_type  i = 0; i < size; ++i)
	{
		if (ssVector[i] == NULL)
		{
			//-- Found an empty entry.
			return static_cast<int>(i);
		}
	}

	//-- Did not find a NULL entry.
	return -1;
}

// ======================================================================
