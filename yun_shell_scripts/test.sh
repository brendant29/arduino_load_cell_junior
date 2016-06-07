#!/bin/bash
# Bail if no file passed in
if [ -z "$1" ] || [ -z "$2" ]; then
	echo "usage: upload.sh /path/to/ardunio/data_file.txt /path/to/serially_complete_data_file.csv"
	exit -1
fi
