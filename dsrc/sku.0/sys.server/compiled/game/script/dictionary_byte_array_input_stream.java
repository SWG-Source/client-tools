/****************************************
 *
 * This class wraps ByteArrayInputStream in order to validate the data from a dictionary_byte_array_output_stream.
 * We store the length of the data in the 1st 4 bytes, and the checksum in the 2nd 4 bytes.
 *
 ***************************************/

package script;

import java.io.ByteArrayInputStream;
import java.io.IOException;

final class dictionary_byte_array_input_stream extends ByteArrayInputStream implements crc
{
	private static final int HEADER_SIZE = 10;
	private static final int SIZE_START = 2;
	private static final int CRC_START = 6;

	public dictionary_byte_array_input_stream(byte[] inbuf) throws IOException
	{
		super(inbuf);

		if (buf[0] == (byte)0xba && buf[1] == (byte)0xba)
		{
			// the actual data is 10 bytes from the start
			pos = HEADER_SIZE;

			// check the length and crc at the beginning of buf
			int incount = 0;
			for ( int i = SIZE_START + 3; i >= SIZE_START; --i )
				incount = (incount << 8) | (buf[i] & 0x000000ff);
			if (count != incount)
			{
				String error = "ERROR! dictionary_byte_array_input_stream received data of length " + count + ", but expected length " + incount;
				System.err.println(error);
				for ( int i = 0; i < count; i += 32 )
				{
					if (i + 32 < count)
				 		printBytes(i, 32);
					else
						printBytes(i, count - i);
				}
				throw new IOException(error);
			}

			int incrc = 0;
			for ( int i = CRC_START + 3; i >= CRC_START; --i )
				incrc = (incrc << 8) | (buf[i] & 0x000000ff);
			int crc = CRC_INIT;
			for ( int i = 10; i < count; ++i )
			{
				crc = crctable[((crc>>24) ^ (buf[i] & 0x000000ff)) & 0x000000FF] ^ (crc << 8);
			}
			if (incrc != crc)
			{
				String error = "ERROR! dictionary_byte_array_input_stream received data with crc " + crc + ", but expected crc " + incrc;
				System.err.println(error);
				for ( int i = 0; i < count; i += 32 )
				{
					if (i + 32 < count)
				 		printBytes(i, 32);
					else
						printBytes(i, count - i);
				}
				throw new IOException(error);
			}
		}
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
