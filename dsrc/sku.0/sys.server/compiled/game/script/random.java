/**
 * Title:        random
 * Description:  Random number generator. Algorithm taken from Numerical
 *		Methods in C, func ran3().
 * Copyright:    Copyright (c) 2001
 * Company:      Sony Online Entertainment
 * @author       Steve Jakab
 * @version 1.0
 */

package script;

public class random
{
	private static java.util.Random sysRand = new java.util.Random();

	private static final long MOD_VALUE   = 0x00000000ffffffffL;
	private static final double MAX_RAND = 4294967296.0;
	private static final long A = 30903;

	private static boolean gaussOne = true;
	private static float gaussY1 = 0;
	private static float gaussY2 = 0;

	private static long x_1     = 0;
	private static long x_carry = 0;
	private static long z_1     = 0;
	private static long z_2     = 0;
	private static long z_carry = 0;

	private static long[] cache = new long[1024];

	private static boolean firstRand = true;


	/**
	 * Computes a random number in the range 0 -> 2^32-1
	 *
	 * @return a random number
	 */
	public static long computeIt()
	{
		long x = A * x_1 + x_carry;
		x_carry = x >> 32;
		x &= MOD_VALUE;
		x_1 = x;

		long z = 2 * z_1 + z_2 + z_carry;
		z_carry = z >>> 32;
		z &= MOD_VALUE;
		z_2 = z_1;
		z_1 = z;

		return ((x + z) & MOD_VALUE);
	}

	/**
	 * Computes a random number in the range [0,1)
	 *
	 * @return a random number
	 */
	public static float rand()
	{
		long r = lrand();
		return (float)(r / MAX_RAND);
	}	// rand()

	/**
	 * Computes a random number with a gaussian distribution with 0 mean and std dev of 1.
	 *
	 * @return a random number
	 */
	public static float gaussRand()
	{
		// this function generates pairs of random numbers; we generate the pair the first call and
		// return one of the numbers, then return the second number for the next call
		if ( gaussOne )
		{
			gaussOne = false;

			// compute the next pair
			float x1, x2, w;
 			do {
				x1 = 2.0f * rand() - 1.0f;
				x2 = 2.0f * rand() - 1.0f;
				w = x1 * x1 + x2 * x2;
			} while ( w >= 1.0 );

			if ( w != 0 )
			{
				w = (float)(Math.sqrt( (-2.0 * Math.log( w ) ) / w ));
				gaussY1 = x1 * w;
				gaussY2 = x2 * w;
			}
			else
			{
				gaussY1 = 0;
				gaussY2 = 0;
			}
			return gaussY1;
		}
		else
		{
			gaussOne = true;
			return gaussY2;
		}
	}	// rand()


	/**
	 * Computes a random float in the range [0,1), reseeding the generator
	 * before doing so.
	 *
	 * @param seed		the new seed
	 *
	 * @return a random float
	 */
	public static float rand(int seed)
	{
		reseed(seed);
		return rand();
	}	// rand(int)

	/**
	 * Computes a random number in the range 0 -> 2^32-1
	 *
	 * @return a random number
	 */
	protected static long lrand()
	{
		if (firstRand)
			reseed((int)(System.currentTimeMillis()), sysRand.nextInt(), 74719);

		long rand = computeIt();
		int index = (int)(rand & 0x03ff);
		long retVal = cache[index];
		cache[index] = computeIt();
		return retVal;
	}	// rand()

	/**
	 * Computes a distributed random integer in the range [min, max]
	 * The random distribution is based on the parameter dist.
	 * When dist is between 0.0-1.0 the entire random number range is used
	 * and the mean distribution is shifted towards that location in the range.
	 * When dist is greater than 1 or less than 0 the random range is reduced
	 * such that at -1.0 the min is always returned and at 2.0 the max.
	 *
	 * @param min			lower value of random integer
	 * @param max			upper value of random integer
	 * @param dist			distribution mean of random number
	 *
	 * @return a random integer
	 */
	public static int distributedRand(int min, int max, float dist)
	{
		boolean inverted = false;
		int _min = min;
		int _max = max;

		if (dist < -1) dist = -1;
		if (dist > 2) dist = 2;

		if (min > max)
		{
			inverted = true;
			min = _max;
			max = _min;
		}

		float mid = min + ((max - min) * dist);

		if (mid < min)  { max += (mid-min); mid = min; }
		if (mid > max)  { min += (mid-max); mid = max; }

		int minRand = rand(min, (int)(mid+0.5f));
		int maxRand = rand((int)(mid+0.5f), max);

		int randNum = rand(minRand, maxRand);

		if (inverted)
			randNum = _min + (_max - randNum);

		return randNum;
	}	// distributedRand(int, int, float)

	/**
	 * Computes a distributed random float in the range [min, max]
	 *
	 * @param min			lower value of random float
	 * @param max			upper value of random float
	 * @param dist			distribution mean of random number
	 *
	 * @return a random float
	 */
	public static float distributedRand(float min, float max, float dist)
	{
		boolean inverted = false;
		float _min = min;
		float _max = max;

		if (dist < -1) dist = -1;
		if (dist > 2) dist = 2;

		if (min > max)
		{
			inverted = true;
			min = _max;
			max = _min;
		}

		float mid = min + ((max - min) * dist);

		if (mid < min)  { max += (mid-min); mid = min; }
		if (mid > max)  { min += (mid-max); mid = max; }

		float minRand = rand(min, mid);
		float maxRand = rand(mid, max);

		float randNum = rand(minRand, maxRand);

		if (inverted)
			randNum = _min + (_max - randNum);

		return randNum;
	}	// distributedRand(float, float, float)

	/**
	 * Computes a random integer in the range [minVal, maxVal]
	 *
	 * @param minVal		lower value of random integer
	 * @param maxVal		upper value of random integer
	 *
	 * @return a random integer
	 */
	protected static int rand(int minVal, int maxVal)
	{
		return (int)(rand() * (maxVal - minVal + 1)) + minVal;
	}	// rand(int, int)

	/**
	 * Computes a random float in the range [minVal, maxVal)
	 *
	 * @param minVal		lower value of random float
	 * @param maxVal		upper value of random float
	 *
	 * @return a random float
	 */
	protected static float rand(float minVal, float maxVal)
	{
		return (rand() * (maxVal - minVal)) + minVal;
	}	// rand(float, float)

	/**
	 * Generates a random number based on rolling multiple dice.
	 *
	 * @param numDie		number of die to roll
	 * @param dieSize		number of sides the die has
	 * @param base			number to add to the result of rolling the dice
	 *
	 * @return the random number
	 */
	protected static int rand(int numDie, int dieSize, int base)
	{
		if (numDie <= 0 || dieSize <= 1)
			return base;

		int result = base;
		for (int i = 0; i < numDie; ++i)
			result += rand(1, dieSize);
		return result;
	}	// rand(int, int, int)

	/**
	 * Generates a normalized random number based on rolling multiple dice.
	 *
	 * @param numDie		number of die to roll
	 * @param dieSize		number of sides the die has
	 *
	 * @return the random number, normalized to [0, 1]
	 */
	protected static float randNormalized(int numDie, int dieSize)
	{
		int result = rand(numDie, dieSize, 0);
		if (result == 0)
			return 0;
		return (float)(result - numDie) / (numDie * (dieSize - 1));
	}	// randNormalized

	/**
	 * Reseeds the random number generator.
	 *
	 * @param seed		the new seed
	 */
	protected static void reseed(int seed)
	{
		switch ( (int)(rand() * 3.0f) )
		{
			case 0 :
				reseed(seed, (int)(System.currentTimeMillis()), sysRand.nextInt());
				break;
			case 1 :
				reseed(sysRand.nextInt(), seed, (int)(System.currentTimeMillis()));
				break;
			case 2 :
				reseed((int)(System.currentTimeMillis()), sysRand.nextInt(), seed);
				break;
		}
	}	// reseed

	/**
	 * Computes a random number with a gaussian distribution with a given mean and std dev.
	 *
	 * @param mean		the number's mean
	 * @param stddev	the number's std dev
	 *
	 * @return a random number
	 */
	protected static float gaussRand(float mean, float stddev)
	{
		return (gaussRand() * stddev) + mean;
	}

	/**
	 * Reseeds the random number generator.
	 *
	 * @param seed		the new seed
	 */
	protected static void reseed(int x1, int z1, int z2)
	{
		firstRand = false;

		x_1 = x1;
		z_1 = z1;
		z_2 = z2;

		for (int i = 0; i < cache.length; ++i)
			cache[i] = computeIt();
	}	// reseed

}	// class random
