int main(int, char **)
{
    unsigned char bytes[] = { 0x01, 0x02, 0x03, 0x04 };
    unsigned int *integer = (unsigned int *) bytes;
    return (*integer == 0x01020304 ?
	    0 : // big endian
	    1);  // little endian
}
