#!/bin/bash
#
# prepare frame data
#
# Convert compressed event dump to frame database
#
# Usage: cd data; ../tools/prepare_data events_201905.csv.gz

echo "Uncompressing event dumps and splitting to per-day files..."
# unzip and split original event dump to per-day files to help sorting
# later (takes ~10 min) 
#(can't split to ten minutes yet, number of files would be too high)
mkdir -p days
time cat "$@" | bunzip2 | gawk -f ../tools/split.awk

echo "Sorting each per-day file..."
# this step could be done parallel (see 'man parallel')
time for f in days/*; do ../tools/sort.sh $f $f.sorted; rm $f; done

# ~5 min
echo "Parsing sorted files and creating master frame..."
time cat days/*.sorted | python3 ../tools/get_ebox_ids.py > master.csv


echo "Splitting data to 10-minute files..."
# split each per-day file into ten-minute chunks (~20 min single thread)
# this step could be done parallel (see 'man parallel')
mkdir -p tens
cd tens
time for f in ../days/*.sorted; do gawk -f ../../tools/split-10m.awk < $f; done

echo "Creating keyframes..."
# create keyframes from master file and 10-minute chunks (~35 min single thread)
# this step could be done parallel (see 'man parallel')
time for f in *.csv; do python3 ../../tools/keyframe.py ../master.csv $f $f.bin binary; done

echo "Inserting keyframes to database..."
# create sqlite DB
sqlite3 -line ../frames.db 'create table snapshot (timestamp integer primary key, frame blob not null);'
# insert keyframes to DB (~4 min)
time for f in *.bin; do python3 ../../tools/insert-keyframe.py ../frames.db $f; done

echo "Inserting delta frames to database..."
sqlite3 -line ../frames.db 'create table delta (timestamp integer primary key, frame blob not null);'
# insert deltas to DB (~4 min)
time for f in ../days/*.sorted; do python3 ../../tools/insert-frames.py ../master.csv ../frames.db $f; done

echo "Create separate table for timestamps..."
sqlite3 -line ../frames.db 'create table timestamps (timestamp integer primary key);'
sqlite3 -line ../frames.db 'insert into timestamps select timestamp from delta where timestamp > 0 order by timestamp asc;'

echo "Done."
