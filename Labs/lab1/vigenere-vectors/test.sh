#!/bin/bash

# test all inputs against all keys

NUMTEST=0
FAILS=0
SCORE=0
DO_COMPILE=1
DO_CHECKSUMS=1
QUIT_ON_ERROR=0
OPTIND=""
ENC="ENCODED"
DEC="DECODED"
GOLD="correct"

usage()
{

cat <<TEXT
test.sh -- test your encoder/decoder
Options:
	-x	Skip compilation step.
	-q	Quit on any error.
	-h	Print this screen.
	-c	Skip checksums check.

TEXT

exit 0

}

maybe_quit()
{
	if [[ $QUIT_ON_ERROR == 1 ]]
	then
		exit 1
	fi

}

make_checksums()
{
	DIR=$1
	echo "'$DIR'... "
	pushd $1 > /dev/null

	VERIFY="checksums/$DIR.verify"

	if [[ -f ../$VERIFY ]]
	then
		rm ../$VERIFY
	fi  

	for i in *
	do  
		md5sum $i >> "../$VERIFY"
	done

	popd > /dev/null

	if ! diff checksums/$DIR $VERIFY
	then
		echo
		echo "--------------------------------------------------"
		echo "                FATAL ERROR!"
		echo "--------------------------------------------------"
		echo "Files in $DIR do not match original test data!"
		echo "You should back up your encoder and decoder source"
		echo "code and re-extract the sample vectors tarball."
		echo "--------------------------------------------------"
		exit 1
	fi  
}

while getopts "xqhc" OPTION
do
	case $OPTION in
		x)
			DO_COMPILE=0
			echo "Skipping compilation by request."
			;;
		q)
			QUIT_ON_ERROR=1
			echo "Will quit on any error."
			;;
		c)
			DO_CHECKSUMS=0
			echo "Skipping integrity check."
			;;
		h)
			usage
			;;
	esac
done

if [[ $DO_CHECKSUMS == 1 ]]
then
	echo "Checking integrity of test data:"
	UNAME=$(uname)
	if [[ $UNAME == "Darwin" ]]
	then
		echo "It appears you are using a Mac/BSD."
		echo "This version of the md5 utility has an incompatible output."
		echo "Run these tests on a Linux computer, or disable integrity"
		echo "checking by running this script with the -c flag. See"
		echo "'./test.sh -h' or the README for more information."
		exit 1
	fi
	make_checksums input
	make_checksums keys
	make_checksums correct
	echo "Done."
fi

echo "Removing stale test data (if it exists)..."
rm $ENC/*
rm $DEC/*
rm log/*

BANNED="strcpy strcat strtok sprintf vsprintf gets strlen"

# check source code for banned functions
echo "Checking for banned functions: $BANNED"
for bfunc in $BANNED
do 
	if grep -n $bfunc vigenere.c &> /dev/null
	then
		echo "BANNED FUNCTION '$bfunc' DETECTED:"
		echo
		grep -n $bfunc vigenere.c
		echo
		echo "Remove all banned functions from your code and try again."
		exit 1
	fi
done

# automatically recompile unless -x option selected
if [[ $DO_COMPILE == 1 ]]
then
	if [[ ! -f vigenere.c ]]
	then
		echo "vigenere.c does not exist!"
		MISSING=1
		echo "'vigenere.c' is missing!"
		exit 1
	fi

	if ! gcc -g -std=c99 vigenere.c -o encoder -D MODE=ENCODE
	then
		echo ":-< : Failed to compile encoder! Quitting."
		exit 1
	else 
		echo ":-D : encoder compiled!"
	fi
	
	if ! gcc -g -std=c99 vigenere.c -o decoder -D MODE=DECODE
	then
		echo ":-< : Failed to compile decoder! Quitting."
		exit 1
	else
		echo ":-D : decoder compiled!"
	fi
fi

for FILEPATH in input/*
do
	FILENAME=${FILEPATH##input/} # get basename of file

	for KEYPATH in keys/*
	do
		KEYNAME=${KEYPATH##keys/} # get basename of key


		for LOGFILE in log/encoder.log log/encoder.err log/decoder.log log/decoder.err
		do
			echo "----- testing $FILEPATH + $KEYNAME -----" >> $LOGFILE
		done

		# TEST ENCODING
		
		NUMTEST=$((NUMTEST + 1))
		# we must quote the use of "$KEY" or spaces, binary
		# or other special characters in the key could confuse our 
		# program or the shell.
		COMMAND="./encoder $KEYPATH $FILEPATH $ENC/$FILENAME.enc.$KEYNAME"
		if ! $COMMAND >> log/encoder.log 2>> log/encoder.err
		then
			echo ":-< : Encoder returned non-zero (crashed?) on $FILEPATH + $KEYPATH!"
			echo -e "\tCommand: $COMMAND"
			maybe_quit
		fi

		# compare the sample encoded version with your version
		if ! diff $GOLD/$FILENAME.enc.$KEYNAME $ENC/$FILENAME.enc.$KEYNAME &> /dev/null
		then
			echo
			echo ":-( : Encoded file $ENC/$FILENAME.enc.$KEYNAME != $GOLD/$FILENAME.enc.$KEYNAME!"
			echo -e "\tCommand: $COMMAND"
			echo
			maybe_quit

			FAILS=$((FAILS + 1))
		else 
			echo ":-D : Encoded $FILEPATH + $KEYPATH matches sample. "
		fi

		# TEST DECODING
		
		NUMTEST=$((NUMTEST + 1))

		# see comment about about why "$KEY" is quoted
		COMMAND="./decoder $KEYPATH $ENC/$FILENAME.enc.$KEYNAME $DEC/$FILENAME.dec.$KEYNAME"
		if ! $COMMAND >> log/decoder.log 2>> log/decoder.err
		then
			echo ":-< : Decoder returned non-zero (crashed?) on $FILEPATH + $KEYPATH!"
			echo -e "\tCommand: $COMMAND"
			maybe_quit
		fi
		# compare the sample decoded version with your decoding
		if ! diff input/$FILENAME $DEC/$FILENAME.dec.$KEYNAME &> /dev/null
		then

			echo
			echo ":-( : Decoded file $DEC/$FILENAME.dec.$KEYNAME != input/$FILENAME!"
			echo -e "Command: \t$COMMAND"
			echo 

			maybe_quit
			FAILS=$((FAILS + 1))
		else
			echo ":-D : Decoded $FILEPATH + $KEYPATH matches original."
		fi

	done

done

SCORE=$((NUMTEST - FAILS))
echo "You passed $SCORE out of $NUMTEST sample tests."

