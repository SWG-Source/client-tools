
#include <string.h>
#include <stdio.h>
#include <exception>

#include "aes.h"
#include "aestst.h"
#include "soeaes.h"




SoeAes::SoeAes()
{
    useDefaultKeys();  
}

SoeAes::SoeAes(unsigned char *pKeyBytes, int numBytesInKey)
{
    alg_in.n_blk = 0;
    alg_in.n_rnd = 0;
    alg_out.n_blk = 0;
    alg_out.n_rnd = 0;

    // ---------------------------------------------------------
    // Confirm the argument key length is either 16, 24, or 32.
    // If not, use the default keys.
    // ---------------------------------------------------------
    if((numBytesInKey != 16) && (numBytesInKey != 24) && (numBytesInKey != 32))
    {
        useDefaultKeys();
    }
    else
    {
        keyByteLength = numBytesInKey;
        memcpy(exh, pKeyBytes, keyByteLength);

        // ------------------------------------------
        // Use the key to build the algorithm tables.
        // ------------------------------------------
        f_enc_key(&alg_in, exh, keyByteLength);
        f_dec_key(&alg_out, exh, keyByteLength);
    }
}


void SoeAes::useDefaultKeys(void)
{
    alg_in.n_blk = 0;
    alg_in.n_rnd = 0;
    alg_out.n_blk = 0;
    alg_out.n_rnd = 0;

	
    unsigned char default_key[32] =  
    {
       0xe2, 0x11, 0xd5, 0xba, 0x30, 0xe9, 0x44, 0xf6,
       0xd1, 0x8a, 0xf0, 0xc8, 0xc6, 0x5d, 0xcf, 0x1c,
       0x76, 0x2e, 0x71, 0x60, 0xf3, 0x8b, 0x4d, 0xa5,
       0x6a, 0x78, 0x4d, 0x90, 0x45, 0x19, 0x0c, 0xfe
    };

    memcpy(exh, default_key, 32);
    keyByteLength = 16; // default to use 16 byte key.  This can be changed to either 24 or 32.  

    // ------------------------------------------
    // Use the key to build the algorithm tables.
    // ------------------------------------------
    f_enc_key(&alg_in, exh, keyByteLength);
    f_dec_key(&alg_out, exh, keyByteLength);  
}


int SoeAes::encrypt(unsigned char *input, unsigned char *output_encrypted, int length)
{
	//
	// In order to optimize the encryption for speed, AES highly recommends
	// using fixed block size.  Therefore, the length must be a multiple of 16.
	//
	if((length % 16) != 0)
		return 1;

	//
	// Go through the input block and encode 16 bytes at a time.	
	//
	for(int i = 0; i < length; i += 16)
	{
		//
		// Encode the input block.  Put result in output block.  
		// Fixed block size of 16 bytes assumed here.
		//
		f_enc_blk(&alg_in, input+i, output_encrypted+i); 
	}

	return 0;
}

int SoeAes::decrypt(unsigned char *input_encrypted, unsigned char *output, int length)
{
	//
	// In order to optimize the encryption for speed, AES highly recommends
	// using fixed block size.  Therefore, the length must be a multiple of 16.
	//
	if((length % 16) != 0)
		return 1;
	
	//
	// Go through the input block and decode 16 bytes at a time.	
	//	
	for(int i = 0; i < length; i += 16)
	{
		//
		// Decode the input block.  Put result in output block.  
		// Fixed block size of 16 bytes assumed here.
		// 		
		f_dec_blk(&alg_out, input_encrypted+i, output+i); 
	}

	return 0;
}

// -----------------------------------------------------------------------------
// This is a utility function used to convert from ascii hex to an array
// of binary bytes.
//
// Example:  Once we encrypted the credit card number, we had to convert the encrypted
// binary into a string of hex characters before inserting into the database field 
// defined as VARCHAR.
//
// Now when we read the encrypted string from the VARCHAR database field, we have to convert 
// the hex string back into an array of binary bytes.  THIS IS WHAT THIS FUNCTION DOES.
// ----------------------------------------------------------------------------
int SoeAes::convertHexStringToHexArray(char *hexString, unsigned char *hexArrayOut, 
									   int maxArrayLength)
{
	//
	// Convert the hex string into a hex array of bytes.
	//
	int keyLength = 0;
	
	int aesEncryptedKeyStringLen = strlen(hexString);
	if(aesEncryptedKeyStringLen > maxArrayLength)
		aesEncryptedKeyStringLen = maxArrayLength;

	aesEncryptedKeyStringLen = aesEncryptedKeyStringLen * 2;

	for(int i = 0; i < aesEncryptedKeyStringLen; i += 2)
	{
		unsigned char byte = 0;
		char nextHexValue[2];
		memcpy(nextHexValue, &hexString[i], 2);				

		byte = asciiHexToInt(nextHexValue[0]) * 16;
		byte += asciiHexToInt(nextHexValue[1]);		

		hexArrayOut[keyLength] = byte; // convert 2 char string into 1 hex byte value;
		keyLength++;
	}

	return keyLength;
}


//
//
// NOTE:
//	The hexStringOut must be greater than or equal to twice the hexArrayLen plus 1.
//
void SoeAes::convertHexArrayToHexString(unsigned char *hexArray, int hexArrayLen, 
									   char *hexStringOut)
{	
	for(int i = 0; i < hexArrayLen; i++)
		sprintf(&hexStringOut[i*2], "%02x", hexArray[i]);

	hexArray[hexArrayLen*2] = 0; // NULL terminate the string
}


int SoeAes::asciiHexToInt(char c)
{
	int byte = 0;

	switch(c)
	{
	case '0':
		byte = 0;
		break;
	case '1':
		byte = 1;
		break;
	case '2':
		byte = 2;
		break;
	case '3':
		byte = 3;
		break;
	case '4':
		byte = 4;
		break;
	case '5':
		byte = 5;
		break;
	case '6':
		byte = 6;
		break;
	case '7':
		byte = 7;
		break;
	case '8':
		byte = 8;
		break;
	case '9':
		byte = 9;
		break;

	case 'a':
	case 'A':
		byte = 10;
		break;
	case 'b':
	case 'B':
		byte = 11;
		break;
	case 'c':
	case 'C':
		byte = 12;
		break;
	case 'd':
	case 'D':
		byte = 13;
		break;
	case 'e':
	case 'E':
		byte = 14;
		break;
	case 'f':
	case 'F':
		byte = 15;
		break;
	}

	return byte;
}