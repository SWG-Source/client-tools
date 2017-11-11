// 
// MonAPITest.cpp
//
// Implement a fully featured test of the Monitoring API
//
// Author: Tim Holcomb

#include <stdio.h>
#include <string>
#include <vector>
#include <time.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#pragma warning (disable : 4786) 
#pragma warning ( disable : 4710 )
#endif

// includes for monitor api
#include "../MonAPI/IMonitor.h"
#include "../MonAPI/soeMonitorListener.h"
#include "Monitordef.h"


//----------------------------------------------------------------

// the following simulates what the game server would do to provide monitoring information
int main()
{
	long timer1 = time(NULL);
	long timer2 = time(NULL) + 60;
	long timer3	= time(NULL);
	int nCount = 0;
	int nConnected = 10;
	int nlogin = 100;
	int flag = 1;
	int x;
	char temp[1024];
	char description[1024];		
	int bDone;

	// set up the Monitor
	soeMonitorListener myMonitorListener;
	IMonitoringAPI * Monitor = CreateMonitoringAPI();
	Monitor->SetListener(&myMonitorListener);
	
	// Load the config list into the monitor object

	myMonitorListener.add("Population",			POPULATION,			MON_PING_5MIN);
	

	myMonitorListener.add("Naboo",NABOO_COUNT,MON_PING_5MIN);

	for( x=1;x<30;x++)
	{
		sprintf(temp,"Naboo.Count.%02d",x);
		myMonitorListener.add(temp,NABOO_COUNT+x,MON_PING_5MIN);
	}

	for( x=0;x<30;x++)
	{
		sprintf(temp,"Naboo.Zones.%02d",x);
		myMonitorListener.add(temp,NABOO_ZONES+x,MON_PING_5MIN);
	}

	for( x=0;x<30;x++)
	{
		sprintf(temp,"Naboo.Process.%02d",x);
		sprintf(description,"sdt-eqoa%02d.sonygames.sony.com",x);
		myMonitorListener.add(temp,NABOO_PROCESS+x,MON_PING_5MIN,description);
	}

	for( x=0;x<30;x++)
	{
		sprintf(temp,"Naboo.Times.%02d",x);
		sprintf(description,"%06d Times",x);
		myMonitorListener.add(temp,NABOO_TIMES+x,MON_PING_30,description);
	}
	

	for( x=0;x<30;x++)
	{
		sprintf(temp,"Zandar.Count.%02d",x);
		myMonitorListener.add(temp,ZANDAR_COUNT+x,MON_PING_5MIN);
	}

	for( x=0;x<30;x++)
	{
		sprintf(temp,"Zandar.Zones.%02d",x);
		myMonitorListener.add(temp,ZANDAR_ZONES+x,MON_PING_5MIN);
	}

	for( x=0;x<30;x++)
	{
		sprintf(temp,"Zandar.Process.%02d",x);
		sprintf(description,"sdt-eqoa%02d.everquestadventures.sony.com",x);
		myMonitorListener.add(temp,ZANDAR_PROCESS+x,MON_PING_5MIN,description);
	}

	for( x=0;x<30;x++)
	{
		sprintf(temp,"Zandar.Times.%02d",x);
		sprintf(description,"%06d Times",x);
		myMonitorListener.add(temp,ZANDAR_TIMES+x,MON_PING_30,description);
	}



	myMonitorListener.dump();

	// bind the monitor to the listening port
	if (Monitor->InitMonitorApi(2200, "login.cfg") == false)
	{
		printf("error initializing monMonitorObject\n");
	}

	// This while loop simulates some update loop that you might 
	// already be using to update other network data
	bDone = false;
	while(bDone == false)
	{
		// updateMonitor() will return false on the 
		// next update cyclte following a call to shutdownMonitor()
		if (Monitor->Update() == false)
		{
			break;
		}

		if( timer3 < time(NULL) )
		{
		struct tm *newtime;
		time_t aclock;
		char temp[128];

			time( &aclock );                 
			newtime = localtime( &aclock );  
			sprintf(temp,"[ %s ]",asctime(newtime));
			myMonitorListener.setDescription(POPULATION,temp);
			timer3 = time(NULL) + 120;
		}

		// Update the counts in the object
		// Test data
		if( timer1 < time(NULL) )
		{

			myMonitorListener.setDescription(NABOO_COUNT,"Naboo is counting");


			nCount++; 		myMonitorListener.set(POPULATION,	nCount);
							
			for( x=0;x<30;x++)	myMonitorListener.set(NABOO_COUNT+x,nCount);
			for( x=0;x<30;x++)	myMonitorListener.set(NABOO_ZONES+x,nCount+1);
			for( x=0;x<30;x++)	myMonitorListener.set(NABOO_PROCESS+x,nCount+2);
			for( x=0;x<30;x++)	myMonitorListener.set(NABOO_TIMES+x,nCount*100);

			for( x=0;x<30;x++)	myMonitorListener.set(ZANDAR_COUNT+x,nCount);
			for( x=0;x<30;x++)	myMonitorListener.set(ZANDAR_ZONES+x,nCount+1);
			for( x=0;x<30;x++)	myMonitorListener.set(ZANDAR_PROCESS+x,nCount+2);
			for( x=0;x<30;x++)	myMonitorListener.set(ZANDAR_TIMES+x,nCount*100);
			
		
			timer1 = time(NULL) + 10;

		}
#ifdef _WIN32
		Sleep(1);
#else
		usleep(100);
#endif
	}

	// run the monitor in a seperate thread.
	Monitor->Release();
	delete Monitor;
	return 0;
}
