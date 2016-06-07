#!/bin/bash
# Bail if no file passed in
if [ -z "$1" ] || [ -z "$2" ]; then
	echo "usage: upload.sh /path/to/ardunio/data_file.csv /path/to/problem/datafile.csv"
	exit -1
fi

# Read lines from stdin
while read line
do
	# Upload to the server
	# Assign the stdout of the curl command to variable 'result'
	# Rails server is expecting a POST with a csv_line parameter looking like (from the Rails log):
	# Parameters: {"csv_line"=>"2016-06-07 13:42:00,0.1,0.2,0.3,0.4"}
	result=`curl --silent --data csv_line="$line" http://winter-runoff.soils.wisc.edu/time_series_data/upload`
	if [ "$result" = "OK" ]; then
		# do OKAY-y things
	else
		# do error-handly things
		# Append to problem file
		echo $line >> $2
	fi
done < $1
