// \addtodoc

#ifndef INC_NXN_ERRORS_H
#define INC_NXN_ERRORS_H

/*! \file		NxNErrors.h
 *
 *  \brief		Collection of all NxN Integrator SDK specific error codes.
 *
 *  \author		Axel Pfeuffer, Helmut Klinger
 *
 *  \version	1.00
 *
 *  \date		2000
 *
	<hr><h2>Error codes</h2>
	The following table contains all possible NxN Integrator SDK specific error codes which can 
	occur during runtime. The define names can be split into 3 Parts: 
	<ul>
		<li><b>NXN_ERR_</b> is just a short cut, that this one is a NxN specific error.
		<li><b>[object]</b> (i.e. INTEGRATOR_) this stands for the object in which the error occured.
		<li><b>[brief description]</b> (i.e. NO_APP_INSTALLED) is a brief description what kind of error occured.
	</ul>
	<table>
		<tr><td><h3><b>Error (define)</b></h3></td><td><h3><b>Description</b></h3></td></tr>
		<tr><td><i>NXN_ERR_INTEGRATOR_NO_APP_INSTALLED					</i></td><td><i>no application to instantiate installed</i></td></tr>
		<tr><td><i>NXN_ERR_INTEGRATOR_CANT_CREATE_INTERNAL_NAMESPACE	</i></td><td><i>internal namespace object could not be created</i></td></tr>
		<tr><td><i>NXN_ERR_INTEGRATOR_COULD_NOT_GET_LAST_WORKSPACE		</i></td><td><i>the last workspace entry from the registry could not be read for some strange reasons!</i></td></tr>
		<tr><td><i>NXN_ERR_INTEGRATOR_COULD_NOT_SET_LAST_WORKSPACE		</i></td><td><i>the last workspace entry from the registry could not be written for some strange reasons!</i></td></tr>
		<tr><td><i>NXN_ERR_INTEGRATOR_A_WORKSPACE_IS_STILL_OPENED		</i></td><td><i>there's still a workspace opened, close this one first!</i></td></tr>
		<tr><td><i>NXN_ERR_INTEGRATOR_NO_WORKSPACE_OPENED				</i></td><td><i>there's no workspace opened or created</i></td></tr>
		<tr><td><i>NXN_ERR_INTEGRATOR_ALREADY_INITIALIZED				</i></td><td><i>the internal intergrator object has been initialized previously</i></td></tr>
		<tr><td><i>NXN_ERR_INTEGRATOR_NOT_INITIALIZED					</i></td><td><i>the internal intergrator object has not been initialized previously</i></td></tr>
		<tr><td><i>NXN_ERR_INTEGRATOR_WRONG_VERSION_OF_DLL_INSTALLED	</i></td><td><i>there's a wrong version of the NxN Integrator SDK dll installed</i></td></tr>
		<tr><td><i>NXN_ERR_INTEGRATOR_NO_USER_SETTINGS_AVAILABLE		</i></td><td><i>the user dependent programs settings could not be restored</i></td></tr>
		<tr><td><i>NXN_ERR_NODE_NO_CURRENTCHILD							</i></td><td><i>no internal current child exists => can happen if the user forgot to call the GetFirstChild() method</i></td></tr>
		<tr><td><i>NXN_ERR_NODE_COULD_NOT_GET_LAST_WORKSPACE			</i></td><td><i>the workspace could not be fetched from the registry</i></td></tr>
		<tr><td><i>NXN_ERR_NODE_COULD_NOT_SET_LAST_WORKSPACE			</i></td><td><i>the workspace could not be set in the registry</i></td></tr>
		<tr><td><i>NXN_ERR_NODE_COULD_NOT_CREATE_CHILD_OBJECT			</i></td><td><i>could not create child object</i></td></tr>
		<tr><td><i>NXN_ERR_NODE_COULD_NOT_GET_INTERNAL_NAMESPACEOBJECT	</i></td><td><i>could not get the pointer to the internal namespace object </i></td></tr>
		<tr><td><i>NXN_ERR_NODE_COULD_NOT_GET_NAMESPACE_TYPE			</i></td><td><i>could not get namespace type </i></td></tr>
		<tr><td><i>NXN_ERR_NODE_DOES_NOT_EXIST_IN_NAMESPACE				</i></td><td><i>the specified node does not exist in the namespace tree of alienbrain </i></td></tr>
		<tr><td><i>NXN_ERR_NODE_EMPTY_STRING_IN_PATHNAME				</i></td><td><i>there is an empty part in the namespace path. i.e. '\Workspace\\MyTest' </i></td></tr>
		<tr><td><i>NXN_ERR_NODE_COULD_NOT_FIND_OBJECT_IN_TREE			</i></td><td><i>could not find the object in the internal tree using the specified namespace path. </i></td></tr>
		<tr><td><i>NXN_ERR_NODE_ADD_AN_OBJECT_TO_INTERNAL_CHILDLIST		</i></td><td><i>could not add an object to the internal child list. </i></td></tr>
		<tr><td><i>NXN_ERR_NODE_COULD_FLUSH_PROPERTIES					</i></td><td><i>could not flush properties of this object. </i></td></tr>
		<tr><td><i>NXN_ERR_NODELIST_COULD_NOT_ADD_NODE					</i></td><td><i>could not add a dbnode object to the list </i></td></tr>
		<tr><td><i>NXN_ERR_NODELIST_COULD_NOT_REMOVE_NODE				</i></td><td><i>could not remove a dbnode object from the list </i></td></tr>
		<tr><td><i>NXN_ERR_NODELIST_OUT_OF_INDEX_RANGE					</i></td><td><i>the specified array index is out of range </i></td></tr>
		<tr><td><i>NXN_ERR_NODELIST_COULD_NOT_FIND_NODE					</i></td><td><i>could not find dbnode object in list </i></td></tr>
		<tr><td><i>NXN_ERR_NODELIST_COULD_SET_PROPERTY					</i></td><td><i>could not set property for complete dbnode list.</i></td></tr>
		<tr><td><i>NXN_ERR_NODELIST_COULD_FLUSH_OBJECTS					</i></td><td><i>could not flush all objects in dbnode list.</i></td></tr>
		<tr><td><i>NXN_ERR_NODELIST_INVALID_OBJECTS						</i></td><td><i>dbnode list contains invalid objects.</i></td></tr>
		<tr><td><i>NXN_ERR_NODELIST_COULD_FLUSH_PROPERTIES				</i></td><td><i>could not flush all properties of all objects in dbnode list.</i></td></tr>
		<tr><td><i>NXN_ERR_NODELIST_LIST_IS_EMPTY						</i></td><td><i>the node list is empty.</i></td></tr>
		<tr><td><i>NXN_ERR_WORKSPACE_COULD_NOT_GET_PROJECTNAME			</i></td><td><i>could not get project name </i></td></tr>
		<tr><td><i>NXN_ERR_WORKSPACE_COULD_NOT_CREATE_PROJECT_OBJECT	</i></td><td><i>could not create a project object </i></td></tr>
		<tr><td><i>NXN_ERR_WORKSPACE_COULD_NOT_FIND_PROJECT				</i></td><td><i>could not find project below workspace</i></td></tr>
		<tr><td><i>NXN_ERR_WORKSPACE_FOUND_NODE_IS_NOT_A_PROJECT		</i></td><td><i>the found node below the workspace is not a project</i></td></tr>
		<tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_LOCALPATH				</i></td><td><i>could not get local path property </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_SERVERPATH				</i></td><td><i>could not get server path property </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_DBPATH					</i></td><td><i>could not get dbpath property </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_KEYWORDS				</i></td><td><i>could not get keywords property </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_WORKINGPATH				</i></td><td><i>could not get working path property </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_COMMENT					</i></td><td><i>could not get comment property </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_MIMETYPE				</i></td><td><i>could not get mime type property </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_HANDLE					</i></td><td><i>could not get handle property </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODE_COULD_NOT_SET_WORKINGPATH				</i></td><td><i>could not set working path property </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODE_COULD_NOT_SET_NAME					</i></td><td><i>could not set name property </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODE_HAS_INVALID_NAME						</i></td><td><i>this node has an invalid name </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODE_NO_PARENT								</i></td><td><i>This object has got no parent. In order to make this method run properly this object needs to have a parent.</i></td></tr>
		<tr><td><i>NXN_ERR_PROJECT_COULD_NOT_GET_USERNAME				</i></td><td><i>could not get log on (=user name) property </i></td></tr>
		<tr><td><i>NXN_ERR_PROJECT_COULD_NOT_GET_HOSTNAME				</i></td><td><i>could not get server name (= host name) property </i></td></tr>
		<tr><td><i>NXN_ERR_PROJECT_NO_FOLDER							</i></td><td><i>the node is not folder </i></td></tr>
		<tr><td><i>NXN_ERR_PROJECT_NO_FILE								</i></td><td><i>the node is not file </i></td></tr>
		<tr><td><i>NXN_ERR_PROJECT_NO_PROJECT_FOUND						</i></td><td><i>could not find a project as one of the parent nodes. </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODELIST_COULD_NOT_ADD_DBNODE				</i></td><td><i>could not add a dbnode object to the list </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODELIST_COULD_NOT_REMOVE_DBNODE			</i></td><td><i>could not remove a dbnode object from the list </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODELIST_OUT_OF_INDEX_RANGE				</i></td><td><i>the specified array index is out of range </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODELIST_COULD_NOT_FIND_DBNODE				</i></td><td><i>could not find dbnode object in list </i></td></tr>
		<tr><td><i>NXN_ERR_DBNODELIST_COULD_SET_PROPERTY				</i></td><td><i>could not set property for complete dbnode list.</i></td></tr>
		<tr><td><i>NXN_ERR_DBNODELIST_COULD_FLUSH_OBJECTS				</i></td><td><i>could not flush all objects in dbnode list.</i></td></tr>
		<tr><td><i>NXN_ERR_DBNODELIST_INVALID_OBJECTS					</i></td><td><i>dbnode list contains invalid objects.</i></td></tr>
		<tr><td><i>NXN_ERR_DBNODELIST_COULD_FLUSH_PROPERTIES			</i></td><td><i>could not flush all properties of all objects in dbnode list.</i></td></tr>
		<tr><td><i>NXN_ERR_DBNODELIST_LIST_IS_EMPTY						</i></td><td><i>the dbnode list is empty.</i></td></tr>
		<tr><td><i>NXN_ERR_FOLDER_NO_FOLDER								</i></td><td><i>the node is not folder </i></td></tr>
		<tr><td><i>NXN_ERR_FOLDER_NO_FILE								</i></td><td><i>the node is not file </i></td></tr>
		<tr><td><i>NXN_ERR_COMMAND_NO_PARAMARRAY						</i></td><td><i>the internal parameter array is missing </i></td></tr>
		<tr><td><i>NXN_ERR_BROWSEFILTER_FILTER_ALREADY_EXISTS			</i></td><td><i>the specified filter command already exists in the list </i></td></tr>
		<tr><td><i>NXN_ERR_BROWSEFILTER_FILTER_DOES_NOT_EXIST			</i></td><td><i>the specified filter command does not exist in the list </i></td></tr>
		<tr><td><i>NXN_ERR_BROWSEFILTER_OUT_OF_RANGE					</i></td><td><i>The specified index is out of the browsefitler's array range.</i></td></tr>
		<tr><td><i>NXN_ERR_GLOBAL_NOT_IMPLEMENTED						</i></td><td><i>this functionality is not implemented! </i></td></tr>
		<tr><td><i>NXN_ERR_GLOBAL_INTERNAL_STRUCTURE_PTR_IS_NULL		</i></td><td><i>The internal data storage structure pointer is NULL! It may possible that you ran out of memory resources.</i></td></tr>
		<tr><td><i>NXN_ERR_GLOBAL_INVALID_PARAM							</i></td><td><i>One of the specified parameters is invalid. Check them for validity.</i></td></tr>
 *	</table>
 */

//---------------------------------------------------------------------------
//	NxNIntegrator SDK - supported languages for errors
//---------------------------------------------------------------------------
#define NXN_ERR_LANGUAGE_ENGLISH							(0)

//---------------------------------------------------------------------------
//	NxNIntegrator SDK - supported languages for errors
//---------------------------------------------------------------------------
#define NXN_ERR_LANGUAGE									NXN_ERR_LANGUAGE_ENGLISH

//---------------------------------------------------------------------------
//	NxNIntegrator SDK - Error codes
//---------------------------------------------------------------------------
#define NXN_ERR_BASE										(0x10000000)			// the error base for all following error codes

/* no application to instantiate installed */
#define NXN_ERR_INTEGRATOR_NO_APP_INSTALLED					(NXN_ERR_BASE+       1) 
/* internal namespace object could not be created */
#define NXN_ERR_INTEGRATOR_CANT_CREATE_INTERNAL_NAMESPACE	(NXN_ERR_BASE+       2) 
/* the last workspace entry from the registry could not be read for some strange reasons! */
#define NXN_ERR_INTEGRATOR_COULD_NOT_GET_LAST_WORKSPACE		(NXN_ERR_BASE+       3) 
/* the last workspace entry from the registry could not be written for some strange reasons! */
#define NXN_ERR_INTEGRATOR_COULD_NOT_SET_LAST_WORKSPACE		(NXN_ERR_BASE+       4) 
/* there's still a workspace opened, close this one first! */
#define NXN_ERR_INTEGRATOR_A_WORKSPACE_IS_STILL_OPENED		(NXN_ERR_BASE+       5) 
/* there's no workspace opened or created */
#define NXN_ERR_INTEGRATOR_NO_WORKSPACE_OPENED				(NXN_ERR_BASE+       6) 
/* the internal intergrator object has been initialized previously */
#define NXN_ERR_INTEGRATOR_ALREADY_INITIALIZED				(NXN_ERR_BASE+       7) 
/* the internal intergrator object has not been initialized previously */
#define NXN_ERR_INTEGRATOR_NOT_INITIALIZED					(NXN_ERR_BASE+       8) 
/* wrong version of nxn integrator sdk dll installed */
#define NXN_ERR_INTEGRATOR_WRONG_VERSION_OF_DLL_INSTALLED	(NXN_ERR_BASE+       9) 
/* the user dependent programs settings could not be restored */
#define NXN_ERR_INTEGRATOR_NO_USER_SETTINGS_AVAILABLE		(NXN_ERR_BASE+      10) 

/* no internal current child exists => can happen if the user forgot to call the GetFirstChild() method */
#define NXN_ERR_NODE_NO_CURRENTCHILD						(NXN_ERR_BASE+      50) 
/* the workspace could not be fetched from the registry */
#define NXN_ERR_NODE_COULD_NOT_GET_LAST_WORKSPACE			(NXN_ERR_BASE+		51) 
/* the workspace could not be set in the registry */
#define NXN_ERR_NODE_COULD_NOT_SET_LAST_WORKSPACE			(NXN_ERR_BASE+		52) 
/* could not create child object */
#define NXN_ERR_NODE_COULD_NOT_CREATE_CHILD_OBJECT			(NXN_ERR_BASE+		53) 
/* could not get the pointer to the internal namespace object */
#define NXN_ERR_NODE_COULD_NOT_GET_INTERNAL_NAMESPACEOBJECT	(NXN_ERR_BASE+		54) 
/* could not get namespace type */
#define NXN_ERR_NODE_COULD_NOT_GET_NAMESPACE_TYPE			(NXN_ERR_BASE+		55) 
/* the specified node does not exist in the namespace tree of alienbrain */
#define NXN_ERR_NODE_DOES_NOT_EXIST_IN_NAMESPACE			(NXN_ERR_BASE+		56) 
/* there is an empty part in the namespace path. i.e. '\Workspace\\MyTest' */
#define NXN_ERR_NODE_EMPTY_STRING_IN_PATHNAME				(NXN_ERR_BASE+		57) 
/* could not find the object in the internal tree using the specified namespace path. */
#define NXN_ERR_NODE_COULD_NOT_FIND_OBJECT_IN_TREE			(NXN_ERR_BASE+		58) 
/* could not add an object to the internal child list. */
#define NXN_ERR_NODE_ADD_AN_OBJECT_TO_INTERNAL_CHILDLIST	(NXN_ERR_BASE+		59) 
/* could not flush all properties of object*/
#define NXN_ERR_NODE_COULD_FLUSH_PROPERTIES					(NXN_ERR_BASE+      60) 

/* could not get project name */
#define NXN_ERR_WORKSPACE_COULD_NOT_GET_PROJECTNAME			(NXN_ERR_BASE+     100) 
/* could not create a project object */
#define NXN_ERR_WORKSPACE_COULD_NOT_CREATE_PROJECT_OBJECT	(NXN_ERR_BASE+     101) 
/* could not find the project below the workspace */
#define NXN_ERR_WORKSPACE_COULD_NOT_FIND_PROJECT			(NXN_ERR_BASE+     102) 
/* the found node below the workspace is not a project */
#define NXN_ERR_WORKSPACE_FOUND_NODE_IS_NOT_A_PROJECT		(NXN_ERR_BASE+     103) 

/* could not get local path property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_LOCALPATH				(NXN_ERR_BASE+     150) 
/* could not get server path property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_SERVERPATH				(NXN_ERR_BASE+     151) 
/* could not get dbpath property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_DBPATH					(NXN_ERR_BASE+     152) 
/* could not get keywords property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_KEYWORDS				(NXN_ERR_BASE+     153) 
/* could not get working path property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_WORKINGPATH			(NXN_ERR_BASE+     154) 
/* could not get comment property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_COMMENT				(NXN_ERR_BASE+     155) 
/* could not get mime type property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_MIMETYPE				(NXN_ERR_BASE+     156) 
/* could not get handle property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_HANDLE					(NXN_ERR_BASE+     157) 
/* could not set working path property */
#define NXN_ERR_DBNODE_COULD_NOT_SET_WORKINGPATH			(NXN_ERR_BASE+     158) 
/* could not set name property */
#define NXN_ERR_DBNODE_COULD_NOT_SET_NAME					(NXN_ERR_BASE+     159) 
/* this node has an invalid name */
#define NXN_ERR_DBNODE_HAS_INVALID_NAME						(NXN_ERR_BASE+	   160) 
/* This object has got no parent. In order to make 
   this method run properly this object needs to 
   have a parent. */
#define NXN_ERR_DBNODE_NO_PARENT							(NXN_ERR_BASE+	   161)


/* could not get log on (=user name) property */
#define NXN_ERR_PROJECT_COULD_NOT_GET_USERNAME				(NXN_ERR_BASE+     200) 
/* could not get server name (= host name) property */
#define NXN_ERR_PROJECT_COULD_NOT_GET_HOSTNAME				(NXN_ERR_BASE+     201) 
/* the node is not folder */
#define NXN_ERR_PROJECT_NO_FOLDER							(NXN_ERR_BASE+     202) 
/* the node is not file */
#define NXN_ERR_PROJECT_NO_FILE								(NXN_ERR_BASE+     203) 
/* could not find a project as one of the parent nodes. */
#define NXN_ERR_PROJECT_NO_PROJECT_FOUND					(NXN_ERR_BASE+     204) 
/* could not find subfolder */
#define NXN_ERR_PROJECT_SUBFOLDER_NOT_FOUND					(NXN_ERR_BASE+     205) 
/* could not find file. */
#define NXN_ERR_PROJECT_FILE_NOT_FOUND						(NXN_ERR_BASE+     206) 
/* the could not create folder */
#define NXN_ERR_PROJECT_FAILED_TO_CREATE_FOLDER				(NXN_ERR_BASE+	   207)

/* could not add a dbnode object to the list */
#define NXN_ERR_DBNODELIST_COULD_NOT_ADD_DBNODE				(NXN_ERR_BASE+     250) 
/* could not remove a dbnode object from the list */
#define NXN_ERR_DBNODELIST_COULD_NOT_REMOVE_DBNODE			(NXN_ERR_BASE+     251) 
/* the specified array index is out of range */
#define NXN_ERR_DBNODELIST_OUT_OF_INDEX_RANGE				(NXN_ERR_BASE+     252) 
/* could not find dbnode object in list */
#define NXN_ERR_DBNODELIST_COULD_NOT_FIND_DBNODE			(NXN_ERR_BASE+     253) 
/* could not set property for complete dbnode list */
#define NXN_ERR_DBNODELIST_COULD_SET_PROPERTY				(NXN_ERR_BASE+     254) 
/* could not flush all objects in dbnode list */
#define NXN_ERR_DBNODELIST_COULD_FLUSH_OBJECTS				(NXN_ERR_BASE+     255) 
/* dbnode list contains invalid objects */
#define NXN_ERR_DBNODELIST_INVALID_OBJECTS					(NXN_ERR_BASE+     256) 
/* could not flush all properties of all objects in dbnode list */
#define NXN_ERR_DBNODELIST_COULD_FLUSH_PROPERTIES			(NXN_ERR_BASE+     257) 
/* the dbnode list is empty */
#define NXN_ERR_DBNODELIST_LIST_IS_EMPTY					(NXN_ERR_BASE+     258) 

/* there are no error messages in CNxNObject needed. */
#define NXN_ERR_OBJECT										(NXN_ERR_BASE+     300) 

/* there are no error messages in CNxNVirtualNode needed. */
#define NXN_ERR_VIRTUALNODE									(NXN_ERR_BASE+     350) 

/* there are no error messages in CNxNFile needed. */
#define NXN_ERR_FILE										(NXN_ERR_BASE+     400) 

/* the node is not folder */
#define NXN_ERR_FOLDER_NO_FOLDER							(NXN_ERR_BASE+     450) 
/* the node is not file */
#define NXN_ERR_FOLDER_NO_FILE								(NXN_ERR_BASE+     451) 
/* the could not create folder */
#define NXN_ERR_FOLDER_FAILED_TO_CREATE_FOLDER				(NXN_ERR_BASE+     452) 
/* could not find subfolder */
#define NXN_ERR_FOLDER_SUBFOLDER_NOT_FOUND					(NXN_ERR_BASE+     453) 
/* could not find file. */
#define NXN_ERR_FOLDER_FILE_NOT_FOUND						(NXN_ERR_BASE+     454) 

/* the internal parameter array is missing */
#define NXN_ERR_COMMAND_NO_PARAMARRAY						(NXN_ERR_BASE+     500) 

/* there are no error messages in CNxNProperty needed. */
#define NXN_ERR_PROPERTY									(NXN_ERR_BASE+     550) 

/* there are no error messages in CNxNPath needed. */
#define NXN_ERR_PATH										(NXN_ERR_BASE+     600) 

/* the specified filter command already exists in the list */
#define NXN_ERR_BROWSEFILTER_FILTER_ALREADY_EXISTS			(NXN_ERR_BASE+     650) 
/* the specified filter command does not exist in the list */
#define NXN_ERR_BROWSEFILTER_FILTER_DOES_NOT_EXIST			(NXN_ERR_BASE+     651)
/* The specified index is out of the browsefitler's array range. */ 
#define NXN_ERR_BROWSEFILTER_OUT_OF_RANGE					(NXN_ERR_BASE+	   652)

/* there are no error messages in CNxNType needed. */
#define NXN_ERR_TYPE										(NXN_ERR_BASE+     700) 

/* there are no error messages in CNxNParam needed. */
#define NXN_ERR_PARAM										(NXN_ERR_BASE+     750) 

/* there are no error messages in CNxNResponse needed. */
#define NXN_ERR_RESPONSE									(NXN_ERR_BASE+     800)

/* there are no error messages in CNxNString needed. */
#define NXN_ERR_STRING										(NXN_ERR_BASE+     850) 

/* could not initialize the xml parser. */
#define NXN_ERR_FINDER_FAILED_TO_INIT_XML_PARSER			(NXN_ERR_BASE+     900) 
/* no root node was specified. */
#define NXN_ERR_FINDER_NO_ROOT_NODE_SPECIFIED				(NXN_ERR_BASE+     901) 
/* could not parse xml output. */
#define NXN_ERR_FINDER_FAILED_TO_PARSE_OUTPUT				(NXN_ERR_BASE+     902) 

/* could not add a node object to the list */
#define NXN_ERR_NODELIST_COULD_NOT_ADD_NODE					(NXN_ERR_BASE+     950) 
/* could not remove a node object from the list */
#define NXN_ERR_NODELIST_COULD_NOT_REMOVE_NODE				(NXN_ERR_BASE+     951) 
/* the specified array index is out of range */				
#define NXN_ERR_NODELIST_OUT_OF_INDEX_RANGE					(NXN_ERR_BASE+     952) 
/* could not find node object in list */					
#define NXN_ERR_NODELIST_COULD_NOT_FIND_NODE				(NXN_ERR_BASE+     953) 
/* could not set property for complete node list */			
#define NXN_ERR_NODELIST_COULD_SET_PROPERTY					(NXN_ERR_BASE+     954) 
/* could not flush all objects in node list */
#define NXN_ERR_NODELIST_COULD_FLUSH_OBJECTS				(NXN_ERR_BASE+     955) 
/* node list contains invalid objects */
#define NXN_ERR_NODELIST_INVALID_OBJECTS					(NXN_ERR_BASE+     956) 
/* could not flush all properties of all objects in node list */
#define NXN_ERR_NODELIST_COULD_FLUSH_PROPERTIES				(NXN_ERR_BASE+     957) 
/* the node list is empty */
#define NXN_ERR_NODELIST_LIST_IS_EMPTY						(NXN_ERR_BASE+     958) 

/* this functionality is not implemented! */
#define NXN_ERR_GLOBAL_NOT_IMPLEMENTED						(NXN_ERR_BASE+    4000) 
/* The internal data storage structure pointer is NULL! 
   It may possible that you ran out of memory resources. */
#define NXN_ERR_GLOBAL_INTERNAL_STRUCTURE_PTR_IS_NULL		(NXN_ERR_BASE+	  4001)
/* One of the specified parameters is invalid. Check them for validity. */
#define NXN_ERR_GLOBAL_INVALID_PARAM						(NXN_ERR_BASE+	  4002)

#endif // INC_NXN_ERRORS_H
