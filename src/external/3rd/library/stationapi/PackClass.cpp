// PackClass.cpp: implementation of the PackClass class.
//
//////////////////////////////////////////////////////////////////////

#include "order.h"
#include "rdp_api.h"
#include "PackClass.h"
#include <malloc.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PackClass::PackClass(long buffersize)
{
    pack_max = buffersize + 16; // minimum buffer is 16
	pack_offset = 0;
	unpack_offset = 0;
    pack_data = NULL;
	unpack_data = NULL;

    pack_data = (char *)calloc(pack_max,1);
}

PackClass::PackClass()
{
    pack_max = PACK_SIZE;
	pack_offset = 0;
	unpack_offset = 0;
    pack_data = NULL;
	unpack_data = NULL;

    pack_data = (char *)calloc(pack_max,1);
}

PackClass::~PackClass()
{
    if (pack_data)
    {
        free(pack_data);
        pack_data = NULL;
    }
}

//-------------------------------
int PackClass::send(connection_t *c)
//-------------------------------
{
	return connection_send(c, pack_data, pack_offset, 1, RDP_SEND_GUARANTEED );
}

//-------------------------------
void PackClass::pack_begin(short int msgtype)
//-------------------------------
{
	msgtype = htog16(msgtype);  // ensure little-endian
	memcpy(&pack_data[0], &msgtype, 2);
    pack_offset = 2;
}

void PackClass::pack_byte(char byte)
{
    if (1 + pack_offset <= pack_max) 
    {
        pack_data[pack_offset] = byte;
        pack_offset++;
    }
}
//-------------------------------
void PackClass::pack_long(long i)
//-------------------------------
{
	i = htog32(i);  // ensure little-endian
    if((sizeof(long) + pack_offset) <= pack_max)
    {
        memcpy(&pack_data[pack_offset], &i, sizeof(long));
        pack_offset += sizeof(long);
    }
}
//-------------------------------
void PackClass::pack_string(char *s)
//-------------------------------
{
	int len = strlen(s);
	if (len == 0)
	{
		pack_data[pack_offset] = '\0';
		pack_offset++;
	}
	else if ((len + 1 + pack_offset) <= pack_max)
	{
		char *dest = pack_data + pack_offset;
		while (*(dest++) = *(s++))
		{
		}
		pack_offset += len + 1;
	}
}
//-------------------------------
void PackClass::pack_struct(void *d, int l)
//-------------------------------
{
	if((l + pack_offset) <= pack_max)
    {
		memcpy(&pack_data[pack_offset], d, l);
		pack_offset += l;
	}
}
//-------------------------------
void PackClass::unpack_begin(void *data)
//-------------------------------
{
    unpack_data = (char *)data;
    unpack_offset = 2;					// skip the message type (2 bytes)
}
//-------------------------------
char PackClass::unpack_byte(void)
//-------------------------------
{
	char byte = 0;
	if (1 + unpack_offset <= pack_max)
    {
		byte = unpack_data[unpack_offset];
		unpack_offset++;
	}
	return byte;
}
//-------------------------------
long PackClass::unpack_long(void)
//-------------------------------
{
    long i = 0;

    if((unpack_offset + sizeof(long)) <= pack_max)
    {
        memcpy(&i, &unpack_data[unpack_offset], sizeof(long));
        unpack_offset += sizeof(long);
    }

	i = htog32(i);  // from little-endian
    return i;
}
//-------------------------------
char* PackClass::unpack_string(void)
//-------------------------------
{
    int         i;

    for(i = 0; ((i + unpack_offset) < pack_max) && unpack_data[i + unpack_offset]; i++)
    {
        pack_data[i] = unpack_data[i + unpack_offset];
    }
    pack_data[i] = '\0';
    unpack_offset += strlen(pack_data) + 1;

    return pack_data;
}
//-------------------------------
void PackClass::unpack_struct(void *dest,long l)
//-------------------------------
{
	memcpy(dest,&unpack_data[unpack_offset],l);
	unpack_offset += l;
}
