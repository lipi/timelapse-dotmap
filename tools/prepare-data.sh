#!/bin/bash
#
# prepare frame data
#
# Convert compressed event dump to frame database
#
# Usage: cd data; ../tools/prepare_data events_201905.csv.gz

echo "Uncompressing event dumps and splitting to per-day files..."
# unzip and split original event dump to per-day files to help sorting
mkdir -p days
time cat "$@" | bunzip2 | gawk -f ../tools/split.awk

echo "Sorting each per-day file..."
# this step could be done parallel (see 'man parallel')
time for f in days/*; do ../tools/sort.sh $f $f.sorted; rm $f; done

# ~5 min
echo "Parsing sorted files and creating master frame..."
time cat days/*.sorted > sorted.csv

echo "Creating database..."
time python3 ../tools/create-db.py -i sorted.csv -o tldm.db

echo "Done."
