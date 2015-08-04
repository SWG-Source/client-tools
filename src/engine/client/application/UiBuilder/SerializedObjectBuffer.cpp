#include "FirstUiBuilder.h"
#include "SerializedObjectBuffer.h"

#include "UIBaseObject.h"
#include "UILoader.h"
#include "UIPage.h"
#include "UISaver.h"
#include "UIUtils.h"

namespace SerializedObjectBufferNamespace
{
	typedef std::map<UINarrowString, UINarrowString> NarrowStringMap;

	class Loader : public UILoader
	{
	public:

		Loader() {}
		virtual bool LoadStringFromResource(const UINarrowString &ResourceName, UINarrowString &Out);

		NarrowStringMap m_files;
	};

	bool Loader::LoadStringFromResource(const UINarrowString &ResourceName, UINarrowString &Out)
	{
		NarrowStringMap::iterator fi = m_files.find(ResourceName);
		if (fi!=m_files.end())
		{
			Out = fi->second;
			return true;
		}
		else
		{
			return false;
		}
	}
}
using namespace SerializedObjectBufferNamespace;

// ==============================================================================

void SerializedObjectBuffer::setBuffer(int size, const char *data)
{
	m_data.resize(size);
	if (size && data)
	{
		memcpy(&m_data[0], data, size);
	}
}

// ==============================================================================

void SerializedObjectBuffer::serialize(UIBaseObject &subTree)
{
	//-- force packing
	if (subTree.IsA(TUIPage))
	{
		UI_ASOBJECT(UIPage, &subTree)->ForcePackChildren();
	}

	std::string NoName("*NONAME");

	// -------------------------------------------------------

	UIString wMainFileName;
	UINarrowString mainFileName;
	if (subTree.GetProperty(UIBaseObject::PropertyName::SourceFile, wMainFileName))
	{
		mainFileName = UIUnicode::wideToNarrow(wMainFileName);
	}
	else
	{
		mainFileName = NoName;
	}

	// -------------------------------------------------------

	// -------------------------------------------------------
	// create individual file buffers
	UISaver			Saver;
	NarrowStringMap Output;
	Saver.SaveToStringSet(Output, Output.end(), subTree);
	// -------------------------------------------------------

	// -------------------------------------------------------
	// pre-allocate destination buffer.
	{
		int finalSize=0;

		finalSize += mainFileName.size() + 1;

		for (NarrowStringMap::iterator i = Output.begin(); i != Output.end();++i)
		{
			const std::string & FileName   = i->first.empty () ? NoName : i->first;
			const std::string & outputData = i->second;

			finalSize += FileName.size() + 1 + outputData.size() + 1;
		}
		m_data.reserve(m_data.size() + finalSize);
	}
	// -------------------------------------------------------

	// -------------------------------------------------------
	// write destination buffer
	m_data.insert(m_data.end(), mainFileName.begin(), mainFileName.end());
	m_data.push_back(0);

	for (NarrowStringMap::iterator i = Output.begin(); i != Output.end();++i)
	{
		const std::string & FileName   = i->first.empty () ? NoName : i->first;
		const std::string & outputData = i->second;

		m_data.insert(m_data.end(), FileName.begin(), FileName.end());
		m_data.push_back(0);
		m_data.insert(m_data.end(), outputData.begin(), outputData.end());
		m_data.push_back(0);
	}
	// -------------------------------------------------------

	/*
	Buffer::iterator bi;
	for (bi=m_data.begin();bi!=m_data.end();++bi)
	{
		Buffer::iterator biNext = bi;
		for (;*biNext!=0 && *biNext!='\n';++biNext);

		int count = biNext-bi;
		assert(count<1024);
		char temp[1024];
		memcpy(temp, &(*bi), count);
		temp[count]=0;
		OutputDebugString(temp);
		OutputDebugString("\n");
		bi=biNext;
	}
	*/
}

// ==============================================================================

UIBaseObject *SerializedObjectBuffer::unserialize()
{
	if (m_data.empty())
	{
		return 0;
	}

	// -------------------------------------------------------
	UINarrowString mainFileName;
	Buffer::iterator dataIter = m_data.begin();

	if (!_extractNextString(mainFileName, dataIter, m_data.end()))
	{
		return 0;
	}

	// -------------------------------------------------------
	Loader loader;
	UINarrowString currentFileName;
	while (_extractNextString(currentFileName, dataIter, m_data.end()))
	{
		UINarrowString &fileData = loader.m_files[currentFileName];
		if (!_extractNextString(fileData, dataIter, m_data.end()))
		{
			return 0;
		}
	}

	if (loader.m_files.empty())
	{
		return 0;
	}

	// -------------------------------------------------------

	UIBaseObject::UIObjectList	 TopLevelObjects;

	if (!loader.LoadFromResource(mainFileName.c_str(), TopLevelObjects, true))
	{
		GetUIOutputStream()->flush ();
		// MessageBox("The file could not be opened, check ui.log for more information", gApplicationName, MB_OK );
		// TODO ERROR
		return false;
	}

	if( TopLevelObjects.size() > 1 )
	{
		// TODO ERROR
		//MessageBox("Error: The file contains more than one root level object", gApplicationName, MB_OK );
		return false;
	}
	
	UIBaseObject * const o = TopLevelObjects.front();

	//UITextStyleManager::GetInstance()->Initialize(static_cast<UIPage *>(o), Loader);
	
	//o->Link ();

	//loader.Lint();

	/*
	if (o->IsA(TUIPage))
	{
		UIManager::gUIManager().SetRootPage( static_cast< UIPage * >( o ) );
	}
	else
	{
		MessageBox("Error: The root level object in the file is not a page", gApplicationName, MB_OK );
		return false;
	}
	*/

	//UIBuilderHistory::install ();

//	if( Loader.ReadOnlyFilesInInput )
//		MessageBox( NULL, "Warning, one or more of the files making up this script are read only.", "UIBuilder", MB_OK );

	/*
	int version = 0;
	if (o->GetPropertyInteger (UIVersion::PropertyVersion, version) && version > UIVersion::ms_version)
	{		
		MessageBox("WARNING: the data you are editing is a newer version than this UIBuilder.", "UIBuilder", MB_OK | MB_ICONWARNING);
	}
	else
		o->SetPropertyInteger (UIVersion::PropertyVersion, UIVersion::ms_version);
	*/

	return o;
}

bool SerializedObjectBuffer::_extractNextString(std::string &o_dest, Buffer::iterator &io_source, const Buffer::iterator &sourceEnd)
{
	o_dest.clear();

	Buffer::iterator i;

	for (i=io_source;i!=sourceEnd && *i!=0;++i);
	if (i==sourceEnd)
	{
		return false;
	}
	// i points to a zero

	o_dest = &(*io_source);

	io_source=++i;

	return true;
}
