// PackClass.h: interface for the PackClass class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PACKCLASS_H__B50E2A21_B4FD_11D4_83BF_00C04F6C1CA6__INCLUDED_)
#define AFX_PACKCLASS_H__B50E2A21_B4FD_11D4_83BF_00C04F6C1CA6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define PACK_SIZE 8192

class PackClass  
{
public:
    // buffersize should be the larger of the buffer to pack or the largest STRING to unpack!
    // if the buffer is for unpacking and there are no strings,
	PackClass(long buffersize);
	PackClass();    // default buffer of PACK_SIZE
	~PackClass();

	void pack_begin(short int msgtype);
	void pack_byte(char byte);
	void pack_long(long i);
	void pack_string(char *s);
	void pack_struct(void *d, int l);
	void unpack_begin(void *data);
	char unpack_byte(void);
	long unpack_long(void);	
	char *unpack_string(void);
	void unpack_struct(void *dest,long l);
	int send(connection_t *c);
private:
    unsigned long pack_max;
	unsigned long pack_offset;
	unsigned long unpack_offset;
	char *pack_data;
	char *unpack_data;
};

#endif // !defined(AFX_PACKCLASS_H__B50E2A21_B4FD_11D4_83BF_00C04F6C1CA6__INCLUDED_)
