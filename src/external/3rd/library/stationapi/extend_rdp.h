#ifndef __EXTEND_RDP_H__
#define __EXTEND_RDP_H__

#include "rdp_api.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------
// start rdp extension
//--------------------------
extern uint32 g_fast_malloc_ready;  // declared in rdp/fast_malloc.c
extern uint32 g_malloc_init_count;  // declared in extend_rdp.c

void ref_fast_malloc_init(uint32 expected_usage);
void ref_fast_malloc_destroy(void);
//-------------------------- 
// end rdp extension
//--------------------------

#ifdef __cplusplus
}
#endif

#endif