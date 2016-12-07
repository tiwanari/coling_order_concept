#!/bin/sh
set -eu

# include parse_yaml function
. util/parse_yaml.sh

# read yaml file
eval $(parse_yaml config.yml "config_")

classifier="svm"

require_params=3
if [ $# -lt $require_params ]; then
    echo "need at least $require_params argument"
    echo "Usage: $0 eval_data_path gold_data_path make_valid_data(true:1,false:other) [c_value (default: 0.01)] [ablation column id (default: 0 = do not)] [kernel (default: 0)]"
    echo "\t- eval_data_path should be located in ${config_directory_results}"
    echo "\t- gold_data_path should be located in ${config_directory_queries}"
    echo "e.g. $0 ver16/feed/queries4 queries4/gold 0 [0.01]"
    exit 1
fi

eval_data_path="${config_directory_results}/$1"
gold_data_path="${config_directory_queries}/$2"
make_valid_data=$3

c_value="0.01"
ablation="0"
kernel="0"

if [ $# -gt 3 ]; then
    c_value=$4
fi
if [ $# -gt 4 ]; then
    ablation=$5
fi
if [ $# -gt 5 ]; then
    kernel=$6
fi

gold_txt="${gold_data_path}/gold.txt"
gold_csv="${gold_data_path}/gold.csv"

output_path_prefix=""
if [ $ablation = "0" ]; then
    output_path_prefix="${eval_data_path}/${classifier}"
else
    output_path_prefix="${eval_data_path}/${classifier}_wo${ablation}"
fi

svm_rank_output="${output_path_prefix}/${classifier}_result_c${c_value}_k${kernel}.txt"
rhos_output="${output_path_prefix}/rhos_c${c_value}_k${kernel}.txt"
avg_output="${output_path_prefix}/rho_averages.csv"

mkdir -p $output_path_prefix

echo "eval_data_path: $eval_data_path"
echo "gold_data_path: $gold_data_path"
echo "make_valid_data: $make_valid_data"

echo "c value: $c_value"
echo "ablation: $ablation"
echo "kernel: $kernel"

echo "gold_txt: $gold_txt"
echo "gold_csv: $gold_csv"

echo "svm result -> $svm_rank_output"
echo "spearman's rho result -> $rhos_output"
echo "append average -> $avg_output"

echo "-----------------------------------------------------"

echo "this script runs evaluation in the following order:"
echo "do this yourself -> ./create_gold_standard.rb"
echo "./create_${classifier}_validation_data.rb"
echo "./run_${classifier}_validation.sh"
echo "./concat_${classifier}_results.rb"
echo "./calc_spearman.rb"


echo "############### ./${classifier}/create_${classifier}_validation_data.rb ###############"
if [ $make_valid_data = "1" ]; then
    # Usage: #{__FILE__} path_to_gold path_to_output
    # e.g. #{__FILE__} ../../dataset/ja/queries/queries4/gold/gold.txt ../../result/gather/ver16/feed/queries4

    ./${classifier}/create_${classifier}_validation_data.rb $gold_txt $eval_data_path

else
    echo "make_valid_data = 0. skip this"
fi
echo "############### !./${classifier}/create_${classifier}_validation_data.rb ###############"


echo "###############     ./${classifier}/run_${classifier}_validation.sh     ###############"
# Usage: $0 train_test_output_path svm_path [c_value (default: 0.01)] [kernel (default: 0)] [ablation (default: 0 = do not)]
# e.g. $0 ../../result/gather/ver1/svm ../../tools/svm_rank/src [0.01] [0] [0]

./${classifier}/run_${classifier}_validation.sh $eval_data_path/${classifier} $config_tool_svm $c_value $kernel $ablation

echo "###############     !./${classifier}/run_${classifier}_validation.sh     ###############"



echo "###############     ./${classifier}/concat_${classifier}_results.rb     ###############"
# Usage: #{__FILE__} path_to_prediction_and_test output_path
# e.g. #{__FILE__} ../../result/gather/ver16/queries4/svm ../../result/gather/ver16/svm_result_c0.01_k0.txt

./${classifier}/concat_${classifier}_results.rb $eval_data_path/${classifier} $svm_rank_output

echo "###############     !./${classifier}/concat_${classifier}_results.rb     ###############"



echo "###############        ./util/calc_spearman.rb       ###############"
# Usage: ./calc_spearman.rb ranking1 ranking2 [show_annotate (any value is ok)]
# e.g. ./calc_spearman.rb ./gold/gold0.csv ../../result/gather/ver1/svm_result.txt

final_result="`./util/calc_spearman.rb $gold_csv $svm_rank_output`"

echo "$final_result" > $rhos_output
echo "$final_result"

avg=`echo "$final_result" | grep "average" -A 1 | tail -n 1`

echo "###############        !./util/calc_spearman.rb       ###############"



echo "------------------ params (show again) -----------------------"
echo "eval_data_path: $eval_data_path"
echo "gold_data_path: $gold_data_path"
echo "make_valid_data: $make_valid_data"

echo "c value: $c_value"
echo "ablation: $ablation"
echo "kernel: $kernel"

echo "gold_txt: $gold_txt"
echo "gold_csv: $gold_csv"

echo "svm result -> $svm_rank_output"
echo "spearman's rho result -> $rhos_output"
echo "append average -> $avg_output"


if [ -f "$avg_output" ]; then
    echo "$avg_output is found. append average."
else
    echo "$avg_output is not found. add first column."
    echo "c, kernel, avg" >> $avg_output
fi

if grep "^${c_value}, ${kernel}, ${avg}" "$avg_output" > /dev/null; then
    # pattern found
    echo "'${c_value}, ${kernel}, ${avg}' exists in ${avg_output}"
else
    # pattern not found
    echo "append '${c_value}, ${kernel}, ${avg}'"
    echo "${c_value}, ${kernel}, ${avg}" >> $avg_output
fi
