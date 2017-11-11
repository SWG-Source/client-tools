//==============================================================================
//  matrix4.h
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef matrix4_h
#define matrix4_h

//==============================================================================
//  INCLUDES
//==============================================================================

#include "mathdefs.h"
#include "vector3.h"

//==============================================================================
// Structure to hold a 4x4 matrix.
//------------------------------------------------------------------------------
//
// Matrices are LOGICALLY represented as follows:
// 
//     +----+----+----+----+
//     | Rx | Ry | Rz | Tx |
//     |----+----+----+----|
//     | Rx | Ry | Rz | Ty |
//     |----+----+----+----|
//     | Rx | Ry | Rz | Tz |
//     |----+----+----+----|
//     |  0 |  0 |  0 |  1 |
//     +----+----+----+----+
// 
// A "vertex" is a "column vector".  To transform a vertex using a matrix, it 
// is "post-multiplied" with the matrix.  Operations performed by a sequence 
// of matrix multiplications occur right to left.  Thus:
// 
//     |X|     | M |     | M |     | M |     |x|
//     |Y|  =  |op3|  *  |op2|  *  |op1|  *  |y|
//     |Z|     |   |     |   |     |   |     |z|
// 
// Or, mathematically:
// 
//     |X|   |7 2 3|   |x|                   X = 7x + 2y + 3z   
//     |Y| = |5 6 9| * |y|        and        Y = 5x + 6y + 9z   
//     |Z|   |8 4 1|   |z|                   Z = 8x + 4y + 1z   
//                        
// This logical representation for matrices is the more common form used in
// current 3D graphics literature.
//
//------------------------------------------------------------------------------
//
// Applicable to the following platforms: WIN32
//
// Matrices are PHYSICALLY represented in memory as follows:
// 
//          0    1    2    3            0    1    2    3
//       +----+----+----+----+       +----+----+----+----+
//     0 | 00 | 04 | 08 | 12 |     0 | Rx | Ry | Rz | Tx |      Ty = M[13]
//       |----+----+----+----|       |----+----+----+----|      Ty = M[3][1]
//     1 | 01 | 05 | 09 | 13 |     1 | Rx | Ry | Rz | Ty |
//       |----+----+----+----|       |----+----+----+----|
//     2 | 02 | 06 | 10 | 14 |     2 | Rx | Ry | Rz | Tz |
//       |----+----+----+----|       |----+----+----+----|
//     3 | 03 | 07 | 11 | 15 |     3 |  0 |  0 |  0 |  1 |
//       +----+----+----+----+       +----+----+----+----+
// 
// Within a matrix4, the data is stored as a two dimensional array:
// 
//     float M[4][4];
// 
// Matrix elements are accessed via M[col][row].  Note that this differs from 
// the way elements are normally accessed in standard 2D arrays in C/C++.  So,
// the "Tx" element is M[3][0].
// 
// This representation is "assignment compatible" with 4x4 matrices in both
// OpenGL and DirectX.  For example:
// 
//     matrix4 M;
// 
//     // OpenGL
//     glMatrixMode ( GL_MODELVIEW );
//     glLoadMatrixf( &M );
// 
//     // DirectX
//     SetTransform( D3DTRANSFORMSTATE_WORLD, &(D3DMATRIX(M)) );
// 
// In the event that a particular platform's physical matrix representation is
// transposed, the cell values in a matrix4 are stored in a protected field:
// 
//     protected:
//         float m_Cell[4][4];
// 
// When working with matrix4 functions, a macro, M(Row,Col), must always be 
// used to access the matrix cells.  Outside of matrix4 functions, cells may be
// accessed using operator()(Row,Col).  Never access the m_Cell values 
// directly.
//
//------------------------------------------------------------------------------
// 
// Given a matrix which transforms source space to a destination space:
// 
//     +----+----+----+----+    R012 = source space x-axis in destination space
//     | R0 | R3 | R6 | Tx |    R345 = source space y-axis in destination space
//     |----+----+----+----|    R678 = source space z-axis in destination space
//     | R1 | R4 | R7 | Ty |       
//     |----+----+----+----|    Txyz = source space origin in destination space
//     | R2 | R5 | R8 | Tz |       
//     |----+----+----+----|    R036 = destination space x-axis in source space
//     |  0 |  0 |  0 |  1 |    R147 = destination space y-axis in source space
//     +----+----+----+----+    R258 = destination space z-axis in source space
//                                 
// Example:                        
//                                 
// Q:  Given a W2V (World to View) matrix, what is the "line of sight" vector 
//     in World space?
// 
// A:  A line of sight unit vector is just a View space unit z vector (0,0,1) 
//     expressed in World space.  We know that "R258" is the destination (view) 
//     space's z-axis in the source (world) space.  This is exactly what we 
//     need.  So:
// 
//         vector3  LOS;           // Line of Sight.
//         LOS.x = W2V(0,2);       // R2
//         LOS.y = W2V(1,2);       // R5
//         LOS.z = W2V(2,2);       // R8
// 
//     Similarly, "view up" and "view left" vectors can easily extracted.  Such
//     vectors are useful for things like 3D sprites for particle systems.
// 
//==============================================================================

class matrix4
{
protected:
        float       m_Cell[4][4];

public:
                    matrix4         ( void );                           // Void constructor.
                    matrix4         ( const matrix4& m );               // Copy constructor.
                    matrix4         ( const vector3&    Axis,
                                      float             Angle );        // Construct from Axis / Angle.

        void        Zero            ( void );                           // Zero the matrix.
        void        Identity        ( void );                           // Make an identity matrix.
        void        Transpose       ( void );                           // Transpose the matrix.
        void        RotateX         ( radian r );                       // Rotate the matrix about the x axis.
        void        RotateY         ( radian r );                       // Rotate the matrix about the y axis.
        void        RotateZ         ( radian r );                       // Rotate the matrix about the z axis.
        void        PreRotateX      ( radian r );                       // Rotate the matrix about the x axis.
        void        PreRotateY      ( radian r );                       // Rotate the matrix about the y axis.
        void        PreRotateZ      ( radian r );                       // Rotate the matrix about the z axis.
        void        Translate       ( const vector3& v );               // Translate the matrix.
        void        PreTranslate    ( const vector3& v );               // Translate the matrix.
        void        Scale           ( float s );                        // Scale the matrix.

        void        SetTranslation  ( const vector3& v );               // Set the translation.

        vector3     operator *      ( const vector3& v ) const;         // Transform a vertex.
        matrix4&    operator *=     ( const matrix4& m );               // Concatenate a matrix.

        float       operator ()     ( int Column, int Row ) const;      // Access (read ) a matrix cell.
        float&      operator ()     ( int Column, int Row );            // Access (write) a matrix cell.

friend  matrix4     operator *      ( const matrix4& m1, const matrix4& m2 );
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

matrix4 operator * ( const matrix4& L, const matrix4& R );

//==============================================================================
//  INLINE FUNCTION IMPLEMENTATIONS
//==============================================================================

#include "matrix4_inline.h"

//==============================================================================
#endif //matrix4_h
//==============================================================================
