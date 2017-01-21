SAMPLE VECTORS FOR VIGENERE LAB
-------------------------------

INTRODUCTION
------------

This archive contains sample input, keys, output and a testing script
to perform encoding/decoding for each input and key combination.

Contents:
	input/		-- sample inputs
	keys/		-- sample keys
	correct/	-- correctly encoded inputs by key
	ENCODED/	-- output directory for test encodings
	DECODED/	-- output directory for test decodings
	checksums/	-- files for integrity testing
	log/		-- output from encoder/decoder
	test.sh		-- runs tests

INSTRUCTIONS
------------

Copy your vigenere.c file into this directory. Run './test.sh'. This
will compile your code to generate encoder and decoder and run all
combinations of inputs and keys. Encoded data is written into ENCODED/
and compared against the correct version in correct/. Decoded data is
written into DECODED/ and compared against the original input in
input/.  Standard output and error for your programs are written to
files in log/.

A special integrity test will verify that your code has not
accidentally modified the test data.

Successes and failures are counted (including compilation) and results
are printed at the end.

See './test.sh -h' for special options, including disabling integrity
testing.

QUESTIONS / PROBLEMS?
---------------------

Contact your instructor or TA immediately!
