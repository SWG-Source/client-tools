// createShaderTemplate.cpp : Defines the entry point for the console application.
//

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/SetupSharedFoundation.h"

#include <stdio.h>
#include <stdlib.h>// ----------------------------------------------------------------------

#include <string.h>

const float Material[] = 
{
	1.0f, 1.0f, 1.0f, 1.0f,   // ambient
	1.0f, 1.0f, 1.0f, 1.0f,   // diffuse
	0.0f, 0.0f, 0.0f, 0.0f,   // emissive
	1.0f, 1.0f, 1.0f, 1.0f,   // specular
	0.0f                      // power
};

const Tag TAG_TAG  = TAG3(T,A,G);
const Tag TAG_TXM  = TAG3(T,X,M);
const Tag TAG_MAIN = TAG (M,A,I,N);
const Tag TAG_MATL = TAG (M,A,T,L);
const Tag TAG_MATS = TAG (M,A,T,S);
const Tag TAG_SSHT = TAG (S,S,H,T);
const Tag TAG_TCSS = TAG (T,C,S,S);
const Tag TAG_TXMS = TAG (T,X,M,S);

void createShaderTemplate(const char *fileName, const char *textureName)
{
	Iff iff(4 * 1024);

	iff.insertForm(TAG_SSHT);
		iff.insertForm(TAG_0000);
	
			// material data
			iff.insertForm(TAG_MATS);
				iff.insertForm(TAG_0000);

					iff.insertChunk(TAG_TAG);
						iff.insertChunkData(static_cast<uint32>(TAG_MAIN));
					iff.exitChunk(TAG_TAG);

					iff.insertChunk(TAG_MATL);
						for (int i = 0; i < sizeof(Material) / sizeof(Material[0]); ++i)
							iff.insertChunkData(Material[i]);
					iff.exitChunk(TAG_MATL);
	
				iff.exitForm(TAG_0000);
			iff.exitForm(TAG_MATS);

			// texture data
			iff.insertForm(TAG_TXMS);
				iff.insertForm(TAG_TXM);
					iff.insertForm(TAG_0000);

						iff.insertChunk(TAG_DATA);
							iff.insertChunkData(static_cast<uint8>(0));
							iff.insertChunkData(static_cast<uint32>(TAG_MAIN));
						iff.exitChunk(TAG_DATA);

						iff.insertChunk(TAG_NAME);
							iff.insertChunkString(textureName);
						iff.exitChunk(TAG_NAME);

					iff.exitForm(TAG_0000);
				iff.exitForm(TAG_TXM);
			iff.exitForm(TAG_TXMS);

			// texture coordinate sets
			iff.insertForm(TAG_TCSS);
				iff.insertChunk(TAG_0000);
					iff.insertChunkData(static_cast<uint32>(TAG_MAIN));
					iff.insertChunkData(static_cast<uint8>(0));
				iff.exitChunk(TAG_0000);
			iff.exitForm(TAG_TCSS);
			
			// effect name
			iff.insertChunk(TAG_NAME);
				iff.insertChunkString("effect\\simplemt1z.eft");
			iff.exitChunk(TAG_NAME);
						
		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_SSHT);

	iff.write(fileName);
}

void createShaderTemplate(const char *textureName)
{
	char buffer[_MAX_PATH];

	// throw away the path
	const char *copy = strrchr(textureName, '\\');
	if (copy)
		++copy;
	else
		copy = textureName;
	strcpy(buffer, copy);

	// replace the extension with .sht
	char *ext = strrchr(buffer, '.');
	if (ext)
	{
		strcpy(ext, ".sht");
		createShaderTemplate(buffer, textureName);
	}
}

int main(int argc, char* argv[])
{
	SetupFoundation::Data data;
	SetupFoundation::setupDefaultConsoleData(&data);
	SetupFoundation::install(&data);

	if (argc > 1)
	{
		for (int i = 1; i < argc; ++i)
			createShaderTemplate(argv[i]);
	}		
	else
	{
		char buffer[_MAX_PATH];
		while (fgets(buffer, sizeof(buffer), stdin))
		{
			char *newline = strrchr(buffer, '\n');
			if (newline)
				*newline = '\0';
			createShaderTemplate(buffer);
		}		
	}

	SetupFoundation::remove();

	return 0;
}

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

static MemoryManager memoryManager(1*1024*1024, false);
