#!/bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "usage: retry_uploads.sh /path/to/problem/datafile.csv /path/to/still/problem/datafile.csv"
    exit -1
fi

problemFile=$1
stillProblemFile=$2

upload.sh $problemFile $stillProblemFile
mv $stillProblemFile $problemFile
touch $stillProblemFile