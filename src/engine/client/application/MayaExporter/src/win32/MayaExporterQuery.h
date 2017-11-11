#ifndef MAYAEXPORTER_QUERIES_H
#define MAYAEXPORTER_QUERIES_H

#include "sharedDatabaseInterface/DBQuery.h"
#include "sharedDatabaseInterface/Bindable.h"

/*
  Put multiple query classes in the same file, since this is how the rest of the game does it, and it drastically
  cuts down on the number of files needed.
*/

//lint -save -e1925 // public symbols

// ======================================================================

namespace DBQuery
{
	/** 
	 *  A query to update the information for a shader asset
	 */
	class UpdateShaderQuery : public DB::Query
	{
	public:
		DB::BindableLong        m_returnCode;
		DB::BindableString<512> m_path;      
		DB::BindableString<512> m_filename;  
		DB::BindableString<512> m_effectName;

	public:
		UpdateShaderQuery();
		void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	private:
		UpdateShaderQuery(const UpdateShaderQuery&);
		UpdateShaderQuery& operator=(const UpdateShaderQuery&);
	};

// ======================================================================

	/** 
	 *  A query to update the information for a mesh asset
	 */
	class UpdateMeshQuery : public DB::Query
	{
	public:
		DB::BindableLong        m_returnCode;   
		DB::BindableString<512> m_path;         
		DB::BindableString<512> m_filename;     
		DB::BindableString<512> m_type;         
		DB::BindableLong        m_numPolygons;  
		DB::BindableLong        m_numVertices;  
		DB::BindableLong        m_numUVSets;    
		DB::BindableLong        m_numHardpoints;
		DB::BindableDouble      m_extentCenterX;
		DB::BindableDouble      m_extentCenterY;
		DB::BindableDouble      m_extentCenterZ;
		DB::BindableDouble      m_extentRadius; 
		DB::BindableDouble      m_extentBoxMinX;
		DB::BindableDouble      m_extentBoxMinY;
		DB::BindableDouble      m_extentBoxMinZ;
		DB::BindableDouble      m_extentBoxMaxX;
		DB::BindableDouble      m_extentBoxMaxY;
		DB::BindableDouble      m_extentBoxMaxZ;

	public:
		UpdateMeshQuery();
		void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	private:
		UpdateMeshQuery(const UpdateMeshQuery&);
		UpdateMeshQuery& operator=(const UpdateMeshQuery&);
	};

// ======================================================================

	/** 
	 *  A query to update the information for an LOD asset
	 */
	class UpdateLODQuery : public DB::Query
	{
	public:
		DB::BindableLong        m_returnCode;
		DB::BindableString<512> m_path;      
		DB::BindableString<512> m_filename;  

	public:
		UpdateLODQuery();
		void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	private:
		UpdateLODQuery(const UpdateLODQuery&);
		UpdateLODQuery& operator=(const UpdateLODQuery&);
	};

// ======================================================================

	/** 
	 *  A query to update the information for a component asset
	 */
	class UpdateComponentQuery : public DB::Query
	{
	public:
		DB::BindableLong        m_returnCode;
		DB::BindableString<512> m_path;      
		DB::BindableString<512> m_filename;  

	public:
		UpdateComponentQuery();
		void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	private:
		UpdateComponentQuery(const UpdateComponentQuery&);
		UpdateComponentQuery& operator=(const UpdateComponentQuery&);
	};

// ======================================================================

#if 0
	/** 
	 *  A query to update the information for a texture asset
	 */
	class UpdateTextureQuery : public DB::Query
	{
	public:
		DB::BindableLong        m_returnCode;
		DB::BindableString<512> m_path;      
		DB::BindableString<512> m_filename;  
		DB::BindableLong        m_size;      
		DB::BindableLong        m_depth;     
		DB::BindableLong        m_numMips;   

	public:
		UpdateTextureQuery();
		void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	private:
		UpdateTextureQuery(const UpdateTextureQuery&);
		UpdateTextureQuery& operator=(const UpdateTextureQuery&);
	};
#endif
// ======================================================================

	/** 
	 *  A query to update the information for a animation asset
	 */
	class UpdateAnimationQuery : public DB::Query
	{
	public:
		DB::BindableLong        m_returnCode;
		DB::BindableString<512> m_path;      
		DB::BindableString<512> m_filename;  
		DB::BindableLong        m_numFrames; 
		DB::BindableLong        m_gender;    
		DB::BindableString<512> m_type;      

	public:
		UpdateAnimationQuery();
		void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	private:
		UpdateAnimationQuery(const UpdateAnimationQuery&);
		UpdateAnimationQuery& operator=(const UpdateAnimationQuery&);
	};

// ======================================================================

	/** 
	 *  A query to update the information for a skeleton asset
	 */
	class UpdateSkeletonQuery : public DB::Query
	{
	public:
		DB::BindableLong        m_returnCode;
		DB::BindableString<512> m_path;      
		DB::BindableString<512> m_filename;  
		DB::BindableLong        m_numBones;  

	public:
		UpdateSkeletonQuery();
		void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	private:
		UpdateSkeletonQuery(const UpdateSkeletonQuery&);
		UpdateSkeletonQuery& operator=(const UpdateSkeletonQuery&);
	};

// ======================================================================

	/** 
	 *  A query to update the information for a texture asset
	 */
	class UpdateTextureQuery : public DB::Query
	{
	public:
		DB::BindableLong        m_returnCode;
		DB::BindableString<512> m_path;      
		DB::BindableString<512> m_filename;  

	public:
		UpdateTextureQuery();
		void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	private:
		UpdateTextureQuery(const UpdateTextureQuery&);
		UpdateTextureQuery& operator=(const UpdateTextureQuery&);
	};

// ======================================================================

	/** 
	 *  A query to clear all the relationships for a given appearance (since we rebuild them completely each export)
	 */
	class ClearRelationshipTreeQuery : public DB::Query
	{
	public:
		DB::BindableLong        m_appearanceNumber;

	public:
		ClearRelationshipTreeQuery();
		void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	private:
		ClearRelationshipTreeQuery(const ClearRelationshipTreeQuery&);
		ClearRelationshipTreeQuery& operator=(const ClearRelationshipTreeQuery&);
	};


// ======================================================================

	/** 
	 *  A query to update the information for a relationship between two assets
	 */
	class UpdateRelationshipQuery : public DB::Query
	{
	public:
		DB::BindableLong        m_appearanceNumber; 
		DB::BindableLong        m_nodeNumber;       
		DB::BindableLong        m_entryNumber;      
		DB::BindableLong        m_exitNumber;       
		DB::BindableLong        m_parentEntryNumber;

	public:
		UpdateRelationshipQuery();
		void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	private:
		UpdateRelationshipQuery(const UpdateRelationshipQuery&);
		UpdateRelationshipQuery& operator=(const UpdateRelationshipQuery&);
	};

// ======================================================================

	/** 
	 *  A query to update the information for a custom property on an asset
	 */
	class UpdatePropertyQuery : public DB::Query
	{
	public:
		DB::BindableLong        m_appearanceNumber;
		DB::BindableString<512> m_propertyName;    
		DB::BindableString<512> m_propertyValue;   

	public:
		UpdatePropertyQuery();
		void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	private:
		UpdatePropertyQuery(const UpdatePropertyQuery&);
		UpdatePropertyQuery& operator=(const UpdatePropertyQuery&);
	};

// ======================================================================

	/** 
	 *  A query to retrieve the names of all the appearances (so the artist can select one)
	 */
	class RetrieveAppearanceQuery : public DB::Query
	{
	public:
		DB::BindableLong        m_categoryNumber;
		DB::BindableString<512> m_fileName;
		DB::BindableString<512> m_assetName;
		DB::BindableLong        m_assetID;
		DB::BindableLong        m_statusID;

	public:
		void SetCategoryNumber(int cat)        {m_categoryNumber = cat;}
		Unicode::String GetAssetName() const   {return m_assetName.getValue();}
		Unicode::String GetFilename() const    {return m_fileName.getValue();}
		int GetAssetID() const                 {return m_assetID.getValue();}
		int GetStatusID() const                {return m_statusID.getValue();}
		RetrieveAppearanceQuery();
		void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	private:
		RetrieveAppearanceQuery(const RetrieveAppearanceQuery&);
		RetrieveAppearanceQuery& operator=(const RetrieveAppearanceQuery&);
	};

// ======================================================================

	/** 
	 *  A query to update the information for an appearance
	 */
	class UpdateAppearanceQuery : public DB::Query
	{
	public:
		DB::BindableLong        m_appearanceNumber;
		DB::BindableString<200> m_author;
		DB::BindableLong        m_statusId;

	public:
		UpdateAppearanceQuery();
		void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	private:
		UpdateAppearanceQuery(const UpdateAppearanceQuery&);
		UpdateAppearanceQuery& operator=(const UpdateAppearanceQuery&);
	};
} //end namespace DBQuery

// ======================================================================
#endif //MAYAEXPORTER_QUERIES_H
