#!/usr/bin/gawk -f
#
# Splits a day's worth of event dump to 10-minute files
#

// {
    date=$1;
    time=$2;
    split(time,tokens,":");
    hours=tokens[1];
    minutes=tokens[2];
    tens=substr(minutes,1,1);
    filename = date "T" hours ":" tens "0.csv"
    print $0 >> filename;
}
