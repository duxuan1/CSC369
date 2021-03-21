#!/bin/bash

GOLDENDIR=/student/cslec/369/assignments/A3-self-test
USER=`whoami`
TESTDIR=A3-self-test-dir
CURDIR=`pwd`

echo "CSC369 A3 Self-Tester Script"
echo "****************************"
echo "This self-tester allows you to confirm that the course team will be able to check out and compile your code. In addition, it also runs a subset of the test cases that will be used to grade your assignment."
echo ""
echo "Checking out SVN repo..."

# Checkout repo
git clone https://mcsmark.utm.utoronto.ca/git/csc369h_20209/repo/bare/duxuan1  "$TESTDIR"

if [ $? -ne 0 ]; then
	echo "Failed to check out repo!"
	exit 1
fi

cd $TESTDIR/A3

if [ ! -e "Makefile" ]; then
	echo "Didn't find Makefile in root testdir, trying /starter..."
	cd starter
fi

# Run make
echo "Running make..."
echo ""

make

if [ $? -ne 0 ]; then
	echo "Failed to compile! This submission will receive a mark of 0."
	cd $CURDIR
	rm -rf $TESTDIR
	exit 1;
fi

# Run traces and diff output
echo ""
echo "Make succeeded! Running traces and diffing output..."
echo ""

cp $GOLDENDIR/traces/* .

MAX=2
MAXTOTAL=10

TOTAL=0
for algo in rand arc fifo lru clock; do
	MARK=0
	for trace in 1_trace 4_trace; do
		./sim -f $trace -m 8 -s 12 -a $algo | tail --lines=7 > $trace.out
		diff -b $trace.out $GOLDENDIR/$algo/$trace.golden.out > /dev/null

		if [ $? -eq 0 ]; then
			MARK=$(($MARK+1))
		fi
	done
		
	if [ $algo = "rand" ]; then
		echo "Mark for pagetable implementation: $MARK/$MAX"
	else
		echo "Mark for $algo: $MARK/$MAX"
	fi
	TOTAL=$(($TOTAL+$MARK))
done

echo ""

echo "Total Mark: $TOTAL/$MAXTOTAL"

rm -rf *trace*

cd $CURDIR
rm -rf $TESTDIR
