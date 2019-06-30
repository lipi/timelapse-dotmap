#!/usr/bin/gawk -f
#
# Splits a day's worth of event dump to per-minute files in 6 seconds
#

/2019-05-01 00:/ {time=$2; split(time,tokens,":"); print >> tokens[2] }
