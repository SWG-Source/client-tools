/****************************************
 *
 * This class wraps ByteArrayOutputStream in order to store a checksum value to validate the data.
 * We store the length of the data in the 1st 4 bytes, and the checksum in the 2nd 4 bytes.
 *
 ***************************************/

package script;

import java.io.ByteArrayOutputStream;

final class dictionary_byte_array_output_stream extends ByteArrayOutputStream implements crc
{
	private static final int HEADER_SIZE = 10;
	private static final int SIZE_START = 2;
	private static final int CRC_START = 6;

	private int crc = CRC_INIT;


	public dictionary_byte_array_output_stream()
	{
		super();
		// assume the default size is at least 10
		count = HEADER_SIZE;
		buf[0] = (byte)0xba;
		buf[1] = (byte)0xba;
	}

	public dictionary_byte_array_output_stream(int size)
	{
		super(size + HEADER_SIZE);
		count = HEADER_SIZE;
		buf[0] = (byte)0xba;
		buf[1] = (byte)0xba;
	}

	public void reset()
	{
		crc = CRC_INIT;
		count = HEADER_SIZE;
	}

	public int size()
	{
		return super.size() - HEADER_SIZE;
	}

	public byte[] toByteArray()
	{
		// write the size and checksum to the head of the buffer
		int shift = 0;
		for ( int i = SIZE_START; i < SIZE_START + 4; ++i )
		{
			buf[i] = (byte)((count >> shift) & 0x000000ff);
			shift += 8;
		}

		shift = 0;
		for ( int i = CRC_START; i < CRC_START + 4; ++i )
		{
			buf[i] = (byte)((crc >> shift) & 0x000000ff);
			shift += 8;
		}

		return super.toByteArray();
	}

	public String toString()
	{
		// write the size and checksum to the head of the buffer
		int shift = 0;
		for ( int i = SIZE_START; i < SIZE_START + 4; ++i )
		{
			buf[i] = (byte)((count >> shift) & 0x000000ff);
			shift += 8;
		}

		shift = 0;
		for ( int i = CRC_START; i < CRC_START + 4; ++i )
		{
			buf[i] = (byte)((crc >> shift) & 0x000000ff);
			shift += 8;
		}
/*
		System.err.println("Dictionary output count = " + count + ", crc = " + crc);
		for ( int i = 0; i < count; i += 32 )
		{
			if (i + 32 < count)
		 		printBytes(i, 32);
			else
				printBytes(i, count - i);
		}
*/
		return super.toString();
	}

	public String toStringDebug()
	{
		// write the size and checksum to the head of the buffer
		int shift = 0;
		for ( int i = SIZE_START; i < SIZE_START + 4; ++i )
		{
			buf[i] = (byte)((count >> shift) & 0x000000ff);
			shift += 8;
		}

		shift = 0;
		for ( int i = CRC_START; i < CRC_START + 4; ++i )
		{
			buf[i] = (byte)((crc >> shift) & 0x000000ff);
			shift += 8;
		}

		System.err.println("Dictionary output count = " + count + ", crc = " + crc);
		for ( int i = 0; i < count; i += 32 )
		{
			if (i + 32 < count)
		 		printBytes(i, 32);
			else
				printBytes(i, count - i);
		}
		return super.toString();
	}

	public void write(byte[] b, int off, int len)
	{
		// update the checksum
		int limit = off + len;
		for ( int i = off; i < limit; ++i )
			crc = crctable[((crc>>24) ^ (b[i] & 0x000000ff)) & 0x000000ff] ^ (crc << 8);

		super.write(b, off, len);
	}

	public void write(int b)
	{
		// update the checksum
		crc = crctable[((crc>>24) ^ (b & 0x000000ff)) & 0x000000ff] ^ (crc << 8);
		super.write(b);
	}

	private void printBytes(int start, int count)
	{
		for ( int i = start; i < start + count; ++i )
		{
			System.err.print((((int)buf[i]) & 0x00ff) + " ");
		}
		System.err.println();
	}
}

