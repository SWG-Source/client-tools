//==============================================================================
//
//  My Memory Manager
//
//==============================================================================
//  Copyright (C) DejaTools, LLC.  All rights reserved.
//==============================================================================

#ifndef MY_MEMORY_MGR
#define MY_MEMORY_MGR

// Make sure these macros are not defined at this point.
#undef MyMalloc
#undef MyRealloc
#undef MyFree
#undef new

//------------------------------------------------------------------------------
//
//  Welcome to "MyMemoryMgr"!
//
//  This memory manager is a super thin wrapper around the standard library 
//  heap functions.  It has the following features:
//
//    - Provides replacement functions for malloc, realloc, and free.
//    - Overrides global operators new and delete.
//    - Via a macro definition, allows you to enable or disable source code
//      information availability to the functions.
//    - Includes complete DejaLib heap tracking instrumentation.
//
//------------------------------------------------------------------------------
//
//  Note that this is a very simple memory manager wrapper.  It is intended
//  only to be used for illustrating DejaLib heap tracking instrumentation, 
//  though it does demonstrate techniques for creating a C++ memory manager.
//
//  Creating a proper centralized memory manager in C++ is made more complicated
//  by the presence of things like:
//
//    - Third party libraries which call standard library heap functions 
//      directly without providing callback hooks for heap requests.  (Shame on 
//      such libraries!)
//    - Certain Windows header files which also redefine new.
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Configurable options:
//
//    - ENABLE_MY_MEMORY_MGR - If defined, enables MyMemoryMgr.  If not defined,
//          then all MyMemory manager funtions compile directly into their
//          standard library heap function analogs.
//
//    - ENABLE_SOURCE_CODE_INFO - If defined, allows MyMemoryMgr to note source
//          code file name and line number.  The file/line information is
//          provided to the DejaLib instrumentation.
//
//------------------------------------------------------------------------------

// Comment/uncomment these two macros as needed.
// Alternately define them based on the project configuration.

#define ENABLE_MY_MEMORY_MGR
#define ENABLE_SOURCE_CODE_INFO

//------------------------------------------------------------------------------
//
//  The interface is as follows:
//
//      MyMalloc            - Replacement for malloc.
//      MyRealloc           - Replacement for realloc.
//      MyFree              - Replacement for free.
//
//      operator new        - Replaces default global operator new.
//      operator new []     - Replaces default global operator new [].
//      operator delete     - Replaces default global operator delete.
//      operator delete []  - Replaces default global operator delete [].
//
//------------------------------------------------------------------------------

void* MyMalloc  (                 size_t Size );
void* MyRealloc ( void*  pMemory, size_t Size );
void  MyFree    ( void*  pMemory              );

#ifdef ENABLE_MY_MEMORY_MGR

void* operator new       ( size_t Size );
void* operator new    [] ( size_t Size );

void  operator delete    ( void* p );
void  operator delete [] ( void* p );

#endif

//==============================================================================
//  IMPLEMENTATION DETAILS
//==============================================================================

#include "MyMemoryMgr_Private.h"

//------------------------------------------------------------------------------
#endif // ifdef MY_MEMORY_MGR
//------------------------------------------------------------------------------
