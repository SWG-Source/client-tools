// ======================================================================
//
// MayaExporterQuery.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "FirstMayaExporter.h"
#if !NO_DATABASE
#include "MayaExporterQuery.h"

using namespace DBQuery;

//////////////////////////////////////////////////////////////////////////

UpdateShaderQuery::UpdateShaderQuery()
{}

void UpdateShaderQuery::getSQL(std::string &sql)
{
	sql = "begin :result := asset_exporter.update_shader(:path, :fileName, :effectName); end;";
}

bool UpdateShaderQuery::bindParameters()
{
	if (!bindParameter(m_returnCode)) return false;
	if (!bindParameter(m_path))       return false;
	if (!bindParameter(m_filename))   return false;
	if (!bindParameter(m_effectName)) return false;
	return true;
}

bool UpdateShaderQuery::bindColumns()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

UpdateMeshQuery::UpdateMeshQuery()
{}

void UpdateMeshQuery::getSQL(std::string &sql)
{
	sql = "begin :result := asset_exporter.update_mesh(:path, :fileName, :type, :numPolygons, :numVertices, :numUVSets, :extentCenterX, :extentCenterY, :extentCenterZ, :extentRadius, :extentBoxMinX, :extentBoxMinY, :extentBoxMinZ, :extentBoxMaxX, :extentBoxMaxY, :extentBoxMaxZ, :numHardpoints); end;";
}

bool UpdateMeshQuery::bindParameters()
{
	if (!bindParameter(m_returnCode))    return false;
	if (!bindParameter(m_path))          return false;
	if (!bindParameter(m_filename))      return false;
	if (!bindParameter(m_type))          return false;
	if (!bindParameter(m_numPolygons))   return false;
	if (!bindParameter(m_numVertices))   return false;
	if (!bindParameter(m_numUVSets))     return false;
	if (!bindParameter(m_extentCenterX)) return false;
	if (!bindParameter(m_extentCenterY)) return false;
	if (!bindParameter(m_extentCenterZ)) return false;
	if (!bindParameter(m_extentRadius))  return false;
	if (!bindParameter(m_extentBoxMinX)) return false;
	if (!bindParameter(m_extentBoxMinY)) return false;
	if (!bindParameter(m_extentBoxMinZ)) return false;
	if (!bindParameter(m_extentBoxMaxX)) return false;
	if (!bindParameter(m_extentBoxMaxY)) return false;
	if (!bindParameter(m_extentBoxMaxZ)) return false;
	if (!bindParameter(m_numHardpoints)) return false;
	return true;
}

bool UpdateMeshQuery::bindColumns()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

UpdateLODQuery::UpdateLODQuery()
{}

void UpdateLODQuery::getSQL(std::string &sql)
{
	sql = "begin :result := asset_exporter.update_lod(:path, :fileName); end;";
}

bool UpdateLODQuery::bindParameters()
{
	if (!bindParameter(m_returnCode)) return false;
	if (!bindParameter(m_path))       return false;
	if (!bindParameter(m_filename))   return false;
	return true;
}

bool UpdateLODQuery::bindColumns()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

UpdateComponentQuery::UpdateComponentQuery()
{}

void UpdateComponentQuery::getSQL(std::string &sql)
{
	sql = "begin :result := asset_exporter.update_component(:path, :fileName); end;";
}

bool UpdateComponentQuery::bindParameters()
{
	if (!bindParameter(m_returnCode)) return false;
	if (!bindParameter(m_path))       return false;
	if (!bindParameter(m_filename))   return false;
	return true;
}

bool UpdateComponentQuery::bindColumns()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

#if 0
UpdateTextureQuery::UpdateTextureQuery()
{}

void UpdateTextureQuery::getSQL(std::string &sql)
{
	sql = "begin :result := asset_exporter.update_texture(:path, :fileName, :size, :depth, :numMips); end;";
}

bool UpdateTextureQuery::bindParameters()
{
	if (!bindParameter(m_returnCode)) return false;
	if (!bindParameter(m_path))       return false;
	if (!bindParameter(m_filename))   return false;
	if (!bindParameter(m_size))       return false;
	if (!bindParameter(m_depth))      return false;
	if (!bindParameter(m_numMips))    return false;
	return true;
}

bool UpdateTextureQuery::bindColumns()
{
	return true;
}
#endif
//////////////////////////////////////////////////////////////////////////

UpdateAnimationQuery::UpdateAnimationQuery()
{}

void UpdateAnimationQuery::getSQL(std::string &sql)
{
	sql = "begin :result := asset_exporter.update_animation(:path, :fileName, :numFrames, :type, :gender); end;";
}

bool UpdateAnimationQuery::bindParameters()
{
	if (!bindParameter(m_returnCode)) return false;
	if (!bindParameter(m_path))       return false;
	if (!bindParameter(m_filename))   return false;
	if (!bindParameter(m_numFrames))  return false;
	if (!bindParameter(m_type))       return false;
	if (!bindParameter(m_gender))     return false;
	return true;
}

bool UpdateAnimationQuery::bindColumns()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

UpdateSkeletonQuery::UpdateSkeletonQuery()
{}

void UpdateSkeletonQuery::getSQL(std::string &sql)
{
	sql = "begin :result := asset_exporter.update_skeleton(:path, :fileName, :numBones); end;";
}

bool UpdateSkeletonQuery::bindParameters()
{
	if (!bindParameter(m_returnCode)) return false;
	if (!bindParameter(m_path))       return false;
	if (!bindParameter(m_filename))   return false;
	if (!bindParameter(m_numBones))   return false;
	return true;
}

bool UpdateSkeletonQuery::bindColumns()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

UpdateTextureQuery::UpdateTextureQuery()
{}

void UpdateTextureQuery::getSQL(std::string &sql)
{
	sql = "begin :result := asset_exporter.update_texture(:path, :fileName); end;";
}

bool UpdateTextureQuery::bindParameters()
{
	if (!bindParameter(m_returnCode)) return false;
	if (!bindParameter(m_path))       return false;
	if (!bindParameter(m_filename))   return false;
	return true;
}

bool UpdateTextureQuery::bindColumns()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

UpdateRelationshipQuery::UpdateRelationshipQuery()
{}

void UpdateRelationshipQuery::getSQL(std::string &sql)
{
	sql = "begin asset_exporter.add_tree_node(:appearanceNumber, :nodeNumber, :entryNumber, :exitNumber, :parentEntryNumber); end;";
}

bool UpdateRelationshipQuery::bindParameters()
{
	if (!bindParameter(m_appearanceNumber))  return false;
	if (!bindParameter(m_nodeNumber))        return false;
	if (!bindParameter(m_entryNumber))       return false;
	if (!bindParameter(m_exitNumber))        return false;
	if (!bindParameter(m_parentEntryNumber)) return false;
	return true;
}

bool UpdateRelationshipQuery::bindColumns()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

ClearRelationshipTreeQuery::ClearRelationshipTreeQuery()
{}

void ClearRelationshipTreeQuery::getSQL(std::string &sql)
{
	sql = "begin asset_exporter.wipe_tree(:appearanceNumber); end;";
}

bool ClearRelationshipTreeQuery::bindParameters()
{
	if (!bindParameter(m_appearanceNumber))  return false;
	return true;
}

bool ClearRelationshipTreeQuery::bindColumns()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

UpdatePropertyQuery::UpdatePropertyQuery()
{}

void UpdatePropertyQuery::getSQL(std::string &sql)
{
	sql = "begin asset_exporter.set_appearance_property(:appearanceNumber, :propertyName, :propertyValue); end;";
}

bool UpdatePropertyQuery::bindParameters()
{
	if (!bindParameter(m_appearanceNumber)) return false;
	if (!bindParameter(m_propertyName))     return false;
	if (!bindParameter(m_propertyValue))    return false;
	return true;
}

bool UpdatePropertyQuery::bindColumns()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

RetrieveAppearanceQuery::RetrieveAppearanceQuery()
{}

void RetrieveAppearanceQuery::getSQL(std::string &sql)
{
	sql = "select name, filename, id, status_id from appearances where category_id = :categoryNumber";
}

bool RetrieveAppearanceQuery::bindParameters()
{
	if (!bindParameter(m_categoryNumber))   return false;
	return true;
}

bool RetrieveAppearanceQuery::bindColumns()
{
	if (!bindCol(m_assetName)) return false;
	if (!bindCol(m_fileName))  return false;
	if (!bindCol(m_assetID))   return false;
	if (!bindCol(m_statusID))  return false;
	return true;
}

//////////////////////////////////////////////////////////////////////////

UpdateAppearanceQuery::UpdateAppearanceQuery()
{}

void UpdateAppearanceQuery::getSQL(std::string &sql)
{
	sql = "begin asset_exporter.update_appearance2(:appearanceNumber, :author, :statusId); end;";
}

bool UpdateAppearanceQuery::bindParameters()
{
	if (!bindParameter(m_appearanceNumber)) return false;
	if (!bindParameter(m_author))           return false;
	if (!bindParameter(m_statusId))         return false;
	return true;
}

bool UpdateAppearanceQuery::bindColumns()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
#endif
