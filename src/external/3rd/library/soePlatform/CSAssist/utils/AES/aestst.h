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

// The following definitions are required for testing only, They are not needed 
// for AES (Rijndael) implementation.  They are used to allow C, C++ and DLL 
// data access and subroutine calls to be expressed in the same form in the 
// testing code.

#define ref_path    "..\\testvals\\"                // path for test vector files
#define out_path    "..\\outvals\\"                 // path for output files
#define dll_path    "..\\aes_dll\\release\\aes"     // path for DLL

#if defined(AES_IN_CPP)

#define f_ctx               AESclass
#define f_blk_len(a,b)      (a)->blk_len((b))
#define f_enc_key(a,b,c)    (a)->enc_key((b),(c))
#define f_dec_key(a,b,c)    (a)->dec_key((b),(c))
#define f_enc_blk(a,b,c)    (a)->enc_blk((b),(c))
#define f_dec_blk(a,b,c)    (a)->dec_blk((b),(c))

#elif !defined(AES_DLL)

#define f_ctx               aes_ctx
#define f_blk_len(a,b)      aes_blk_len((b),(a))
#define f_enc_key(a,b,c)    aes_enc_key((b),(c),(a))
#define f_dec_key(a,b,c)    aes_dec_key((b),(c),(a))
#define f_enc_blk(a,b,c)    aes_enc_blk((b),(c),(a))
#define f_dec_blk(a,b,c)    aes_dec_blk((b),(c),(a))

#else

#define f_ctx   aes_ctx
typedef aes_rval g_blk_len(unsigned int, aes_ctx*);
typedef aes_rval g_enc_key(const unsigned char*, unsigned int, aes_ctx*);
typedef aes_rval g_dec_key(const unsigned char*, unsigned int, aes_ctx*);
typedef aes_rval g_enc_blk(const unsigned char*, unsigned char*, const aes_ctx*);
typedef aes_rval g_dec_blk(const unsigned char*, unsigned char*, const aes_ctx*);

typedef struct  // initialised with subroutine addresses when the DLL is loaded
{   g_blk_len    *fn_blk_len;
    g_enc_key    *fn_enc_key;
    g_dec_key    *fn_dec_key;
    g_enc_blk    *fn_enc_blk;
    g_dec_blk    *fn_dec_blk;
} fn_ptrs;

#define f_dat(a,b)          (a->b)
#define f_blk_len(a,b)      (fn.fn_blk_len)((b),(a))
#define f_enc_key(a,b,c)    (fn.fn_enc_key)((b),(c),(a))
#define f_dec_key(a,b,c)    (fn.fn_dec_key)((b),(c),(a))
#define f_enc_blk(a,b,c)    (fn.fn_enc_blk)((b),(c),(a))
#define f_dec_blk(a,b,c)    (fn.fn_dec_blk)((b),(c),(a))

#endif
