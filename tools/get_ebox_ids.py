#!/usr/bin/python3
#
# Read events from CSV file and print:
# - list of ebox IDs
# - the first location of each ebox
#
# Input file format is CSV, fields: timestamp, ebox ID, latitude, longitude
#
# IMPORTANT: requires input sorted by timestamp in ascending order
#

import sys

def process_lines(fd):
    '''Process lines in file descriptor fd'''
    ebox = {}
    for num,line in enumerate(fd):
        try:
            ts,eid,lat,lon = line[:-1].split(',')
            if float(lat) == 0: 
                continue # ignore invalid (0,0) locations
            if not eid in ebox: # only use the first location
                ebox[eid] = (lat,lon)
                
        except ValueError as ex:
            print('{} when processing line {} ({})'.format(ex, num, line))
            
    return ebox

def write_ebox_to_file(ebox, fd):
    for num,key in enumerate(ebox):
        lat,lon = ebox[key]
        fd.write('{},{},{},{}\n'.format(num, key, lat, lon))
    

if __name__ == '__main__':
    if len(sys.argv) >= 2:
        with open(sys.argv[1], 'r') as f:
            ebox = process_lines(f)
    else:
         ebox = process_lines(sys.stdin)

    if len(sys.argv) == 3:
        with open(sys.argv[2], 'w') as f:
            write_ebox_to_file(ebox, f)
    else:
        write_ebox_to_file(ebox, sys.stdout)
        
