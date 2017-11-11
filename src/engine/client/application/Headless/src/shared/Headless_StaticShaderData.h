#ifndef INCLUDED_Headless_StaticShaderData_H
#define INCLUDED_Headless_StaticShaderData_H


#include "clientGraphics/StaticShader.h"

class Headless_StaticShaderData : public StaticShaderGraphicsData
{
public:
	virtual void  update(const StaticShader & ) {}
	virtual int   getTextureSortKey() const { return 0; }
	
};

#endif // INCLUDED_Headless_StaticShaderData_H