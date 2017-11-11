// ======================================================================
//
// DataCombiner.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_DataCombiner_H
#define INCLUDED_DataCombiner_H

// ======================================================================

class DataCombiner
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	enum OperationType
	{
		OT_copy,
		OT_skip,
		//---
		OT_COUNT
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	/**
	 * Represents an operation to be performed on one of the input streams,
	 * possibly affecting the output stream.
	 */

	class Operation
	{
	public:

		Operation(OperationType opType, int inputStreamIndex, int byteCount);

		OperationType  getOperationType() const;
		int            getInputStreamIndex() const;
		int            getByteCount() const;

	private:

		// Disabled.
		Operation();

	public:

		uint8   m_operationType;
		uint8   m_inputStreamIndex;
		uint16  m_byteCount;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	static void process(const Operation *operations, int operationCount, int loopCount, const void *inputStream1, const void *inputStream2, void *outputStream);

};

// ======================================================================

inline DataCombiner::OperationType DataCombiner::Operation::getOperationType() const
{
	return static_cast<DataCombiner::OperationType>(m_operationType);
}

// ----------------------------------------------------------------------

inline int DataCombiner::Operation::getInputStreamIndex() const
{
	return static_cast<int>(m_inputStreamIndex);
}

// ----------------------------------------------------------------------

inline int DataCombiner::Operation::getByteCount() const
{
	return static_cast<int>(m_byteCount);
}

// ======================================================================

#endif
