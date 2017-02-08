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

int transpose_buffer(char *out, char *in, unsigned int dim) {
	/* do a columnar encipher/decipher
	 * from in to out
	 * using box of size dim*dim
	 * since it's a square, enciphering and deciphering is the same
	 */

	/* your elegant code goes here */
	unsigned int size = dim * dim;
	int position = 0;
	for (int i = 0; i < size; ++i)
	{
		position = (i % dim) * dim + (i / dim);
		out[position] = in[i];
	}
	return 0;

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
	if((OUTPUT = fopen(output, "a")) ==NULL){
		printf("Problem opening output file '%s'; errno: %d\n", output, errno);
		return 1;
	}
	/* print 'bytes' bytes from buffer to output file one char at a time */
	for (int i = 0; i < bytes; ++i)
	{
		fprintf(OUTPUT, "%c", buffer[i]);
	}
	/* optional: wipe buffer using memset */
	memset(buffer, '\0', bufsize);
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
	if(rbuf_index < bufsize){
		buffer[rbuf_index] = 'X';
		rbuf_index++;
		padded++;
		while(rbuf_index < bufsize){
			buffer[rbuf_index] = 'Y';
			rbuf_index++;
			padded++;
		}
	}
	return padded;
}
	
int unpad_buffer(char *buffer, unsigned int bufsize) {

	/* unpads a buffer of a given size
	 *  buffer -- buffer containing padded data
	 *  bufsize -- size of 'buffer'
	 */

	int unpadded = 0;
	unsigned int rbuf_index = bufsize -1;
	/* code goes here */
	while((buffer[rbuf_index] == ('Y' || 'X')) && (rbuf_index >= 0))
	{
		buffer[rbuf_index] = '\0';
		unpadded++;
		rbuf_index--;
	}

	return unpadded;

}
	
int main(int argc, char *argv[]) {

	int i = 0; /* iterator we'll reuse */

	if (argc < 4) {
		printf("Missing arguments!\n\n");
		printf("Usage: encoder dim infile outfile ['notrans']\n\n");
		printf("Note: outfile will be overwritten.\n");
		printf("Optional '1' as last parameter will disable transposition.\n");
		return 1;
	}

	/* give input and output nicer names */
	unsigned int dim = atoi(argv[1]); 	/* dimension of the box */
	char *input = argv[2]; 				/* input file path */
	char *output = argv[3];				/* output file path */

	/* use 'transmode' to determine if we are just padding or also
	 * doing transposition. very helpful for debugging! */

	unsigned int transmode = TRANS;		/* default is TRANS */
	if (argc > 4 && (atoi(argv[4]) == 1)) {
		printf("Warning: Transposition disabled\n");
		transmode = NOTRANS;
	}

	unsigned int rbuf_count = 0;
	unsigned int bufsize = dim * dim;
	char read_buf[bufsize]; /* buffer for reading and padding */
	char write_buf[bufsize]; /* buffer for transposition */

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


	/* loop through the input file, reading into a buffer and 
	 * processing the buffer when 1) the buffer is full or
	 * 2) the file has ended (or in the case of decoding, when
	 * the last block is being processed.
	 */

	int rbuf_index = 0; /* index into the read buffer */
	int symbol; /* we will read each input byte into 'symbol' */

	/******************
	 *  do stuff here *
	 ******************/
	if (MODE == ENCODE)
	{
		i = 0;
		while(symbol = fgetc(INPUT) != EOF)
		{
			if (((rbuf_index % bufsize) == 0) && i != 0)
			{
				/* read buffer is full, transpose and call dump buffer to write the block */
				transpose_buffer(write_buf, read_buf, bufsize);
				dump_buffer(read_buf, bufsize, bufsize, write_buf);
			}
			read_buf[rbuf_index % bufsize] = symbol;
			rbuf_index++;
			bytesleft--;
			i++;
		}
		unsigned int padded = pad_buffer(read_buf, bufsize, (rbuf_index % bufsize));
		transpose_buffer(write_buf, read_buf, bufsize);
		dump_buffer(read_buf, bufsize, bufsize, write_buf);
	}
	else{//MODE == DECODE
		unsigned int num_Blocks = bytesleft / bufsize;
		i = 0;
		while(num_Blocks > 1 && (symbol = fgetc(INPUT) != EOF))
		{
			if (((rbuf_index % bufsize) == 0) && i != 0)
			{
				/* read buffer is full, transpose and call dump buffer to write the block */
				transpose_buffer(write_buf, read_buf, bufsize);
				dump_buffer(read_buf, bufsize, bufsize, write_buf);
				num_Blocks--;
			}
			read_buf[rbuf_index % bufsize] = symbol;
			rbuf_index++;
			bytesleft--;
			i++;
		}
		i = 0;
		while(bytesleft > 0)
		{
			symbol = fgetc(INPUT);
			/* read in last block of data before tranposing, unpadding and writing */
			if (((rbuf_index % bufsize) == 0) && i != 0)
			{
				/* last buffer is full,  transpose and unpad, then write out */
				transpose_buffer(write_buf, read_buf, bufsize);
				unsigned int unpadded = unpad_buffer(read_buf, bufsize);
				dump_buffer(read_buf, bufsize, (bufsize - unpadded), write_buf);
			}
			read_buf[rbuf_index % bufsize] = symbol;
			rbuf_index++;
			bytesleft--;
			i++;
		}
	}

	fclose(INPUT);

	return 0;

}


