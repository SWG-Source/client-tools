// ======================================================================
//
// Direct3d11_Unimplemented.h
// copyright (c) 2026 Galaxies Reborn
//
// Accounting for Gl_api slots this backend has not implemented yet.
//
// Every slot the engine can call must be assigned to something -- several are
// dereferenced through NOT_NULL in Graphics.cpp -- so during bring-up there is
// no way to avoid having entry points that do not work yet. What there is a way
// to avoid is having them lie. An empty body returns control to the engine as
// though the work happened, and the result is a renderer that looks wired up
// and silently drops a feature: fog whose density constant is never uploaded,
// an alpha fade that never fades, a scissor rectangle that never clips.
//
// So every unimplemented slot goes through here instead, and gets:
//
//   - a hit counter, in every configuration including PRODUCTION
//   - one WARNING naming the slot, the first time it is reached. WARNING is
//     live in Release (Fatal.h:47), so this survives into a shipping build
//   - a DEBUG_FATAL, which is a NOP in Release (Fatal.h:42-44) but stops a
//     developer build on the spot
//   - a line in an end-of-run report, so a run that touched twelve
//     unimplemented slots says so rather than leaving it to be noticed
//
// Slots that must return an object use DX11_NOT_IMPLEMENTED_FATAL instead.
// There is no honest value for those: returning null hands the engine a null
// *GraphicsData that it stores and dereferences later, turning a missing
// feature into an access violation with nothing in the log to explain it.
//
// ======================================================================

#ifndef INCLUDED_Direct3d11_Unimplemented_H
#define INCLUDED_Direct3d11_Unimplemented_H

// ======================================================================

class Direct3d11_UnimplementedSlot
{
public:

	explicit Direct3d11_UnimplementedSlot(char const *slotName);

	void          hit();
	void          hitFatal();

	static void   report();
	static int    getSlotsReached();

private:

	Direct3d11_UnimplementedSlot();
	Direct3d11_UnimplementedSlot(Direct3d11_UnimplementedSlot const &);
	Direct3d11_UnimplementedSlot &operator =(Direct3d11_UnimplementedSlot const &);

private:

	char const                    *m_slotName;
	int                            m_hits;
	bool                           m_warned;
	Direct3d11_UnimplementedSlot  *m_next;

	// Intrusive list of every slot object. A raw pointer with static storage is
	// zero-initialised before any constructor runs, so registration from a
	// function-local static is safe whenever the first call happens.
	static Direct3d11_UnimplementedSlot *ms_first;
};

// ======================================================================

// For slots that return void, or a value the engine can survive being wrong.
#define DX11_NOT_IMPLEMENTED(slotName)                                        \
	do                                                                        \
	{                                                                         \
		static Direct3d11_UnimplementedSlot s_unimplementedSlot(slotName);    \
		s_unimplementedSlot.hit();                                            \
	} while (false)

// For slots that must return an object. Fatal in every configuration.
#define DX11_NOT_IMPLEMENTED_FATAL(slotName)                                  \
	do                                                                        \
	{                                                                         \
		static Direct3d11_UnimplementedSlot s_unimplementedSlot(slotName);    \
		s_unimplementedSlot.hitFatal();                                       \
	} while (false)

// ======================================================================

#endif
