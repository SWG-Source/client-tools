// ======================================================================
//
// WireTime.h
//
// Legacy time fields are signed 32-bit time_t on the wire: Win32 builds
// define _USE_32BIT_TIME_T and the stock server builds with -m32. 64-bit
// builds keep time_t internally and convert once, here, at the wire.
//
// ======================================================================

#ifndef INCLUDED_WireTime_H
#define INCLUDED_WireTime_H

#include <cstdint>
#include <ctime>
#include <limits>

// ======================================================================

inline int32_t toWireTime(time_t const time)
{
	FATAL(time < std::numeric_limits<int32_t>::min() || time > std::numeric_limits<int32_t>::max(),
		("toWireTime: %lld does not fit the legacy signed 32-bit time field", static_cast<long long>(time)));
	return static_cast<int32_t>(time);
}

// ======================================================================

#endif
