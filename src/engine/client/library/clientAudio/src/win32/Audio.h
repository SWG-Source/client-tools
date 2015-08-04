// ============================================================================
//
// Audio.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_Audio_H
#define INCLUDED_Audio_H

#include "clientAudio/SoundId.h"

class AudioSampleInformation;
class CellProperty;
class CrcString;
class Iff;
class Object;
class SampleId;
class Vector;
class Plane;

namespace AudioCapture
{
class ICallback;
}

// ============================================================================
//
// Audio
//
// ============================================================================

//-----------------------------------------------------------------------------
class Audio
{
// These classes are friends to access the protected functions of this class

friend class Sound2;
friend class Sound2d;
friend class Sound3d;
friend class ConfigClientAudio;
friend class SoundTemplate;
friend class Sound2dTemplate;
friend class SoundTemplateWidget;
friend class Sample2d;
friend class Sample3d;
friend class SampleStream;

public:

	enum AttenuationMethod
	{
		AM_none,
		AM_2d,
		AM_3d
	};

	enum PlayBackStatus
	{
		PS_doesNotExist = 0,
		PS_notStarted,
		PS_playing,
		PS_done
	};

	enum SoundCategory
	{
		SC_ambient = 0,
		SC_explosion,
		SC_item,
		SC_movement,
		SC_userInterface,
		SC_vehicle,
		SC_vocalization,
		SC_weapon,
		SC_backGroundMusic,
		SC_playerMusic,
		SC_machine,
		SC_installation,
		SC_combatMusic,
		SC_voiceover,
		SC_bufferedSound,
		SC_bufferedMusic,
		SC_count
	};

	enum RoomType
	{
		RT_alley = 0,
		RT_arena,
		RT_auditorium,
		RT_bathRoom,
		RT_carpetedHallway,
		RT_cave,
		RT_city,
		RT_concertHall,
		RT_dizzy,
		RT_drugged,
		RT_forest,
		RT_generic,
		RT_hallway,
		RT_hangar,
		RT_livingRoom,
		RT_mountains,
		RT_paddedCell,
		RT_parkingLot,
		RT_plain,
		RT_psychotic,
		RT_quarry,
		RT_room,
		RT_sewerPipe,
		RT_stoneCorridor,
		RT_stoneRoom,
		RT_underWater,
		RT_notSupported
	};

public:

	// Setup
	//----------------------------------

	static bool        install();
	static void        remove();

	static void        lock();
	static void        unLock();

	static void        setLargePreMixBuffer();
	static void        setNormalPreMixBuffer();
	static void        silenceAllNonBackgroundMusic();
	static void        unSilenceAllNonBackgroundMusic();
	static void        fadeAllNonVoiceover();
	static void        unFadeAllNonVoiceover();

	static void        fadeAll();
	static void        unfadeAll();

	static void setSoundFallOffPower(int const power);

	// Enabled/Disable the entire audio system

	static bool        isEnabled();
	static void        setEnabled(bool enabled);

	static bool        isMilesEnabled();
	static void *      getMilesDigitalDriver();

	static void        setRequestedMaxNumberOfSamples(int const max);
	static int         getRequestedMaxNumberOfSamples();

	static void        setMaxCached2dSampleSize(int bytes);
	static int         getMaxCached2dSampleSize();

	//static void        setMaxCacheSize(int bytes);
	//static int         getMaxCacheSize();

	// Update
	//----------------------------------

	static void        alter(float const deltaTime, Object const *listener);
	static void        serve();

	// Volumes
	//----------------------------------

	static void        setSoundVolume(SoundId const &soundId, float volume);
	static void        setSoundPitchDelta(SoundId const &soundId, float pitchDelta);

	static void        setMasterVolume(float volume);
	static float       getMasterVolume();
	static float       getDefaultMasterVolume();

	static void        setSoundEffectVolume(float volume);
	static float       getSoundEffectVolume();
	static float       getDefaultSoundEffectVolume();

	static void        setBackGroundMusicVolume(float volume);
	static float       getBackGroundMusicVolume();
	static float       getDefaultBackGroundMusicVolume();

	static void        setPlayerMusicVolume(float volume);
	static float       getPlayerMusicVolume();
	static float       getDefaultPlayerMusicVolume();

	static void        setUserInterfaceVolume(float volume);
	static float       getUserInterfaceVolume();
	static float       getDefaultUserInterfaceVolume();

	static void        setFadeAllFactor(float factor);
	static float       getFadeAllFactor();
	static float       getDefaultFadeAllFactor();

	static void        setBufferedSoundVolume(float volume);
	static void        setBufferedMusicVolume(float volume);

	static void        setAmbientEffectVolume(float volume);
	static float       getAmbientEffectVolume();
	static float       getDefaultAmbientEffectVolume();

	// Sounds
	//----------------------------------

	static SoundId playSound(char const * const path); // Only use for stereo sounds
	static SoundId     playSound(char const *path, CellProperty const * const parentCell);
	static SoundId     playSound(char const *path, Vector const &position, CellProperty const * const parentCell);
	static SoundId     attachSound(char const *path, Object const *object, char const *hardPointName = NULL);

	static SoundId     createSoundId(char const *path);
	static void        playSound(SoundId &soundId, Vector const *position, CellProperty const * const parentCell);

	static void        playBufferedSound(char const * const buffer, uint32 bufferLength, char const * const extension);
	static void        playBufferedMusic(char const * const buffer, uint32 bufferLength, char const * const extension);

	static void        stopBufferedSound();
	static void        stopBufferedMusic();
	static void        silenceNonBufferedMusic(bool silence);

	static SoundId     restartSound(char const *path, Vector const *position = NULL, float const fadeOutTime = 0.0f);
	static void        restartSound(SoundId &soundId, Vector const *position = NULL, float const fadeOutTime = 0.0f);

	static void        detachSound(SoundId const &soundId, float const fadeOutTime = 0.0f);
	static void        detachSound(Object const &object, float const fadeOutTime = 0.0f);

	static void        stopSound(SoundId const &soundId, float const fadeOutTime = 0.0f);
	static void        stopAllSounds(float const fadeOutTime = 0.0f);

	static bool        isSoundPlaying(SoundId const &soundId);
	static void        setAutoDelete(SoundId const &soundId, bool const autoDelete);

	static void        setCurrentSoundTime(SoundId const &soundId, int const milliSecond);
	static bool        getCurrentSoundTime(SoundId const &soundId, int &totalMilliSeconds, int &currentMilliSeconds);
	static bool        getCurrentSoundTime(SoundId const &soundId, int &milliSecond);
	static bool        getTotalSoundTime(SoundId const &soundId, int &milliSecond);

	static void        setSoundPosition_w(SoundId const &soundId, Vector const &position_w);
	static Vector      getSoundPosition_w(SoundId const &soundId);

	typedef void (*EndOfSampleCallBack)();
	static void        setEndOfSampleCallBack(SoundId const &soundId, EndOfSampleCallBack callBack);

	typedef stdvector<Sound2*>::fwd SoundVector;
	static void        getSoundsAttachedToObject (const Object & obj, SoundVector & sv);
	static Sound2 *    getSoundById (const SoundId & soundId);
	static void transferOwnershipOfSounds(Object const & previousOwner, Object const & newOwner, Plane const * partition);

	// Sound Status
	//----------------------------------

	static PlayBackStatus getSoundPlayBackStatus(SoundId const &soundId);

	// Environmental effects
	//----------------------------------
	
	static void                        setRoomType(RoomType const roomType);
	static RoomType                    getRoomType();
	static char const * const          getRoomTypeString();

	static bool                        isRoomTypeSupported();
	static void                        setSampleOcclusion(SampleId const &sampleId, float const occlusion);
	static void                        setSampleObstruction(SampleId const &sampleId, float const obstruction);

	// Providers of 3d
	//----------------------------------

	static std::string const &         getCurrent3dProvider();
	static bool                        set3dProvider(std::string const &provider);
	static bool                        setDefault3dProvider();
	static stdvector<std::string>::fwd get3dProviders();

	// Utilities
	//----------------------------------

	static Object const * const   getListener();
	static Vector const &         getListenerPosition();
	static int                    getCacheHitCount();
	static int                    getCacheMissCount();
	static int                    getCachedSampleCount();
	static int                    getCurrentCacheSize();
	static void                   getCurrentSample(SoundId const &soundId, TemporaryCrcString &path);
	static void                   getCurrentSampleTime(SoundId const &soundId, float &timeTotal, float &timeCurrent);
	static int                    getDigitalCpuPercent();
	static int                    getDigitalLatency();
	static int                    getMaxDigitalMixerChannels();
	static std::string            getMilesVersion();
	static int                    getSample2dCount();
	static int                    getSample3dCount();
	static AudioSampleInformation getSampleInformation(std::string const &path);
	static int                    getSampleSize(char const *path);
	static int                    getSampleStreamCount();
	static int                    getSampleCount();
	static std::string            getSampleType(void *fileImage, int fileSize);
	static int                    getSoundCount();
	static float                  getSoundTemplateVolume(SoundId const &soundId);
	static float                  getSoundAttenuation(SoundId const &soundId);
	static float                  getSoundVolume(SoundId const &soundId);
	static float                  getSoundPitchDelta(SoundId const &soundId);  // Adjustment in half steps
	static bool                   getLoopOffsets(CrcString const &path, int &loopStartOffset, int &loopEndOffset);
	static bool                   isSampleAtEnd(SoundId const &soundId);
	static float                  getDistanceAtVolumeCutOff(SoundId const &soundId);

	static bool                   isToolApplication();
	static void                   setToolApplication(bool const toolApplication);

	// SoundEditor access
	//----------------------------------

	static SoundId                playSound(Iff &iff);
	static SoundId                playSound(Iff &iff, Vector const &position);
	static float                  getFallOffDistance(float const minDistance);

	static bool                   isSampleValid(SampleId const &sampleId);
	static bool                   isSoundValid(SoundId const &soundId);
	static bool                   isSampleForSoundIdPlaying(SoundId const &soundId);
	static void                   releaseSampleId(Sound2 const &sound);
	static bool                   queueSample(Sound2 &sound, bool const soundIsAlreadyPlaying);

	static void                   setDebugEnabled(bool const enabled);
	static bool                   isDebugEnabled();

	// AudioCapture
	//----------------------------------
#if PRODUCTION == 0
	static bool                   getAudioCaptureConfig(int& samplesPerSec, int& bitsPerSample, int& channels);
	static bool                   startAudioCapture(AudioCapture::ICallback* pCallback);
	static void                   stopAudioCapture();
#endif // PRODUCTION

protected:

	// Helper functions that friend classes access, these are the only functions
	// that friend classes should access

	static SoundId        attachSound(Iff &iff, Object const *object, char const *hardPointName=0);

	static PlayBackStatus getSamplePlayBackStatus(SampleId const &sampleId);
	static float          getSampleVolume(SampleId const &sampleId);
	static int            getSamplePlayBackRate(SampleId const &sampleId);

	static void           getSampleTime(SampleId const &sampleId, float &timeTotal, float &timeCurrent);
	static int            getSampleCurrentTime(SampleId const &sampleId);
	static void           setSampleCurrentTime(SampleId const &sampleId, int const milliSeconds);
	static int            getSampleTotalTime(SampleId const &sampleId);

	static void           setSamplePlayBackRate(SampleId const &sampleId, int const playBackRate, float const playBackRateDelta = 0.0f);
	static void           setSampleVolume(SampleId const &sampleId, float const volume);

	static float          getSampleEffectsLevel(SampleId const &sampleId);
	static void           setSampleEffectsLevel(SampleId const &sampleId, float const effectLevel);

	static void           setSamplePosition_w(SampleId const &sampleId, Vector const &position_w);

	static bool           getSoundWorldPosition(SoundId const &soundId, Vector &position);

	static CrcString const *increaseReferenceCount(const char *path, bool const cacheSample);
	static void             decreaseReferenceCount(CrcString const &path);

	static float          getObstruction();
	static float          getOcclusion();

#ifdef _DEBUG
	// Runtime statistics

	static void      showAudioDebug();
	static void      debugDumpAudioText();
#endif // _DEBUG

private:

	static SoundId        playSound(Iff &iff, Vector const * const position, CellProperty const * const parentCell);
	static SoundId        playSound(char const *path, Vector const * const position, CellProperty const * const parentCell);
	static void startSample(Sound2 & sound);

	// Disabled

	Audio();
	Audio(Audio const &);
	Audio &operator=(Audio const &);
	~Audio();
};

// ============================================================================

#endif // INCLUDED_Audio_H
