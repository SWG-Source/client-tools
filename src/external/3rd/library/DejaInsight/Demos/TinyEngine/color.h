//==============================================================================
//  color.h
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef color_h
#define color_h

//==============================================================================
//  INCLUDES
//==============================================================================

#include "mathdefs.h"
#include "../../DejaLib.h"

//==============================================================================
//  TYPES
//==============================================================================

struct color
{
    color() {};
    color( int aR, int aG, int aB, int aA = 255 )
    {
        R = (unsigned char)aR;
        G = (unsigned char)aG;
        B = (unsigned char)aB;
        A = (unsigned char)aA;
    }
    color( float aR, float aG, float aB, float aA = 1.0f )
    {
        R = (unsigned char)(aR * 255.0f);
        G = (unsigned char)(aG * 255.0f);
        B = (unsigned char)(aB * 255.0f);
        A = (unsigned char)(aA * 255.0f);
    }

    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char A;
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

inline
color Blend( color& c1, color& c2, float t )
{
    float t1 = 1.0f - t;
    int R = (int)(c1.R * t1) + (int)(c2.R * t);
    int G = (int)(c1.G * t1) + (int)(c2.G * t);
    int B = (int)(c1.B * t1) + (int)(c2.B * t);
    int A = (int)(c1.A * t1) + (int)(c2.A * t);

    return color( MIN(R,255), MIN(G,255), MIN(B,255), MIN(A,255) );
}

inline 
void DejaDescriptor( const color& Color )
{
    DEJA_TYPE ( Color, color );
    DEJA_FIELD( Color.R );
    DEJA_FIELD( Color.G );
    DEJA_FIELD( Color.B );
    DEJA_FIELD( Color.A );
}

//==============================================================================
#endif // color_h
//==============================================================================
