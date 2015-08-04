#ifndef INC_NXN_CLASSLIST_H
#define INC_NXN_CLASSLIST_H

/*!	\file		NxNClassList.h
 *	
 *	\brief		This file contains methods for extending the Integrator SDK classfactory
 *				with new objects.
 *	
 *	\author		Axel Pfeuffer, Helmut Klinger
 *	
 *	\version	1.00
 *	
 *	\date		2000
 *	
 *	\mod
 *		[ap]-05-11-2000 file created.
 *	\endmod
 */


// declare this class as dynamic
#define NXN_DECLARE_DYNCREATE(ClassName)	public:\
												static CNxNObject* NXNINTEGRATORSDK_API_CALL ClassName::_Create() \
												{ return new ClassName; }

// class list declaration
#define DECLARE_INTERNAL_CLASSLIST()		extern NXNINTEGRATORSDK_API CNxNNode* (*pfnCreateUserObject)(const CNxNType& tObjectType);\
											extern NXNINTEGRATORSDK_API CNxNNode* CreateNxNObject(const CNxNType& tObjectType);

// class list start
#define DECLARE_CLASSLIST()					extern NXNINTEGRATORSDK_API CNxNNode* (*pfnCreateUserObject)(const CNxNType& tObjectType);\
											extern CNxNNode* CreateUserObject(CNxNType& tObjectType);\
											class CFunctionInitializer {\
												public:\
													static CFunctionInitializer fi;\
													CFunctionInitializer()\
													{\
														pfnCreateUserObject = CreateUserObject;\
													}\
											};\
											CFunctionInitializer CFunctionInitializer::fi;

// class list begin
#define BEGIN_CLASSLIST()					CNxNNode* CreateUserObject(const CNxNType& tObjectType) {\
												CNxNNode* pObject = NULL;\

// class list entry
#define	CLASS(tClassType, CClassEntry)			if(tClassType.DerivesFrom(tObjectType)) { \
													pObject = new CClassEntry();\
												} else\

// class list end
#define END_CLASSLIST()							{ pObject = NULL; }\
												return pObject;\
											}

#endif // INC_NXN_CLASSLIST_H

