#!/bin/bash
# Bail if no files passed in
if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]; then
	echo "usage: process_datafiles.sh /path/to/ardunio/data_folder /path/to/serially_complete_archive.csv /path/to/problem/datafile.csv {/path/to/archive/folder}"
	exit -1
fi
arduino_folder=$1
archive_file=$2
problem_file=$3
archive_folder=$4

# arduino writes data to file in data folder named with seconds-since-epoch (hex)
# arduino periodically creates new file and writes to it instead
num_files=`ls $arduino_folder | wc -l`
((num_files--))
if [ "$num_files" -gt 0 ]; then
	files=`ls -tr $arduino_folder | head -$num_files`
	echo "there are $num_files to process"
	for file in $files
	do
		# append to archive
		cat $file >> $archive_file
		upload.sh $file $problem_file
		if [ -z "$archive_folder" ] && [ -d "$archive_folder" ]; then
			mv $file ${archive_folder}/
		else
			rm $file
		fi
	done
fi # otherwise, wait till next time

# another script should take call upload.sh ${problem_file} stillproblem.csv
# mv stillproblem.csv problem.csv
# touch problem.csv
