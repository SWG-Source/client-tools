// ======================================================================
//
// DataChangeListener.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DataChangeListener_H
#define INCLUDED_DataChangeListener_H

//-----------------------------------------------------------------

struct DataChangeListener
{
	virtual void dataChanged () = 0;
	virtual void currentEditingChanged () = 0;
	virtual void readOnlyChanged () = 0;
	virtual ~DataChangeListener () = 0;
};


#endif

//-----------------------------------------------------------------
