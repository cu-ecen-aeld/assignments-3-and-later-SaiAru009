#!/bin/bash
# Author: Sai Arun

if [ $# -lt 2 ]
then
	exit 1
elif [ -d $1 ]
then
	x=$(find $1 -type f | wc -l)
	y=$(find $1 -type f -exec grep $2 {} \; | wc -l)
	echo "The number of files are ${x} and the number of matching lines are ${y}"
else
	exit 1
fi
