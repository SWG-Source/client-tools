// ======================================================================
//
// color.java
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

package script;

public class color
{
	public static final color BLACK = new color(  0,   0,   0, 255);
	public static final color BLUE  = new color(  0,   0, 255, 255);
	public static final color GREEN = new color(  0, 255,   0, 255);
	public static final color RED   = new color(255,   0,   0, 255);
	public static final color WHITE = new color(255, 255, 255, 255);

	public int getR()
		{
			return m_r;
		}

	public int getG()
		{
			return m_g;
		}

	public int getB()
		{
			return m_b;
		}

	public int getA()
		{
			return m_a;
		}

	public color(int r, int g, int b, int a)
		{
			if ((r < 0) || (r > 255) ||
			    (g < 0) || (g > 255) ||
			    (g < 0) || (g > 255) ||
			    (g < 0) || (g > 255))
			{
				throw new IllegalArgumentException("color value arg out of valid range 0..255");
			}

			m_r = r;
			m_g = g;
			m_b = b;
			m_a = a;
		}

	private final int m_r;
	private final int m_g;
	private final int m_b;
	private final int m_a;
}
