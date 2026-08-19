/* Sample program for configure to test IPv6 support on target
platforms. We check for the required IPv6 data structures. */

#if defined(__hpux)
#define _HPUX_SOURCE
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
    sockaddr_in6 tmp;
    sockaddr_storage tmp2;
    (void)tmp;
    (void)tmp2;

    return 0;
}
