#!/usr/bin/python3
#
# Parse location data and create database for TLDM
#
import pprint
import argparse
import struct
import sys
from datetime import datetime
from typing import Set
import sqlite3
import logging

SNAPSHOT_SECONDS = 600  # create snapshot every 10 minutes


def text_to_seconds(text):
    """
    Convert ISO datetime string to seconds-since-Epoch

    >>> text_to_seconds('2019-06-09 12:00:00.000')
    1560038400
    """
    dt = datetime.strptime(text.split('.')[0], '%Y-%m-%d %H:%M:%S')
    return int(dt.timestamp())


def line_to_data(line):
    """Process a CSV line. Return seconds, name, latitude, longitude"""
    dt, name, lat, lon = line.split(',')
    ts = text_to_seconds(dt)
    return ts, name, float(lat), float(lon)


def valid_location(lat, lon):
    return lat != 0.0 and lon != 0.0


def find_first_locations(lines):
    """
    Expects lines of comma-separated values: timestamp,name,latitude,longitude

    Return dictionary (of dot names and their first location) and list (timestamps)
    """
    logger.info('Finding first locations...')
    first_locations = {}
    timestamps: Set[int] = set()
    for num, line in enumerate(lines):
        try:
            timestamp, name, lat, lon = line_to_data(line)
            if not valid_location(lat, lon):
                continue
            if name not in first_locations:  # record the first location
                first_locations[name] = (lat, lon)
            timestamps.add(timestamp)

        except ValueError as ex:
            print('{} when processing line {} ({})'.format(ex, num, line))

    logger.info('Found {} locations'.format(len(first_locations)))
    return first_locations, sorted(list(timestamps))


def check_read(filename):
    return check_file(filename, 'r')


def check_write(filename):
    return check_file(filename, 'w')


def check_file(filename, flag):
    try:
        with open(filename, flag):
            return filename
    except (FileNotFoundError, PermissionError) as ex:
        raise argparse.ArgumentTypeError('{} - {}'.format(filename, ex.strerror))


def create_slots(dots):
    slots = {}
    for slot, name in enumerate(dots):
        slots[name] = slot
    return slots


def create_snapshot(dots, debug=False):
    """Create binary array of lat/lon of each dot"""
    snapshot = bytes()
    s = struct.Struct('< f f')
    for name in dots:
        lat, lon = dots[name]
        if not valid_location(lat, lon):
            continue
        b = s.pack(lat, lon)
        snapshot += b

    debug_snapshot(snapshot, debug)
    return snapshot


def debug_snapshot(snapshot, debug=False):
    if not debug:
        return
    s = struct.Struct('< f f')
    print('snapshot:')
    for lat, lon in s.iter_unpack(snapshot):
        print((lat, lon))
    return


def create_delta(dots, slots, debug=False):
    """
    Create binary array of slot/lat/lon of each dot in dots,
    using 'slot's for the name-slot lookup.

    If 'dots' only has changes of on second, the return value is the delta.
    """
    delta = bytes()
    s = struct.Struct('< i f f')
    for name in dots:
        lat, lon = dots[name]
        if not valid_location(lat, lon):
            continue
        slot = slots[name]
        b = s.pack(slot, lat, lon)
        delta += b

    debug_delta(delta, debug)
    return delta


def debug_delta(delta, debug=False):
    if not debug:
        return
    s = struct.Struct('< i f f')
    print('delta:')
    for slot, lat, lon in s.iter_unpack(delta):
        print((slot, lat, lon))
    return


def insert_slots(slots, conn):
    """Insert name/slot pairs into 'slots' table"""
    cursor = conn.cursor()
    sql = 'REPLACE INTO slots (name, slot) VALUES (?, ?)'
    for name in slots:
        cursor.execute(sql, (name, slots[name]))
    conn.commit()
    logger.info('Inserted {} slots into DB'.format(len(slots)))


def insert_timestamps(timestamps, conn):
    """Insert timestamps into timestamps table"""
    cursor = conn.cursor()
    sql = 'REPLACE INTO timestamps (timestamp) VALUES (?)'
    seconds = sorted(list(timestamps))
    for second in seconds:
        cursor.execute(sql, (second,))
    conn.commit()
    logger.info('Inserted {} timestamps into DB'.format(len(timestamps)))


def insert_snapshot(timestamp, keyframe, conn):
    """Insert keyframe into snapshot table"""
    cursor = conn.cursor()
    sql = 'REPLACE INTO snapshot (timestamp, frame) VALUES (?, ?)'
    cursor.execute(sql, (timestamp, sqlite3.Binary(keyframe)))
    conn.commit()
    logger.info('Inserted snapshot at {} into DB'.format(timestamp))


def insert_delta(timestamp, frame, conn):
    """Insert frame into delta table"""
    cursor = conn.cursor()
    sql = 'REPLACE INTO delta (timestamp, frame) VALUES (?, ?)'
    cursor.execute(sql, (timestamp, sqlite3.Binary(frame)))
    # do not commit every deltas, only snapshots (to reduce write frequency)
    logger.debug('Inserted delta at {} into DB'.format(timestamp))


def open_database(filename):
    conn = sqlite3.connect(filename)
    cursor = conn.cursor()
    commands = [
        'CREATE TABLE slots (name text primary key, slot integer);',
        'CREATE TABLE timestamps (timestamp integer primary key);',
        'CREATE TABLE snapshot (timestamp integer primary key, frame blob not null);',
        'CREATE TABLE delta (timestamp integer primary key, frame blob not null);'
    ]
    for sql in commands:
        cursor.execute(sql)
        conn.commit()
    logger.info('Created DB at {}'.format(filename))
    return conn


def close_database(conn):
    conn.commit()
    conn.close()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO,
                        format='%(asctime)s %(name)s %(levelname)s %(message)s',)
    logger = logging.getLogger('tldm')

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--infile', type=check_read,
                        help='input file with timestamps and locations')
    parser.add_argument('-o', '--outfile', type=check_write,
                        help='database file for dots')
    parser.add_argument('-v', '--verbose', action="store_true",
                        help='print debug info')
    args = parser.parse_args()

    #
    # first pass: collect all locations and timestamps
    #

    db_connection = open_database(args.outfile)
    with open(args.infile, 'r') as infile:
        dots, timestamps = find_first_locations(infile)
    insert_timestamps(timestamps, db_connection)
    slots = create_slots(dots)
    insert_slots(slots, db_connection)
    keyframe = create_snapshot(dots, args.verbose)
    insert_snapshot(timestamps[0], keyframe, db_connection)

    #
    # seconds pass: create all snapshots and deltas
    #
    timestamp = 0
    delta_dots = {}
    with open(args.infile, 'r') as infile:
        for line in infile:
            ts, name, lat, lon = line_to_data(line)
            if not valid_location(lat, lon):
                continue
            if timestamp and ts != timestamp:  # new second, dump delta
                frame = create_delta(delta_dots, slots, debug=args.verbose)
                insert_delta(timestamp, frame, db_connection)

                dots.update(delta_dots)
                delta_dots = {}

                if (ts % SNAPSHOT_SECONDS) == 0:  # new snapshot is needed
                    keyframe = create_snapshot(dots, debug=args.verbose)
                    insert_snapshot(ts, keyframe, db_connection)

            timestamp = ts
            delta_dots[name] = lat, lon

            dots[name] = lat, lon

        # all lines have been processed, force last delta
        frame = create_delta(delta_dots, slots, debug=args.verbose)
        insert_delta(timestamp, frame, db_connection)

    close_database(db_connection)
