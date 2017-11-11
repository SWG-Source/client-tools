// ============================================================================
//
// ConfigClientAudio.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ConfigClientAudio_H
#define INCLUDED_ConfigClientAudio_H

//-----------------------------------------------------------------------------
class ConfigClientAudio
{
public:

	static void  install();
	static void  remove();

public:

	static int   getMaxCached2dSampleSize();
	static float getObstruction();
	static float getOcclusion();

private:

	// Disabled

	ConfigClientAudio();
	~ConfigClientAudio();
	ConfigClientAudio(const ConfigClientAudio &);
	ConfigClientAudio &operator =(const ConfigClientAudio &);
};

// ============================================================================

#endif // INCLUDED_ConfigClientAudio_H
