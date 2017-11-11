//From http://www.gueck.com/s.a.o/

/*----------------------------------------------------------
* gumi_atoi.h
*
* Public Domain by Mikael Gueck <gumi@iki.fi>
* 2002-07-24
*
* My own implementation of atoi, just for the fun of it.
*
* Compile with: 
* -fstack-check -Wall -Wshadow -Wwrite-strings \
* -W -Wpointer-arith -pedantic -Wcast-align -Wstrict-prototypes
*
*/

#ifndef GUMI_ATOI_H
#define GUMI_ATOI_H

#define STRUCT_ELEMENTS(a,b) sizeof(a)/sizeof(struct b)

/*----------------------------------------------------------
* These characters are valid input, we don't stop on these.
*/
static struct valid_chars_struct {
	char c;
	int i;
	int isnum;
} valid_chars[] = {
	{'0', 0, 1},
	{'1', 1, 1},
	{'2', 2, 1},
	{'3', 3, 1},
	{'4', 4, 1},
	{'5', 5, 1},
	{'6', 6, 1},
	{'7', 7, 1},
	{'8', 8, 1},
	{'9', 9, 1},
	{'+', 0, 0},
	{'-', 0, 0},
};

/*----------------------------------------------------------
* Check whether we should stop on this character (return 0)
* or continue (return 1).
*/
int is_valid_char(char c) {
	unsigned int i;
	for(i=0; i<STRUCT_ELEMENTS(valid_chars, valid_chars_struct); i++) {
		if(c == valid_chars[i].c) {
			return 1;
		}
	}
	return 0;
}

/*----------------------------------------------------------
* Checks whether a character is in the base-10 number set.
* Return 1 if it is, and 0 if it's not.
*/
int is_number(char c) {
	unsigned int i;
	for(i=0; i<STRUCT_ELEMENTS(valid_chars, valid_chars_struct); i++) {
		if(c == valid_chars[i].c && valid_chars[i].isnum) {
			return 1;
		}
	}
	return 0;
}

/*----------------------------------------------------------
* Returns the integer number corresponding to a certain
* base-10 numeric character, including zero. Also returns
* zero if the character is not numeric, so be sure to check
* beforehand with is_number().
*/
int which_number(char c) {
	unsigned int i;
	for(i=0; i<STRUCT_ELEMENTS(valid_chars, valid_chars_struct); i++) {
		if(c == valid_chars[i].c) {
			return valid_chars[i].i;
		}
	}
	return 0;
}

int power_ten(int n, int p) {
	int i, ret = n;
	for(i=1; i<=p; i++) {
		ret *= 10;
	}
	return ret;
}

int gumi_atoi(char *str) {
	char *p;
	int i, ret;

	/* Find the first invalid character in the string. */
	p = str;
	while(*p != '\0' && is_valid_char(*p)) {
		p++;
	}

	/* Track back to the start while adding numbers. */
	i = 0;
	ret = 0;
	while(p > str) {
		p--;
		if(is_number(*p)) {
			ret += power_ten(which_number(*p), i);
		}
		i++;
	}

	/* Check whether the number was negative. */
	if(str[0] == '-') {
		ret *= -1;
	}

	return ret;
}

#endif