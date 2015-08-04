#ifndef LINUX
#pragma once
#endif
#ifndef _MMaterial
#define _MMaterial
//
// *****************************************************************************
//
// Copyright (C) 1998-2001 Alias|Wavefront Inc.
//
// These coded instructions, statements and computer programs contain
// unpublished information proprietary to Alias|Wavefront Inc. and are 
// protected by Canadian and US federal copyright laws. They may not be 
// disclosed to third parties or copied or duplicated, in whole or in part, 
// without prior written consent of Alias|Wavefront Inc.
//
// Unpublished-rights reserved under the Copyright Laws of the United States.
//
// *****************************************************************************
//
// CLASS:    MMaterial
//
//    This class is used in the draw functions of user defined shapes
//    (see MPxSurfaceShapeUI) for setting up and querying materials
//    used in shaded mode drawing.
//
// *****************************************************************************
//
// CLASS DESCRIPTION (MMaterial)
//
// *****************************************************************************
//
#if defined __cplusplus

// *****************************************************************************

// INCLUDED HEADER FILES

#include <maya/MStatus.h>
#include <maya/MTypes.h>
#include <maya/MObject.h>
#include <maya/M3dView.h>

// *****************************************************************************

// DECLARATIONS

class MDagPath;
class MVector;
class MDrawData;
class MPxHwShaderNode;

// *****************************************************************************

// CLASS DECLARATION (MMaterial)

/// Hardware shading material class used in MPxSurfaceShapeUI (OpenMayaUI)
/**
*/
#ifdef _WIN32
#pragma warning(disable: 4522)
#endif // _WIN32

class OPENMAYAUI_EXPORT MMaterial  
{
public:
	///
	MMaterial();
	///
	MMaterial( const MMaterial& in );
	///
	~MMaterial();

public:
	// Evaluation methods.
	///
	MStatus		evaluateMaterial(M3dView&, const MDagPath& );
	///
	MStatus		evaluateShininess();
	///
	MStatus		evaluateDiffuse();
	///
	MStatus		evaluateEmission();
	///
	MStatus		evaluateSpecular();
	///
	MStatus		evaluateTexture( MDrawData & data );
	///
	MStatus		evaluateTextureTransformation();

	///
	bool		materialIsTextured() const;

	// Draw methods.
    // Can call these after calling correct evaluate method above.
	///
	MStatus		setMaterial(const MDagPath&, bool hasTransparency);
	///
	MStatus		getShininess( float & );
	///
	MStatus		getDiffuse( MColor & );
	///
	MStatus		getEmission( MColor & );
	///
	MStatus		getSpecular( MColor & );
	///
    MStatus		getHasTransparency( bool & );
	///
	MStatus		getTextureTransformation(float& scaleU,
										 float& scaleV,
										 float& translateU,
										 float& translateV,
										 float& rotate);

	// This method sets the texture so that it can be used in
	// the specified view.
	///
	void		applyTexture( M3dView&, MDrawData& );

	///
	MPxHwShaderNode *	getHwShaderNode( MStatus * ReturnStatus = NULL );

protected:
// No protected members

private:
	const char*	 className() const;



    MMaterial( void* in );

	void*	 fMaterial;
};

#ifdef _WIN32
#pragma warning(default: 4522)
#endif // _WIN32

// *****************************************************************************
#endif /* __cplusplus */
#endif /* _MMaterial */
