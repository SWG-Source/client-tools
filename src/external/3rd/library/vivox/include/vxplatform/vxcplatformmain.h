#pragma once


#ifdef SN_TARGET_PS3
#include <sys/paths.h>
#include <sys/prx.h>
#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <sys/timer.h>
#include <sys/paths.h>
#include <cell/error.h>
#include <stdio.h>
#include <sys/process.h>
#include <sys/paths.h>
#include <sys/prx.h>
#include <sys/return_code.h>
#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <sys/timer.h>
#include <netex/net.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netex/sockinfo.h>
#include <arpa/inet.h>
#include <netex/net.h>
#include <netex/errno.h>
#include <netex/libnetctl.h>
#include <sys/spu_initialize.h>
#include <assert.h>

#endif
#ifdef _WIN32
#define strcasecmp stricmp
#else
#include <iostream>
#ifndef SN_TARGET_PS3
#include <ext/stdio_filebuf.h>
#endif
#endif

#ifdef SN_TARGET_PS3

static int isMounted(const char *path)
{
    int i, err;
    CellFsStat status;

    printf("Waiting for mounting\n");
    for (i = 0; i < 15; i++) {
        err = cellFsStat(path, &status);
        if (err == CELL_FS_SUCCEEDED) {
            printf("Waiting for mounting done\n");
            return 1;
        }
        sys_timer_sleep(1);
        printf(".\n");
    }
    printf("Waiting for mounting failed\n");
    return 0;
}


static int if_up_with(int index)
{
    int timeout_count = 10;
    int state;
    int ret;

    (void)index;
    ret = cellNetCtlInit();
    if (ret < 0) {
        printf("cellNetCtlInit() failed(%x)\n", ret);
        return (-1);
    }
    for (;;) {
        ret = cellNetCtlGetState(&state);
        if (ret < 0) {
            printf("cellNetCtlGetState() failed(%x)\n", ret);
            return (-1);
        }
        if (state == CELL_NET_CTL_STATE_IPObtained) {
            break;
        }
        sys_timer_usleep(500 * 1000);
        timeout_count--;
        if (index && timeout_count < 0) {
            printf("if_up_with(%d) timeout\n", index);
            return (0);
        }
    }
    return (0);
}

class EarlyStaticInit
{
public:
    EarlyStaticInit()
    {
		int res = cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
        assert(res >= 0);
        res = cellSysmoduleLoadModule(CELL_SYSMODULE_RTC);
        assert(res >= 0);
        res = cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
        assert(res >= 0);
        res = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL);
        assert(res >= 0);
        res = cellSysmoduleLoadModule(CELL_SYSMODULE_HTTP);
        assert(res >= 0);
        res = cellSysmoduleLoadModule(CELL_SYSMODULE_HTTPS);
        assert(res >= 0);
        res = cellSysmoduleLoadModule(CELL_SYSMODULE_HTTP_UTIL);
        assert(res >= 0);
   		res = cellSysmoduleLoadModule(CELL_SYSMODULE_MIC);
        assert(res >= 0);
		res = cellSysmoduleLoadModule(CELL_SYSMODULE_AUDIO);
        assert(res >= 0);
		res = cellSysmoduleLoadModule(CELL_SYSMODULE_AVCONF_EXT);
        assert(res >= 0);


		res = sys_net_initialize_network(); 
		assert(res >= 0);
		//static char __libnet_memory[1024 * 1024]; 

		//sys_net_initialize_parameter_t __libnet_param; 
		//__libnet_param.memory = __libnet_memory; 
		//__libnet_param.memory_size = sizeof(__libnet_memory); 
		//__libnet_param.flags = 0; 
	

		//res = sys_net_initialize_network_ex(&__libnet_param); 
		//assert (res >=0);

       	res = if_up_with(1);
        assert(res >= 0);

		res = isMounted(SYS_APP_HOME);
		assert(res >=0);

		sys_spu_initialize( 6, 0 );

	}
};

static EarlyStaticInit ps3EarlyStaticInit __attribute__((init_priority(101)));
SYS_PROCESS_PARAM (1001, 1024 * 512);

#endif
