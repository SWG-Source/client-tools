#ifndef INCLUDED_FirstSwgClientAutomation_H
#define INCLUDED_FirstSwgClientAutomation_H

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/FirstSharedFoundation.h"

#include <atlbase.h>

#define DECLARE_AUTO_PTR( klass ) struct klass; typedef CComPtr<klass> klass##AutoPtr

#endif // INCLUDED_FirstSwgClientAutomation_H