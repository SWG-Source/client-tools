// \addtodoc

#ifndef INC_NXN_ERRORS_H
#define INC_NXN_ERRORS_H

/*! \file       NxNErrors.h
 *
 *  \brief      Collection of all NxN Integrator SDK specific error codes.
 *
 *  \author     Axel Pfeuffer, Helmut Klinger
 *
 *  \version    1.00
 *
 *  \date       2000
 *
    <hr><h2>Error codes</h2>
    The following table contains all possible NxN Integrator SDK specific error codes that can 
    occur during runtime. The names of the error constants consist of three parts each: <br><br>

    \htmlonly
    <table border=0>
        <tr><td><b>NXN_ERR_</b></td><td>prefix that is used for every NxN specific error</td>
        <tr><td><b>[object]</b>(e.g. INTEGRATOR_)</td><td>denotes the object on which the error occurred</td>
        <tr><td><b>[brief description]</b>(e.g. NO_APP_INSTALLED)</td><td> is a brief description what kind of error occurred</td>
    </table>
    \endhtmlonly

    <br>
    \htmlonly
    <table border=0>
        <tr><td><b>Error constant name</b></td><td><b>Description</b></td></tr>
        <tr><td><i>NXN_ERR_INTEGRATOR_NO_APP_INSTALLED                  </i></td><td>no application to instantiate installed</td></tr>
        <tr><td><i>NXN_ERR_INTEGRATOR_CANT_CREATE_INTERNAL_NAMESPACE    </i></td><td>internal namespace object could not be created</td></tr>
        <tr><td><i>NXN_ERR_INTEGRATOR_COULD_NOT_GET_LAST_WORKSPACE      </i></td><td>the last workspace entry could not be read from the registry</td></tr>
        <tr><td><i>NXN_ERR_INTEGRATOR_COULD_NOT_SET_LAST_WORKSPACE      </i></td><td>the last workspace entry could not be written to the registry</td></tr>
        <tr><td><i>NXN_ERR_INTEGRATOR_A_WORKSPACE_IS_STILL_OPENED       </i></td><td>the workspace object is already created</td></tr>
        <tr><td><i>NXN_ERR_INTEGRATOR_NO_WORKSPACE_OPENED               </i></td><td>the workspace object is not created</td></tr>
        <tr><td><i>NXN_ERR_INTEGRATOR_ALREADY_INITIALIZED               </i></td><td>the internal integrator object has already been initialized previously</td></tr>
        <tr><td><i>NXN_ERR_INTEGRATOR_NOT_INITIALIZED                   </i></td><td>the internal integrator object has not been initialized yet</td></tr>
        <tr><td><i>NXN_ERR_INTEGRATOR_WRONG_VERSION_OF_DLL_INSTALLED    </i></td><td>a wrong version of the NxN Integrator SDK DLL is installed</td></tr>
        <tr><td><i>NXN_ERR_INTEGRATOR_NO_USER_SETTINGS_AVAILABLE        </i></td><td>the user dependent program settings could not be restored</td></tr>
        <tr><td><i>NXN_ERR_INTEGRATOR_COULD_NOT_GET_IMAGELIST           </i></td><td>could not get the image list from the namespace</td></tr>
        <tr><td><i>NXN_ERR_NODE_NO_CURRENTCHILD                         </i></td><td>no internal current child exists (happens if the user did not call GetFirstChild())</td></tr>
        <tr><td><i>NXN_ERR_NODE_COULD_NOT_GET_LAST_WORKSPACE            </i></td><td>the workspace could not be fetched from the registry</td></tr>
        <tr><td><i>NXN_ERR_NODE_COULD_NOT_SET_LAST_WORKSPACE            </i></td><td>the workspace could not be set in the registry</td></tr>
        <tr><td><i>NXN_ERR_NODE_COULD_NOT_CREATE_CHILD_OBJECT           </i></td><td>could not create child object</td></tr>
        <tr><td><i>NXN_ERR_NODE_COULD_NOT_GET_INTERNAL_NAMESPACEOBJECT  </i></td><td>could not get the pointer to the internal namespace object </td></tr>
        <tr><td><i>NXN_ERR_NODE_COULD_NOT_GET_NAMESPACE_TYPE            </i></td><td>could not get namespace type </td></tr>
        <tr><td><i>NXN_ERR_NODE_DOES_NOT_EXIST_IN_NAMESPACE             </i></td><td>the specified node does not exist in the namespace tree</td></tr>
        <tr><td><i>NXN_ERR_NODE_EMPTY_STRING_IN_PATHNAME                </i></td><td>there is an empty part in the namespace path, e.g. '\\Workspace\\\MyTest'</td></tr>
        <tr><td><i>NXN_ERR_NODE_COULD_NOT_FIND_OBJECT_IN_TREE           </i></td><td>could not find the object in the internal tree using the specified namespace path</td></tr>
        <tr><td><i>NXN_ERR_NODE_ADD_AN_OBJECT_TO_INTERNAL_CHILDLIST     </i></td><td>could not add an object to the internal child list</td></tr>
        <tr><td><i>NXN_ERR_NODE_COULD_FLUSH_PROPERTIES                  </i></td><td>could not flush the properties of this object</td></tr>
        <tr><td><i>NXN_ERR_NODE_COULD_NOT_CREATE_CHILD_NODE             </i></td><td>could not create internal child node</td></tr>
        <tr><td><i>NXN_ERR_NODE_COULD_NOT_GET_ICON                      </i></td><td>could not get the namespace icon information for a node</td></tr>
        <tr><td><i>NXN_ERR_NODELIST_COULD_NOT_ADD_NODE                  </i></td><td>could not add a CNxNDbNode object to the list</td></tr>
        <tr><td><i>NXN_ERR_NODELIST_COULD_NOT_REMOVE_NODE               </i></td><td>could not remove a CNxNDbNode object from the list</td></tr>
        <tr><td><i>NXN_ERR_NODELIST_OUT_OF_INDEX_RANGE                  </i></td><td>the specified array index is out of bounds</td></tr>
        <tr><td><i>NXN_ERR_NODELIST_COULD_NOT_FIND_NODE                 </i></td><td>could not find CNxNDbNode object in list</td></tr>
    </table><br><br>
    <table>
        <tr><td><b>Error constant name</b></td><td><b>Description</b></td></tr>
        <tr><td><i>NXN_ERR_NODELIST_COULD_SET_PROPERTY                  </i></td><td>could not set property for all objects in node list</td></tr>
        <tr><td><i>NXN_ERR_NODELIST_COULD_FLUSH_OBJECTS                 </i></td><td>could not flush all objects for all nodes in node list</td></tr>
        <tr><td><i>NXN_ERR_NODELIST_INVALID_OBJECTS                     </i></td><td>node list contains invalid objects</td></tr>
        <tr><td><i>NXN_ERR_NODELIST_COULD_FLUSH_PROPERTIES              </i></td><td>could not flush all properties of all objects in node list</td></tr>
        <tr><td><i>NXN_ERR_NODELIST_LIST_IS_EMPTY                       </i></td><td>the node list is empty</td></tr>
        <tr><td><i>NXN_ERR_WORKSPACE_COULD_NOT_GET_PROJECTNAME          </i></td><td>could not get project name</td></tr>
        <tr><td><i>NXN_ERR_WORKSPACE_COULD_NOT_CREATE_PROJECT_OBJECT    </i></td><td>could not create a project object</td></tr>
        <tr><td><i>NXN_ERR_WORKSPACE_COULD_NOT_FIND_PROJECT             </i></td><td>could not find project below workspace</td></tr>
        <tr><td><i>NXN_ERR_WORKSPACE_FOUND_NODE_IS_NOT_A_PROJECT        </i></td><td>the node found below the workspace is not a project</td></tr>
        <tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_LOCALPATH               </i></td><td>could not get local path property </td></tr>
        <tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_SERVERPATH              </i></td><td>could not get server path property </td></tr>
        <tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_DBPATH                  </i></td><td>could not get dbpath property </td></tr>
        <tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_KEYWORDS                </i></td><td>could not get keywords property </td></tr>
        <tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_WORKINGPATH             </i></td><td>could not get working path property </td></tr>
        <tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_COMMENT                 </i></td><td>could not get comment property </td></tr>
        <tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_MIMETYPE                </i></td><td>could not get mime type property </td></tr>
        <tr><td><i>NXN_ERR_DBNODE_COULD_NOT_GET_HANDLE                  </i></td><td>could not get handle property </td></tr>
        <tr><td><i>NXN_ERR_DBNODE_COULD_NOT_SET_WORKINGPATH             </i></td><td>could not set working path property </td></tr>
        <tr><td><i>NXN_ERR_DBNODE_COULD_NOT_SET_NAME                    </i></td><td>could not set name property </td></tr>
        <tr><td><i>NXN_ERR_DBNODE_HAS_INVALID_NAME                      </i></td><td>this node has an invalid name </td></tr>
        <tr><td><i>NXN_ERR_DBNODE_NO_PARENT                             </i></td><td>this object has got no parent. In order to make this method run properly this object needs to have a parent</td></tr>
        <tr><td><i>NXN_ERR_DBNODE_NO_ROOT                               </i></td><td>this object has got no root pointer.</td></tr>
        <tr><td><i>NXN_ERR_PROJECT_COULD_NOT_GET_USERNAME               </i></td><td>could not get logon user name property </td></tr>
        <tr><td><i>NXN_ERR_PROJECT_COULD_NOT_GET_HOSTNAME               </i></td><td>could not get logon server name property </td></tr>
        <tr><td><i>NXN_ERR_PROJECT_NO_FOLDER                            </i></td><td>this node is not folder </td></tr>
    </table><br><br>
    <table>
        <tr><td><b>Error constant name</b></td><td><b>Description</b></td></tr>
        <tr><td><i>NXN_ERR_PROJECT_NO_FILE                              </i></td><td>this node is not file </td></tr>
        <tr><td><i>NXN_ERR_PROJECT_NO_PROJECT_FOUND                     </i></td><td>could not find the project node </td></tr>
        <tr><td><i>NXN_ERR_DBNODELIST_COULD_NOT_ADD_DBNODE              </i></td><td>could not add a CNxNDbNode object to the list </td></tr>
        <tr><td><i>NXN_ERR_DBNODELIST_COULD_NOT_REMOVE_DBNODE           </i></td><td>could not remove a CNxNDbNode object from the list </td></tr>
        <tr><td><i>NXN_ERR_DBNODELIST_OUT_OF_INDEX_RANGE                </i></td><td>the specified array index is out of bounds </td></tr>
        <tr><td><i>NXN_ERR_DBNODELIST_COULD_NOT_FIND_DBNODE             </i></td><td>could not find CNxNDbNode object in list </td></tr>
        <tr><td><i>NXN_ERR_DBNODELIST_COULD_SET_PROPERTY                </i></td><td>could not set property for all objects in dbnode list</td></tr>
        <tr><td><i>NXN_ERR_DBNODELIST_COULD_FLUSH_OBJECTS               </i></td><td>could not flush all objects for all nodes in dbnode list</td></tr>
        <tr><td><i>NXN_ERR_DBNODELIST_INVALID_OBJECTS                   </i></td><td>dbnode list contains invalid objects</td></tr>
        <tr><td><i>NXN_ERR_DBNODELIST_COULD_FLUSH_PROPERTIES            </i></td><td>could not flush all properties of all objects in dbnode list</td></tr>
        <tr><td><i>NXN_ERR_DBNODELIST_LIST_IS_EMPTY                     </i></td><td>the dbnode list is empty</td></tr>
        <tr><td><i>NXN_ERR_FOLDER_NO_FOLDER                             </i></td><td>this node is not folder </td></tr>
        <tr><td><i>NXN_ERR_FOLDER_NO_FILE                               </i></td><td>this node is not file </td></tr>
        <tr><td><i>NXN_ERR_COMMAND_NO_PARAMARRAY                        </i></td><td>the internal parameter array is not initialized </td></tr>
        <tr><td><i>NXN_ERR_BROWSEFILTER_FILTER_ALREADY_EXISTS           </i></td><td>the specified filter command already exists in the list </td></tr>
        <tr><td><i>NXN_ERR_BROWSEFILTER_FILTER_DOES_NOT_EXIST           </i></td><td>the specified filter command does not exist in the list </td></tr>
        <tr><td><i>NXN_ERR_BROWSEFILTER_OUT_OF_RANGE                    </i></td><td>the specified index is out of the browse filter's array bounds</td></tr>
        <tr><td><i>NXN_ERR_GLOBAL_NOT_IMPLEMENTED                       </i></td><td>this functionality is not implemented </td></tr>
        <tr><td><i>NXN_ERR_GLOBAL_INTERNAL_STRUCTURE_PTR_IS_NULL        </i></td><td>the internal data storage structure pointer is NULL! It may possible that you ran out of memory</td></tr>
        <tr><td><i>NXN_ERR_GLOBAL_INVALID_PARAM                         </i></td><td>one of the specified parameters is invalid</td></tr>
    </table>
    \endhtmlonly
 */

//---------------------------------------------------------------------------
//  NxNIntegrator SDK - supported languages for errors
//---------------------------------------------------------------------------
#define NXN_ERR_LANGUAGE_ENGLISH                            (0)

//---------------------------------------------------------------------------
//  NxNIntegrator SDK - supported languages for errors
//---------------------------------------------------------------------------
#define NXN_ERR_LANGUAGE                                    NXN_ERR_LANGUAGE_ENGLISH

//---------------------------------------------------------------------------
//  NxNIntegrator SDK - Error codes
//---------------------------------------------------------------------------
#define NXN_ERR_BASE                                        (0x10000000)            // the error base for all following error codes

/* no application to instantiate installed */
#define NXN_ERR_INTEGRATOR_NO_APP_INSTALLED                 (NXN_ERR_BASE+       1) 
/* internal namespace object could not be created */
#define NXN_ERR_INTEGRATOR_CANT_CREATE_INTERNAL_NAMESPACE   (NXN_ERR_BASE+       2) 
/* the last workspace entry from the registry could not be read for some strange reasons! */
#define NXN_ERR_INTEGRATOR_COULD_NOT_GET_LAST_WORKSPACE     (NXN_ERR_BASE+       3) 
/* the last workspace entry from the registry could not be written for some strange reasons! */
#define NXN_ERR_INTEGRATOR_COULD_NOT_SET_LAST_WORKSPACE     (NXN_ERR_BASE+       4) 
/* there's still a workspace opened, close this one first! */
#define NXN_ERR_INTEGRATOR_A_WORKSPACE_IS_STILL_OPENED      (NXN_ERR_BASE+       5) 
/* there's no workspace opened or created */
#define NXN_ERR_INTEGRATOR_NO_WORKSPACE_OPENED              (NXN_ERR_BASE+       6) 
/* the internal intergrator object has been initialized previously */
#define NXN_ERR_INTEGRATOR_ALREADY_INITIALIZED              (NXN_ERR_BASE+       7) 
/* the internal intergrator object has not been initialized previously */
#define NXN_ERR_INTEGRATOR_NOT_INITIALIZED                  (NXN_ERR_BASE+       8) 
/* wrong version of nxn integrator sdk dll installed */
#define NXN_ERR_INTEGRATOR_WRONG_VERSION_OF_DLL_INSTALLED   (NXN_ERR_BASE+       9) 
/* the user dependent programs settings could not be restored */
#define NXN_ERR_INTEGRATOR_NO_USER_SETTINGS_AVAILABLE       (NXN_ERR_BASE+      10) 
/* the namespace image list could not be retrieved */
#define NXN_ERR_INTEGRATOR_COULD_NOT_GET_IMAGELIST          (NXN_ERR_BASE+      11) 

/* no internal current child exists => can happen if the user forgot to call the GetFirstChild() method */
#define NXN_ERR_NODE_NO_CURRENTCHILD                        (NXN_ERR_BASE+      50) 
/* the workspace could not be fetched from the registry */
#define NXN_ERR_NODE_COULD_NOT_GET_LAST_WORKSPACE           (NXN_ERR_BASE+      51) 
/* the workspace could not be set in the registry */
#define NXN_ERR_NODE_COULD_NOT_SET_LAST_WORKSPACE           (NXN_ERR_BASE+      52) 
/* could not create child object */
#define NXN_ERR_NODE_COULD_NOT_CREATE_CHILD_OBJECT          (NXN_ERR_BASE+      53) 
/* could not get the pointer to the internal namespace object */
#define NXN_ERR_NODE_COULD_NOT_GET_INTERNAL_NAMESPACEOBJECT (NXN_ERR_BASE+      54) 
/* could not get namespace type */
#define NXN_ERR_NODE_COULD_NOT_GET_NAMESPACE_TYPE           (NXN_ERR_BASE+      55) 
/* the specified node does not exist in the namespace tree of alienbrain */
#define NXN_ERR_NODE_DOES_NOT_EXIST_IN_NAMESPACE            (NXN_ERR_BASE+      56) 
/* there is an empty part in the namespace path. i.e. '\Workspace\\MyTest' */
#define NXN_ERR_NODE_EMPTY_STRING_IN_PATHNAME               (NXN_ERR_BASE+      57) 
/* could not find the object in the internal tree using the specified namespace path. */
#define NXN_ERR_NODE_COULD_NOT_FIND_OBJECT_IN_TREE          (NXN_ERR_BASE+      58) 
/* could not add an object to the internal child list. */
#define NXN_ERR_NODE_ADD_AN_OBJECT_TO_INTERNAL_CHILDLIST    (NXN_ERR_BASE+      59) 
/* could not flush all properties of object*/
#define NXN_ERR_NODE_COULD_FLUSH_PROPERTIES                 (NXN_ERR_BASE+      60) 
/* could not create child node */
#define NXN_ERR_NODE_COULD_NOT_CREATE_CHILD_NODE            (NXN_ERR_BASE+      61) 
/* invalid node */
#define NXN_ERR_NODE_INVALID                                (NXN_ERR_BASE+      62) 
/* could not get icon */
#define NXN_ERR_NODE_COULD_NOT_GET_ICON                     (NXN_ERR_BASE+      63) 

/* could not get project name */
#define NXN_ERR_WORKSPACE_COULD_NOT_GET_PROJECTNAME         (NXN_ERR_BASE+     100) 
/* could not create a project object */
#define NXN_ERR_WORKSPACE_COULD_NOT_CREATE_PROJECT_OBJECT   (NXN_ERR_BASE+     101) 
/* could not find the project below the workspace */
#define NXN_ERR_WORKSPACE_COULD_NOT_FIND_PROJECT            (NXN_ERR_BASE+     102) 
/* the found node below the workspace is not a project */
#define NXN_ERR_WORKSPACE_FOUND_NODE_IS_NOT_A_PROJECT       (NXN_ERR_BASE+     103) 

/* could not get local path property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_LOCALPATH              (NXN_ERR_BASE+     150) 
/* could not get server path property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_SERVERPATH             (NXN_ERR_BASE+     151) 
/* could not get dbpath property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_DBPATH                 (NXN_ERR_BASE+     152) 
/* could not get keywords property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_KEYWORDS               (NXN_ERR_BASE+     153) 
/* could not get working path property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_WORKINGPATH            (NXN_ERR_BASE+     154) 
/* could not get comment property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_COMMENT                (NXN_ERR_BASE+     155) 
/* could not get mime type property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_MIMETYPE               (NXN_ERR_BASE+     156) 
/* could not get handle property */
#define NXN_ERR_DBNODE_COULD_NOT_GET_HANDLE                 (NXN_ERR_BASE+     157) 
/* could not set working path property */
#define NXN_ERR_DBNODE_COULD_NOT_SET_WORKINGPATH            (NXN_ERR_BASE+     158) 
/* could not set name property */
#define NXN_ERR_DBNODE_COULD_NOT_SET_NAME                   (NXN_ERR_BASE+     159) 
/* this node has an invalid name */
#define NXN_ERR_DBNODE_HAS_INVALID_NAME                     (NXN_ERR_BASE+     160) 
/* This object has got no parent. In order to make 
   this method run properly this object needs to 
   have a parent. */
#define NXN_ERR_DBNODE_NO_PARENT                            (NXN_ERR_BASE+     161)
#define NXN_ERR_DBNODE_NO_ROOT                              (NXN_ERR_BASE+     162)


/* could not get log on (=user name) property */
#define NXN_ERR_PROJECT_COULD_NOT_GET_USERNAME              (NXN_ERR_BASE+     200) 
/* could not get server name (= host name) property */
#define NXN_ERR_PROJECT_COULD_NOT_GET_HOSTNAME              (NXN_ERR_BASE+     201) 
/* the node is not folder */
#define NXN_ERR_PROJECT_NO_FOLDER                           (NXN_ERR_BASE+     202) 
/* the node is not file */
#define NXN_ERR_PROJECT_NO_FILE                             (NXN_ERR_BASE+     203) 
/* could not find a project as one of the parent nodes. */
#define NXN_ERR_PROJECT_NO_PROJECT_FOUND                    (NXN_ERR_BASE+     204) 
/* could not find subfolder */
#define NXN_ERR_PROJECT_SUBFOLDER_NOT_FOUND                 (NXN_ERR_BASE+     205) 
/* could not find file. */
#define NXN_ERR_PROJECT_FILE_NOT_FOUND                      (NXN_ERR_BASE+     206) 
/* the could not create folder */
#define NXN_ERR_PROJECT_FAILED_TO_CREATE_FOLDER             (NXN_ERR_BASE+     207)

/* could not add a dbnode object to the list */
#define NXN_ERR_DBNODELIST_COULD_NOT_ADD_DBNODE             (NXN_ERR_BASE+     250) 
/* could not remove a dbnode object from the list */
#define NXN_ERR_DBNODELIST_COULD_NOT_REMOVE_DBNODE          (NXN_ERR_BASE+     251) 
/* the specified array index is out of range */
#define NXN_ERR_DBNODELIST_OUT_OF_INDEX_RANGE               (NXN_ERR_BASE+     252) 
/* could not find dbnode object in list */
#define NXN_ERR_DBNODELIST_COULD_NOT_FIND_DBNODE            (NXN_ERR_BASE+     253) 
/* could not set property for complete dbnode list */
#define NXN_ERR_DBNODELIST_COULD_SET_PROPERTY               (NXN_ERR_BASE+     254) 
/* could not flush all objects in dbnode list */
#define NXN_ERR_DBNODELIST_COULD_FLUSH_OBJECTS              (NXN_ERR_BASE+     255) 
/* dbnode list contains invalid objects */
#define NXN_ERR_DBNODELIST_INVALID_OBJECTS                  (NXN_ERR_BASE+     256) 
/* could not flush all properties of all objects in dbnode list */
#define NXN_ERR_DBNODELIST_COULD_FLUSH_PROPERTIES           (NXN_ERR_BASE+     257) 
/* the dbnode list is empty */
#define NXN_ERR_DBNODELIST_LIST_IS_EMPTY                    (NXN_ERR_BASE+     258) 

/* there are no error messages in CNxNObject needed. */
#define NXN_ERR_OBJECT                                      (NXN_ERR_BASE+     300) 

/* there are no error messages in CNxNVirtualNode needed. */
#define NXN_ERR_VIRTUALNODE                                 (NXN_ERR_BASE+     350) 

/* there are no error messages in CNxNFile needed. */
#define NXN_ERR_FILE                                        (NXN_ERR_BASE+     400) 

/* the node is not folder */
#define NXN_ERR_FOLDER_NO_FOLDER                            (NXN_ERR_BASE+     450) 
/* the node is not file */
#define NXN_ERR_FOLDER_NO_FILE                              (NXN_ERR_BASE+     451) 
/* the could not create folder */
#define NXN_ERR_FOLDER_FAILED_TO_CREATE_FOLDER              (NXN_ERR_BASE+     452) 
/* could not find subfolder */
#define NXN_ERR_FOLDER_SUBFOLDER_NOT_FOUND                  (NXN_ERR_BASE+     453) 
/* could not find file. */
#define NXN_ERR_FOLDER_FILE_NOT_FOUND                       (NXN_ERR_BASE+     454) 

/* the internal parameter array is missing */
#define NXN_ERR_COMMAND_NO_PARAMARRAY                       (NXN_ERR_BASE+     500) 

/* there are no error messages in CNxNProperty needed. */
#define NXN_ERR_PROPERTY                                    (NXN_ERR_BASE+     550) 

/* there are no error messages in CNxNPath needed. */
#define NXN_ERR_PATH                                        (NXN_ERR_BASE+     600) 

/* the specified filter command already exists in the list */
#define NXN_ERR_BROWSEFILTER_FILTER_ALREADY_EXISTS          (NXN_ERR_BASE+     650) 
/* the specified filter command does not exist in the list */
#define NXN_ERR_BROWSEFILTER_FILTER_DOES_NOT_EXIST          (NXN_ERR_BASE+     651)
/* The specified index is out of the browsefitler's array range. */ 
#define NXN_ERR_BROWSEFILTER_OUT_OF_RANGE                   (NXN_ERR_BASE+     652)

/* there are no error messages in CNxNType needed. */
#define NXN_ERR_TYPE                                        (NXN_ERR_BASE+     700) 

/* there are no error messages in CNxNParam needed. */
#define NXN_ERR_PARAM                                       (NXN_ERR_BASE+     750) 

/* there are no error messages in CNxNResponse needed. */
#define NXN_ERR_RESPONSE                                    (NXN_ERR_BASE+     800)

/* there are no error messages in CNxNString needed. */
#define NXN_ERR_STRING                                      (NXN_ERR_BASE+     850) 

/* could not initialize the xml parser. */
#define NXN_ERR_FINDER_FAILED_TO_INIT_XML_PARSER            (NXN_ERR_BASE+     900) 
/* no root node was specified. */
#define NXN_ERR_FINDER_NO_ROOT_NODE_SPECIFIED               (NXN_ERR_BASE+     901) 
/* could not parse xml output. */
#define NXN_ERR_FINDER_FAILED_TO_PARSE_OUTPUT               (NXN_ERR_BASE+     902) 

/* could not add a node object to the list */
#define NXN_ERR_NODELIST_COULD_NOT_ADD_NODE                 (NXN_ERR_BASE+     950) 
/* could not remove a node object from the list */
#define NXN_ERR_NODELIST_COULD_NOT_REMOVE_NODE              (NXN_ERR_BASE+     951) 
/* the specified array index is out of range */             
#define NXN_ERR_NODELIST_OUT_OF_INDEX_RANGE                 (NXN_ERR_BASE+     952) 
/* could not find node object in list */                    
#define NXN_ERR_NODELIST_COULD_NOT_FIND_NODE                (NXN_ERR_BASE+     953) 
/* could not set property for complete node list */         
#define NXN_ERR_NODELIST_COULD_SET_PROPERTY                 (NXN_ERR_BASE+     954) 
/* could not flush all objects in node list */
#define NXN_ERR_NODELIST_COULD_FLUSH_OBJECTS                (NXN_ERR_BASE+     955) 
/* node list contains invalid objects */
#define NXN_ERR_NODELIST_INVALID_OBJECTS                    (NXN_ERR_BASE+     956) 
/* could not flush all properties of all objects in node list */
#define NXN_ERR_NODELIST_COULD_FLUSH_PROPERTIES             (NXN_ERR_BASE+     957) 
/* the node list is empty */
#define NXN_ERR_NODELIST_LIST_IS_EMPTY                      (NXN_ERR_BASE+     958) 

/* there are no error messages in CNxNDiskItem needed. */
#define NXN_ERR_DISKITEM                                    (NXN_ERR_BASE+    1100) 

/* there are no error messages in CNxNDiskFolder needed. */
#define NXN_ERR_DISKFOLDER                                  (NXN_ERR_BASE+    1150) 

/* there are no error messages in CNxNDiskFile needed. */
#define NXN_ERR_DISKFILE                                    (NXN_ERR_BASE+    1200) 

/* there are no error messages in CNxNVersionControlItem needed. */
#define NXN_ERR_VERSIONCONTROLITEM                          (NXN_ERR_BASE+    1250) 

/* there are no error messages in CNxNHistory needed. */
#define NXN_ERR_HISTORY                                     (NXN_ERR_BASE+    1300) 

/* there are no error messages in CNxNHistoryVersion needed. */
#define NXN_ERR_HISTORYVERSION                              (NXN_ERR_BASE+    1350) 

/* there are no error messages in CNxNHistoryLabel needed. */
#define NXN_ERR_HISTORYLABEL                                (NXN_ERR_BASE+    1400) 

/* this functionality is not implemented! */
#define NXN_ERR_GLOBAL_NOT_IMPLEMENTED                      (NXN_ERR_BASE+    4000) 
/* The internal data storage structure pointer is NULL! 
   It may possible that you ran out of memory resources. */
#define NXN_ERR_GLOBAL_INTERNAL_STRUCTURE_PTR_IS_NULL       (NXN_ERR_BASE+    4001)
/* One of the specified parameters is invalid. Check them for validity. */
#define NXN_ERR_GLOBAL_INVALID_PARAM                        (NXN_ERR_BASE+    4002)

#endif // INC_NXN_ERRORS_H
