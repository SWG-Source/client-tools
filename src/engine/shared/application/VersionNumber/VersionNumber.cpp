#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// char const ms_version[] = "ApplicationVersionMarker 000 123456789012345678901234567890";

static char const s_marker[] = "ApplicationVersionMarker ";
static int const  s_markerLength = strlen(s_marker);
static int        s_flags = 3;
static int        s_space = 1;
static int        s_versionLength = 30;

static void usage()
{
	fprintf(stderr, "usage: VersionNumber <-r | -w> image [bootleg] [publish] [clear_text_version] [masked_text_version]\n");
}

inline bool isTrue(char const * string)
{
	char c = tolower(string[0]);
	return (c == '1' || c == 't' || c == 'y');
}

inline void modifyCharacter(char *&update, char with, int & length, bool &modified)
{
	if (*update != with)
	{
		modified = true;
		*update = with;
	}

	++update;
	--length;
}

static int function(int argc, char**argv)
{
	if (argc < 2)
	{
		usage();
		return 1;
	}

	// determine whether we are reading or writing the version number
	bool read = strcmp(argv[1], "-r") == 0;
	bool write = strcmp(argv[1], "-w") == 0;
	if (!read && !write)
	{
		usage();
		return 1;
	}

	// verify command line arguments
	if (read && argc != 3)
	{
		usage();
		return 1;
	}

	char const * brandText = NULL;
	if (write)
	{
		brandText = argv[5];

		if (argc < 6 || argc > 7)
		{
			usage();
			return 1;
		}

		int length = strlen(brandText);
		if (argc == 7)
			length += strlen(argv[6]);
		if (length > s_versionLength-2)
		{
			fprintf(stderr, "combined length of clear_text_version and masked_text_version is limited to %d characters\n", s_versionLength-2);
			return 1;
		}

		bool const bootleg = isTrue(argv[3]);
		bool const publish = isTrue(argv[4]);
		if ((bootleg && publish) || (!bootleg && !publish))
		{
			fprintf(stderr, "either bootleg or publish must be true, but not both\n");
			return 1;
		}
	}

	// open the file
	FILE *file = fopen(argv[2], "rb");
	if (!file)
	{
		fprintf(stderr, "could not open file %s\n", argv[2]);
		usage();
		return 1;
	}

	// get the file length
	fseek(file, 0, SEEK_END);
	const int fileLength = ftell(file);
	fseek(file, 0, SEEK_SET);

	// read the file into ram
	char *fileImage = new char[fileLength];
	fread(fileImage, 1, fileLength, file);
	fclose(file);

	// find the version number
	int const searchLength = fileLength - s_markerLength;
	char *brand = NULL;
	for (int i = 0; i < searchLength; ++i)
		if (memcmp(fileImage + i, s_marker, s_markerLength) == 0)
		{
			// make sure it is only found once
			for (int j = i+1; j < searchLength; ++j)
				if (memcmp(fileImage + j, s_marker, s_markerLength) == 0)
				{
					fprintf(stderr, "version marker found multiple places, aborting\n");
					return 1;
				}

			brand = fileImage + i + s_markerLength;
			break;
		}

	// handle missing version number
	if (!brand)
	{
		fprintf(stderr, "version marker was not found, aborting\n");
		return 1;
	}

	// write out the image
	if (write)
	{
		char * write    = brand;
		int    length   = s_flags + s_space + s_versionLength;
		bool   modified = false;

		// write out the patched version text flag
		modifyCharacter(write, '1', length, modified);

		// write out the bootleg flag
		modifyCharacter(write, isTrue(argv[3]) ? '1' : '0', length, modified);

		// write out the publish flag
		modifyCharacter(write, isTrue(argv[4]) ? '1' : '0', length, modified);

		// a space delimiter
		modifyCharacter(write, ' ', length, modified);

		// write out the clear text version number
		for (int i = 0; brandText[i]; ++i)
			modifyCharacter(write, brandText[i], length, modified);
		modifyCharacter(write, '\0', length, modified);

		// write out the masked text version number
		if (argc == 7)
		{
			for (int i = 0; argv[6][i]; ++i)
				modifyCharacter(write, static_cast<char>(argv[6][i] | 0x80), length, modified);
		}

		// null terminate the masked text verion number, if any
		modifyCharacter(write, '\0', length, modified);

		// pad the rest of the space
		while (length)
			modifyCharacter(write, '\0', length, modified);

		if (modified)
		{
			// write out the modified version
			FILE *file = fopen(argv[2], "r+b");

				if (!file)
				{
					fprintf(stderr, "could not open file %s for writing\n", argv[2]);
					return 1;
				}

				int const offset = brand - fileImage;
				fseek(file, offset, SEEK_SET);
				fwrite(brand, 1, s_versionLength, file);

			fclose(file);
		}
	}

	// read the branding information
	if (brand[0] == '0')
	{
		printf("%s: no version information present\n", argv[2]);
	}
	else
	{
		// skip the patched flag
		++brand;

		// get the bootleg flag
		char const * bootleg = (brand[0] == '0') ? "" : " bootleg";
		++brand;

		// get the publish flag
		char const * publish = (brand[0] == '0') ? "" : " publish";
		++brand;

		// skip the space
		++brand;

		// get the version name
		char const *version = brand;
		while (brand[0])
			++brand;
		++brand;

		// print out the masked version string
		char masked[32] = { '\0' };
		if (brand[0])
		{
			for (int i = 0; brand[i]; ++i)
				brand[i] &= 0x7f;
			sprintf(masked, " [%s]", brand);
		}

		printf("%s: %s%s%s%s\n", argv[2], version, bootleg, publish, masked);
	}

	return 0;
}

int main(int argc, char**argv)
{
	return function(argc, argv);
}
