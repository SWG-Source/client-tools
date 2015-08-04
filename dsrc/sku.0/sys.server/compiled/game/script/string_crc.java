// ======================================================================
//
// string_crc.java
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

package script;

// ======================================================================

public final class string_crc implements crc
{
	public static int getStringCrc(String s)
	{
		int crc = CRC_INIT;
		if (s != null)
		{
			char[] c = s.toCharArray();
			for (int i = 0; i < c.length; ++i)
				crc = crctable[((crc>>24) ^ (c[i]&0xff)) & 0xff] ^ (crc << 8);
		}
		return crc ^ CRC_INIT;
	}
}

// ======================================================================

