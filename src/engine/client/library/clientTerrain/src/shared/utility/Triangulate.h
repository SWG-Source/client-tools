/*****************************************************************/
/** Static class to triangulate any contour/polygon efficiently **/
/** You should replace Vector2d with whatever your own Vector   **/
/** class might be.  Does not support polygons with holes.      **/
/** Uses STL vectors to represent a dynamic array of vertices.  **/
/** This code snippet was submitted to FlipCode.com by          **/
/** John W. Ratcliff (jratcliff@verant.com) on July 22, 2000    **/
/** I did not write the original code/algorithm for this        **/
/** this triangulator, in fact, I can't even remember where I   **/
/** found it in the first place.  However, I did rework it into **/
/** the following black-box static class so you can make easy   **/
/** use of it in your own code.  Simply replace Vector2d with   **/
/** whatever your own Vector implementation might be.           **/
/*****************************************************************/

//-------------------------------------------------------------------

#ifndef INCLUDED_Triangulate_H
#define INCLUDED_Triangulate_H

//-------------------------------------------------------------------

#include "sharedMath/Vector2d.h"

//-------------------------------------------------------------------

class Triangulate
{
public:

	typedef stdvector<Vector2d>::fwd Input;
	typedef stdvector<int>::fwd      Output;

	// triangulate a contour/polygon, places results in vector as series of triangles.
	static bool Process (const Input& contour, Output& result);

private:

	// compute area of a contour/polygon
	static float Area(const Input& contour);

	// decide if point Px/Py is inside triangle defined by (Ax,Ay) (Bx,By) (Cx,Cy)
	static bool InsideTriangle (float Ax, float Ay, float Bx, float By, float Cx, float Cy, float Px, float Py);

	static bool Snip (const Input& contour,int u,int v,int w,int n,int *V);
};

//-------------------------------------------------------------------

#endif
