// ======================================================================
//
// SwgSpaceQuestEditorDoc.cpp
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#include "FirstSwgSpaceQuestEditor.h"
#include "SwgSpaceQuestEditorDoc.h"

#include "Configuration.h"
#include "DialogMissionTemplate.h"
#include "MainFrame.h"
#include "Resource.h"
#include "sharedFile/TreeFile.h"

// ======================================================================

namespace SwgSpaceQuestEditorDocNamespace
{
	bool isInvalidStringId(CString const & stringId)
	{
		if (stringId.IsEmpty())
			return true;

		if (stringId[0] < 'a' || stringId[0] > 'z')
			return true;

		for (int i = 1; i < stringId.GetLength(); ++i)
		{
			if (stringId[i] >= 'a' && stringId[i] <= 'z')
				continue;
			
			if (isdigit(stringId[i]))
				continue;
			
			if (stringId[i] == '_')
				continue;

			return true;
		}

		return false;
	}
}

using namespace SwgSpaceQuestEditorDocNamespace;

// ======================================================================

IMPLEMENT_DYNCREATE(SwgSpaceQuestEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(SwgSpaceQuestEditorDoc, CDocument)
	//{{AFX_MSG_MAP(SwgSpaceQuestEditorDoc)
	ON_COMMAND(ID_BUTTON_P4EDIT, OnButtonP4edit)
	ON_COMMAND(ID_BUTTON_SCAN, OnButtonScan)
	ON_COMMAND(ID_BUTTON_INFORMATION, OnButtonInformation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

SwgSpaceQuestEditorDoc::SwgSpaceQuestEditorDoc() :
	m_missionTemplateType(),
	m_spaceQuest()
{
	// TODO: add one-time construction code here

}

// ----------------------------------------------------------------------

SwgSpaceQuestEditorDoc::~SwgSpaceQuestEditorDoc()
{
}

// ----------------------------------------------------------------------

BOOL SwgSpaceQuestEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	DialogMissionTemplate dlg;
	if (dlg.DoModal() != IDOK)
		return FALSE;

	m_missionTemplateType = dlg.getResult();

	return TRUE;
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void SwgSpaceQuestEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void SwgSpaceQuestEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

// ----------------------------------------------------------------------

BOOL SwgSpaceQuestEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	m_missionTemplateType = Configuration::extractMissionTemplateType(lpszPathName);
	if (!Configuration::isValidMissionTemplateType(m_missionTemplateType))
		return FALSE;

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	if (!m_spaceQuest.load(Configuration::extractRootName(lpszPathName), m_missionTemplateType))
		return FALSE;
	
	return TRUE;
}

// ----------------------------------------------------------------------

BOOL SwgSpaceQuestEditorDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	UpdateAllViews(0, H_aboutToSave);

	if (!m_spaceQuest.save(Configuration::extractRootName(lpszPathName), m_missionTemplateType))
		return FALSE;

	SetModifiedFlag(false);

	safe_cast<MainFrame *>(AfxGetMainWnd())->refreshDirectory();

	return TRUE;
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorDoc::SetModifiedFlag(BOOL const bModified)
{
	CDocument::SetModifiedFlag(bModified);
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorDoc::OnButtonP4edit() 
{
	edit(true);
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorDoc::edit(bool interactive) const
{
	CString pathName = Configuration::extractRootName(GetPathName());
	pathName.MakeLower();

	if (pathName.GetLength() == 0)
	{
		if (interactive)
			MessageBox(0, "Please save the quest before editing/adding to Perforce.", AfxGetApp()->m_pszAppName, MB_OK);
		else
			SQE_CONSOLE_WARNING("Quest must be saved before editing/adding to Perforce");
		return;
	}

	DEBUG_REPORT_LOG(true, ("Perforce add/edit of %s\n", (LPCSTR)pathName));

	//-- Mission Data Table
	{
		CString dataTableName(Configuration::createServerMissionDataTableName(pathName, m_missionTemplateType));
		CONSOLE_EXECUTE("p4 edit " + dataTableName);
		CONSOLE_EXECUTE("p4 add " + dataTableName);

		dataTableName.Replace("/dsrc/", "/data/");
		dataTableName.Replace(".tab", ".iff");	
		CONSOLE_EXECUTE("p4 edit " + dataTableName);
		CONSOLE_EXECUTE("p4 add " + dataTableName);
	}

	//-- String Table
	{
		CString sharedStringTableName(Configuration::createSharedStringTableName(pathName, m_missionTemplateType));
		CONSOLE_EXECUTE("p4 edit " + sharedStringTableName);
		CONSOLE_EXECUTE("p4 add " + sharedStringTableName);
	}

	Configuration::MissionTemplate const * const missionTemplate = Configuration::getMissionTemplate(m_missionTemplateType);
	if (missionTemplate && missionTemplate->getNumberOfQuestStringSuffixTemplates() > 0)
	{
		//-- Quest Task Data Table
		{
			CString dataTableName(Configuration::createSharedQuestTaskDataTableName(pathName, m_missionTemplateType));
			CONSOLE_EXECUTE("p4 edit " + dataTableName);
			CONSOLE_EXECUTE("p4 add " + dataTableName);

			dataTableName.Replace("/dsrc/", "/data/");
			dataTableName.Replace(".tab", ".iff");	
			CONSOLE_EXECUTE("p4 edit " + dataTableName);
			CONSOLE_EXECUTE("p4 add " + dataTableName);
		}

		//-- Quest List Data Table
		{
			CString dataTableName(Configuration::createSharedQuestListDataTableName(pathName, m_missionTemplateType));
			CONSOLE_EXECUTE("p4 edit " + dataTableName);
			CONSOLE_EXECUTE("p4 add " + dataTableName);

			dataTableName.Replace("/dsrc/", "/data/");
			dataTableName.Replace(".tab", ".iff");	
			CONSOLE_EXECUTE("p4 edit " + dataTableName);
			CONSOLE_EXECUTE("p4 add " + dataTableName);
		}
	}
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorDoc::OnButtonScan() 
{
	scan(true);
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorDoc::scan(bool const showHeader)
{
	typedef std::vector<CString> StringList;
	StringList warningList;

	Configuration::MissionTemplate const * const missionTemplate = Configuration::getMissionTemplate(m_missionTemplateType);
	if (missionTemplate)
	{
		{
			CString spaceZone;

			{
				//-- Search for spaceZone
				for (int i = 0; i < missionTemplate->getNumberOfPropertyTemplates(); ++i)
				{
					Configuration::MissionTemplate::PropertyTemplate const & propertyTemplate = missionTemplate->getPropertyTemplate(i);
					if (propertyTemplate.m_propertyType == Configuration::MissionTemplate::PropertyTemplate::PT_spaceZone)
					{
						if (!spaceZone.IsEmpty())
							MessageBox(0, "EDITOR CONFIGURATION ERROR: Found more than one property of type PT_spaceZone for mission template " + m_missionTemplateType + ".  Revisit error checking system because asommers assumes there is only one.", AfxGetApp()->m_pszAppName, MB_OK);

						spaceZone = m_spaceQuest.getProperty(propertyTemplate.m_columnName);
					}
				}
			}

			if (spaceZone.IsEmpty())
				MessageBox(0, "ERROR(2): see asommers", AfxGetApp()->m_pszAppName, MB_OK);

			//-- Validate all properties is a valid quest
			for (int i = 0; i < missionTemplate->getNumberOfPropertyTemplates(); ++i)
			{
				Configuration::MissionTemplate::PropertyTemplate const & propertyTemplate = missionTemplate->getPropertyTemplate(i);
				if (propertyTemplate.m_required && m_spaceQuest.getProperty(propertyTemplate.m_columnName).IsEmpty())
					warningList.push_back("ERROR: property [" + propertyTemplate.m_columnName + "] is required by the mission template, but is empty\n");

				switch (propertyTemplate.m_propertyType)
				{
				case Configuration::MissionTemplate::PropertyTemplate::PT_spaceZone:
					{
						if (!Configuration::isValidSpaceZone(m_spaceQuest.getProperty(propertyTemplate.m_columnName)))
							warningList.push_back("ERROR: property [" + propertyTemplate.m_columnName + "] is not a valid space zone\n");
					}
					break;

				case Configuration::MissionTemplate::PropertyTemplate::PT_questName:
					{
						//-- Make sure the quest name points to a valid file
						StringList stringList;
						Configuration::unpackString(m_spaceQuest.getProperty(propertyTemplate.m_columnName), stringList, ':');
						
						if (stringList.size() == 2)
						{
							CString dataTableName = Configuration::getServerMissionDataTablePath() + '/' + Configuration::getSpaceQuestDirectory() + '/' + stringList[0] + '/' + stringList[1] + ".tab";

							if (!TreeFile::exists(dataTableName))
								warningList.push_back("ERROR: property [" + propertyTemplate.m_columnName + "] points to mission template " + dataTableName + " which does not exist\n");

							dataTableName.Replace("/dsrc/", "/data/");
							dataTableName.Replace(".tab", ".iff");

							if (!TreeFile::exists(dataTableName))
								warningList.push_back("ERROR: property [" + propertyTemplate.m_columnName + "] points to mission template " + dataTableName + " which does not exist\n");
						}
					}
					break;

				case Configuration::MissionTemplate::PropertyTemplate::PT_objectTemplate:
					{
						//-- Make sure the object template name points to a valid file
						CString const objectTemplateName = m_spaceQuest.getProperty(propertyTemplate.m_columnName);
						if (!objectTemplateName.IsEmpty())
						{
							CString fullPathName = Configuration::getServerMissionDataTablePath() + "/../" + objectTemplateName;
							fullPathName.Replace("dsrc", "data");

							if (!TreeFile::exists(fullPathName))
								warningList.push_back("ERROR: property [" + propertyTemplate.m_columnName + "] points to object template " + objectTemplateName + " which does not exist\n");
							else
								if (!propertyTemplate.m_propertyData.IsEmpty() && objectTemplateName.Find(propertyTemplate.m_propertyData) != 0)
									warningList.push_back("ERROR: property [" + propertyTemplate.m_columnName + "] points to object template " + objectTemplateName + " which does not match the desired prefix [" + propertyTemplate.m_propertyData + "]\n");
						}
					}
					break;

				case Configuration::MissionTemplate::PropertyTemplate::PT_spaceFaction:
					{
						//-- Make sure this is in the list of space factions
						CString const & spaceFaction = m_spaceQuest.getProperty(propertyTemplate.m_columnName);

						if (!spaceFaction.IsEmpty() && !Configuration::isValidSpaceFaction(spaceFaction))
							warningList.push_back("ERROR: property [" + propertyTemplate.m_columnName + "] points invalid space faction " + spaceFaction + "\n");
					}
					break;

				case Configuration::MissionTemplate::PropertyTemplate::PT_spaceMobile:
					{
						//-- Make sure this is in the list of space mobiles
						CString const & spaceMobile = m_spaceQuest.getProperty(propertyTemplate.m_columnName);

						if (!spaceMobile.IsEmpty() && !Configuration::isValidSpaceMobile(spaceMobile))
							warningList.push_back("ERROR: property [" + propertyTemplate.m_columnName + "] points invalid space mobile " + spaceMobile + "\n");
					}
					break;

				case Configuration::MissionTemplate::PropertyTemplate::PT_spaceMobileList:
					{
						StringList spaceMobileList;
						Configuration::unpackString(m_spaceQuest.getProperty(propertyTemplate.m_columnName), spaceMobileList, '|');

						for (size_t i = 0; i < spaceMobileList.size(); ++i)
						{
							//-- Make sure this is in the list of space mobiles
							CString const & spaceMobile = spaceMobileList[i];

							if (!Configuration::isValidSpaceMobile(spaceMobile))
								warningList.push_back("ERROR: property [" + propertyTemplate.m_columnName + "] points invalid space mobile " + spaceMobile + "\n");
						}
					}
					break;

				case Configuration::MissionTemplate::PropertyTemplate::PT_navPoint:
					{
						//-- Make sure this is in the list of nav points
						CString const & navPoint = m_spaceQuest.getProperty(propertyTemplate.m_columnName);

						if (!navPoint.IsEmpty() && !Configuration::isValidNavPoint(spaceZone, navPoint, missionTemplate->getAllowNavPointsInMultipleZones()))
							warningList.push_back("ERROR: property [" + propertyTemplate.m_columnName + "] points invalid nav point " + (!navPoint.IsEmpty() ? navPoint : "<blank>") + " for space zone " + (!spaceZone.IsEmpty() ? spaceZone : "<blank>") + "\n");
					}
					break;

				case Configuration::MissionTemplate::PropertyTemplate::PT_navPointList:
					{
						StringList navPointList;
						Configuration::unpackString(m_spaceQuest.getProperty(propertyTemplate.m_columnName), navPointList, '|');

						for (size_t i = 0; i < navPointList.size(); ++i)
						{
							//-- Make sure this is in the list of nav points
							CString const & navPoint = navPointList[i];

							if (!Configuration::isValidNavPoint(spaceZone, navPoint, missionTemplate->getAllowNavPointsInMultipleZones()))
								warningList.push_back("ERROR: property [" + propertyTemplate.m_columnName + "] points invalid nav point " + navPoint + " for space zone " + (!spaceZone.IsEmpty() ? spaceZone : "<blank>") + "\n");
						}
					}
					break;

				case Configuration::MissionTemplate::PropertyTemplate::PT_spawner:
					{
						//-- Make sure this is in the list of spawners
						CString const & spawner = m_spaceQuest.getProperty(propertyTemplate.m_columnName);

						if (!spawner.IsEmpty() && !Configuration::isValidSpawner(spaceZone, spawner))
							warningList.push_back("ERROR: property [" + propertyTemplate.m_columnName + "] points invalid spawner " + spawner + " for space zone " + spaceZone + "\n");
					}
					break;

				case Configuration::MissionTemplate::PropertyTemplate::PT_spawnerList:
					{
						StringList spawnerList;
						Configuration::unpackString(m_spaceQuest.getProperty(propertyTemplate.m_columnName), spawnerList, '|');

						for (size_t i = 0; i < spawnerList.size(); ++i)
						{
							//-- Make sure this is in the list of spawners
							CString const & spawner = spawnerList[i];

							if (!Configuration::isValidSpawner(spaceZone, spawner))
								warningList.push_back("ERROR: property [" + propertyTemplate.m_columnName + "] points invalid spawner " + spawner + " for space zone " + spaceZone + "\n");
						}
					}
					break;

				case Configuration::MissionTemplate::PropertyTemplate::PT_cargo:
					{
						CString spaceMobilePropertyName;

						CString const & cargo = m_spaceQuest.getProperty(propertyTemplate.m_columnName);
						if (!cargo.IsEmpty())
						{
							bool valid = false;

							//-- Search for the specified property template name of m_propertyData
							Configuration::MissionTemplate::PropertyTemplate const * const spaceMobilePropertyTemplate = missionTemplate->getPropertyTemplate(propertyTemplate.m_propertyData);
							if (spaceMobilePropertyTemplate)
							{
								spaceMobilePropertyName = m_spaceQuest.getProperty(spaceMobilePropertyTemplate->m_columnName);

								StringList spaceMobileList;
								Configuration::unpackString(spaceMobilePropertyName, spaceMobileList, '|');

								for (StringList::iterator iter = spaceMobileList.begin(); iter != spaceMobileList.end(); ++iter)
								{
									CString const & spaceMobile = *iter;

									if (Configuration::isValidSpaceMobile(spaceMobile))
									{
										CString const cargoName = Configuration::getSpaceMobileCargoName(spaceMobile);
										if (!cargoName.IsEmpty())
										{
											StringList const * const stringList = Configuration::getCargo(cargoName);
											if (stringList && std::find(stringList->begin(), stringList->end(), cargo) != stringList->end())
											{
												valid = true;
												break;
											}
										}
									}
								}
							}

							if (!valid)
								warningList.push_back("ERROR: cargo " + (!cargo.IsEmpty() ? cargo : "<None>") + " is not valid for space mobile " + (!spaceMobilePropertyName.IsEmpty() ? spaceMobilePropertyName : "<None>") + "\n");
						}
					}
					break;

				case Configuration::MissionTemplate::PropertyTemplate::PT_integer:
				case Configuration::MissionTemplate::PropertyTemplate::PT_bool:
				case Configuration::MissionTemplate::PropertyTemplate::PT_enumList:
				case Configuration::MissionTemplate::PropertyTemplate::PT_string:
				case Configuration::MissionTemplate::PropertyTemplate::PT_notImplemented:
					break;

				default:
					warningList.push_back("PropertyTemplate " + propertyTemplate.m_columnName + " was not scanned -- see asommers\n");
					break;
				}
			}
		}

		{
			//-- Verify that all string ids are specified
			for (int i = 0; i < missionTemplate->getNumberOfStringTemplates(); ++i)
			{
				Configuration::MissionTemplate::StringTemplate const & stringTemplate = missionTemplate->getStringTemplate(i);
				if (stringTemplate.m_required && m_spaceQuest.getString(stringTemplate.m_stringId).IsEmpty())
					warningList.push_back("WARNING: string [" + stringTemplate.m_stringId + "] is required by the mission template, but is empty\n");
			}
		}
	}
	else
		warningList.push_back("ERROR: " + m_missionTemplateType + " is not a valid mission template.\n");

	if (showHeader || !warningList.empty())
	{
		SQE_CONSOLE_WARNING("---------- Scanning " + GetPathName() + " ----------\r\n");

		for (uint i = 0; i < warningList.size(); ++i)
			SQE_CONSOLE_WARNING(warningList[i]);
		
		SQE_CONSOLE_WARNING("Scan complete.\n");
	}
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorDoc::save()
{
	OnSaveDocument(GetPathName());
}

// ----------------------------------------------------------------------

CString const & SwgSpaceQuestEditorDoc::getMissionTemplateType() const
{
	return m_missionTemplateType;
}

// ----------------------------------------------------------------------

void SwgSpaceQuestEditorDoc::OnButtonInformation() 
{
	CString const & information = Configuration::getMissionTemplate(m_missionTemplateType)->getInformation();
	if (!information.IsEmpty())
		MessageBox(0, information, m_missionTemplateType, MB_ICONINFORMATION | MB_OK);
	else
		MessageBox(0, "Information has not been defined for mission template " + m_missionTemplateType, 0, MB_ICONWARNING | MB_OK);
}

// ======================================================================


