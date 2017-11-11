/*
 -------------------------------------------------------------------------
 Copyright (c) 2001, Dr Brian Gladman <brg@gladman.me.uk>, Worcester, UK.
 All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary 
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright 
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products 
      built using this software without specific written permission. 

 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness 
 and fitness for purpose.
 -------------------------------------------------------------------------
 Issue Date: 15/01/2002
*/

// Correct Output (for variable block size - BLOCK_SIZE undefined):

// lengths:  block = 16 bytes, key = 16 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c
// input   = 3243f6a8885a308d313198a2e0370734
// encrypt = 3925841d02dc09fbdc118597196a0b32
// decrypt = 3243f6a8885a308d313198a2e0370734

// lengths:  block = 16 bytes, key = 20 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160
// input   = 3243f6a8885a308d313198a2e0370734
// encrypt = 231d844639b31b412211cfe93712b880
// decrypt = 3243f6a8885a308d313198a2e0370734

// lengths:  block = 16 bytes, key = 24 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da5
// input   = 3243f6a8885a308d313198a2e0370734
// encrypt = f9fb29aefc384a250340d833b87ebc00
// decrypt = 3243f6a8885a308d313198a2e0370734

// lengths:  block = 16 bytes, key = 28 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d90
// input   = 3243f6a8885a308d313198a2e0370734
// encrypt = 8faa8fe4dee9eb17caa4797502fc9d3f
// decrypt = 3243f6a8885a308d313198a2e0370734

// lengths:  block = 16 bytes, key = 32 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d9045190cfe
// input   = 3243f6a8885a308d313198a2e0370734
// encrypt = 1a6e6c2c662e7da6501ffb62bc9e93f3
// decrypt = 3243f6a8885a308d313198a2e0370734

// lengths:  block = 20 bytes, key = 16 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c
// input   = 3243f6a8885a308d313198a2e03707344a409382
// encrypt = 16e73aec921314c29df905432bc8968ab64b1f51
// decrypt = 3243f6a8885a308d313198a2e03707344a409382

// lengths:  block = 20 bytes, key = 20 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160
// input   = 3243f6a8885a308d313198a2e03707344a409382
// encrypt = 0553eb691670dd8a5a5b5addf1aa7450f7a0e587
// decrypt = 3243f6a8885a308d313198a2e03707344a409382

// lengths:  block = 20 bytes, key = 24 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da5
// input   = 3243f6a8885a308d313198a2e03707344a409382
// encrypt = 73cd6f3423036790463aa9e19cfcde894ea16623
// decrypt = 3243f6a8885a308d313198a2e03707344a409382

// lengths:  block = 20 bytes, key = 28 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d90
// input   = 3243f6a8885a308d313198a2e03707344a409382
// encrypt = 601b5dcd1cf4ece954c740445340bf0afdc048df
// decrypt = 3243f6a8885a308d313198a2e03707344a409382

// lengths:  block = 20 bytes, key = 32 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d9045190cfe
// input   = 3243f6a8885a308d313198a2e03707344a409382
// encrypt = 579e930b36c1529aa3e86628bacfe146942882cf
// decrypt = 3243f6a8885a308d313198a2e03707344a409382

// lengths:  block = 24 bytes, key = 16 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d
// encrypt = b24d275489e82bb8f7375e0d5fcdb1f481757c538b65148a
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d

// lengths:  block = 24 bytes, key = 20 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d
// encrypt = 738dae25620d3d3beff4a037a04290d73eb33521a63ea568
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d

// lengths:  block = 24 bytes, key = 24 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da5
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d
// encrypt = 725ae43b5f3161de806a7c93e0bca93c967ec1ae1b71e1cf
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d

// lengths:  block = 24 bytes, key = 28 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d90
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d
// encrypt = bbfc14180afbf6a36382a061843f0b63e769acdc98769130
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d

// lengths:  block = 24 bytes, key = 32 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d9045190cfe
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d
// encrypt = 0ebacf199e3315c2e34b24fcc7c46ef4388aa475d66c194c
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d

// lengths:  block = 28 bytes, key = 16 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9
// encrypt = b0a8f78f6b3c66213f792ffd2a61631f79331407a5e5c8d3793aceb1
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9

// lengths:  block = 28 bytes, key = 20 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9
// encrypt = 08b99944edfce33a2acb131183ab0168446b2d15e958480010f545e3
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9

// lengths:  block = 28 bytes, key = 24 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da5
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9
// encrypt = be4c597d8f7efe22a2f7e5b1938e2564d452a5bfe72399c7af1101e2
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9

// lengths:  block = 28 bytes, key = 28 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d90
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9
// encrypt = ef529598ecbce297811b49bbed2c33bbe1241d6e1a833dbe119569e8
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9

// lengths:  block = 28 bytes, key = 32 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d9045190cfe
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9
// encrypt = 02fafc200176ed05deb8edb82a3555b0b10d47a388dfd59cab2f6c11
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa9

// lengths:  block = 32 bytes, key = 16 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8
// encrypt = 7d15479076b69a46ffb3b3beae97ad8313f622f67fedb487de9f06b9ed9c8f19
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8

// lengths:  block = 32 bytes, key = 20 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8
// encrypt = 514f93fb296b5ad16aa7df8b577abcbd484decacccc7fb1f18dc567309ceeffd
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8

// lengths:  block = 32 bytes, key = 24 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da5
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8
// encrypt = 5d7101727bb25781bf6715b0e6955282b9610e23a43c2eb062699f0ebf5887b2
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8

// lengths:  block = 32 bytes, key = 28 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d90
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8
// encrypt = d56c5a63627432579e1dd308b2c8f157b40a4bfb56fea1377b25d3ed3d6dbf80
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8

// lengths:  block = 32 bytes, key = 32 bytes
// key     = 2b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a784d9045190cfe
// input   = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8
// encrypt = a49406115dfb30a40418aafa4869b7c6a886ff31602a7dd19c889dc64f7e4e7a
// decrypt = 3243f6a8885a308d313198a2e03707344a4093822299f31d0082efa98ec4e6c8

#include <iostream>
#include <iomanip>
#include <memory.h>
//#include <windows.h>

#ifdef AES_IN_CPP
#include "aescpp.h"
#else
#include "aes.h"
#endif
#include "aestst.h"

// Use this define if testing aespp.c

//#define AESX

typedef unsigned char   byte;
typedef unsigned long   word;

#if defined(AES_DLL)
fn_ptrs fn;
#endif

extern "C" void out_state(long s0, long s1, long s2, long s3)
{
    std::cout << '\n' << std::hex << std::setfill('0');
    std::cout << std::setw(8) << s0;
    std::cout << std::setw(8) << s1;
    std::cout << std::setw(8) << s2;
    std::cout << std::setw(8) << s3;
}

void oblk(char m[], byte v[], word n = 16)
{
    std::cout << std::hex << std::setfill('0') << '\n' << m;

    for(word i = 0; i < n; ++i)
    
        std::cout << std::setw(2) << static_cast<word>(v[i]);
}

void message(const char *s)   { std::cout << s; }

#ifdef  AES_DLL

#include "windows.h"

HINSTANCE init_dll(fn_ptrs& fn)
{   HINSTANCE   h_dll;

    if(!(h_dll = LoadLibrary(dll_path)))
    {
        std::cout << "\n\nDynamic link Library AES_DLL not found\n\n"; return 0;
    }

    fn.fn_blk_len = (g_blk_len*)GetProcAddress(h_dll, "_aes_blk_len@8");
    fn.fn_enc_key = (g_enc_key*)GetProcAddress(h_dll, "_aes_enc_key@12");
    fn.fn_dec_key = (g_dec_key*)GetProcAddress(h_dll, "_aes_dec_key@12");
    fn.fn_enc_blk = (g_enc_blk*)GetProcAddress(h_dll, "_aes_enc_blk@12");
    fn.fn_dec_blk = (g_dec_blk*)GetProcAddress(h_dll, "_aes_dec_blk@12");

#if !defined(BLOCK_SIZE)
    if(!fn.fn_enc_key || !fn.fn_dec_key || !fn.fn_enc_blk  || !fn.fn_dec_blk || !fn.fn_blk_len)
#else
    if(!fn.fn_enc_key || !fn.fn_dec_key || !fn.fn_enc_blk  || !fn.fn_dec_blk)
#endif
    {
        std::cout << "\n\nRequired DLL Entry Point(s) not found\n\n"; 
        FreeLibrary(h_dll); 
        return 0;
    }

    return h_dll;
}

#endif  // AES_DLL

byte pih[32] = // hex digits of pi
{
    0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
    0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34,
    0x4a, 0x40, 0x93, 0x82, 0x22, 0x99, 0xf3, 0x1d,
    0x00, 0x82, 0xef, 0xa9, 0x8e, 0xc4, 0xe6, 0xc8
};

byte exh[32] =  // hex digits of e
{
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
    0x76, 0x2e, 0x71, 0x60, 0xf3, 0x8b, 0x4d, 0xa5,
    0x6a, 0x78, 0x4d, 0x90, 0x45, 0x19, 0x0c, 0xfe
};

byte res[5][5][32] = 
{ 
    {
        { 0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 
          0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32 
        },
        {
         0x23, 0x1d, 0x84, 0x46, 0x39, 0xb3, 0x1b, 0x41, 
         0x22, 0x11, 0xcf, 0xe9, 0x37, 0x12, 0xb8, 0x80
        },
        { 0xf9, 0xfb, 0x29, 0xae, 0xfc, 0x38, 0x4a, 0x25, 
          0x03, 0x40, 0xd8, 0x33, 0xb8, 0x7e, 0xbc, 0x00
        },
        {
          0x8f, 0xaa, 0x8f, 0xe4, 0xde, 0xe9, 0xeb, 0x17,
          0xca, 0xa4, 0x79, 0x75, 0x02, 0xfc, 0x9d, 0x3f
        },
        { 0x1a, 0x6e, 0x6c, 0x2c, 0x66, 0x2e, 0x7d, 0xa6, 
          0x50, 0x1f, 0xfb, 0x62, 0xbc, 0x9e, 0x93, 0xf3
        }
    },
    {
        {
          0x16, 0xe7, 0x3a, 0xec, 0x92, 0x13, 0x14, 0xc2, 
          0x9d, 0xf9, 0x05, 0x43, 0x2b, 0xc8, 0x96, 0x8a, 
          0xb6, 0x4b, 0x1f, 0x51
        },
        {
          0x05, 0x53, 0xeb, 0x69, 0x16, 0x70, 0xdd, 0x8a, 
          0x5a, 0x5b, 0x5a, 0xdd, 0xf1, 0xaa, 0x74, 0x50, 
          0xf7, 0xa0, 0xe5, 0x87
        },
        { 
          0x73, 0xcd, 0x6f, 0x34, 0x23, 0x03, 0x67, 0x90, 
          0x46, 0x3a, 0xa9, 0xe1, 0x9c, 0xfc, 0xde, 0x89, 
          0x4e, 0xa1, 0x66, 0x23
        },
        { 
          0x60, 0x1b, 0x5d, 0xcd, 0x1c, 0xf4, 0xec, 0xe9,
          0x54, 0xc7, 0x40, 0x44, 0x53, 0x40, 0xbf, 0x0a,
          0xfd, 0xc0, 0x48, 0xdf
        },
        {
          0x57, 0x9e, 0x93, 0x0b, 0x36, 0xc1, 0x52, 0x9a, 
          0xa3, 0xe8, 0x66, 0x28, 0xba, 0xcf, 0xe1, 0x46, 
          0x94, 0x28, 0x82, 0xcf 
        }
    },
    {
        { 0xb2, 0x4d, 0x27, 0x54, 0x89, 0xe8, 0x2b, 0xb8, 
          0xf7, 0x37, 0x5e, 0x0d, 0x5f, 0xcd, 0xb1, 0xf4, 
          0x81, 0x75, 0x7c, 0x53, 0x8b, 0x65, 0x14, 0x8a
        },
        {
          0x73, 0x8d, 0xae, 0x25, 0x62, 0x0d, 0x3d, 0x3b, 
          0xef, 0xf4, 0xa0, 0x37, 0xa0, 0x42, 0x90, 0xd7, 
          0x3e, 0xb3, 0x35, 0x21, 0xa6, 0x3e, 0xa5, 0x68 
        },
        { 0x72, 0x5a, 0xe4, 0x3b, 0x5f, 0x31, 0x61, 0xde, 
          0x80, 0x6a, 0x7c, 0x93, 0xe0, 0xbc, 0xa9, 0x3c,
          0x96, 0x7e, 0xc1, 0xae, 0x1b, 0x71, 0xe1, 0xcf
        },
        {
          0xbb, 0xfc, 0x14, 0x18, 0x0a, 0xfb, 0xf6, 0xa3,
          0x63, 0x82, 0xa0, 0x61, 0x84, 0x3f, 0x0b, 0x63,
          0xe7, 0x69, 0xac, 0xdc, 0x98, 0x76, 0x91, 0x30
        },
        { 0x0e, 0xba, 0xcf, 0x19, 0x9e, 0x33, 0x15, 0xc2, 
          0xe3, 0x4b, 0x24, 0xfc, 0xc7, 0xc4, 0x6e, 0xf4, 
          0x38, 0x8a, 0xa4, 0x75, 0xd6, 0x6c, 0x19, 0x4c
        }
    },
    {
        {
          0xb0, 0xa8, 0xf7, 0x8f, 0x6b, 0x3c, 0x66, 0x21, 
          0x3f, 0x79, 0x2f, 0xfd, 0x2a, 0x61, 0x63, 0x1f, 
          0x79, 0x33, 0x14, 0x07, 0xa5, 0xe5, 0xc8, 0xd3, 
          0x79, 0x3a, 0xce, 0xb1
        },
        {
          0x08, 0xb9, 0x99, 0x44, 0xed, 0xfc, 0xe3, 0x3a, 
          0x2a, 0xcb, 0x13, 0x11, 0x83, 0xab, 0x01, 0x68, 
          0x44, 0x6b, 0x2d, 0x15, 0xe9, 0x58, 0x48, 0x00, 
          0x10, 0xf5, 0x45, 0xe3
        },
        {
          0xbe, 0x4c, 0x59, 0x7d, 0x8f, 0x7e, 0xfe, 0x22, 
          0xa2, 0xf7, 0xe5, 0xb1, 0x93, 0x8e, 0x25, 0x64, 
          0xd4, 0x52, 0xa5, 0xbf, 0xe7, 0x23, 0x99, 0xc7, 
          0xaf, 0x11, 0x01, 0xe2
        },
        {
          0xef, 0x52, 0x95, 0x98, 0xec, 0xbc, 0xe2, 0x97,
          0x81, 0x1b, 0x49, 0xbb, 0xed, 0x2c, 0x33, 0xbb,
          0xe1, 0x24, 0x1d, 0x6e, 0x1a, 0x83, 0x3d, 0xbe,
          0x11, 0x95, 0x69, 0xe8
        },
        {
          0x02, 0xfa, 0xfc, 0x20, 0x01, 0x76, 0xed, 0x05, 
          0xde, 0xb8, 0xed, 0xb8, 0x2a, 0x35, 0x55, 0xb0, 
          0xb1, 0x0d, 0x47, 0xa3, 0x88, 0xdf, 0xd5, 0x9c, 
          0xab, 0x2f, 0x6c, 0x11
        }
    },
    {
        { 0x7d, 0x15, 0x47, 0x90, 0x76, 0xb6, 0x9a, 0x46, 
          0xff, 0xb3, 0xb3, 0xbe, 0xae, 0x97, 0xad, 0x83, 
          0x13, 0xf6, 0x22, 0xf6, 0x7f, 0xed, 0xb4, 0x87, 
          0xde, 0x9f, 0x06, 0xb9, 0xed, 0x9c, 0x8f, 0x19
        },
        {
          0x51, 0x4f, 0x93, 0xfb, 0x29, 0x6b, 0x5a, 0xd1, 
          0x6a, 0xa7, 0xdf, 0x8b, 0x57, 0x7a, 0xbc, 0xbd, 
          0x48, 0x4d, 0xec, 0xac, 0xcc, 0xc7, 0xfb, 0x1f, 
          0x18, 0xdc, 0x56, 0x73, 0x09, 0xce, 0xef, 0xfd
        },
        { 0x5d, 0x71, 0x01, 0x72, 0x7b, 0xb2, 0x57, 0x81, 
          0xbf, 0x67, 0x15, 0xb0, 0xe6, 0x95, 0x52, 0x82, 
          0xb9, 0x61, 0x0e, 0x23, 0xa4, 0x3c, 0x2e, 0xb0, 
          0x62, 0x69, 0x9f, 0x0e, 0xbf, 0x58, 0x87, 0xb2
        },
        {
          0xd5, 0x6c, 0x5a, 0x63, 0x62, 0x74, 0x32, 0x57,
          0x9e, 0x1d, 0xd3, 0x08, 0xb2, 0xc8, 0xf1, 0x57,
          0xb4, 0x0a, 0x4b, 0xfb, 0x56, 0xfe, 0xa1, 0x37,
          0x7b, 0x25, 0xd3, 0xed, 0x3d, 0x6d, 0xbf, 0x80
        },
        { 0xa4, 0x94, 0x06, 0x11, 0x5d, 0xfb, 0x30, 0xa4, 
          0x04, 0x18, 0xaa, 0xfa, 0x48, 0x69, 0xb7, 0xc6, 
          0xa8, 0x86, 0xff, 0x31, 0x60, 0x2a, 0x7d, 0xd1, 
          0x9c, 0x88, 0x9d, 0xc6, 0x4f, 0x7e, 0x4e, 0x7a
        }
    }
};

#if defined(AESX)
#define INC 1
#else
#define INC 2
#endif

#if BLOCK_SIZE == 16
#define STR 0
#define CNT 1
#elif BLOCK_SIZE == 24
#define STR 2
#define CNT 3
#elif BLOCK_SIZE == 32
#define STR 4
#define CNT 5
#elif !defined(BLOCK_SIZE)
#define STR 0
#define CNT 5
#elif !defined AESX
#error Illegal block size
#elif BLOCK_SIZE == 20
#define STR 1
#define CNT 2
#elif BLOCK_SIZE == 28
#define STR 3
#define CNT 4
#else
#error Illegal block size
#endif

/*void cycles(volatile unsigned __int64 *rtn)    
{   
    __asm   // read the Pentium Time Stamp Counter
    {   cpuid
        rdtsc
        mov     ecx,rtn
        mov     [ecx],eax
        mov     [ecx+4],edx
        cpuid
    }
}*/


int soe_encrypt(unsigned char *input, unsigned char *output_encrypted, int length = 16);
int soe_decrypt(unsigned char *input_encrypted, unsigned char *output, int length = 16);

#if 0
int main(void)
{   byte        out[32], ret[32], err = 0;
    f_ctx   alg;

#if defined(AES_DLL)
    HINSTANCE   h_dll;
    fn_ptrs     fn;

    if(!(h_dll = init_dll(fn))) return -1;
#endif

#if !defined(AES_IN_CPP)
    alg.n_blk = 0;
    alg.n_rnd = 0;
#endif

    message("\nRun tests for the AES algorithm");
#if defined(AES_DLL)
    message(" (DLL Version)");
#endif
#if defined(AES_IN_CPP)
    message(" (CPP Version)");
#endif

    for(int bi = STR; bi < CNT; ++bi)
    {
#if defined(AES_DLL)
            if(fn.fn_blk_len) f_blk_len(&alg, 16 + 4 * bi);
#elif !defined(BLOCK_SIZE)
            f_blk_len(&alg, 16 + 4 * bi);
#else
            if(16 + 4 * bi != BLOCK_SIZE) continue;
#endif
        for(int ki = 0; ki < 5; ki += INC)
        {
            std::cout << "\n\n// lengths:  block = " << std::dec 
                << 16 + 4 * bi << " bytes, key = " << 16 + 4 * ki << " bytes";
            //f_enc_key((aes_ctx *)&alg, (const unsigned char *)exh, (unsigned int)(16 + 4 * ki)); 
            f_enc_key(&alg, exh, (16 + 4 * ki)); 
            oblk("// key     = ", exh, 16 + 4 * ki); 
            oblk("// input   = ", pih, 16 + 4 * bi); 
            f_enc_blk(&alg, pih, out); 
            oblk("// encrypt = ", out, 16 + 4 * bi);
            if(memcmp(out, res[bi][ki], 16 + 4 * bi)) err++;
            f_dec_key(&alg, exh, 16 + 4 * ki); 
            f_dec_blk(&alg, out, ret); 
            oblk("// decrypt = ", ret, 16 + 4 * bi); 
            if(memcmp(ret, pih, 16 + 4 * bi)) err++;
        }
    }

    message("\n\nThese values are ");
    message(err ? "in error\n\n" : "correct\n\n");


	unsigned char input[256];
	unsigned char encrypted[256];
	unsigned char output[256];

	strcpy((char *)input, "hello there. The dog went down the block to take a walk");
	soe_encrypt(input, encrypted, strlen((char *)input)+1);
	soe_decrypt(encrypted, output, strlen((char *)input)+1);

#if defined(AES_DLL)
    if(h_dll) FreeLibrary(h_dll);
#endif
    return 0;
}
#endif


int soe_encrypt(unsigned char *input, unsigned char *output_encrypted, int length)
{
	f_ctx   alg;
    alg.n_blk = 0;
    alg.n_rnd = 0;

	printf("// input string  = %s\n", input);

	//
	// Use the key to build the algorithm tables.
	//
	f_enc_key(&alg, exh, 16); 
	oblk("// key     = ", exh, 16);		// Using 16 byte key(128 bits). 

	//
	// Go through the input block and encode 16 bytes at a time.	
	//
	for(int i = 0; i < length; i += 16)
	{
		oblk("// input   = ", input+i, 16); 
		//
		// Encode the input block.  Put result in output block.  
		// Fixed block size of 16 bytes assumed here.
		//
		f_enc_blk(&alg, input+i, output_encrypted+i); 
		oblk("// encrypt = ", output_encrypted+i, 16);
	}

	return 0;
}

int soe_decrypt(unsigned char *input_encrypted, unsigned char *output, int length)
{
	f_ctx   alg;
    alg.n_blk = 0;
    alg.n_rnd = 0;
	//
	// Use the key to build the algorithm tables.
	//
    f_dec_key(&alg, exh, 16); 
    oblk("\n// key     = ", exh, 16); 

	//
	// Go through the input block and decode 16 bytes at a time.	
	//	
	for(int i = 0; i < length; i += 16)
	{
	    oblk("// input   = ", input_encrypted+i, 16);
		//
		// Decode the input block.  Put result in output block.  
		// Fixed block size of 16 bytes assumed here.
		//     
		f_dec_blk(&alg, input_encrypted+i, output+i); 
		oblk("// decrypt = ", output+i, 16);
	}


	printf("\n\n// output string = %s\n", output);

	return 0;
}
