

#ifndef SOE_AES_H
#define SOE_AES_H


#include <string.h>
#include <stdio.h>

#include "aes.h"
#include "aestst.h"



class SoeAes 
{
	f_ctx alg_in;				// AES algorithm tables (encrypt)
	f_ctx alg_out;				// AES algorithm tables (decrypt)

	unsigned char exh[32];	// store 16, 24, or 32 byte key
	int keyByteLength;		// number of bytes in key(should be 16, 24, or 32)

	void useDefaultKeys(void);
	int asciiHexToInt(char c);

public:
	//
	// The constructor without any argument will use a default 16 byte key.
	// You can optionally specify a 16, 24, or 32 byte key to use.
	//
	SoeAes();
	SoeAes(unsigned char *pKeyBytes, int numBytesInKey);
	~SoeAes(){}

	//
	// Pass in byte array, and length of byte array to encrypt or decrypt.
	// The length field is number of bytes in the array.
	//
	// The length must be a multiple of 16 in order to optimize the algorithm for speed.
	//
	// The input array will be encrypted or decrypted and the result will
	// be returned in the output array.  These routines do not allocate any
	// memory.  The output array must be provided by the client making the call.
	//
	int encrypt(unsigned char *input, unsigned char *output_encrypted, int length);
	int decrypt(unsigned char *input_encrypted, unsigned char *output, int length);


	//
	// Utility routines (completely optional)
	//
	// Some routines are useful if storing an encrypted string in a VARCHAR 
	// database field.
	//
	int convertHexStringToHexArray(char *hexString, unsigned char *hexArrayOut, 
								   int maxArrayLength);

	void convertHexArrayToHexString(unsigned char *hexArray, int hexArrayLen, 
	 							    char *hexStringOut);

};

#endif
