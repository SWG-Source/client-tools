#include "allsys.h"
#include "rdp_api.h"
#include "extend_rdp.h"

//--------------------------
// start rdp extension
//--------------------------
// reference count version of fast_malloc_init/destroy

uint32 g_malloc_init_count = 0;

void ref_fast_malloc_init(uint32 expected_usage)
{
    if (!g_fast_malloc_ready)
    {
        g_malloc_init_count = 0;
        fast_malloc_init(expected_usage);
    }
    else if (!g_malloc_init_count)
    {   // fast_malloc_init was apparently called outside our control, so account for it.
        g_malloc_init_count = 1;
    }
    g_malloc_init_count++;
}

void ref_fast_malloc_destroy(void)
{
    if (!g_fast_malloc_ready)
    {   // unexpected case!
        printf("ref_fast_malloc_destroy found nothing to destroy!");
        g_malloc_init_count = 0;
    }
    else if (!--g_malloc_init_count)
    {
        fast_malloc_destroy();  // dealloc on last one
    }
}
//-------------------------- 
// end rdp extension
//--------------------------
