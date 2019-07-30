#!/bin/bash
#
# Sort event table dump by timestamp (first field in CSV)
#
# It sorts a day's worth of event dump in 30 seconds.
#

# using the C locale avoids the overhead of having to parse UTF-8 and
# processing complex sort orders so improves performance dramatically

export LC_ALL=C

time nice sort --parallel=8 -t "," -k 1 $1 > $2
