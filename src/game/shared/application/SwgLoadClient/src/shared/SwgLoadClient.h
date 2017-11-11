// SwgLoadClient.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_SwgLoadClient_H
#define	_INCLUDED_SwgLoadClient_H

//-----------------------------------------------------------------------

#include <vector>

class Client;
class Timer;

//-----------------------------------------------------------------------

class SwgLoadClient
{
public:
	SwgLoadClient();
	~SwgLoadClient();

	static void  quit();
	static void  run();

private:
	SwgLoadClient & operator = (const SwgLoadClient & rhs);
	SwgLoadClient(const SwgLoadClient & source);

	static SwgLoadClient & instance ();

	void         makeClient();
	void         update();

private:
	Timer *                clientCreateTimer;
	std::vector<Client *>  clients;
	bool                   done;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_SwgLoadClient_H
