#!/usr/bin/python3
#
# Create keyframe data using master frame and events since
#
# Usage:   ./keyframe.py <masterfile> <eventdump> <outfile> [binary]
# Example: ./keyframe.py master.csv sorted-2019-05-01.csv keyframe-2019-05-01.csv
#          ./keyframe.py master.csv sorted-2019-05-01.csv keyframe-2019-05-01.bin bin
#

import sys
import struct

def read_master(fd):
    '''Read master keyframe from file descriptor fd'''
    slot = []
    ebox = {}
    for line in fd:
        try:
            row,eid,lat,lon = line[:-1].split(',')
            slot.append(int(eid))
            ebox[int(eid)] = (float(lat),float(lon))
            
        except ValueError as ex:
            print('{} when processing line {} ({})'.format(ex, row, line))
            
    return slot,ebox


def read_lines(fd):
    '''Process lines in file descriptor fd

    Returns last location in ebox dictionary
    '''
    latest = {}
    for num,line in enumerate(fd):
        try:
            ts,eid,lat,lon = line[:-1].split(',')
            if float(lat) == 0: 
                continue # ignore invalid (0,0) locations
            latest[int(eid)] = (float(lat),float(lon)) # always update, so we end up with the latest
                
        except ValueError as ex:
            print('{} when processing line {} ({})'.format(ex, num, line))
            
    return latest


def write_keyframe(slot, master, latest, fd, binary=False):
    keyframe = []
    s = struct.Struct('< f f')
    for i,eid in enumerate(slot):
        try:
            lat,lon = latest[eid]
        except:
            lat,lon = master[eid]
            keyframe.append((lat,lon))
        if binary:
            bin = s.pack(lat,lon)
            fd.write(bin)
        else:
            fd.write('{},{},{}\n'.format(i, lat, lon))
    return keyframe


if __name__ == '__main__':
    if len(sys.argv) < 4:
        print('Usage:   ./keyframe.py <masterfile> <eventdump> <outfile>')
        print('Example: ./keyframe.py master.csv sorted-2019-05-01.csv keyframe-2019-05-01.csv')
        exit(1)

    slot = []
    master = {}
    
    with open(sys.argv[1], 'r') as f:
        slot,master = read_master(f)

    with open('1970-01-01T00:00.bin', 'wb') as f:
        write_keyframe(slot, master, [], f, True)
    with open('1970-01-01T00:00.csv', 'w') as f:
        write_keyframe(slot, master, [], f)

    with open(sys.argv[2], 'r') as f:
        latest = read_lines(f)

    if len(sys.argv) == 5 and 'bin' in sys.argv[4]:
        with open(sys.argv[3], 'wb') as f:
            write_keyframe(slot, master, latest, f, True)
    else:
        with open(sys.argv[3], 'w') as f:
            write_keyframe(slot, master, latest, f)
        
