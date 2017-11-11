/*****************************************************************************
 *
 * dPVS
 * -----------------------------------------
 *
 * (C) 1999-2001 Hybrid Holding, Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Hybrid Holding, and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Hybrid Holding and legal action against the party in breach.
 *
 * Description: 	Statistics / profiling code
 *
 * $Archive: /dpvs/implementation/sources/dpvsStatistics.cpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 31.07.01 12:14 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#include "dpvsStatistics.hpp"
#include "dpvsDatabase.hpp"
#include "dpvsMemory.hpp"
#include "dpvsInstanceCount.hpp"
#include "dpvsDebug.hpp"

#include <cstring>

using namespace DPVS;

//------------------------------------------------------------------------
// Static variables
//------------------------------------------------------------------------

Statistics::Stat	Statistics::s_statistics;
Statistics::Stat	Statistics::s_pushedStatistics;
int					Statistics::s_statisticsPosition = 0;

// NB: THIS LIST _MUST_ BE UPDATED WHENEVER NEW STATISTICS ARE ADDED!!!!!!!!!!!!
const char*			Statistics::s_statisticNames[Library::STAT_MAX] =
{
    "STAT_SILHOUETTECACHEINSERTIONS",			
    "STAT_SILHOUETTECACHEREMOVALS",           
    "STAT_SILHOUETTECACHEQUERIES",            
    "STAT_SILHOUETTECACHEQUERYITERS",         
    "STAT_SILHOUETTECACHEHITS",               
    "STAT_SILHOUETTECACHEMEMORYUSED",         
	"STAT_SILHOUETTECACHECONGESTED",			
    "STAT_OBJECTMATRIXUPDATES",               
    "STAT_OBJECTCAMERAMATRIXUPDATES",         
	"STAT_OBJECTVPTFAILED",					
    "STAT_SPHEREBOUNDSQUERIED",               
    "STAT_CAMERARESOLVEVISIBILITYCALLS",      
    "STAT_CAMERAVISIBILITYCALLBACKS",         
    "STAT_HZBUFFERLEVELUPDATES",              
    "STAT_WRITEQUEUEPOINTQUERIES",            
    "STAT_WRITEQUEUESILHOUETTEQUERIES",       
    "STAT_WRITEQUEUEOBJECTQUERIES",           
	"STAT_WRITEQUEUEOCCLUDEESDEPTHREJECTED",	
    "STAT_WRITEQUEUEWRITESREQUESTED",         
    "STAT_WRITEQUEUEWRITESPERFORMED",         
    "STAT_WRITEQUEUEWRITESPOSTPONED",         
    "STAT_WRITEQUEUEWRITESDISCARDED",         
    "STAT_WRITEQUEUEOVERFLOW",                
    "STAT_WRITEQUEUEOVERFLOWEVERYTHING",      
    "STAT_WRITEQUEUEDEPTHWRITES",             
    "STAT_WRITEQUEUEDEPTHCLEARS",             
    "STAT_WRITEQUEUEFLUSHES",                 
    "STAT_WRITEQUEUEBUCKETFLUSHWORK",         
    "STAT_WRITEQUEUEHIDDENOCCLUDERS",         
    "STAT_WRITEQUEUEFRONTCLIPPINGOCCLUDERSTESTED",
    "STAT_WRITEQUEUEFRONTCLIPPINGOCCLUDERSUSED",  
    "STAT_OCCLUSIONSILHOUETTEQUERIES",		
    "STAT_OCCLUSIONPOINTQUERIES",				
    "STAT_OCCLUSIONRECTANGLEQUERIES",			
    "STAT_OCCLUSIONACCURATEPOINTQUERIES",     
    "STAT_OCCLUSIONACCURATEBLOCKQUERIES",     
    "STAT_OCCLUSIONACCURATEPOINTUSEFUL",      
    "STAT_OCCLUSIONACCURATEBLOCKUSEFUL",      
    "STAT_OCCLUSIONBUFFERBUCKETSCLEARED",     
    "STAT_OCCLUSIONBUFFERBUCKETSPROCESSED",   
	"STAT_OCCLUSIONBUFFEREDGESRASTERIZED",	
	"STAT_OCCLUSIONBUFFEREDGESTESTED",		
	"STAT_OCCLUSIONBUFFEREDGESCLIPPING",		
	"STAT_OCCLUSIONBUFFEREDGESSINGLEBUCKET",	
	"STAT_OCCLUSIONBUFFEREDGEPIXELS",			
	"STAT_OCCLUSIONBUFFERTESTEDGEPIXELS",		
	"STAT_OCCLUSIONBUFFEREXACTZTESTS",		
    "STAT_DATABASETRAVERSALS",                
    "STAT_DATABASENODESINSERTED",             
    "STAT_DATABASENODESREMOVED",              
    "STAT_DATABASENODEDIRTYUPDATES",          
    "STAT_DATABASENODESTRAVERSED",            
    "STAT_DATABASELEAFNODESTRAVERSED",        
    "STAT_DATABASENODESSKIPPED",              
    "STAT_DATABASENODESVFTESTED",             
    "STAT_DATABASENODESVFCULLED",             
    "STAT_DATABASENODESVFCULLED2",            
    "STAT_DATABASENODESOCCLUSIONTESTED",      
    "STAT_DATABASENODESOCCLUSIONCULLED",      
	"STAT_DATABASENODEVPTFAILED",				
	"STAT_DATABASENODEVPTSUCCEEDED",			
	"STAT_DATABASENODEVPTUPDATED",			
    "STAT_DATABASENODESVISIBLE",              
    "STAT_DATABASENODESPLITS",                
    "STAT_DATABASENODENEWROOTS",              
    "STAT_DATABASENODESFRONTCLIPPING",        
    "STAT_DATABASEOBSTATUSCHANGES",           
    "STAT_DATABASEOBSINSERTED",               
    "STAT_DATABASEOBSREMOVED",                
    "STAT_DATABASEOBSUPDATED",                
    "STAT_DATABASEOBSUPDATEPROCESSED",        
    "STAT_DATABASEOBINSTANCESTRAVERSED",      
    "STAT_DATABASEOBSTRAVERSED",              
    "STAT_DATABASEOBSVFTESTED",               
    "STAT_DATABASEOBSVFCULLED",               
    "STAT_DATABASEOBSVFEXACTTESTED",          
    "STAT_DATABASEOBSVFEXACTCULLED",          
    "STAT_DATABASEOBSVISIBLE",                
    "STAT_DATABASEOBSVISIBILITYPARENTCULLED", 
    "STAT_DATABASEOBSOCCLUSIONSKIPPED",       
    "STAT_DATABASEOBSOCCLUSIONTESTED",        
    "STAT_DATABASEOBSOCCLUSIONCULLED",        
    "STAT_DATABASEOBSBACKFACETESTED",			
    "STAT_DATABASEOBSBACKFACECULLED",			
    "STAT_DATABASEOBNEWVISIBLEPOINTS",        
    "STAT_DATABASEINSTANCESINSERTED",         
    "STAT_DATABASEINSTANCESREMOVED",          
    "STAT_DATABASEINSTANCESMOVED",            
    "STAT_ROIACTIVE",                         
    "STAT_ROISTATECHANGES",                   
    "STAT_ROIOBJECTOVERLAPTESTS",             
    "STAT_ROIOBJECTOVERLAPS",                 
    "STAT_HOAX0",                             
    "STAT_HOAX1",                             
    "STAT_HOAX2",                             
    "STAT_HOAX3",                             
    "STAT_HOAX4",                             
    "STAT_HOAX5",                             
    "STAT_HOAX6",                             
    "STAT_HOAX7",                             
	"STAT_MODELRECTANGLESQUERIED",			
	"STAT_MODELEXACTRECTANGLESQUERIED",		
	"STAT_MODELTESTSILHOUETTESQUERIED",		
	"STAT_MODELTESTSILHOUETTESCLIPPED",		
	"STAT_MODELTESTSILHOUETTESREJECTED",		
	"STAT_MODELWRITESILHOUETTESQUERIED",		
	"STAT_MODELTOPOLOGYCOMPUTED",				
    "STAT_MODELDERIVEDMEMORYUSED",			
	"STAT_MODELDERIVED",						
	"STAT_TIME",                              
	"STAT_MEMORYUSED",						
	"STAT_MEMORYCURRENTALLOCATIONS",			
	"STAT_MEMORYOPERATIONS",					
	"STAT_MEMORYEXTERNALOPERATIONS",			
	"STAT_LIVECAMERAS",						
	"STAT_LIVECELLS",							
	"STAT_LIVEMODELS",						
	"STAT_LIVEOBJECTS",						
	"STAT_LIVEPHYSICALPORTALS",				
	"STAT_LIVEREGIONSOFINFLUENCE",			
	"STAT_LIVEVIRTUALPORTALS",				
	"STAT_LIVENODES",							
	"STAT_LIVEINSTANCES"						
};

/*****************************************************************************
 *
 * Function:		DPVS::Statistics::Stat::reset()
 *
 * Description:		Resets all statistics counters
 *
 *****************************************************************************/

void Statistics::Stat::reset (void)							
{ 
	::memset (this,0,sizeof(Stat)); 
	m_time = Debug::getServices()->getTime(); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Statistics::Stat::Stat()
 *
 * Description:		Initializes all statistics to zero
 *
 *****************************************************************************/

Statistics::Stat::Stat (void) 
{ 
	::memset (this,0,sizeof(Stat)); 
}

/*****************************************************************************
 *
 * Function:		DPVS::Statistics::getStatistic()
 *
 * Description:		Returns a statistic counter
 *
 * Parameters:		s = statistics enumeration
 *
 * Returns:			Corresponding statistics value
 *
 *****************************************************************************/

float Statistics::getStatistic (Library::Statistic s)					
{ 
	//--------------------------------------------------------------------
	// Basic statistics
	//--------------------------------------------------------------------

	if (s >= 0 && s < Library::STAT_MAX)
	{
		switch (s)
		{
			case Library::STAT_TIME:					return (float)s_statistics.m_time; 
			case Library::STAT_MEMORYUSED:				return (float)Memory::getMemoryReserved();
			case Library::STAT_MEMORYCURRENTALLOCATIONS:return (float)Memory::getAllocationCount();
			case Library::STAT_LIVECAMERAS:				return (float)InstanceCount::getInstanceCount(InstanceCount::CAMERA);
			case Library::STAT_LIVECELLS:				return (float)InstanceCount::getInstanceCount(InstanceCount::CELL);
			case Library::STAT_LIVEMODELS:				return (float)InstanceCount::getInstanceCount(InstanceCount::MODEL);
			case Library::STAT_LIVEOBJECTS:				return (float)InstanceCount::getInstanceCount(InstanceCount::OBJECT);
			case Library::STAT_LIVEREGIONSOFINFLUENCE:	return (float)InstanceCount::getInstanceCount(InstanceCount::REGIONOFINFLUENCE);
			case Library::STAT_LIVEPHYSICALPORTALS:		return (float)InstanceCount::getInstanceCount(InstanceCount::PHYSICALPORTAL);
			case Library::STAT_LIVEVIRTUALPORTALS:		return (float)InstanceCount::getInstanceCount(InstanceCount::VIRTUALPORTAL);
			case Library::STAT_LIVENODES:				return (float)IDatabase::getNodeCount(); 
			case Library::STAT_LIVEINSTANCES:			return (float)IDatabase::getInstanceCount(); 
			default:									return (float)(s_statistics.m_stat[s]); 
		}
	}

	//--------------------------------------------------------------------
	// Failure
	//--------------------------------------------------------------------

	DPVS_API_ASSERT(false && "Library::getStatistic() -- invalid enumeration");
	return 0.f;								// wass?
}

//------------------------------------------------------------------------
