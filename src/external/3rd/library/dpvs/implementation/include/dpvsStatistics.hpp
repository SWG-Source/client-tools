#ifndef __DPVSSTATISTICS_HPP
#define __DPVSSTATISTICS_HPP
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
 * $Archive: /dpvs/implementation/include/dpvsStatistics.hpp $
 * $Author: wili $ 
 * $Revision: #1 $
 * $Modtime: 31.07.01 12:17 $
 * $Date: 2003/01/22 $
 * 
 ******************************************************************************/

#if !defined (__DPVSPRIVATEDEFS_HPP)
#	include "dpvsPrivateDefs.hpp"
#endif

#if !defined (__DPVSLIBRARY_HPP)
#	include "dpvsLibrary.hpp"
#endif

namespace DPVS
{

/******************************************************************************
 *
 * Class:           DPVS::Statistics
 *
 * Description:     Statistics & profiling class
 *
 *****************************************************************************/

class Statistics
{
private:
					Statistics();											// cannot make instances
	struct Stat
	{
				Stat	(void);
		void	reset	(void);
		float	m_time;														// time in seconds
		int		m_stat[Library::STAT_MAX];									// integer statistics
	};

	static Stat			s_statistics;										// current statistics
	static Stat			s_pushedStatistics;									// pushed statistics	
	static int			s_statisticsPosition;								// counter for stat stack
	static const char*	s_statisticNames[Library::STAT_MAX];				// statistic names
public:
	static const char* getStatisticName		(Library::Statistic s)				
	{ 
		if (s < 0 || s >= Library::STAT_MAX)
			return "";														// return empty string
		return s_statisticNames[s];											// here we go again...
	}

	static float	getStatistic			(Library::Statistic s);
	static void		incStatistic			(Library::Statistic s)				{ DPVS_ASSERT(s>=0 && s < Library::STAT_MAX); ++s_statistics.m_stat[s];		}
	static void		incStatistic			(Library::Statistic s, int value)	{ DPVS_ASSERT(s>=0 && s < Library::STAT_MAX); s_statistics.m_stat[s]+=value;	}
	static void		setStatistic			(Library::Statistic s, int value)	{ DPVS_ASSERT(s>=0 && s < Library::STAT_MAX); s_statistics.m_stat[s]=value;	}
	static void		resetStatistics			(void)								{ s_statistics.reset(); }
	static void		pushStatistics			(void)								{ s_pushedStatistics=s_statistics; ++s_statisticsPosition; DPVS_ASSERT(s_statisticsPosition<=1); }
	static void		popStatistics			(void)								{ s_statistics=s_pushedStatistics; --s_statisticsPosition; DPVS_ASSERT(s_statisticsPosition>=0); }
};

} // DPVS

//------------------------------------------------------------------------
#endif // __DPVSSTATISTICS_HPP
