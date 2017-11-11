/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Mon Mar 11 14:19:47 2002
 */
/* Compiler settings for C:\work\swg\current\src\engine\shared\application\hash_plugin\hash_plugin.odl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID LIBID_Hash_plugin = {0x51A4F3D7,0x95EA,0x4D99,{0xAC,0x23,0x71,0x6E,0x37,0x75,0x42,0x1D}};


const IID IID_ICommands = {0x7C47B940,0x9F5C,0x42CD,{0x92,0x5D,0xF6,0x2A,0x9B,0xE6,0x67,0xE5}};


const CLSID CLSID_Commands = {0xDC3A24A8,0x3E78,0x4E00,{0x84,0x86,0x1F,0xE9,0xDC,0x5C,0xF8,0x06}};


#ifdef __cplusplus
}
#endif

