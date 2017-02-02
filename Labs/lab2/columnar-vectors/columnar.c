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
	unsigned int col_num = 0;
	unsigned int row_num = 0;
	unsigned int index = 0;
	const unsigned int NUM_COLS = dim;
	const unsigned int NUM_ROWS = dim;
	char temp_buffer[dim][dim];
	/* Transfer input buffer contents to temp_buffer(2D array)
	* for simpler transposition indexing */
	for (int y = 0; y < NUM_ROWS; ++y)
	{
		for (int x = 0; x < NUM_COLS; ++x)
		{
			temp_buffer[x][y] = in[index];
			index++;
		}
	}
	/* Transpose data in temp_buffer and write to 'out' buffer */
	index = 0;
	for (int x = 0; x < NUM_COLS; ++x)
	{
		for (int y = 0; y < NUM_ROWS; ++y)
		{
			out[index] = temp_buffer[x][y];
			index++;
		}
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
	if((OUTPUT = fopen(output, "w+")) ==NULL){
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
	/* first case, buffer isn't full fill remainder with single X followed by Y's */
    if(buffer[rbuf_index] == '\0'){
        buffer[rbuf_index] = 'X';
        rbuf_index++;
        padded++;

        while(rbuf_index < bufsize){
            buffer[rbuf_index] = 'Y';
            rbuf_index++;
            padded++;
        }
    } else{
        /* buffer is full i.e. buffer[rbuf_index] != null byte */
        printf("CODE SHOULD NOT REACH THIS POINT IN PAD_BUFFER!!\n\n");
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
	while(buffer[rbuf_index] == 'Y'){
		buffer[rbuf_index] = '\0';
		unpadded++;
		rbuf_index--;
	}
	if (buffer[rbuf_index] == 'X')
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
	char read_buf[bufsize];/* buffer for reading and padding */
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
	i = 0;
	while(bytesleft > 0){
		if((symbol = fgetc(INPUT)) != EOF){
			if (((rbuf_index % bufsize) == 0) && (read_buf[rbuf_index % bufsize] != '\0') && (i !=0) )
			{
				/* read buffer is full, and its not the first iteration,
				 * transpose and call dump buffer to write the block */
				transpose_buffer(write_buf, read_buf, bufsize);
				dump_buffer(read_buf, bufsize, bufsize, output);
			}
			read_buf[rbuf_index % bufsize] = symbol;
			rbuf_index++;
			bytesleft--;
			i++;
		}
		else{
			//reached the EOF signal, pad buffer, then transpose and finally dump buffer
			pad_buffer(read_buf, bufsize, (rbuf_index % bufsize));
			transpose_buffer(write_buf, read_buf, bufsize);
			dump_buffer(read_buf, bufsize, (rbuf_index % bufsize), output);
		}
	}

	fclose(INPUT);

	return 0;

}


