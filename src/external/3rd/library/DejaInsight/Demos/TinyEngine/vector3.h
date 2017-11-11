//==============================================================================
// vector3.h
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef vector3_h
#define vector3_h

//==============================================================================
//  INCLUDES
//==============================================================================

#include "mathdefs.h"

//==============================================================================
//  TYPES
//==============================================================================

struct vector3
{
public:
    float       x;                                                      // X.
    float       y;                                                      // Y.
    float       z;                                                      // Z.

                vector3         ( void );                               // Void constructor.
                vector3         ( float x, float y, float z );          // Component constructor.
                vector3         ( const vector3& v );                   // Copy constructor.

    void        Set             ( float x, float y, float z );          // Set the vector.
    vector3&    operator ()     ( float x, float y, float z );          // Set the vector.

    void        RotateX         ( radian Rx );                          // Rotate vector on x axis.
    void        RotateY         ( radian Ry );                          // Rotate vector on y axis.
    void        RotateZ         ( radian Rz );                          // Rotate vector on z axis.

    void        Normalize       ( void );                               // Normalize the vector.
    float       GetLength       ( void ) const;                         // Get the length of the vector.
    float       GetLengthSqr    ( void ) const;                         // Get the length squared.
    radian      GetPitch        ( void ) const;                         // Get pitch of the vector.
    radian      GetYaw          ( void ) const;                         // Get yaw   of the vector.
    void        GetPitchYaw     ( radian& Pitch, radian& Yaw ) const;   // Get pitch and yaw of the vector.

    vector3     operator -      ( void ) const;                         // Unary negation.
    vector3&    operator +=     ( const vector3& v );                   // Add into vector.
    vector3&    operator -=     ( const vector3& v );                   // Subtract from vector.
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

inline vector3  operator +      ( const vector3& v1, const vector3& v2 );
inline vector3  operator -      ( const vector3& v1, const vector3& v2 );
inline vector3  operator *      ( const vector3& v1, float s );
inline bool     operator ==     ( const vector3& v1, const vector3& v2 );
inline bool     operator !=     ( const vector3& v1, const vector3& v2 );

inline float    Dot             ( const vector3& v1, const vector3& v2 );
inline vector3  Cross           ( const vector3& v1, const vector3& v2 );

       void     DejaDescriptor  ( const vector3& );

//==============================================================================
//  INLINE FUNCTION IMPLEMENTATIONS
//==============================================================================

#include "vector3_inline.h"

//==============================================================================
#endif // vector3_h
//==============================================================================
