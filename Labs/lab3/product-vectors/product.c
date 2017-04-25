#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define TRANS 1
#define NOTRANS 0

#ifndef DEBUG
	#define DEBUG 0
#endif 

#define ENCODE 0
#define DECODE 1

#ifndef MODE
	#define MODE ENCODE
#endif


int print_buffer(char *buf, unsigned int bytes) {
	/* takes in a pointer to a buffer and prints out as many
	 * bytes as specified */

	/* insert code here */
	for (int i = 0; i < bytes; ++i)
	{
		printf("%c", buf[i]);
	}

	return bytes;

}

int dump_buffer(char *buffer, unsigned int bufsize, 
				unsigned int bytes, char *output) {

	/* prints a buffer one character at a time to a file using %c
	 * takes in:
	 *  buffer -- pointer to a buffer
	 *  bufsize -- size of 'buffer'
	 *  bytes -- number of bytes from buffer to print
	 *  output -- path to the file to open and output to
	 */
	
	/* open the output or quit on error */
	FILE *OUTPUT;
   	if ((OUTPUT = fopen(output, "ab")) == NULL) {
		printf("Problem truncating output file '%s'; errno: %d\n", output, errno);
		return 1;
	}
	/* print 'bytes' bytes from buffer to output file one char at a time */
	for (int i = 0; i < bytes; ++i)
	{
		fprintf(OUTPUT, "%c", buffer[i]);
	}
	/* optional: wipe buffer using memset */
	memset(buffer, 0, bufsize);
	/* close output file */
	fclose(OUTPUT);

	return bytes;

}


int pad_buffer(char *buffer, unsigned int bufsize, unsigned int rbuf_index) {


	/* pad_buffer pads the empty space in a buffer
	 *  buffer -- pointer to buffer
	 *  bufsize -- size of 'buffer'
	 *  rbuf_index -- first "empty" spot in buffer, i.e.,
	 *                put the 'X' at rbuf_index and fill the
	 *                rest with 'Y' characters.
	 */

	int padded = 0;

	/* code goes here */
	/* special case where rbuf_index is currently at last position of array, need to dump contents before padding
		otherwise we can't accomodate 'XY' --i.e. the minimum amount of padding required to signal the 
		end of the plaintext */
	// if( (rbuf_index % bufsize) == (bufsize - 1)){
	// 		buffer[rbuf_index] = 'X';
	// 		rbuf_index++;
	// 		padded++

	// 	}
	buffer[rbuf_index] = 'X';
	rbuf_index++;
	padded++;

	for (int i = rbuf_index; i < bufsize; ++i)
	{
		buffer[i] = 'Y';
		padded++;
	}


	return padded;

}
	
int unpad_buffer(char *buffer, unsigned int bufsize) {

	/* unpads a buffer of a given size
	 *  buffer -- buffer containing padded data
	 *  bufsize -- size of 'buffer'
	 */

	int unpadded = 0;

	for (int i = bufsize -1; i > 0; i--)
	{
		/* iterate through buffer until we find the beginning of the padding 'X' 
		*	then return the index 1 before that location */
		if (buffer[i] == 'X')
		{
			return --i;
		}
		unpadded++;
	}
	//Should already have found its location in the upper loop
	return unpadded;

}

int transpose_buffer(char *out, char *in, unsigned int dim) {
	/* do a columnar encipher/decipher
	 * from in to out
	 * using box of size dim*dim
	 * since it's a square, enciphering and deciphering is the same
	 */

	/* your elegant code goes here */
	unsigned int size = dim * dim;
	unsigned int position = 0;
	for (int i = 0; i < size; ++i)
	{
		position = (i % dim) * dim + (i / dim);
		out[position] = in[i];
	}

	return 0;

}
void vigenere_buffer(char *buffer, char *keyData, unsigned int keyLegth, unsigned int bytes, int mode);

void product_buffer(char *outBuf, char *inBuf, unsigned int dim, char *keyData, 
					unsigned int keyLegth, unsigned int vig_bytes, int mode, unsigned int rounds);

void correct_buffers(char *inBuf, char *outBuf, unsigned int bufsize);

int main(int argc, char *argv[]) {

	int i = 0; /* iterator we'll reuse */

	if (argc < 5) {
		printf("Missing arguments!\n\n");
		printf("Usage: encoder dim infile outfile ['notrans']\n\n");
		printf("Note: outfile will be overwritten.\n");
		printf("Optional '1' as last parameter will disable transposition.\n");
		return 1;
	}

	/* give input and output nicer names */
	unsigned int rounds = atoi(argv[1]); 	/* number of rounds to do */
	char *keyData = argv[2];				/* key file path */
	char *input = argv[3]; 				/* input file path */
	char *output = argv[4];				/* output file path */
	unsigned int keyLegth = 0;			/* length of keyData */

	/* use 'transmode' to determine if we are just padding or also
	 * doing transposition. very helpful for debugging! */

	unsigned int transmode = TRANS;		/* default is TRANS */
	if (argc > 4 && (atoi(argv[4]) == 1)) {
		printf("Warning: Transposition disabled\n");
		transmode = NOTRANS;
	}

	// unsigned int rbuf_count = 0;
	const unsigned int dim = 4;
	unsigned int bufsize = dim * dim;
	char read_buf[bufsize]; /* buffer for reading and padding */
	char write_buf[bufsize]; /* buffer for transposition */
	memset(read_buf, 0, bufsize);
	memset(write_buf, 0, bufsize);
	// memset(keyData, 0, bufsize);

	FILE *KEYINPUT;
	if ((KEYINPUT = fopen(keyData, "r")) == NULL)
	{
		printf("Problem opening key file '%s'; errno: %d\n", input, errno);
		return 1;
	}

	/* open the input or quit on error. */
	FILE *INPUT;
   	if ((INPUT = fopen(input, "r")) == NULL) {
		printf("Problem opening input file '%s'; errno: %d\n", input, errno);
		return 1;
	}


	/* get length of input file */
	unsigned int filesize;		/* length of file in bytes */
	unsigned int bytesleft;		/* counter we reduce on reading */
	struct stat filestats;		/* struct for file stats */
	int err;
	if ((err = stat(input, &filestats)) < 0) {
		printf("error statting file! Error: %d\n", err);
	}

	filesize = filestats.st_size;
	bytesleft = filesize;

	if (DEBUG) printf("Size of 'input' is: %u bytes\n", filesize);

	/* truncate output file if it exists */

	FILE *OUTPUT;
   	if ((OUTPUT = fopen(output, "w+")) == NULL) {
		printf("Problem truncating output file '%s'; errno: %d\n", output, errno);
		return 1;
	}
	fclose(OUTPUT);	/* file is reopened and reclosed for in dump_buffer() */

	/* we will read each input byte into 'symbol' */
	int symbol; 

	/* ******KEY FILE INPUT ****** */
	/*loop through the key file, read into a buffer until EOF or reached 16 bytes */
	while((symbol = (int)fgetc(KEYINPUT)) != EOF){
		keyData[keyLegth] = symbol;
		keyLegth++;
	}

	/* loop through the input file, reading into a buffer and 
	 * processing the buffer when 1) the buffer is full or
	 * 2) the file has ended (or in the case of decoding, when
	 * the last block is being processed.
	 */

	int rbuf_index = 0; /* index into the read buffer */
	
	/******************
	 *  do stuff here *
	 ******************/
	
	if (MODE == ENCODE)
	{
		while((symbol = (int)fgetc(INPUT)) != EOF){
			read_buf[rbuf_index % bufsize] = symbol;
			rbuf_index++;

			if (rbuf_index % bufsize == 0)
			{
				/* This Section executes only when our read buffer is 'full' */
				product_buffer(write_buf, read_buf, dim, keyData, keyLegth, bufsize, MODE, rounds);

				/* after performing shifts dump buffer before next cycle */
				dump_buffer(write_buf, bufsize, bufsize, output);
				// rbuf_index = 0;
			}
		}
		/* EOF reached, pad buffer, transpose it and finally dump it */
		// if ( (rbuf_index % bufsize) == (bufsize - 1) )
		// {
		// 	/* code */
		// }
		pad_buffer(read_buf, bufsize, (rbuf_index % bufsize));

		product_buffer(write_buf, read_buf, dim, keyData, keyLegth, bufsize, MODE, rounds);
		// for (int i = 0; i < rounds; ++i)
		// {
		// 	vigenere_buffer(read_buf, keyData,keyLegth, bufsize, MODE);
		// 	transpose_buffer(write_buf, read_buf, dim);
		// }
		// void product_buffer(outBuf, inBuf, dim, keyData, keyLegth, vig_bytes, mode, rounds);
		dump_buffer(write_buf, bufsize, bufsize, output);
	}
	else if(MODE == DECODE){
		rbuf_index = 0;
		unsigned int blocks = (int) bytesleft / bufsize;
		unsigned int written_blocks = 0;
		while((symbol = (int)fgetc(INPUT)) != EOF){
			read_buf[rbuf_index % bufsize] = symbol;
			rbuf_index++;
			if ((rbuf_index % bufsize == 0) && ((rbuf_index / bufsize) < blocks))
			{
				// EXPECTED VALUES: product_buffer(outBuf, inBuf, dim, keyData, keyLegth, vig_bytes, mode, rounds);
				product_buffer(write_buf, read_buf, dim, keyData, keyLegth, bufsize, MODE, rounds);
				// for (int i = 0; i < rounds; ++i)
				// {
				// 	transpose_buffer(write_buf, read_buf, dim);
				// 	vigenere_buffer(read_buf, keyData,keyLegth, bufsize, MODE);
				// }
				dump_buffer(write_buf, bufsize, bufsize, output);
				written_blocks++;
				// rbuf_index = 0;
			}
		}
		
		/* Exited first while loop because we are out of input 
		*  transpose the buffer, unpad it, and finally dump the buffer */
		product_buffer(write_buf, read_buf, dim, keyData, keyLegth, bufsize, MODE, rounds);
		// for (int i = 0; i < rounds; ++i)
		// {
		// 	//decode style
		// 	transpose_buffer(write_buf, read_buf, dim);
		// 	vigenere_buffer(read_buf, keyData,keyLegth, bufsize, MODE);
		// }

		unsigned int unpadded = unpad_buffer(write_buf, bufsize);
		dump_buffer(write_buf, bufsize, (unpadded % bufsize), output);
	}

	fclose(INPUT);

	return 0;/* END MAIN */
}

void vigenere_buffer(char *buffer, char *keyData, unsigned int keyLegth, unsigned int bytes, int mode){

	if (MODE == DECODE) {
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

		for (int i = 0; i < bytes; ++i)
		{
			buffer[i] = (int)((buffer[i] - keyData[i % keyLegth]) % 256);
		}

	}
	else if (MODE == ENCODE){
		/***************************
		 * OK, LET'S GET ENCODING! *
		 ***************************/

		for (int i = 0; i < bytes; ++i)
		{
			buffer[i] = (int)((buffer[i] + keyData[i % keyLegth]) % 256);
		}
	}

}

void correct_buffers(char *inBuf, char *outBuf, unsigned int bufsize){
	/* Function used to correct the contents of product_buffer() function in the event that the number of rounds were odd 
		which would cause the final rounds output to have been written to the inBuf.  This function copies the contents of inBuf to 
		outBuf so that the functions back in Main() that occur after product_buffer() have the proper contents i.e. the outBuf */
	for (int i = 0; i < bufsize; ++i)
	{
		outBuf[i] = inBuf[i];
	}
}

void product_buffer(char *outBuf, char *inBuf, unsigned int dim, char *keyData, 
					unsigned int keyLegth, unsigned int vig_bytes, int mode, unsigned int rounds){
	typedef enum { false, true } bool;
	bool lastOdd; // used to indicate whether our last round wrote to the input buffer, if so inbuf contents need to be copied to outBuf
	if (mode == ENCODE)
	{
		for (int i = 0; i < rounds; ++i)
		{
			/* ENCODE section performs the vigenere shift, followed by the transposition, 
				for the number of rounds specified during the function call */
			if ( (i % 2) == 0)
			{
				vigenere_buffer(inBuf, keyData, keyLegth, vig_bytes, mode);
				transpose_buffer(outBuf, inBuf, dim);
				lastOdd = false;
			}
			else// i % 2 == 1
			{
				vigenere_buffer(outBuf, keyData, keyLegth, vig_bytes, mode);
				transpose_buffer(inBuf, outBuf, dim);
				lastOdd = true;
			}
			
		}
		if (lastOdd) //ended on an odd # of rounds, correct the buffer contents before leaving product_buffer()
		{
			correct_buffers(inBuf, outBuf, vig_bytes);
		}
	}
	else
	{
		for (int i = 0; i < rounds; ++i)
		{
			/* DECODE section performs the transposition, followed by vigenere UNSHIFT,
				for the number of rounds specified during the function call */
			if ( (i % 2) == 0)
			{
				transpose_buffer(outBuf, inBuf, dim);
				vigenere_buffer(outBuf, keyData, keyLegth, vig_bytes, mode);
				lastOdd = false;
			}
			else // i % 2 == 1
			{
				transpose_buffer(inBuf, outBuf, dim);
				vigenere_buffer(inBuf, keyData, keyLegth, vig_bytes, mode);
				lastOdd = true;
			}
		}
		if (lastOdd)
		{
			correct_buffers(inBuf, outBuf, vig_bytes);
		}
	}
}
/*  BELOW ARE FUNCTION REFERENCES ONLY  */
// void product_buffer(trans_outBuf, trans_inBuf, dim, vig_buffer, keyData, keyLegth, vig_bytes, mode, rounds);
		/*for (int i = 0; i < rounds; ++i)
		{
			//decode style
			transpose_buffer(write_buf, read_buf, dim);
			vigenere_buffer(read_buf, keyData,keyLegth, bufsize, MODE);
		}*/
		/*for (int i = 0; i < rounds; ++i)
		{
			//encode style
			vigenere_buffer(read_buf, keyData,keyLegth, bufsize, MODE);
			transpose_buffer(write_buf, read_buf, dim);
		}*/
