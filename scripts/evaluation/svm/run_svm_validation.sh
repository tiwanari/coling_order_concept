#!/bin/sh
set -u

require_params=2
if [ $# -lt $require_params ]; then
    echo "need at least $require_params argument"
    echo "Usage: $0 train_test_output_path svm_path [c_value (default: 0.01)] [kernel (default: 0)] [ablation (default: 0 = do not)]"
    echo "e.g. $0 ../../result/gather/ver1/svm ../../tools/svm_rank/src [0.01] [0] [0]"
    exit 1
fi

train_test_output_path=$1
train_path=$train_test_output_path/train_data
test_path=$train_test_output_path/test_data
output_path=$1/validation
svm_path=$2

c_value="0.01"
kernel="0"
ablation="0"
if [ $# -gt 2 ]; then
    c_value=$3
fi
if [ $# -gt 3 ]; then
    kernel=$4
fi
if [ $# -gt 4 ]; then
    ablation=$5
fi

echo "-------------- params ---------------"
echo "train path: $train_path"
echo "test path: $test_path"
echo "svm path: $svm_path"
echo "c_value: $c_value"
echo "kernel: $kernel"
echo "ablation: $ablation"
echo "write to $output_path"
echo "-------------------------------------"

files=`find ${train_path}/ -name "train*" -type f | wc -l`

mkdir -p $train_path
mkdir -p $test_path
mkdir -p $output_path/model
mkdir -p $output_path/prediction

echo "${files} files are found"

for i in `seq 1 ${files}`
do
    echo "--------------- data $i --------------"
    suffix=`expr $i - 1`
    train_data="$train_path/train$suffix.txt"
    test_data="$test_path/test$suffix.txt"
    model_data="$output_path/model/model$suffix"
    prediction_data="$output_path/prediction/prediction$suffix.txt"

    echo "c -> $c_value"
    echo "kernel -> $kernel"
    echo "test -> $test_data"
    echo "train -> $train_data"
    echo "model -> $model_data"
    echo "prediction -> $prediction_data"

    echo "-- train --"
    $svm_path/train -c $c_value -s 8 -a $ablation $train_data $model_data
    echo "-- test --"
    $svm_path/predict -a $ablation $test_data $model_data $prediction_data

    # echo "-- train --"
    # $svm_path/svm_rank_learn -c $c_value -t $kernel $train_data $model_data
    # echo "-- test --"
    # $svm_path/svm_rank_classify $test_data $model_data $prediction_data

    echo "-------------------------------------"
done

echo "--------- params (show again) --------"
echo "train path: $train_path"
echo "test path: $test_path"
echo "svm path: $svm_path"
echo "c_value: $c_value"
echo "kernel: $kernel"
echo "ablation: $ablation"
echo "write to $output_path"
echo "-------------------------------------"
