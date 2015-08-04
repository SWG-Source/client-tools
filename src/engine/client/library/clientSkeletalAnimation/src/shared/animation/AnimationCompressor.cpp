// ======================================================================
//
// AnimationCompressor.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientSkeletalAnimation/FirstClientSkeletalAnimation.h"
#include "clientSkeletalAnimation/AnimationCompressor.h"

#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedMath/CompressedQuaternion.h"
#include "sharedMath/Quaternion.h"

#include <map>
#include <stdio.h>
#include <string.h>
#include <vector>

// ======================================================================

#ifdef _DEBUG

// ======================================================================

namespace AnimationCompressorNamespace
{
	typedef std::vector<Quaternion>            QuaternionVector;
	typedef std::vector<CompressedQuaternion>  CompressedQuaternionVector;
	typedef std::vector<int16>                 FrameNumberVector;
	typedef std::map<uint8, int>               FormatUsageMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const Tag TAG_AROT = TAG(A,R,O,T);
	const Tag TAG_ATRN = TAG(A,T,R,N);
	const Tag TAG_CHNL = TAG(C,H,N,L);
	const Tag TAG_CKAT = TAG(C,K,A,T);
	const Tag TAG_KFAT = TAG(K,F,A,T);
	const Tag TAG_LOCR = TAG(L,O,C,R);
	const Tag TAG_LOCT = TAG(L,O,C,T);
	const Tag TAG_MESG = TAG(M,E,S,G);
	const Tag TAG_MSGS = TAG(M,S,G,S);
	const Tag TAG_QCHN = TAG(Q,C,H,N);
	const Tag TAG_SROT = TAG(S,R,O,T);
	const Tag TAG_STRN = TAG(S,T,R,N);
	const Tag TAG_XFIN = TAG(X,F,I,N);
	const Tag TAG_XFRM = TAG(X,F,R,M);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	int                         s_skipCount;
	
	QuaternionVector            s_rotations;
	CompressedQuaternionVector  s_compressedRotations;
	FrameNumberVector           s_rotationFrameNumbers;

	FormatUsageMap              s_formatUsageMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool readChomp(FILE *file, char *buffer, size_t bufferLength);
	void createCompressedFile(Iff &inputIff, char const *compressedFilename);

	bool createCompressedIff(Iff &inputIff, Iff &outputIff);
	void createCompressedIff_0003(Iff &inputIff, Iff &outputIff);
}

using namespace AnimationCompressorNamespace;

// ======================================================================

bool AnimationCompressorNamespace::readChomp(FILE *file, char *buffer, size_t bufferLength)
{
	char const *const resultString = fgets(buffer, static_cast<int>(bufferLength), file);
	if (!resultString)
		return false;

	// Chop off trailing newline if present.
	size_t const readLength = strlen(buffer);
	if ((readLength > 0) && (buffer[readLength - 1] == '\n'))
		buffer[readLength - 1] = '\0';

	return true;
}

// ----------------------------------------------------------------------

void AnimationCompressorNamespace::createCompressedFile(Iff &inputIff, char const *compressedFilename)
{
	NOT_NULL(compressedFilename);

	//-- Create the output Iff.
	Iff outputIff(inputIff.getRawDataSize());

	const bool compressedIt = createCompressedIff(inputIff, outputIff);

	//-- Write the output.
	if (compressedIt)
	{
		const bool success = outputIff.write(compressedFilename);
		FATAL(!success, ("failed to write animation file [%s].", compressedFilename));
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("SKIPPING:%s\n", compressedFilename));
		++s_skipCount;
	}
}

// ----------------------------------------------------------------------

bool AnimationCompressorNamespace::createCompressedIff(Iff &inputIff, Iff &outputIff)
{
	inputIff.enterForm(TAG_KFAT);
	outputIff.insertForm(TAG_CKAT);

	const Tag sourceVersion = inputIff.getCurrentName();

	bool compressedIt = false;

	switch (sourceVersion)
	{
		case TAG_0002:
			DEBUG_FATAL(true, ("cannot compressed version 2."));
			break;

		case TAG_0003:
			createCompressedIff_0003(inputIff, outputIff);
			compressedIt = true;
			break;

		default:
			{
				char buffer[5];
				ConvertTagToString(sourceVersion, buffer);
				FATAL(true, ("unsupported animation version [%s].", buffer));
			}
			break;
	}

#if 0
	if (!inputIff.atEndOfForm())
	{
		const Tag nextTag = inputIff.getCurrentName();
		char buffer2[5];
		ConvertTagToString(nextTag, buffer2);
		FATAL(true, ("INFO: currentName [%s].\n", buffer2));
	}
#endif

	outputIff.exitForm(TAG_CKAT);
	inputIff.exitForm(TAG_KFAT, !compressedIt);

	return compressedIt;
}

// ----------------------------------------------------------------------

void AnimationCompressorNamespace::createCompressedIff_0003(Iff &inputIff, Iff &outputIff)
{
	inputIff.enterForm(TAG_0003);
	outputIff.insertForm(TAG_0001);

		//-- Convert info chunk.
		inputIff.enterChunk(TAG_INFO);
		outputIff.insertChunk(TAG_INFO);
		
			outputIff.insertChunkData(inputIff.read_float());

			outputIff.insertChunkData(static_cast<int16>(inputIff.read_int32()));

			const int transformCount = static_cast<int>(inputIff.read_int32());
			outputIff.insertChunkData(static_cast<int16>(transformCount));

			const int animatedRotationChannelCount = static_cast<int>(inputIff.read_int32());
			outputIff.insertChunkData(static_cast<int16>(animatedRotationChannelCount));

			const int staticRotationChannelCount = static_cast<int>(inputIff.read_int32());
			outputIff.insertChunkData(static_cast<int16>(staticRotationChannelCount));

			const int animatedTranslationChannelCount = static_cast<int>(inputIff.read_int32());
			outputIff.insertChunkData(static_cast<int16>(animatedTranslationChannelCount));

			const int staticTranslationChannelCount = static_cast<int>(inputIff.read_int32());
			outputIff.insertChunkData(static_cast<int16>(staticTranslationChannelCount));

		outputIff.exitChunk(TAG_INFO);
		inputIff.exitChunk(TAG_INFO);

		// Handle XFRM form.
		inputIff.enterForm(TAG_XFRM);
		outputIff.insertForm(TAG_XFRM);

		char transformName[256];

		for (int transformIndex = 0; transformIndex < transformCount; ++transformIndex)
		{
#if 0
			char currentTag[5];
			ConvertTagToString(inputIff.getCurrentName(), currentTag);
			DEBUG_REPORT_LOG(true, ("next tag: expecting XFIN: [%s].\n", currentTag));
#endif

			inputIff.enterChunk(TAG_XFIN);
			outputIff.insertChunk(TAG_XFIN);

				inputIff.read_string(transformName, sizeof(transformName) - 1);
				outputIff.insertChunkString(transformName);

				outputIff.insertChunkData(inputIff.read_int8());
				outputIff.insertChunkData(static_cast<int16>(inputIff.read_int32()));
				outputIff.insertChunkData(static_cast<uint8>(inputIff.read_uint32()));

				for (int j = 0; j < 3; ++j)
					outputIff.insertChunkData(static_cast<int16>(inputIff.read_int32()));

			outputIff.exitChunk(TAG_XFIN);
			inputIff.exitChunk(TAG_XFIN);
		}
	
		outputIff.exitForm(TAG_XFRM);
		inputIff.exitForm(TAG_XFRM);

		// Animated rotation channels.
		if (animatedRotationChannelCount > 0)
		{
			inputIff.enterForm(TAG_AROT);
			outputIff.insertForm(TAG_AROT);

				for (int i = 0; i < animatedRotationChannelCount; ++i)
				{
					//-- Read in source rotations.
					inputIff.enterChunk(TAG_QCHN);

						s_rotations.clear();
						s_rotationFrameNumbers.clear();

						const int keyframeCount = static_cast<int>(inputIff.read_int32());

						for (int j = 0; j < keyframeCount; ++j)
						{
							s_rotationFrameNumbers.push_back(static_cast<int16>(inputIff.read_int32()));
							s_rotations.push_back(inputIff.read_floatQuaternion());
						}

					inputIff.exitChunk(TAG_QCHN);

					//-- Determine compression format for rotation channel.
					uint8  xFormat;
					uint8  yFormat;
					uint8  zFormat;

					CompressedQuaternion::getOptimalCompressionFormat(s_rotations, xFormat, yFormat, zFormat);

					++s_formatUsageMap[xFormat];
					++s_formatUsageMap[yFormat];
					++s_formatUsageMap[zFormat];

					//-- Compress the values.
					CompressedQuaternion::compressRotations(s_rotations, xFormat, yFormat, zFormat, s_compressedRotations);

					//-- Write compressed rotation channel.
					outputIff.insertChunk(TAG_QCHN);

						outputIff.insertChunkData(static_cast<int16>(keyframeCount));
						outputIff.insertChunkData(xFormat);
						outputIff.insertChunkData(yFormat);
						outputIff.insertChunkData(zFormat);

						for (int k = 0; k < keyframeCount; ++k)
						{
							outputIff.insertChunkData(s_rotationFrameNumbers[static_cast<FrameNumberVector::size_type>(k)]);
							outputIff.insertChunkData(s_compressedRotations[static_cast<CompressedQuaternionVector::size_type>(k)].getCompressedValue());
						}
					
					outputIff.exitChunk(TAG_QCHN);
				}

			outputIff.exitForm(TAG_AROT);
			inputIff.exitForm(TAG_AROT);
		}		
		else if (inputIff.enterForm(TAG_AROT, true))
		{
			// Open and close empty form.
			inputIff.exitForm(TAG_AROT);
		}
	
		// Static rotation channels.
		if (staticRotationChannelCount > 0)
		{
			inputIff.enterChunk(TAG_SROT);
			outputIff.insertChunk(TAG_SROT);

				uint8 xFormat = 0;
				uint8 yFormat = 0;
				uint8 zFormat = 0;

				for (int i = 0; i < staticRotationChannelCount; ++i)
				{
					// Read rotation.
					const Quaternion rotation = inputIff.read_floatQuaternion();

					// Determine compression format that yields optimal precision for this single value.
					xFormat = CompressedQuaternion::getOptimalCompressionFormat(rotation.x, rotation.x);
					yFormat = CompressedQuaternion::getOptimalCompressionFormat(rotation.y, rotation.y);
					zFormat = CompressedQuaternion::getOptimalCompressionFormat(rotation.z, rotation.z);

					++s_formatUsageMap[xFormat];
					++s_formatUsageMap[yFormat];
					++s_formatUsageMap[zFormat];

					outputIff.insertChunkData(xFormat);
					outputIff.insertChunkData(yFormat);
					outputIff.insertChunkData(zFormat);
					outputIff.insertChunkData(CompressedQuaternion(rotation, xFormat, yFormat, zFormat).getCompressedValue());
				}

			outputIff.exitChunk(TAG_SROT);
			inputIff.exitChunk(TAG_SROT);
		}
		else if (inputIff.enterChunk(TAG_SROT, true))
		{
			// Open and close empty form.
			inputIff.exitChunk(TAG_SROT);
		}

		// Animated float channels.
		if (animatedTranslationChannelCount > 0)
		{
#if 0
			Tag const currentName = inputIff.getCurrentName();
			if (currentName != TAG_ATRN)
			{
				char tmpTagName[5];
				ConvertTagToString(currentName, tmpTagName);
				FATAL(true, ("expecting ATRN, found [%s].", tmpTagName));
			}
			
#endif
			inputIff.enterForm(TAG_ATRN);
			outputIff.insertForm(TAG_ATRN);

				for (int i = 0; i < animatedTranslationChannelCount; ++i)
				{
					inputIff.enterChunk(TAG_CHNL);
					outputIff.insertChunk(TAG_CHNL);

						const int keyframeCount = static_cast<int>(inputIff.read_int32());
						outputIff.insertChunkData(static_cast<int16>(keyframeCount));

						for (int j = 0; j < keyframeCount; ++j)
						{
							outputIff.insertChunkData(static_cast<int16>(inputIff.read_int32()));
							outputIff.insertChunkData(inputIff.read_float());
						}

					outputIff.exitChunk(TAG_CHNL);
					inputIff.exitChunk(TAG_CHNL);
				}

			outputIff.exitForm(TAG_ATRN);
			inputIff.exitForm(TAG_ATRN);
		}		
		else if (inputIff.enterForm(TAG_ATRN, true))
		{
			// Open and close empty form.
			inputIff.exitForm(TAG_ATRN);
		}

		// Static float channels.
		if (staticTranslationChannelCount > 0)
		{
			inputIff.enterChunk(TAG_STRN);
			outputIff.insertChunk(TAG_STRN);

				for (int i = 0; i < staticTranslationChannelCount; ++i)
					outputIff.insertChunkData(inputIff.read_float());

			outputIff.exitChunk(TAG_STRN);
			inputIff.exitChunk(TAG_STRN);
		}
		else if (inputIff.enterChunk(TAG_STRN, true))
		{
			// Open and close empty form.
			inputIff.exitChunk(TAG_STRN);
		}

		if (inputIff.enterForm(TAG_MSGS, true))
		{
			outputIff.insertForm(TAG_MSGS);

				inputIff.enterChunk(TAG_INFO);
				outputIff.insertChunk(TAG_INFO);

					outputIff.insertChunkData(inputIff.read_int16());

				outputIff.exitChunk(TAG_INFO);
				inputIff.exitChunk(TAG_INFO);

				while (!inputIff.atEndOfForm())
				{
					inputIff.enterChunk(TAG_MESG);
					outputIff.insertChunk(TAG_MESG);

						while (inputIff.getChunkLengthLeft(1))
							outputIff.insertChunkData(inputIff.read_uint8());

					outputIff.exitChunk(TAG_MESG);
					inputIff.exitChunk(TAG_MESG);
				}

			outputIff.exitForm(TAG_MSGS);
			inputIff.exitForm(TAG_MSGS);
		}

		// locomotion translation data.
		if (inputIff.enterChunk(TAG_LOCT, true))
		{
			outputIff.insertChunk(TAG_LOCT);

				while (inputIff.getChunkLengthLeft(1))
					outputIff.insertChunkData(inputIff.read_uint8());

			outputIff.exitChunk(TAG_LOCT);
			inputIff.exitChunk(TAG_LOCT);
		}

		// locomotion rotation data.
		if (inputIff.enterChunk(TAG_LOCR, true))
		{
			outputIff.insertChunk(TAG_QCHN);

				const int keyCount = static_cast<int>(inputIff.read_int16());

				s_rotations.clear();
				s_rotationFrameNumbers.clear();

				for (int i = 0; i < keyCount; ++i)
				{
					s_rotationFrameNumbers.push_back(inputIff.read_int16());
					s_rotations.push_back(inputIff.read_floatQuaternion());
				}


				//-- Determine compression format for rotation channel.
				uint8  xFormat;
				uint8  yFormat;
				uint8  zFormat;

				CompressedQuaternion::getOptimalCompressionFormat(s_rotations, xFormat, yFormat, zFormat);

				++s_formatUsageMap[xFormat];
				++s_formatUsageMap[yFormat];
				++s_formatUsageMap[zFormat];

				//-- Compress the values.
				CompressedQuaternion::compressRotations(s_rotations, xFormat, yFormat, zFormat, s_compressedRotations);

				//-- Write compressed rotation channel.
				outputIff.insertChunkData(static_cast<int16>(keyCount));
				outputIff.insertChunkData(xFormat);
				outputIff.insertChunkData(yFormat);
				outputIff.insertChunkData(zFormat);

				for (int k = 0; k < keyCount; ++k)
				{
					outputIff.insertChunkData(s_rotationFrameNumbers[static_cast<FrameNumberVector::size_type>(k)]);
					outputIff.insertChunkData(s_compressedRotations[static_cast<CompressedQuaternionVector::size_type>(k)].getCompressedValue());
				}

			outputIff.exitChunk(TAG_QCHN);
			inputIff.exitChunk(TAG_LOCR);
		}

	outputIff.exitForm(TAG_0001);
	inputIff.exitForm(TAG_0003);
}

// ======================================================================

void AnimationCompressor::compressAnimations(char const *responseFilename)
{
	DEBUG_REPORT_LOG(true, ("AnimationCompression: START.\n"));
	s_skipCount = 0;

	// input response file format:
	//   full path to output directory, including trailing backslash.
	//   [repeat 1..number of animation files to convert]
	//     full path to uncompressed animation file to export.

	//-- For Lint --- it doesn't know I need to know about Quaternion.
	Quaternion q;
	q.debugDump();

	//-- Open the response file.
	FILE *const responseFile = fopen(responseFilename, "r");
	FATAL(!responseFile, ("failed to open response file [%s].", responseFilename));

	//-- Tell TreeFile system we want to enable full pathname loading.
	TreeFile::addSearchAbsolute(1024);

	//-- Load input filenames first so we can print out completion percentage.
	typedef std::vector<char*>  StringVector;
	StringVector                inputFilenameVector;

	{
		char sourcePathname[2 * MAX_PATH];

		while (readChomp(responseFile, sourcePathname, sizeof(sourcePathname)))
			inputFilenameVector.push_back(DuplicateString(sourcePathname));
	}

	//-- Do the compression for each file.
	float const sourceCount = static_cast<float>(inputFilenameVector.size());
	float       processedCount = 0.0f;

	StringVector::iterator const endIt = inputFilenameVector.end();
	for (StringVector::iterator it = inputFilenameVector.begin(); it != endIt; ++it, processedCount += 1.0f)
	{
		char const *const inputPathname = *it;
		NOT_NULL(inputPathname);

		Iff sourceIff(inputPathname);

		if (sourceIff.getCurrentName() == TAG_KFAT)
		{
			//-- Create the compressed file from the source file.
			DEBUG_REPORT_LOG(true, ("Compressing [%s] (%.2f %% of total %d files complete).\n", inputPathname, 100.0f * processedCount / sourceCount, static_cast<int>(sourceCount)));
			createCompressedFile(sourceIff, inputPathname);
		}
		else
			DEBUG_REPORT_LOG(true, ("Skipping [%s] (%.2f %% of total %d files complete).\n", inputPathname, 100.0f * processedCount / sourceCount, static_cast<int>(sourceCount)));

		//-- Cleanup filename.
		delete [] *it;
	}

	DEBUG_REPORT_LOG(true, ("  Format usage data: BEGIN\n"));

		const FormatUsageMap::iterator fuEndIt = s_formatUsageMap.end();
		for (FormatUsageMap::iterator fuIt = s_formatUsageMap.begin(); fuIt != fuEndIt; ++fuIt)
		{
			DEBUG_REPORT_LOG(true, ("    0x%02x: %d\n", static_cast<int>(fuIt->first), fuIt->second));
		}

	DEBUG_REPORT_LOG(true, ("  Format usage data: END\n"));


	DEBUG_REPORT_LOG(true, ("AnimationCompression: END (%d skipped).\n", s_skipCount));
}

// ======================================================================

#endif

// ======================================================================
