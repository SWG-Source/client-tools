#include "FirstUiBuilder.h"
#include "UiBuilderLoader.h"

#include <sys/types.h>
#include <sys/stat.h>

UIBuilderLoader::FileTimeMap UIBuilderLoader::s_fileTimes;

bool UIBuilderLoader::LoadStringFromResource( const UINarrowString &ResourceName, UINarrowString &Out )
{
	FILE *fp = fopen(ResourceName.c_str(), "rb+");

	if (!fp)
	{
		fp = fopen(ResourceName.c_str(), "rb");

		if (fp)
		{
			ReadOnlyFilesInInput = true;
		}
		else
		{
			return false;
		}
	}
	
	struct _stat statbuf;
	const int result = _stat(ResourceName.c_str (), &statbuf);
	if (!result)
	{
		const int t = statbuf.st_mtime;
		s_fileTimes[ResourceName] = t;
	}

	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	char *rawdata = new char[len];
	fread(rawdata, len, 1, fp);
	fclose(fp);

	Out.assign(rawdata, len);
	delete rawdata;

	return true;
}
