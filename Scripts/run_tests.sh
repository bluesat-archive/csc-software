#!/bin/bash
location=$1
files=`/usr/bin/find $location -name 'test_*.exe'`
output=$1/AutoTestResults.txt

if [ -e $output ]
then
	rm $output
fi
for x in $files
do
	filename=$(basename $x)
	echo  "Running test: $filename"
	$x
	echo  ""
	echo  "<Next Test>"
done >> $output

exit 0