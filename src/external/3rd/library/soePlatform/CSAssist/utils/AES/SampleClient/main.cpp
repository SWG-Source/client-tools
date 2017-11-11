
#include "soeaes.h"

void dump_hex(unsigned char *hex, int numBytes);
int getline(char *line, int max);

int main()
{
	char input[256];
	char encrypted[256];
	char result[256];
	int input_length;
	SoeAes encryptor;

	memset(input, 0, 256);
	memset(encrypted, 0, 256);
	memset(result, 0, 256);
#if 1
	printf("Enter your string to encrypt: ");
	while(getline(input, 256) == 0)
		printf("Try again! Enter your string to encrypt: ");
#else
	strcpy(input, "Hello there, what is your name?  What a wonderful day is it!");
#endif
	input_length = strlen(input)+1;
	//
	// Round up the length to be a multiple of 16 bytes.
	// This is required for optimized encryption at compile time.
	//
	int rv = input_length % 16;
	if(rv != 0)
		input_length = input_length + (16 - rv);

	//
	// Display the input byte array.
	//
	printf("INPUT STRING = %s\n", input);
	printf("INPUT LENGTH = %d\n", input_length);
	printf("INPUT HEX = \n");
	dump_hex((unsigned char *)input, input_length);
	

	//
	// Encrypt the input byte array.
	//
	encryptor.encrypt((unsigned char *)input, (unsigned char *)encrypted, input_length);
	//
	// Display the encrypted byte array.
	//
	printf("\nENCRYPTED HEX = \n");
	dump_hex((unsigned char *)encrypted, input_length);


	//
	// Decrypt the byte array.
	//
	encryptor.decrypt((unsigned char *)encrypted, (unsigned char *)result, input_length);
	//
	// Display the decrypted byte array.
	// This should match the original input byte array.
	//
	printf("\nRESULT STRING = %s\n", result);
	printf("RESULT HEX = \n");
	dump_hex((unsigned char *)result, input_length);

	return 0;
}



void dump_hex(unsigned char *hex, int numBytes)
{
	for(int i = 1; i <= numBytes; i++)
	{
		printf("%02x", *(hex+i-1));
		if((i % 30) == 0)
			printf("\n");
	}
	printf("\n");
}

int getline(char *line, int max)
{
	if(fgets(line, max, stdin) == NULL)
		return 0;
	else
		return strlen(line);
}