// ======================================================================
//
// SetupUi.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupUi_H
#define INCLUDED_SetupUi_H

// ======================================================================

class SetupUi
{
public:

	class Data
	{
	public:

		Data ();

	public:

		UiReport::SetFlagsFunction      m_reportSetFlagsFunction;
		UiReport::PrintfFunction        m_reportPrintfFunction;
		UiReport::ProfilerEnterFunction m_profilerEnterFunction;
		UiReport::ProfilerLeaveFunction m_profilerLeaveFunction;
		UiReport::MemoryManagerVerifyFunction m_memoryManagerVerifyFunction;
		bool                            m_memoryBlockManagerDebugDumpOnRemove;
	};

public:

	static void install (const Data& data);
	static void remove ();

	static bool isInstalled ();

private:

	SetupUi ();
	SetupUi (const SetupUi&);
	SetupUi& operator= (const SetupUi&);
};

// ======================================================================

#endif
