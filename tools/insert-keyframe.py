#!/usr/bin/python3
#
# Insert keyframe data into DB
#
# Keyframe timestamp is deducted from filename
#
# Usage: ./insert-keyframe.py frames.db keyframe-00.bin

import sys
import sqlite3
from datetime import datetime

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage:   ./insert-keyframe.py <database> <keyframe> [keyframe [keyframe]...]')
        print('Example: ./insert-keyframe.py dotm.db "2019-05-01T19:10.csv.bin"')
        print('         ./insert-keyframe.py dotm.db *.bin')
        exit(1)

    conn = sqlite3.connect(sys.argv[1])
    cursor = conn.cursor()
        
    for path in sys.argv[2:]:
        with open(path, 'rb') as f:
            filename = path.split('/')[-1]
            basename = filename.split('.')[0]
            dt = datetime.strptime(basename, '%Y-%m-%dT%H:%M')
            ts = int(dt.timestamp())

            # 'bug': the input contains data starting from timestamp, not ending
            # workaround: add 10 minutes to timestamp
            ts += 600
            
            data = f.read()
            print('basename: {} timestamp: {} data size:{}'.format(basename, ts, len(data)))
            sql = 'REPLACE INTO keyframe (timestamp, frame) VALUES (?, ?)'
            cursor.execute(sql, (ts, sqlite3.Binary(data)))
            conn.commit()

    conn.close()
                           
    
