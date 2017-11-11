// ======================================================================
//
// BuildoutAreaSupport.h
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BuildoutAreaSupport_H
#define INCLUDED_BuildoutAreaSupport_H

// ======================================================================

class CellProperty;
class CrcString;
class NetworkId;
class Object;
class Transform;
struct ClientBuildoutAreaRow;
struct ServerBuildoutAreaRow;

// ======================================================================

class BuildoutAreaSupport
{
public:
	static void getBuildoutAreaList(std::string const &sceneName, std::vector<std::string> &areaNames, std::vector<std::string> &areaLocations, std::vector<std::string> &areaStatuses);
	static void saveBuildoutArea(std::string const &areaName);
	static void saveBuildoutArea(std::string const &areaName, stdvector<ServerBuildoutAreaRow>::fwd const &serverRows, stdvector<ClientBuildoutAreaRow>::fwd const &clientRows);

	static Object* createNewObject(CrcString const &templateName, CellProperty const *cellProperty, Transform const &transform_p);
	static void unlock(Object const &obj);
	static void unlockAll(bool value);
	static void unlockNonStructures(bool value);
	static void unlockServerOnly(bool value);
	static bool deleteBuildoutObject(Object &obj);
	static bool setObjectTransform(Object &obj, Transform const &transform_p);
	static void attachScript(Object const &obj, std::string const &scriptName);
	static void detachScript(Object const &obj, std::string const &scriptName);
	static void setObjvar(Object const &obj, std::string const &objvarName, int value);
	static void setObjvar(Object const &obj, std::string const &objvarName, float value);
	static void setObjvar(Object const &obj, std::string const &objvarName, std::string const &value);
	static void removeObjvar(Object const &obj, std::string const &objvarName);
	static void populateServerObjectData(NetworkId const &networkId);
	static bool openBuildoutFilesForEditing( const std::string &areaName );
	static void addServerOnlyObjectsToWorldSnapshot();
};

// ======================================================================

#endif // INCLUDED_BuildoutAreaSupport_H

