========================================================================
       DEVELOPER STUDIO ADD-IN : hash_plugin
========================================================================


The Add-in Wizard has created this hash_plugin DLL for you.  This DLL not only
demonstrates the basics of creating a Developer Studio add-in, but it is also
a starting point for writing your own add-in.

An add-in mainly does two things.
	(1) It adds commands to Developer Studio, which can then be tied
	    to keystrokes or toolbar buttons by the user or programmatically
		by the add-in.
	(2) It responds to events fired by Developer Studio.
In both cases, the add-in code has access to the full Developer Studio
Automation Object Model, and may manipulate those objects to affect the
behavior of Developer Studio.

This file contains a summary of what you will find in each of the files that
make up your hash_plugin DLL.


hash_plugin.h
	This is the main header file for the DLL.  It declares the
	CHash_pluginApp class.

hash_plugin.cpp
	This is the main DLL source file.  It contains the class CHash_pluginApp.
	It also contains the OLE entry points required of inproc servers.

hash_plugin.dsp
    This file (the project file) contains information at the project level and
    is used to build a single project or subproject. Other users can share the
    project (.dsp) file, but they should export the makefiles locally.

hash_plugin.odl
    This file contains the Object Description Language source code for the
    type library of your DLL.

hash_plugin.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  This file can be directly edited in Microsoft
	Developer Studio.

res\hash_plugin.rc2
    This file contains resources that are not edited by Microsoft 
	Developer Studio.  You should place all resources not
	editable by the resource editor in this file.

hash_plugin.def
    This file contains information about the DLL that must be
    provided to run with Microsoft Windows.  It defines parameters
    such as the name and description of the DLL.  It also exports
	functions from the DLL.

hash_plugin.clw
    This file contains information used by ClassWizard to edit existing
    classes or add new classes.  ClassWizard also uses this file to store
    information needed to create and edit message maps and dialog data
    maps and to create prototype member functions.

/////////////////////////////////////////////////////////////////////////////
Add-in-specific files:

DSAddIn.cpp, DSAddIn.h
    These files contain the CDSAddIn class, which implements the
    IDSAddIn interface.  This interface contains handlers
    for connecting and disconnecting the add-in.

Commands.cpp, Commands.h
    These files contain the CCommands class, which implements your
    command dispatch interface.  This interface contains one method
    for each command you add to Developer Studio.  It already implements
    a sample command (Hash_pluginCommand) which displays a message
    box when it is invoked.  You will probably want to rename and modify this
	command, as well as add your own commands.


/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named hash_plugin.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Developer Studio reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
