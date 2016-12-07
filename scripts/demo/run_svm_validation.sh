#!/bin/sh
set -eu

require_params=4
if [ $# -lt $require_params ]; then
    echo "need at least ${require_params} argument"
    echo "Usage: ${0} path_to_svm_src path_to_test path_to_model path_to_prediction(output)"
    exit 1
fi

path_to_svm_src=$1
path_to_test=$2
path_to_model=$3
path_to_output=$4

$path_to_svm_src/predict $path_to_test $path_to_model $path_to_output
