//======================================================================
//
// ProsePackageManagerClient.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ProsePackageManagerClient_H
#define INCLUDED_ProsePackageManagerClient_H

//======================================================================

class ProsePackage;

//----------------------------------------------------------------------

class ProsePackageManagerClient
{
public:

	static bool appendTranslation      (const ProsePackage & pp, Unicode::String & result);
	static bool setTranslation		   (const ProsePackage & pp, Unicode::String & result);
	static int  appendAllProsePackages (const Unicode::String & oob, Unicode::String & result, int positionStart = -1, int positionEnd = -1);
};

//======================================================================

#endif
