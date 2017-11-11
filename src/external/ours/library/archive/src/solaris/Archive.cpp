
//---------------------------------------------------------------------
#include "FirstArchive.h"

#include "Archive.h"
#include "ByteStream.h"
#include "htog.h"
#include <assert.h>

namespace Archive
{

//---------------------------------------------------------------------
/**
	Retrieves the target from the data buffer.

	@author Justin Randall
*/
void get(const ByteStream & source, ByteStream & target)
{
	const unsigned int newDataSize = source.getSize() - source.getReadPosition();

	if(newDataSize > 0)
	{
		const unsigned char * const newData = &source.getBuffer()[newDataSize];
		target.put(newData, newDataSize);
		source.setReadPosition(source.getReadPosition() + newDataSize);
	}
}

//---------------------------------------------------------------------
/**
	Retrieves the target from the data buffer.

	@author Justin Randall
*/
void get(const ByteStream & source, double & target)
{
	source.get(&target, 8);
	target = (double)htog64((int64)target);
}

//---------------------------------------------------------------------
/**
	Retrieves the target from the data buffer.

	@author Justin Randall
*/
void get(const ByteStream & source, unsigned long int & target)
{
	source.get(&target, 4);
	target = htog32(target);
}

//---------------------------------------------------------------------
/**
	Retrieves the target from the data buffer.

	@author Justin Randall
*/
void get(const ByteStream & source, signed long int & target)
{
	source.get(&target, 4);
	target = htog32(target);
}

//---------------------------------------------------------------------
/**
	Retrieves the target from the data buffer.

	@author Justin Randall
*/
void get(const ByteStream & source, unsigned int & target)
{
	source.get(&target, 4);
	target = htog32(target);
}

//---------------------------------------------------------------------
/**
	Retrieves the target from the data buffer.

	@author Justin Randall
*/
void get(const ByteStream & source, signed int & target)
{
	source.get(&target, 4);
	target = htog32(target);
}

//---------------------------------------------------------------------
/**
	Retrieves the target from the data buffer.

	@author Justin Randall
*/
void get(const ByteStream & source, float & target)
{
	source.get(&target, 4);
	target = (float)htog32((uint32)target);
}

//---------------------------------------------------------------------
/**
	Retrieves the target from the data buffer.

	@author Justin Randall
*/
void get(const ByteStream & source, unsigned short int & target)
{
	source.get(&target, 2);
	target = htog16(target);
}

//---------------------------------------------------------------------
/**
	Retrieves the target from the data buffer.

	@author Justin Randall
*/
void get(const ByteStream & source, signed short int & target)
{
	source.get(&target, 2);
	target = htog16(target);
}

//---------------------------------------------------------------------
/**
	Retrieves the target from the data buffer.

	@author Justin Randall
*/
void get(const ByteStream & source, unsigned char & target)
{
	source.get(&target, 1);
}

//---------------------------------------------------------------------
/**
	Retrieves the target from the data buffer.

	@author Justin Randall
*/
void get(const ByteStream & source, signed char & target)
{
	source.get(&target, 1);
}

//---------------------------------------------------------------------
/**
	Retrieves the target from the data buffer.

	@author Justin Randall
*/
void get(const ByteStream & source, std::string & target)
{
	assert(source.getBuffer() != NULL);
	const unsigned int readSize = strlen(reinterpret_cast<const char * const>(&source.getBuffer()[source.getReadPosition()])) + 1;
	target = reinterpret_cast<const char *>(&source.getBuffer()[source.getReadPosition()]);
	source.setReadPosition(source.getReadPosition() + readSize);
}

//---------------------------------------------------------------------
/**
	Retrieves the target from the data buffer.

	@author Justin Randall
*/
void get(const ByteStream & source, bool & target)
{
	source.get(&target, 1);
}

//---------------------------------------------------------------------
/**
	Add the source data to the ByteStream.

	@param const ByteStream & source   A reference to the source data that
	                                will be appended to the ByteStream 
									buffer.

	@author Justin Randall
*/
void put(ByteStream & target, const ByteStream & source)
{
	const unsigned char * const b = &source.getBuffer()[source.getReadPosition()];
	const unsigned int readSize = source.getSize() - source.getReadPosition();
	target.put(b, readSize);
	source.setReadPosition(source.getReadPosition() + readSize);
}

//---------------------------------------------------------------------
/**
	Add the source data to the ByteStream.

	@param const double source    A reference to the source data that
	                              will be appended to the ByteStream 
	                              buffer.

	@author Justin Randall
*/
void put(ByteStream & target, const double value)
{
	double value1 = (double)htog64((int64)value);	
	target.put(&value1, 8);
}

//---------------------------------------------------------------------
/**
	Add the source data to the ByteStream.

	@param const unsigned long int source   A reference to the source 
	                                        data that will be appended 
	                                        to the ByteStream bufffer.

	@author Justin Randall
*/
void put(ByteStream & target, const unsigned long int value)
{
	unsigned long int value1 = htog32(value); 
	target.put(&value1, 4);
}

//---------------------------------------------------------------------
/**
	Add the source data to the ByteStream.

	@param const signed long int source     A reference to the source 
	                                        data that will be appended 
	                                        to the ByteStream bufffer.

	@author Justin Randall
*/
void put(ByteStream & target, const signed long int value)
{
	signed long int value1 = htog32(value);
	target.put(&value1, 4);
}

//---------------------------------------------------------------------
/**
	Add the source data to the ByteStream.

	@param const unsigned int source        A reference to the source 
	                                        data that will be appended 
	                                        to the ByteStream bufffer.

	@author Justin Randall
*/
void put(ByteStream & target, const unsigned int value)
{
	unsigned int value1 = htog32(value);	
	target.put(&value1, 4);
}

//---------------------------------------------------------------------
/**
	Add the source data to the ByteStream.

	@param const signed int source          A reference to the source 
	                                        data that will be appended 
	                                        to the ByteStream bufffer.

	@author Justin Randall
*/
void put(ByteStream & target, const signed int value)
{
	signed int value1 = htog32(value);	
	target.put(&value1, 4);
}

//---------------------------------------------------------------------
/**
	Add the source data to the ByteStream.

	@param const float source               A reference to the source 
	                                        data that will be appended 
	                                        to the ByteStream bufffer.

	@author Justin Randall
*/
void put(ByteStream & target, const float value)
{
	float value1 = (float)htog32((uint32)value);	
	target.put(&value1, 4);
}

//---------------------------------------------------------------------
/**
	Add the source data to the ByteStream.

	@param const unsigned short int source   A reference to the source 
	                                         data that will be appended 
	                                         to the ByteStream bufffer.

	@author Justin Randall
*/
void put(ByteStream & target, const unsigned short int value)
{
	unsigned short int value1 = htog16(value);	
	target.put(&value1, 2);
}

//---------------------------------------------------------------------
/**
	Add the source data to the ByteStream.

	@param const signed short int source    A reference to the source 
	                                        data that will be appended 
	                                        to the ByteStream bufffer.

	@author Justin Randall
*/
void put(ByteStream & target, const signed short int value)
{
	signed short int value1 = htog16(value);	
	target.put(&value1, 2);
}

//---------------------------------------------------------------------
/**
	Add the source data to the ByteStream.

	@param const unsigned char source       A reference to the source 
	                                        data that will be appended 
	                                        to the ByteStream bufffer.

	@author Justin Randall
*/
void put(ByteStream & target, const unsigned char value)
{
	target.put(&value, 1);
}

//---------------------------------------------------------------------
/**
	Add the source data to the ByteStream.

	@param const signer char source         A reference to the source 
	                                        data that will be appended 
	                                        to the ByteStream bufffer.

	@author Justin Randall
*/
void put(ByteStream & target, const signed char value)
{
	target.put(&value, 1);
}

//---------------------------------------------------------------------
/**
	Add the source data to the ByteStream.

	@param const std::string & source       A reference to the source 
	                                        data that will be appended 
	                                        to the ByteStream bufffer.

	@author Justin Randall
*/
void put(ByteStream & target, const std::string & source)
{
	target.put(source.c_str(), source.size() + 1);
}

//---------------------------------------------------------------------
/**
	Add the source data to the ByteStream.

	@param const bool source                A reference to the source 
	                                        data that will be appended 
	                                        to the ByteStream bufffer.

	@author Justin Randall
*/
void put(ByteStream & target, const bool & source)
{
	target.put(&source, 1);
}

//---------------------------------------------------------------------
/**
	@brief put raw binary data into the ByteStream

	@param source      source data to append to the ByteStream at the current
	                   write position
	@param sourceSize  The size of the source data

	@author Justin Randall
*/
void put(ByteStream & target, const unsigned char * const source, const unsigned int sourceSize)
{
	target.put(source, sourceSize);
}

//---------------------------------------------------------------------

}//namespace Archive
