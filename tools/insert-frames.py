#!/usr/bin/python3
#
# Insert frame data into DB
#
# Frame timestamp is deducted from filename
#

import sys
import struct
import sqlite3
from datetime import datetime

def insert_frame(conn, cursor, timestamp, frame):
    sql = 'REPLACE INTO delta (timestamp, frame) VALUES (?, ?)'
    cursor.execute(sql, (timestamp, sqlite3.Binary(frame)))
    conn.commit()
#    print('Written {} bytes ({} slots) into database at {}'.format(len(frame), len(frame) / 12, timestamp))

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage:   ./insert-frame.py <master> <database> <sorted-events [sorted-events] ...>')
        print('Example: ./insert-frame.py master.csv frames.db "2019-05-01T19:10.csv"')
        print('         ./insert-frame.py master.csv frames.db *.sorted')
        exit(1)

    master = {}
    with open(sys.argv[1], 'r') as f:
        for line in f:
            slot,eid,lat,lon = line.split(',')
            master[eid] = slot
    print('Got {} slots'.format(slot))

    conn = sqlite3.connect(sys.argv[2])
    cursor = conn.cursor()
        
    for path in sys.argv[3:]:
        with open(path, 'r') as f:
            timestamp = 0
            frame = bytes()
            s = struct.Struct('< i f f')
            for line in f:
                date,eid,lat,lon = line.split(',')
                dt = datetime.strptime(date.split('.')[0], '%Y-%m-%d %H:%M:%S')
                ts = int(dt.timestamp())

                if ts != timestamp:
                    insert_frame(conn, cursor, timestamp, frame)
                    timestamp = ts
                    frame = bytes()

                try:
                    slot = master[eid]
                except KeyError:
                    print('Unknown ID {}'.format(eid))
                binary = s.pack(int(slot),float(lat),float(lon))
                frame += binary

            insert_frame(conn, cursor, ts, frame)

    conn.close()
                           
    
