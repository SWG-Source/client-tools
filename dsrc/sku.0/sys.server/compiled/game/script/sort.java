package script;

import java.util.*;

/**
 * A class that contains sorting algorithms: currently has only
 * one - a shell sort.
 *
 * <p>
 * The sorting algorithms are static and take a single: the vector
 * that is to be sorted.  The vector is assumed to contain objects
 * that implement the Comparable interface (which defines a single
 * method: compareTo).
 *
 * <p>
 * For example, given the class XYZ:
 * <pre>
 *     class XYZ {
 *         // A simple class
 *         XYZ(int i) {
 *             val = i;
 *         }
 *         private int val;
 *     }
 * <pre>
 * changing it to:
 * <pre>
 *     class XYZ implements Comparable {        // <== Note the implements
 *         // A simple class
 *         XYZ(int i) {
 *             val = i;
 *         }
 *         private int val;
 *         // The added compare method
 *         public int compareTo(Object o) throws IllegalArgumentException {
 *             // Passed the wrong type of object?
 *             if (!(o instanceof XYZ))
 *                 throw new IllegalArgumentException();
 *             XYZ other = (XYZ) o;
 *             // the compare: sorts in ascending order
 *             return val - other.val;
 *         }
 *     }
 * </pre>
 * allows a Vector of XYZ's to be sorted with
 * <pre>
 *     Vector v = new Vector();
 *     v.addElement(new XYZ(2));
 *     v.addElement(new XYZ(1));
 *     v.addElement(new XYZ(3));
 *     Sort.shellSort(v);
 * </pre>
 *
 * @see Comparable
 * @see java.util.Vector
 */
public
class sort {
    /**
     * A shell sort algorithm that sorts a Vector of Comparable objects.
     *
     * @param v		the vector to sort
     *
     * @see Comparable
     * @see java.util.Vector
     *
     * Based on code by Joel Renkema obtained from
     *	http://kepler.covenant.edu/~renkema/java/
     * the original sorted arrays of integers.
     */
    public static void sort(Vector v)
	throws IllegalArgumentException
    {
	int distance = v.size() / 2;	   //  Split array into segments

        // Continue until array cannot be broken into fewer segments
	while (distance > 0)   {
	    // Sort each individual segment
	    for (int segment = 0; segment < distance; segment++)
		segmentedInsertionSort(v, distance, segment);
	    distance /= 2;	//  Break array into fewer segments
	}
    }

    /**
     * Sorts a particular segment using a modified insertion sort alogorithm.
     *
     * @param v		the vector to sort in
     * @param distance	the distance to sort within
     * @param segment	the segment to sort
     */
    private static void segmentedInsertionSort(Vector v, int distance,
						int segment)
	    throws IllegalArgumentException
    {
	int j;

	for (int i = segment + distance; i < v.size(); i += distance) {
	    j = i;	// start at the current processing position
	    while (j >= distance) {
		Comparable a = (Comparable) v.elementAt(j);
		Comparable b = (Comparable) v.elementAt(j - distance);
		if (a.compareTo(b) <= 0) {
		    swap(v, j, j - distance);
		    j -= distance;
		} else
		    break;   // Exit when position is found
	    }
	}
    }

    /**
     * Swap two elements of a vector.
     *
     * @param v		the vector being modified
     * @param i		the first element to swap
     * @param j		the second element to swap
     */
    private static void swap(Vector v, int i, int j) {
       Object temp;
       temp = v.elementAt(i);
       v.setElementAt(v.elementAt(j), i);
       v.setElementAt(temp, j);
    }

    /**
     * To ensure no one creates an instance of the sort class...
     */
    private sort() {
    }
}

