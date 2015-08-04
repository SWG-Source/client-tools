// ======================================================================
//
// ClientCombatPlaybackManager.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_ClientCombatPlaybackManager_H
#define INCLUDED_ClientCombatPlaybackManager_H

// ======================================================================

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Tag.h"

class ClientCombatActionInfo;
class CrcLowerString;
class Iff;
class Object;
class PlaybackScript;
class PlaybackScriptTemplate;

// ======================================================================

class ClientCombatPlaybackManager
{
public:

	// See declaration following class.
	class VariableFiller;

	typedef VariableFiller *(*VariableFillerCreateFunction)(Iff &iff);
	typedef void            (*VariableFillerPostLoadFunction)();

	// Logically should be private but needs to be public.
	class SingleScriptSelector;
	friend class SingleScriptSelector;
	
	class RandomScriptSelector;
	friend class RandomScriptSelector;

	class BaseTypeScriptSelector;
	friend class BaseTypeScriptSelector;

	class AttackerTypeScriptSelector;
	friend class AttackerTypeScriptSelector;

	class DefenderTypeScriptSelector;
	friend class DefenderTypeScriptSelector;

	class BasePostureScriptSelector;
	friend class BasePostureScriptSelector;

	class AttackerEndPostureScriptSelector;
	friend class AttackerEndPostureScriptSelector;

	class DefenderEndPostureScriptSelector;
	friend class DefenderEndPostureScriptSelector;
	
	class DefenderDispositionScriptSelector;
	friend class DefenderDispositionScriptSelector;

public:

	static void install(const std::string &combatManagerFilename);

	static void registerVariableFiller(const Tag &fillerTag, VariableFillerCreateFunction createFunction, VariableFillerPostLoadFunction postLoadFunction = NULL);
	static void deregisterVariableFiller(const Tag &fillerTag);

	static void stopAllPlaybackScriptsForObject(Object const &object, int8 newPosture);
	static void handleCombatAction(const ClientCombatActionInfo &info);

	static int           getCombatActionKeyCount();
	static std::string  &getCombatActionKeyString(int index);

	// @todo move to ConfigClientGame.
	static bool getLogCombatManagerActivity();

#ifdef _DEBUG
	static void 	               lookupActionName(uint32 actionId, char *buffer, int bufferLength);
#endif

private:

	class BaseStringVariableFiller;

	class StringVariableFiller;
	friend class StringVariableFiller;

	class FloatVariableFiller;
	friend class FloatVariableFiller;

	class IntVariableFiller;
	friend class IntVariableFiller;

	class ScriptSelector;

	struct VariableFillerBinding;

	typedef stdset<CrcLowerString>::fwd                    CrcLowerStringSet;
	typedef stdmap<const uint32, std::string>::fwd         CrcStringMap;
	typedef stdvector<const PlaybackScriptTemplate*>::fwd  PlaybackScriptTemplateVector;
	typedef stdmap<const uint32, ScriptSelector*>::fwd     ScriptSelectorMap;
	typedef stdvector<ScriptSelector*>::fwd                ScriptSelectorVector;
	typedef stdmap<const Tag, VariableFillerBinding>::fwd  VariableFillerBindingMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum AppearanceType
	{
		AT_none       = -1,
		AT_skeletal   = 0,
		AT_static,
		//---
		AT_count
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	static void                  remove();

	static void                  load(const std::string &combatManagerFilename);
	static void                  load_0002(Iff &iff);

	static int                   getAppearanceTypeIndex(const Object &object);
	static const char *const     getAppearanceTypeName(int typeIndex);

	static void                  executePostLoadFunctions();
	static void                  preloadPlaybackScriptTemplates(const CrcLowerStringSet &names);

	static const ScriptSelector *lookupScriptSelector(uint32 attackerActionId);

	static int                   findNullScriptSelectorIndex(const ScriptSelectorVector &ssVector);

private:

	static bool                          ms_installed;

	static ScriptSelectorMap             ms_scriptSelectorMap;
	static VariableFillerBindingMap      ms_variableFillerBindingMap;

	static CrcLowerStringSet             ms_playbackScriptTemplateNames;
	static PlaybackScriptTemplateVector  ms_preloadedPlaybackScriptTemplates;

#ifdef _DEBUG
	static CrcStringMap                  ms_crcLookupMap;
#endif

	static bool                          ms_logCombatManagerActivity;

};

// ======================================================================

class ClientCombatPlaybackManager::VariableFiller
{
public:

	virtual ~VariableFiller();

	virtual void fillPlaybackScriptVariable(PlaybackScript &playbackScript) const = 0;

protected:

	explicit VariableFiller(const Tag &variableTag);

	const Tag &getVariableTag() const;

	// disabled
	VariableFiller();
	VariableFiller(const VariableFiller&);
	VariableFiller &operator =(const VariableFiller&);

private:

	Tag  m_variableTag;

};

// ======================================================================
// class ClientCombatPlaybackManager::VariableFiller
// ======================================================================

inline const Tag &ClientCombatPlaybackManager::VariableFiller::getVariableTag() const
{
	return m_variableTag;
}

// ======================================================================
// class ClientCombatPlaybackManager
// ======================================================================

inline bool ClientCombatPlaybackManager::getLogCombatManagerActivity()
{
	return ms_logCombatManagerActivity;
}

// ======================================================================

#endif
