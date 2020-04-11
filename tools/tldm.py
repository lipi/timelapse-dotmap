#!/usr/bin/python3
#
# Parse location data and create database for TLDM
#

import sys
import pprint
import argparse


def find_first_locations(lines):
    """
    Process lines in file descriptor fd
    File format is CSV -- TODO: pass format as argument

    Return dictionary of dot IDs and their first location
    """
    first_locations = {}
    for num, line in enumerate(lines):
        try:
            timestamp, name, lat, lon = line[:-1].split(',')
            if float(lat) == 0:
                continue  # ignore invalid (0,0) locations
            if name not in first_locations:  # record the first location
                first_locations[name] = (lat, lon)

        except ValueError as ex:
            print('{} when processing line {} ({})'.format(ex, num, line))

    return first_locations


def save_first_locations(first_locations, fd):
    for num, key in enumerate(first_locations):
        lat, lon = first_locations[key]
        fd.write('{},{},{},{}\n'.format(num, key, lat, lon))


def check_read(filename):
    return check_file(filename, 'r')


def check_write(filename):
    return check_file(filename, 'w')


def check_file(filename, flag):
    try:
        fd = open(filename, flag)
        return fd
    except (FileNotFoundError, PermissionError) as ex:
        raise argparse.ArgumentTypeError('{} - {}'.format(filename, ex.strerror))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--infile', type=check_read, default=sys.stdin,
                        help='input file with timestamps and locations (CSV)')
    parser.add_argument('-o', '--outfile', type=check_write, default=sys.stdout,
                        help='output file for dots database')
    args = parser.parse_args()

    with args.infile:
        dots = find_first_locations(args.infile)
    with args.outfile:
        save_first_locations(dots, args.outfile)
