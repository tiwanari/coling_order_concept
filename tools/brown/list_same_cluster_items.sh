#!/bin/sh

require_params=2
if [ $# -lt $require_params ]; then
    echo "need at least $require_params argument"
    echo "Usage: $0 input_file concept"
    echo "e.g. $0 ./feed-2009_1_10.c1024 消防士"
    exit 1
fi

input_file=$1
concept=$2
echo "concept: $concept"

result=`cat $input_file | grep "\t${concept}\t"`

set -- $result
cluster=$1
echo "cluster: $cluster"

cat $input_file | egrep "^$1\t" | less
