// ======================================================================
//
// DialogAddSkeleton.cpp
// Copyright 2001, 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "FirstViewer.h"
#include "DialogAddSkeleton.h"

#include "clientSkeletalAnimation/BasicSkeletonTemplate.h"
#include "clientSkeletalAnimation/SkeletonTemplate.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/VoidMemberFunction.h"

#include <algorithm>
#include <numeric>
#include <vector>

// ======================================================================
// class DialogAddSkeleton
// ======================================================================

DialogAddSkeleton::DialogAddSkeleton(const SkeletonTemplateVector &skeletonTemplates, CWnd* pParent /*=NULL*/)
:	CDialog(DialogAddSkeleton::IDD, pParent),
	m_filePathEdit(),
	m_joints(),
	m_filePath(),
	m_jointNames(new CrcLowerStringVector),
	m_attachmentName(new CrcLowerString(CrcLowerString::empty))
{
	//{{AFX_DATA_INIT(DialogAddSkeleton)
	m_filePath = _T("");
	//}}AFX_DATA_INIT

	//-- fill joint names with transform names from lowest LOD SkeletonTemplate.
	//   I use the lowest LOD transform names because these transforms are used for 
	//   attachments, and all attachment-point transforms need to exist down to the
	//   lowest LOD skeleton.

	BasicSkeletonTemplateVector  basicSkeletonTemplates;
	fetchLowestLodBasicSkeletonTemplates(skeletonTemplates, basicSkeletonTemplates);

	// get # joints
	const int jointCount = std::accumulate(basicSkeletonTemplates.begin(), basicSkeletonTemplates.end(), 0, BasicSkeletonTemplate::pointerJointCountAccumulator);
	m_jointNames->reserve(static_cast<size_t>(jointCount));

	const BasicSkeletonTemplateVector::const_iterator endIt = basicSkeletonTemplates.end();
	for (BasicSkeletonTemplateVector::const_iterator it = basicSkeletonTemplates.begin(); it != endIt; ++it)
	{
		const BasicSkeletonTemplate &basicSkeletonTemplate = *NON_NULL(*it);

		const int localJointCount = basicSkeletonTemplate.getJointCount();
		for (int i = 0; i < localJointCount; ++i)
		{
			const CrcLowerString *const leakedJointName = new CrcLowerString(basicSkeletonTemplate.getJointName(i).getString());
			m_jointNames->push_back(leakedJointName);
		}
	}

	//-- release local reference to BasicSkeletonTemplate instances
	std::for_each(basicSkeletonTemplates.begin(), basicSkeletonTemplates.end(), VoidMemberFunction(&BasicSkeletonTemplate::release));
}

// ----------------------------------------------------------------------

DialogAddSkeleton::~DialogAddSkeleton()
{
	delete m_jointNames;
	delete m_attachmentName;
}

// ----------------------------------------------------------------------

void DialogAddSkeleton::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogAddSkeleton)
	DDX_Control(pDX, IDC_EDIT_FILE_PATH, m_filePathEdit);
	DDX_Control(pDX, IDC_LIST_JOINTS, m_joints);
	DDX_Text(pDX, IDC_EDIT_FILE_PATH, m_filePath);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		// entering dialog

		//-- fill joint list with joint name pointers
		IGNORE_RETURN(m_joints.InsertString(0, "__NOT ATTACHED__"));
		IGNORE_RETURN(m_joints.SetItemData(0, static_cast<DWORD>(-1)));

		DWORD stringIndex = 0;
		const CrcLowerStringVector::const_iterator itEnd = m_jointNames->end();
		for (CrcLowerStringVector::const_iterator it = m_jointNames->begin(); it != itEnd; ++it, ++stringIndex)
		{
			const CrcLowerString &crcJointName = *NON_NULL(*it);
			const int             itemIndex    = m_joints.AddString(crcJointName.getString());
			IGNORE_RETURN(m_joints.SetItemData(itemIndex, stringIndex));
		}
	}
	else
	{
		// leaving dialog

		//-- get selected item
		const int selectedItemIndex = m_joints.GetCurSel();
		if (selectedItemIndex == LB_ERR)
		{
			m_attachmentName->setString("");
		}
		else
		{
			const int stringIndex = static_cast<int>(m_joints.GetItemData(selectedItemIndex));
			if (stringIndex < 0)
				m_attachmentName->setString("");
			else
			{
				const char *const jointName = (*m_jointNames)[static_cast<size_t>(stringIndex)]->getString();
				m_attachmentName->setString(jointName);
			}
		}
	}
}

// ----------------------------------------------------------------------

BEGIN_MESSAGE_MAP(DialogAddSkeleton, CDialog)
	//{{AFX_MSG_MAP(DialogAddSkeleton)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------

void DialogAddSkeleton::OnButtonBrowse() 
{
	CString  lastFilename;

	//-- retrieve current filename in path edit control
	m_filePathEdit.GetWindowText(lastFilename);
	
	//-- open a file dialog
	CFileDialog dlg(TRUE, ".skt", lastFilename, 0, "Skeleton Templates (*.skt)|*.skt||");

	if (dlg.DoModal() == IDOK)
	{
		//-- install filename into edit
		m_filePathEdit.SetWindowText(dlg.GetPathName());
	}
}

// ======================================================================
// private static member functions
// ======================================================================

void DialogAddSkeleton::fetchLowestLodBasicSkeletonTemplates(const SkeletonTemplateVector &skeletonTemplates, BasicSkeletonTemplateVector &basicSkeletonTemplates)
{
	//-- setup destination vector
	basicSkeletonTemplates.resize(skeletonTemplates.size());

	size_t index = 0;
	const SkeletonTemplateVector::const_iterator endIt = skeletonTemplates.end();
	for (SkeletonTemplateVector::const_iterator it = skeletonTemplates.begin(); it != endIt; ++it, ++index)
	{
		const int lodIndex            = (*it)->getDetailCount() - 1;
		basicSkeletonTemplates[index] = (*it)->fetchBasicSkeletonTemplate(lodIndex);
		NOT_NULL(basicSkeletonTemplates[index]);
	}
}

// ======================================================================
