/*
*   IBuffer declaration
*
*   Author: Richard Jayne
*
*   (C) 2008 Sony Online Entertainment, LLC
*/

#ifndef CAPTURECOMMON_IBUFFER_H
#define CAPTURECOMMON_IBUFFER_H

#include "Types.h"
#include "Smart/SmartPtr.h"

namespace CaptureCommon
{

class IBuffer
{
public:
    typedef Smart::SmartPtrT<IBuffer, Smart::ReleaseFunc<IBuffer> > Ptr;
    virtual ~IBuffer(){};
    virtual void Release() = 0;
    virtual byte* Data() = 0;
    virtual uint32 MaxSize() const = 0;
    virtual uint32 Size() const = 0;
    virtual void Size(uint32 size) = 0;
    virtual void Fill(const byte* pData, uint32 dataSize) = 0;
};

} // CaptureCommon

#endif // CAPTURECOMMON_IBUFFER_H
