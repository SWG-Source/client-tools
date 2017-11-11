// ======================================================================
//
// Md5sum.cpp
//
// copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "FirstMd5sum.h"
#include "Md5sum.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/DebugHelp.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Md5.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedFile/TreeFile.h"

#include <stdio.h>

// ======================================================================

static int globalArgc;
static char * * globalArgv;

// ======================================================================

bool md5sum(char const * fileName, Md5 & md5)
{
	FILE * f = fopen(fileName, "rb");
	if (!f)
		return false;

	int const bufferSize = 8 * 1024;
	char buffer[bufferSize];
	int r;
	while ((r = fread(buffer, sizeof(char), bufferSize, f)) != 0)
		md5.update(buffer, r);

	fclose(f);
	md5.finish();

	return true;
}

// ----------------------------------------------------------------------

void check(char const * fileName)
{
	int files = 0, failed = 0;

	FILE * f = fopen(fileName, "rb");
	if (f)
	{
		char buffer[1024];
		
		while (fgets(buffer, sizeof(buffer), f))
		{
			// make sure the line appears valid
			++files;
			int length = strlen(buffer);
			if (length < 36 || buffer[32] != ' ' || buffer[33] != '*')
			{
				++failed;
				fprintf(stderr, "%d does not appear to be a valid md5 signature line\n", files);
			}
			else
			{
				// remove trailing newlines and carriage returns
				while (buffer[length-1] == '\n' || buffer[length-1] == '\r')
					buffer[--length] = '\0';

				// get teh file name in its own pointer
				char const * file = buffer+34;

				// compute the md5sum
				Md5 md5;
				if (!md5sum(file, md5))
				{
					++failed;
					printf("%s: FAILED open or read\n", file);
				}
				else
				{
					// null terminate the input string
					buffer[32] = '\0';

					// generate the compare string
					char buffer2[48];
					md5.getValue().format(buffer2, sizeof(buffer2));

					// compare the md5 signatures
					if (_stricmp(buffer, buffer2) != 0)
					{
						++failed;
						printf("%s: FAILED\n", file);
					}
					else
					{
						printf("%s: OK\n", file);
					}
				}
			}
		}

		if (failed == 0)
		{
			printf("All %d files compared okay\n", files);
		}
		else
		{
			printf("%d of %d files failed\n", failed, files);
		}

		fclose(f);
	}
	else
		fprintf(stderr, "could not open file %s to check md5sums", fileName);
}

// ----------------------------------------------------------------------

void md5sum(char const * fileName)
{
	Md5 md5;
	if (md5sum(fileName, md5))
	{
		char buffer[64];
		md5.getValue().format(buffer, sizeof(buffer));
		printf("%s *%s\n", buffer, fileName);
	}
	else
		fprintf(stderr, "could not open file %s to compute md5sum", fileName);
}

// ----------------------------------------------------------------------

void md5sum()
{
	bool checkMode = false;

	for (int i = 1; i < globalArgc; ++i)
	{
		if (i == 1 && strcmp(globalArgv[i], "-c") == 0)
			checkMode = true;
		else
			if (checkMode)
				check(globalArgv[i]);
			else
				md5sum(globalArgv[i]);
	}
}

// ======================================================================

int main(int argc, char **argv)
{
	//-- thread
	SetupSharedThread::install();

	//-- debug
	SetupSharedDebug::install(4096);

	{
		SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_console);
		SetupSharedFoundation::install(data);
	}

	globalArgc = argc;
	globalArgv = argv;

	SetupSharedFoundation::callbackWithExceptionHandling(md5sum);
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}

// ======================================================================
