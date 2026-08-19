#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

int main (int argc, char *argv[])
{
	char *fp, *buf = "";
	int len;
	len = SearchPath (NULL, argv[1], NULL, 0, buf, &fp);
	if (len == 0) {
		LPVOID lpMsgBuf;
		if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		                  FORMAT_MESSAGE_FROM_SYSTEM |
    		                  FORMAT_MESSAGE_IGNORE_INSERTS,
                                  NULL,
                                  GetLastError(),
                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                  (LPTSTR) &lpMsgBuf,
                                  0,
                                  NULL )) {
			printf ("%s", lpMsgBuf);
			LocalFree( lpMsgBuf );
			return 1;
		}
	}
	buf = malloc (len);
	len = SearchPath (NULL, argv[1], NULL, len, buf, &fp);
	printf ("%s\n", buf);
	free (buf);
	return 0;
}
