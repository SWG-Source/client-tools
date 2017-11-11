// ======================================================================
//
// DataCombiner.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/DataCombiner.h"

#include <limits>

// ======================================================================

DataCombiner::Operation::Operation(OperationType opType, int inputStreamIndex, int byteCount) :
	m_operationType(static_cast<uint8>(opType)),
	m_inputStreamIndex(static_cast<uint8>(inputStreamIndex)),
	m_byteCount(static_cast<uint16>(byteCount))
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, static_cast<int>(opType), static_cast<int>(OT_COUNT));
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, inputStreamIndex, 2);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, byteCount, static_cast<int>(std::numeric_limits<uint16>::max()));
}

// ======================================================================
/**
 * Combine data from two input streams into a single output stream.
 *
 * NOTE: this function currently does no rails checking for staying within
 * the bounds of the input or output buffers.
 *
 * @param operations      an array of operations to perform on the buffers.
 * @param operationCount  the number of operations in the C-style operations array.
 * @param loopCount       the number of times the operations will be processed.
 * @param inputStream1    the first input stream on which to operate.
 * @param inputStream2    the second input stream on which to operate.
 * @param outputStream    the output stream where output operations will send data.
 */

void DataCombiner::process(const Operation *operations, int operationCount, int loopCount, const void *inputStream1, const void *inputStream2, void *outputStream)
{
	// @todo -TRF- add rails.  Probably should have a byte count for all data streams so we can catch overflows.

	//-- Validate parameters.
	NOT_NULL(operations);
	NOT_NULL(inputStream1);
	NOT_NULL(inputStream2);
	NOT_NULL(outputStream);

	if (operationCount < 1)
	{
		DEBUG_WARNING(true, ("invalid operation count [%d].", operationCount));
		return;
	}

	if (loopCount < 1)
	{
		DEBUG_WARNING(true, ("invalid loop count [%d].", loopCount));
		return;
	}

	//-- Process the byte stream.
	const byte *inputByteStream[2];
	byte       *outputByteStream = reinterpret_cast<byte*>(outputStream);

	inputByteStream[0] = reinterpret_cast<const byte*>(inputStream1);
	inputByteStream[1] = reinterpret_cast<const byte*>(inputStream2);

	for (int loopIndex = 0; loopIndex < loopCount; ++loopIndex)
	{
		for (int operationIndex = 0; operationIndex < operationCount; ++operationIndex)
		{
			const Operation &op = operations[operationIndex];

			switch (op.getOperationType())
			{
				case OT_copy:
					{
						// Get the input stream index and byte count.
						const int byteCount        = op.getByteCount();
						const int inputStreamIndex = op.getInputStreamIndex();

						// Copy the data to the output stream.
						memcpy(outputByteStream, inputByteStream[inputStreamIndex], static_cast<size_t>(byteCount));

						// Update stream pointers.
						inputByteStream[inputStreamIndex] += byteCount;
						outputByteStream                  += byteCount;
					}
					break;
				
				case OT_skip:
					{
						// Get the input stream index and byte count.
						const int byteCount        = op.getByteCount();
						const int inputStreamIndex = op.getInputStreamIndex();

						// Skip the bytes.
						inputByteStream[inputStreamIndex] += byteCount;
					}
					break;

				case OT_COUNT:
				default:
					DEBUG_FATAL(true, ("invalid operation type [%d].", static_cast<int>(op.getOperationType())));
			}
		}
	}
}

// ======================================================================
