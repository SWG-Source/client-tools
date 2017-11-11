// ======================================================================
//
// ExporterLog.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef EXPORTERLOG_H
#define EXPORTERLOG_H

//-------------------------------------------------------------------

#include "sharedMath/Vector.h"  //non-static class member
#include "MayaHierarchy.h" //access MayaHierarchy::Node*

#include <string>          //defined as non-static nested class members, also used as return type
#include <list>            //used as return type
#include <set>
#include <vector>

class AlienbrainImporter;
class DatabaseImporter;
class ExportStaticMesh;
class Hardpoint;
class Messenger;
class MFnMesh;
class Transform;

//lint -save -e1925 // public symbols

//-------------------------------------------------------------------

/**
 * Builds and exports information about the maya export process.  
 * This module also maintains data necessary for the Alienbrain import process 
 * (since they both need the same data).
 */
class ExporterLog
{
	friend AlienbrainImporter;
	friend DatabaseImporter;

public:

	//public nested classes, can't hide them within .cpp file
	class BaseLogInfo
	{
	public:
		BaseLogInfo(const std::string &name, const std::string &type);
		virtual ~BaseLogInfo();

		std::string m_filename;                    
		stdvector<BaseLogInfo *>::fwd m_children;  
		std::string m_type;                        

		BaseLogInfo * find(const std::string &name)
		{
			int x;
			for (x = static_cast<int>(m_children.size()-1); x >= 0; --x)
			{
				if (name == m_children[static_cast<size_t>(x)]->m_filename)
				{
					return m_children[static_cast<size_t>(x)];
				}
			}
			return 0;
		}

		template <class T>
		void findFirst(T &result)
		{
			result = T(0);
			int x;
			for (x = m_children.size()-1; x >= 0; --x)
			{
				result = dynamic_cast<T>(m_children[x]);
				if (result)
					return;
			}
		}

		template <class T>
			void filter(std::vector<T> &m_newChildren) const
		{
			int x;
			for (x = m_children.size()-1; x >= 0; --x)
			{
				T result = dynamic_cast<T>(m_children[x]);
				if (result)
				{
					m_newChildren.push_back(result);
				}
			}
		}

		void add(BaseLogInfo * t); // adds the object if it is not already present, deletes it if it is.

		template <class T>
		void addOne(T toAdd)
		{
			for(std::vector<BaseLogInfo *>::reverse_iterator rev_it = m_children.rbegin(); rev_it != m_children.rend(); ++rev_it)
			{
				T result = dynamic_cast<T>((*rev_it));
				if (result)
				{
					delete result;
					//convert reverse iterator to regular iterator for erase call
					std::vector<BaseLogInfo *>::iterator it = rev_it.base();
					m_children.erase(it);
				}
				m_children.push_back(toAdd);
			}
		}
	private:
		BaseLogInfo();
		BaseLogInfo(const BaseLogInfo &o);
		BaseLogInfo &operator =(const BaseLogInfo &);
	};

	class ComponentLogInfo: public BaseLogInfo
	{
	public:
		explicit ComponentLogInfo(const std::string &filename);

	private:
		ComponentLogInfo();
		ComponentLogInfo(const ComponentLogInfo &o);
		ComponentLogInfo &operator =(const ComponentLogInfo &);
	};

	class LODLogInfo: public BaseLogInfo
	{
	public:
		explicit LODLogInfo(const std::string &filename);

	private:
		LODLogInfo();
		LODLogInfo(const LODLogInfo &o);
		LODLogInfo &operator =(const LODLogInfo &);
	};

	class ShaderLogInfo: public BaseLogInfo
	{
	public:
		///the filename of the effect used
		std::string m_effectFilename;                    
		///the referenced textures
		stdvector<std::string>::fwd m_textureFilenameList;

		ShaderLogInfo(const std::string &filename, const std::string &effectname);

	private:
		ShaderLogInfo();
		ShaderLogInfo(const ShaderLogInfo &o);
		ShaderLogInfo &operator =(const ShaderLogInfo &);
	};

	class MeshLogInfo: public BaseLogInfo
	{
	public:
		///the number of vertices in the mesh
		int                     m_numberOfVertices; 
		///the number of polygons in the mesh
		int                     m_numberOfPolygons; 
		///the center of the extent sphere
		Vector                  m_sphereCenter;     
		///the extent radius
		real                    m_sphereRadius;     
		///the extent box min
		Vector                  m_boxMin;           
		///the extent box max
		Vector                  m_boxMax;           
		///the hardpoints that this mesh uses
		stdvector<Hardpoint*>::fwd   m_hardpointList;
		int                     m_numUVSets;        

		explicit MeshLogInfo(const std::string &filename);
		virtual ~MeshLogInfo();

	private:
		MeshLogInfo();
		MeshLogInfo(const MeshLogInfo &o);
		MeshLogInfo &operator =(const MeshLogInfo &);
	};

	class TextureLogInfo: public BaseLogInfo
	{
	public:
		explicit TextureLogInfo(const std::string &filename);

	private:
		TextureLogInfo();
		TextureLogInfo(const TextureLogInfo &o);
		TextureLogInfo &operator =(const TextureLogInfo &);
	};

	class SkeletalTemplateLogInfo: public BaseLogInfo
	{
	public:
		explicit SkeletalTemplateLogInfo(const std::string &filename);

	private:
		SkeletalTemplateLogInfo();
		SkeletalTemplateLogInfo(const SkeletalTemplateLogInfo &o);
		SkeletalTemplateLogInfo &operator =(const SkeletalTemplateLogInfo &);
	};
	
	class SkeletalAnimationLogInfo: public BaseLogInfo
	{
	public:
		explicit SkeletalAnimationLogInfo(const std::string &filename);

	private:
		SkeletalAnimationLogInfo();
		SkeletalAnimationLogInfo(const SkeletalAnimationLogInfo &o);
		SkeletalAnimationLogInfo &operator =(const SkeletalAnimationLogInfo &);
	};
	
	class SkeletalMeshGeneratorLogInfo : public MeshLogInfo
	{
	public:
		explicit SkeletalMeshGeneratorLogInfo(const std::string &filename);

	private:
		SkeletalMeshGeneratorLogInfo();
		SkeletalMeshGeneratorLogInfo(const SkeletalMeshGeneratorLogInfo &o);
		SkeletalMeshGeneratorLogInfo &operator =(const SkeletalMeshGeneratorLogInfo &);
	};

public:
	static void install (Messenger *newMessenger);
	static void remove (void);
	
	static std::string getBaseDir();
	static void setBaseDir(const std::string& baseDir);

	//accessors
	static std::string           getLogFilename();
	static std::string           getSourceFilename();
	static std::string           getAuthor();
	static std::string           getAssetGroup();
	static std::string           getAssetName();
	static int                   getAssetId();
	static const std::vector<std::string>& getSourceTexturesFilenames();

	static std::set<std::string> const & getClientDestinationFiles();
	static std::set<std::string> const & getSharedDestinationFiles();
	static std::set<std::string> const & getServerDestinationFiles();
	
	//mutators
	static void setLogFilename(const std::string& l);
	static void setMayaExporterVersion(const std::string& version);
	static void setMayaCommand(const std::string& command);
	static void setMayaExportOptions(const std::string& options);
	static void setAssetGroup(const std::string& assetGroup);
	static void setAssetName(const std::string& assetName);
	static void setAssetId(int assetId);
	static bool loadLogFile(const std::string& logFilename);
	static void addClientDestinationFile(const std::string& destinationFile);
	static void addSharedDestinationFile(const std::string& destinationFile);
	static void addServerDestinationFile(const std::string& destinationFile);

		//-- source file info
	static void setSourceFilename (const std::string& newSourceFilename);
	static void setAuthor (const std::string& newAuthor);
	static void setSourceNumberOfVertices (int newSourceNumberOfVertices);
	static void setSourceNumberOfPolygons (int newSourceNumberOfPolygons);
	static void setSourceNumberOfUVSets (int newSourceNumberOfUVSets);
	static void addSourceShader (const std::string& newShaderName);
	static void addSourceTexture (const std::string& newTextureName);
	static void addSkeletalTemplate(const std::string& newSKT);
	static void addSkeletalAnimation(const std::string& newANS);

	static void addDestinationShaderToCurrentMesh (const std::string& newShaderName, const std::string& newEffectFilename);
	static void addDestinationTextureToCurrentShader (const std::string& newTextureName);

	static void buildStaticMesh(const MayaHierarchy::Node* root);
	static void buildSkeletalTemplate(const MayaHierarchy::Node* root);
	static void buildSkeletalAnimation(const MayaHierarchy::Node* root);
	static void buildSkeletalMeshGenerator(const std::string &meshName, int vertexCount, int polygonCount);

	static void pushCurrentObject(const std::string &name);
	static void processChildObjects(const MayaHierarchy::Node *root);
	static void popCurrentObject();

	//-- extents
	static void setExtentForCurrentMesh(const Vector& center, real radius, const Vector& boxMin, const Vector& boxMax);
	static void setUVSetsForCurrentMesh(int numUVSets);

	//-- hardpoints
	static void addHardpointToCurrentMesh (const std::string& newName, const Transform& newTransform);

	static void reset (void);
	static bool writeStaticMesh(const std::string& filename, bool interactive);
	static bool writeSkeletalTemplate(const std::string& filename, bool interactive);
	static bool writeSkeletalAnimation(const std::string& filename, bool interactive);
	static bool writeSkeletalMeshGenerator(const std::string& filename, bool interactive);
	static bool writeSkeletalAppearanceTemplate(const std::string& filename, bool interactive);

	//--
	static const Vector& getCurrentSphereCenter (void);
	static real          getCurrentSphereRadius (void);

private:
	static void buildLogInfo(const MayaHierarchy::Node* root);
	static void buildMeshLogInfo(const MayaHierarchy::Node* root);

	static void buildGenericLogInfo(const MayaHierarchy::Node* root, const char * type);

	static void writeSharedData(const std::string& filename);
	static void writeLOD(int indentLevel, const LODLogInfo* lod, bool isBase = false, const char* base = NULL);
	static void writeComponent(int indentLevel, const ComponentLogInfo* component, bool isBase = false, const char* base = NULL);
	static void writeMesh(int indentLevel, MeshLogInfo* mesh, bool isBase = false, const char* base = NULL);
	static void writeShader(int indentLevel, ShaderLogInfo* shader);
	static void writeHardpoint(int indentLevel, const Hardpoint* hardpoint);
	static void writeTexture(int indentLevel, const std::string& textureFilename);

private:
	//disabled
	ExporterLog (void);
	~ExporterLog (void);
	ExporterLog (const ExporterLog&);
	ExporterLog& operator= (const ExporterLog&);

private:
	///is the system installed?
	static bool               ms_installed;
	///name of the source Maya file
	static std::string        ms_sourceFilename;
	///name of the author
	static std::string        ms_author;
	///the number of vertices in the source file
	static int                ms_sourceNumberOfVertices;
	///the number of polygons in the source file
	static int                ms_sourceNumberOfPolygons;
	///the number of UV sets in the source file
	static int                ms_sourceNumberOfUVSets;

	///the children are a list of all objects
	static BaseLogInfo * ms_objects;
	///a stack of current objects
	static stdvector<BaseLogInfo *>::fwd ms_currentObjects;

	///the current shader
	static ShaderLogInfo*       ms_currentShader;

	///a handle to the log file
	static FILE*              ms_outfile;

	///the log filename
	static std::string        ms_logFilename;
	///name of the maya command used (so we can auto export)
	static std::string        ms_mayaCommand;
	///command line options used to export (so we can auto export)
	static std::string        ms_exportOptions;
	///the name of the asset group we're importing into
	static std::string        ms_assetGroupName;
	///the name of the asset for the database
	static std::string        ms_assetName;
	///the asset it for the database
	static int                ms_assetId;
	
	///logfile version number, possibly useful in the future for parsing different versions of files
	static const int          ms_versionNumber;
	//the destination files created by an export
	static stdset<std::string>::fwd ms_destClientFileContainer;
	static stdset<std::string>::fwd ms_destSharedFileContainer;
	static stdset<std::string>::fwd ms_destServerFileContainer;
	//the source files used by an export
	static stdset<std::string>::fwd ms_sourceFileContainer;
	static stdvector<std::string>::fwd ms_sourceTexturesContainer;
	///the base directory that we're exporting to
	static std::string        ms_baseDir;
	///the version of the mayaExporter used
	static std::string        ms_mayaExporterVersion;

	static const std::string  ms_meshDirectory;
	static const std::string  ms_lodDirectory;
	static const std::string  ms_componentDirectory;
	//MUST be named messenger for #define's, can't make it ms_messenger (see Messenger.h)
	static Messenger*   messenger;

};

//-------------------------------------------------------------------

/** Set the name of the source file
  */
inline void ExporterLog::setSourceFilename (const std::string& newSourceFilename)
{
	if (!ms_installed)
		return;

	ms_sourceFilename = newSourceFilename;
}

//-------------------------------------------------------------------

/** Set the name of the author
  */
inline void ExporterLog::setAuthor (const std::string& newAuthor)
{
	if (!ms_installed)
		return;

	ms_author = newAuthor;
}

//-------------------------------------------------------------------

/** Set the number of source vertices
  */
inline void ExporterLog::setSourceNumberOfVertices (int newSourceNumberOfVertices)
{
	if (!ms_installed)
		return;

	ms_sourceNumberOfVertices = newSourceNumberOfVertices;
}

//-------------------------------------------------------------------

/** Set the number of source polygons
  */
inline void ExporterLog::setSourceNumberOfPolygons (int newSourceNumberOfPolygons)
{
	if (!ms_installed)
		return;

	ms_sourceNumberOfPolygons = newSourceNumberOfPolygons;
}

//-------------------------------------------------------------------

/** Set the number of source UV sets
  */
inline void ExporterLog::setSourceNumberOfUVSets (int newSourceNumberOfUVSets)
{
	if (!ms_installed)
		return;

	ms_sourceNumberOfUVSets = newSourceNumberOfUVSets;
}

//-------------------------------------------------------------------

/** Set the name of the Maya command to use to do the export
  */
inline void ExporterLog::setMayaCommand(const std::string& command)
{
	if (!ms_installed)
		return;

	ms_mayaCommand = command;
}

//-------------------------------------------------------------------

/** Set the name of the Maya node to use to do the export
  */
inline void ExporterLog::setMayaExportOptions(const std::string& options)
{
	if (!ms_installed)
		return;

	ms_exportOptions = options;
}

//-------------------------------------------------------------------

/** Set the name of the asset to use to store the files in the AssetDatabase

  */
inline void ExporterLog::setAssetName(const std::string& assetName)
{
	if (!ms_installed)
		return;

	ms_assetName = assetName;
}

//-------------------------------------------------------------------

/** Set the name of the asset group to use to store the files the AssetDatabase
  */
inline void ExporterLog::setAssetGroup(const std::string& assetGroup)
{
	if (!ms_installed)
		return;

	ms_assetGroupName = assetGroup;
}

//-------------------------------------------------------------------

/** Set the id of the asset to use to store the files the AssetDatabase
  */
inline void ExporterLog::setAssetId(int assetId)
{
	if (!ms_installed)
		return;

	ms_assetId = assetId;
}

//-------------------------------------------------------------------

/** Set the mayaExporter Version
  */
inline void ExporterLog::setMayaExporterVersion(const std::string& version)
{
	if (!ms_installed)
		return;

	ms_mayaExporterVersion = version;
}

//-------------------------------------------------------------------

inline std::string ExporterLog::getBaseDir()
{
	return ms_baseDir;
}

//-------------------------------------------------------------------

inline void ExporterLog::setBaseDir(const std::string& baseDir)
{
	ms_baseDir = baseDir;
}

//-------------------------------------------------------------------

inline std::string ExporterLog::getLogFilename()
{
	return ms_logFilename;
}

//-------------------------------------------------------------------

inline void ExporterLog::setLogFilename(const std::string& l)
{
	ms_logFilename = l;
}

//-------------------------------------------------------------------

inline std::string ExporterLog::getSourceFilename()
{
	return ms_sourceFilename;
}

//-------------------------------------------------------------------

inline std::string ExporterLog::getAuthor()
{
	return ms_author;
}

//-------------------------------------------------------------------

inline std::set<std::string> const & ExporterLog::getClientDestinationFiles() 
{
	return ms_destClientFileContainer;
}

//-------------------------------------------------------------------

inline std::set<std::string> const & ExporterLog::getSharedDestinationFiles() 
{
	return ms_destSharedFileContainer;
}

// ----------------------------------------------------------------------

inline std::set<std::string> const & ExporterLog::getServerDestinationFiles()
{
	return ms_destServerFileContainer;
}

//-------------------------------------------------------------------

inline std::string ExporterLog::getAssetGroup()
{
	return ms_assetGroupName;
}

//-------------------------------------------------------------------

inline std::string ExporterLog::getAssetName()
{
	return ms_assetName;
}

//-------------------------------------------------------------------

inline int ExporterLog::getAssetId()
{
	return ms_assetId;
}

//-------------------------------------------------------------------

inline void ExporterLog::addClientDestinationFile(const std::string& destinationFile)
{
	IGNORE_RETURN(ms_destClientFileContainer.insert(destinationFile));
}

//-------------------------------------------------------------------

inline void ExporterLog::addSharedDestinationFile(const std::string& destinationFile)
{
	IGNORE_RETURN(ms_destSharedFileContainer.insert(destinationFile));
}

// ----------------------------------------------------------------------

inline void ExporterLog::addServerDestinationFile(const std::string& destinationFile)
{
	IGNORE_RETURN(ms_destServerFileContainer.insert(destinationFile));
}

//-------------------------------------------------------------------

#endif
