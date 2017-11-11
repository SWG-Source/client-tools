// ======================================================================
//
// FilesystemTree.cpp
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "FilesystemTree.h"

#include <windows.h>

//-----------------------------------------------------------------

namespace
{
	namespace MyOs
	{
		int listFilesGeneric(const char*dirname, char**& children, size_t bits, size_t exclude)
		{
			std::vector<char*> result;
			
			WIN32_FIND_DATA data;
			
			HANDLE handle = FindFirstFile(dirname,&data);
			
			if(handle != INVALID_HANDLE_VALUE)
			{	
				do
				{
					if((data.dwFileAttributes& exclude) == 0u &&
						(data.dwFileAttributes& bits) != 0u &&
						data.cFileName [0] != '.')
						result.push_back(DuplicateString(data.cFileName));
				}
				while(FindNextFile(handle,&data));
				
				IGNORE_RETURN(FindClose(handle));
				
				if(!result.empty())
				{
					children = new char* [result.size()];			
					IGNORE_RETURN(std::copy(result.begin(), result.end(), children));
				}
			}
			
			return static_cast<int>(result.size());
		}

		inline int listDirectories(const char*dirname, char**& children)
		{
			return listFilesGeneric(dirname, children, FILE_ATTRIBUTE_DIRECTORY, 0);
		}

		inline int listFiles(const char*dirname, char**& children)
		{
			return listFilesGeneric(dirname, children, 0xffffffff, FILE_ATTRIBUTE_DIRECTORY);
		}
	}
}
// ======================================================================

FilesystemTree::FilesystemNode::FilesystemNode()
: Node()
, m_filter("*")
{
}

//-----------------------------------------------------------------

FilesystemTree::FilesystemNode::FilesystemNode(const FilesystemNode& rhs)
: Node(rhs)
, m_filter(rhs.m_filter)
{
}

//-----------------------------------------------------------------

FilesystemTree::FilesystemNode::~FilesystemNode()
{
}

//-----------------------------------------------------------------

AbstractFilesystemTree::Node* FilesystemTree::FilesystemNode::clone() const
{
	return new FilesystemNode(*this);
}

//-----------------------------------------------------------------

void FilesystemTree::FilesystemNode::setFilter(const std::string& filter)
{
	m_filter = filter;
}

//-----------------------------------------------------------------

FilesystemTree::FilesystemTree()
: AbstractFilesystemTree(),
  m_rootPath(),
  m_filter("*")
{
}

//-----------------------------------------------------------------

FilesystemTree::FilesystemTree(const FilesystemTree& rhs)
: AbstractFilesystemTree(rhs),
  m_rootPath(rhs.m_rootPath),
  m_filter(rhs.m_filter)
{
}

//-----------------------------------------------------------------

FilesystemTree::~FilesystemTree()
{
}

//-----------------------------------------------------------------

void FilesystemTree::setRootPath(const std::string& path)
{
	m_rootPath = path;
}

//-----------------------------------------------------------------

void FilesystemTree::setFilter(const std::string& filter)
{
	m_filter = filter;
}

//-----------------------------------------------------------------

void FilesystemTree::populateTree()
{
	if(m_rootNode)
		delete m_rootNode;

	m_rootNode = 0;

	FilesystemNode* newNode = new FilesystemNode;

	newNode->name = m_rootPath;
	newNode->type = Node::Folder;
	newNode->setFilter( m_filter );
	
	m_rootNode = newNode;
	m_rootNode->populateTree(m_rootNode->name);
	
}

//-----------------------------------------------------------------

void FilesystemTree::FilesystemNode::populateTree(const std::string& path)
{
	
	{
		char** subdirs = 0;
		const int numSubdirs = MyOs::listDirectories((path + "/*").c_str(), subdirs);
		
		for(int i = 0; i < numSubdirs; ++i)
		{
			FilesystemNode* newNode = new FilesystemNode;
			newNode->name = subdirs [i];
			newNode->type = Folder;
			newNode->setFilter(m_filter);

			newNode->populateTree(path + "/" + newNode->name);
			
			children.push_back(newNode);
			delete[] subdirs [i];
			subdirs [i] = 0;
		}
		
		if(numSubdirs)
			delete[] subdirs;
		
	}
	
	{
		char** files = 0;
		std::string filespec(path);

		filespec += "/";
		filespec += m_filter;

		const int numFiles = MyOs::listFiles(filespec.c_str(), files);
		
		for(int i = 0; i < numFiles; ++i)
		{
			FilesystemNode* newNode = new FilesystemNode;
			newNode->name = files [i];
			newNode->type = File;

			children.push_back(newNode);

			delete[] files [i];
			files [i] = 0;
		}
		
		if(numFiles)
			delete[] files;
	}
}

// ======================================================================
