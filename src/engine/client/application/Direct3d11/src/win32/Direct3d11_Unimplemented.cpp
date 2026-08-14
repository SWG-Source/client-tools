// ======================================================================
//
// Direct3d11_Unimplemented.cpp
// copyright (c) 2026 Galaxies Reborn
//
// ======================================================================

#include "FirstDirect3d11.h"
#include "Direct3d11_Unimplemented.h"

// ======================================================================

Direct3d11_UnimplementedSlot *Direct3d11_UnimplementedSlot::ms_first;

// ======================================================================

Direct3d11_UnimplementedSlot::Direct3d11_UnimplementedSlot(char const *slotName)
	: m_slotName(slotName),
	  m_hits(0),
	  m_warned(false),
	  m_next(ms_first)
{
	ms_first = this;
}

// ----------------------------------------------------------------------

void Direct3d11_UnimplementedSlot::hit()
{
	++m_hits;

	if (!m_warned)
	{
		m_warned = true;
		WARNING(true, ("Direct3d11: Gl_api slot %s is not implemented yet. Whatever it controls is not happening.", m_slotName));
	}

	DEBUG_FATAL(true, ("Direct3d11: Gl_api slot %s is not implemented yet.", m_slotName));
}

// ----------------------------------------------------------------------
/**
 * Report a slot that has no honest return value.
 *
 * The engine stores what the create* slots return and dereferences it later, so
 * returning null converts a missing feature into an access violation somewhere
 * else entirely. Fatal in every configuration, including PRODUCTION.
 */

void Direct3d11_UnimplementedSlot::hitFatal()
{
	++m_hits;
	m_warned = true;

	FATAL(true, ("Direct3d11: Gl_api slot %s is not implemented yet and has no valid value to return. The engine would store a null and dereference it later.", m_slotName));
}

// ----------------------------------------------------------------------
/**
 * Write one line per unimplemented slot that was actually reached.
 *
 * Called from the backend's remove(). Slots that were never touched are not
 * listed: the interesting number is what this run needed and did not get.
 */

void Direct3d11_UnimplementedSlot::report()
{
	int reached = 0;

	for (Direct3d11_UnimplementedSlot *slot = ms_first; slot; slot = slot->m_next)
		if (slot->m_hits)
		{
			++reached;
			WARNING(true, ("Direct3d11: unimplemented slot %s was called %d time(s) this run.", slot->m_slotName, slot->m_hits));
		}

	if (reached)
		WARNING(true, ("Direct3d11: %d unimplemented Gl_api slot(s) were reached this run.", reached));
}

// ----------------------------------------------------------------------

int Direct3d11_UnimplementedSlot::getSlotsReached()
{
	int reached = 0;

	for (Direct3d11_UnimplementedSlot const *slot = ms_first; slot; slot = slot->m_next)
		if (slot->m_hits)
			++reached;

	return reached;
}

// ======================================================================
