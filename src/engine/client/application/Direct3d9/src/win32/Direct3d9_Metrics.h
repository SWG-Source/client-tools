// ======================================================================
//
// Direct3d9_Metrics.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Direct3d9_Metrics_H
#define INCLUDED_Direct3d9_Metrics_H

#ifdef _DEBUG

// ======================================================================

class Direct3d9_Metrics
{
public:

	static void install();
	static void remove();

	static void reset();

	enum
	{
		MAX_PASSES = 4
	};

	// draw statistics
	static int     vertices;
	static int     indices;
	static int     points;
	static int     lines;
	static int     triangles;
	static int     drawPrimitiveCalls;
	static int     drawIndexedPrimitiveCalls;
	static int     perPassDrawPrimitiveCalls[MAX_PASSES];
	static int     perPassDrawIndexedPrimitiveCalls[MAX_PASSES];
	static int     perPassTriangles[MAX_PASSES];

	// texture statistics
	static int     setTextureCalls;
	static int     setTextureCacheMisses;
	static int     setTextureUniqueTextures;
	static int     textureMemoryTotal;
	static int     textureMemoryUsed;
	static int     textureMemoryCreated;
	static int     textureMemoryDestroyed;
	static int     textureMemoryModified;

	// vertex buffer statistics
	static int     setVertexDeclarationCacheMisses;
	static int     setVertexDeclarationCalls;
	static int     setVertexBufferCalls;
	static int     setVertexBufferUniqueVertexBuffers;
	static int     setStreamSourceCalls;
	static int     setStreamSourceCacheMisses;
	static int     vertexBufferMemoryTotal;
	static int     vertexBufferMemoryUsed;
	static int     vertexBufferMemoryCreated;
	static int     vertexBufferMemoryDestroyed;
	static int     vertexBufferMemoryModified;
	static int     vertexBufferMemoryDynamic;
	static int     vertexBufferDiscards;

	// index buffer statistics
	static int     setIndexBufferCalls;
	static int     setIndexBufferUniqueIndexBuffers;
	static int     setIndexBufferCacheMisses;
	static int     indexBufferMemoryTotal;
	static int     indexBufferMemoryUsed;
	static int     indexBufferMemoryCreated;
	static int     indexBufferMemoryDestroyed;
	static int     indexBufferMemoryModified;
	static int     indexBufferMemoryDynamic;
	static int     indexBufferDiscards;

	// misc stats
	static int     setViewportCalls;
	static int     setTransformCalls;
	static int     setMaterialCalls;
	static int     setRenderStateCalls;
	static int     setRenderStateCacheMisses;
	static int     setSamplerStateCalls;
	static int     setSamplerStateCacheMisses;
	static int     setTextureStageStateCalls;
	static int     setTextureStageStateCacheMisses;
	static int     setVertexShaderCalls;
	static int     setVertexShaderCacheMisses;

	// vertex and pixel shader constants information
	static int     setVertexShaderConstantsCalls;
	static int     setVertexShaderConstantsRegisters;
	static int     setPixelShaderConstantsCalls;
	static int     setPixelShaderConstantsRegisters;

	static int     reusedStaticShaders;
	static int     reusedShaderImplementations;

private:

	Direct3d9_Metrics();
	Direct3d9_Metrics(Direct3d9_Metrics const &);
	Direct3d9_Metrics & operator =(Direct3d9_Metrics const &);


	static void  debugReportDraw();
	static void  debugReportDrawPerPass();
	static void  debugReportTexture();
	static void  debugReportVertexBuffer();
	static void  debugReportIndexBuffer();
	static void  debugReportMisc();
	static void  debugReportTriangleRate();
	static void  debugReportSetConstants();
};

// ======================================================================

#endif
#endif
