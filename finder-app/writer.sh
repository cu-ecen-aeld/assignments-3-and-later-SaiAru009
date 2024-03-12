#!/bin/bash

if [ $# -lt 2 ]
then
	exit 1
else
	echo $2 > $1
fi

if [ ! -s $1 ]
then
	exit 1
fi
