#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DEBUG 0

#define ENCODE 1 /* the macro ENCODE expands to the number 1 */
#define DECODE 0 /* DECODE expands to 0 */
#ifndef MODE 	 /* if the macro MODE wasn't defined during compilation... */
	#define MODE ENCODE	/* define it to be ENCODE */
#endif
	

int main(int argc, char *argv[]) {

	/* perform a simple caesear cipher
	 * -------------------------------
	 * accepts three arguments:
	 * 1. a shift value (integer)
	 * 2. a filename to draw input from
	 * 3. a filename to write output to
	 */

	int i = 0; /* iterator we'll reuse */

	if (DEBUG) {
		/* change DEBUG to 0 or remove this code */

		printf("This program is named '%s'\n", argv[0]);
		printf("There are %d arguments. They are:\n", argc);

		/* loop through, printing the name and value of each 
		 * argument string.
		 */

		for (i = 0; i <= argc; i++) {

			printf("\t%d --> %s\n", i, argv[i]);

		}
	}

	if (argc < 4) {
		printf("Missing arguments!\n\n");
		printf("Usage: caesar shift infile outfile\n\n");
		printf("Note: outfile will be overwritten.\n");
		return 1;
	}

	/* get program arguments */

	/* caesar needs a shift value */
	/* The first argument, 'argv[1]' is a string, not a number.
	 * To use it as our shift value, we must first convert it to an integer. */

	// int shift = atoi(argv[1]); /* atoi means "ASCII to integer" */
	// if (shift > 255 || shift < 0) {
	// 	printf("Value of shift must be between 0-255.\n");
	// 	exit(1);
	// }

	/* Note: vigenere instead takes a path to a key file rather than a shift
	 * value.  See the lab manual for instructions on how to read a file into a
	 * character array.
	 */

	char *keyFilePath = argv[1]; /* key file path */
	int keyData[128]; /* Array for holding key bytes, up to 128 */
	unsigned int keyLength = 0;  /* Length of keyData */

	char *input = argv[2]; /* input file name */
	char *output = argv[3]; /* output file name */

	/* Try to open Key file path to read in data or quit with error */
	FILE *KEYINPUT; 	/*Pointer to the key file being used */
	if((KEYINPUT = fopen(keyFilePath, "r")) == NULL){
		printf("Problem opening key file '%s'; errno: %d\n", keyFilePath, errno);
		return 1;
	}

	/* open the input or quit on error */
	FILE *INPUT;
   	if ((INPUT = fopen(input, "r")) == NULL) {
		printf("Problem opening input file '%s'; errno: %d\n", input, errno);
		return 1;
	}
	
	/* open the output or quit on error */
	FILE *OUTPUT;
   	if ((OUTPUT = fopen(output, "w+")) == NULL) {
		printf("Problem opening output file '%s'; errno: %d\n", output, errno);
		return 1;
	}

	int symbol; /* we will read each input byte into 'symbol' */
	char cipher; /* we will encode each output byte in 'cipher' */
	char plainText; /*used to store each output byte during decode operation */
	int curPosInKey = 0;  /* variable used to track position within keyData Array */

	/* read in keyFile info until 128 bytes 
	* or until the EOF is found	then
	* increment the keyLength variable		*/		
	while(keyLength < (128) && (symbol = fgetc(KEYINPUT)) != EOF ){	
		keyData[keyLength] = symbol;								
		keyLength++;												
	}

	if (MODE != ENCODE) {
		/*****CAESER SHIFT DECODE SECTION*****/
			/* mode is DECODE, so "unshift" the characters */
			/* shift = 256 - shift; */
		/*****END CAESER SHIFT SECTION**/

		/****VIGENERE DECODE SECTON****/
		/*  1. take the int value of the input 'symbol'
		*  2. subtract the int value of the 'key' from the 'symbol'
		*  3. then perform modulo arithmetic using 256 to wrap numbers back around if necessary
		*  4. cast the new value to char and store
		*/
		while((symbol = fgetc(INPUT)) != EOF){
			curPosInKey = curPosInKey % keyLength;
			plainText = (char)( (symbol - keyData[curPosInKey]) % 256);
			curPosInKey++;

			fprintf(OUTPUT, "%c", plainText);

		}

	}
	
	// if (DEBUG) {
	// 	printf("The shift value is %d\n", shift);
	// }
	if (MODE == ENCODE){
		/***************************
		 * OK, LET'S GET ENCODING! *
		 ***************************/

		/* read individual characters from INPUT until we hit the EOF */
		while ((symbol = fgetc(INPUT)) != EOF) {

			/* Do the Vigenere shift:
			*  1. take the integer value of the input 'symbol'
			*  2. add the integer value of the 'key' to the 'symbol'
			*  3. then perform modulo arithmetic using 256 to wrap numbers back around if necessary
			*  4. cast the new value to char and store into 'cipher' for output
			*/
			curPosInKey = curPosInKey % keyLength;
			cipher = (char)( (symbol + keyData[curPosInKey]) % 256 );
			curPosInKey++;


			/* Do the Caesar Shift:
			 * 1. take the integer value of 'symbol'
			 * 2. add the shift value
			 * 4. cast to char and store into 'cipher' 
			 */

			//cipher = (char)(symbol + shift); /* char is 8-bits so wraps automatically */

			if (DEBUG) {
				/* leaving this code turned on will make your program very slow! */
				printf("symbol is: %d == %02x became %02x\n", symbol, symbol, cipher);
			}

			/* write the number 'cipher' as a character to OUTPUT */
			fprintf(OUTPUT, "%c", cipher);

		}

		fclose(KEYINPUT);
		fclose(INPUT);
		fclose(OUTPUT);
	}
	return 0;

}
