#!/bin/sh
set -eu

# include parse_yaml function
. util/parse_yaml.sh

# read yaml file
eval $(parse_yaml config.yml "config_")

classifier="svr"

require_params=3
if [ $# -lt $require_params ]; then
    echo "need at least $require_params argument"
    echo "Usage: $0 eval_data_path gold_data_path make_valid_data(true:1,false:other) [c_value (default: 0.01)] [ablation column id (default: 0 = do not)] [type (default: 11)]"
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
type="11"

if [ $# -gt 3 ]; then
    c_value=$4
fi
if [ $# -gt 4 ]; then
    ablation=$5
fi
if [ $# -gt 5 ]; then
    type=$6
fi

gold_txt="${gold_data_path}/gold.txt"
gold_csv="${gold_data_path}/gold.csv"

output_path_prefix=""
if [ $ablation = "0" ]; then
    output_path_prefix="${eval_data_path}/${classifier}"
else
    output_path_prefix="${eval_data_path}/${classifier}_wo${ablation}"
fi

svr_rank_output="${output_path_prefix}/${classifier}_result_c${c_value}_t${type}.txt"
rhos_output="${output_path_prefix}/rhos_c${c_value}_t${type}.txt"
avg_output="${output_path_prefix}/rho_averages.csv"

mkdir -p $output_path_prefix

echo "eval_data_path: $eval_data_path"
echo "gold_data_path: $gold_data_path"
echo "make_valid_data: $make_valid_data"

echo "c value: $c_value"
echo "ablation: $ablation"
echo "type: $type"

echo "gold_txt: $gold_txt"
echo "gold_csv: $gold_csv"

echo "svr result -> $svr_rank_output"
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
    # Usage: #{__FILE__} path_to_gold path_to_svm_data
    # e.g. #{__FILE__} ../../dataset/ja/queries/queries4/gold/gold.txt ../../result/gather/ver16/feed/queries4

    ./${classifier}/create_${classifier}_validation_data.rb $gold_txt $eval_data_path

else
    echo "make_valid_data = 0. skip this"
fi
echo "############### !./${classifier}/create_${classifier}_validation_data.rb ###############"


echo "###############     ./${classifier}/run_${classifier}_validation.sh     ###############"
# Usage: $0 train_test_output_path svr_path [c_value (default: 0.01)] [type (default: 11)] [ablation (default: 0 = do not)]
# e.g. $0 ../../result/gather/ver1/svr ../../tools/svr/liblinear [0.01] [11] [0]

./${classifier}/run_${classifier}_validation.sh $eval_data_path/${classifier} $config_tool_svr $c_value $type $ablation

echo "###############     !./${classifier}/run_${classifier}_validation.sh     ###############"


echo "###############     ./${classifier}/concat_${classifier}_results.rb     ###############"
# Usage: #{__FILE__} path_to_prediction_and_test output_path
# e.g. #{__FILE__} ../../result/gather/ver16/queries4/svr ../../result/gather/ver16/svr_result_c0.01_t11.txt

./${classifier}/concat_${classifier}_results.rb $eval_data_path/${classifier} $svr_rank_output

echo "###############     !./${classifier}/concat_${classifier}_results.rb     ###############"


echo "###############        ./util/calc_spearman.rb       ###############"
# Usage: ./calc_spearman.rb ranking1 ranking2 [show_annotate (any value is ok)]
# e.g. ./calc_spearman.rb ./gold/gold0.csv ../../result/gather/ver1/svr_result.txt

final_result="`./util/calc_spearman.rb $gold_csv $svr_rank_output`"

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
echo "type: $type"

echo "gold_txt: $gold_txt"
echo "gold_csv: $gold_csv"

echo "svr result -> $svr_rank_output"
echo "spearman's rho result -> $rhos_output"
echo "append average -> $avg_output"


if [ -f "$avg_output" ]; then
    echo "$avg_output is found. append average."
else
    echo "$avg_output is not found. add first column."
    echo "c, type, avg" >> $avg_output
fi

if grep "^${c_value}, ${type}, ${avg}" "$avg_output" > /dev/null; then
    # pattern found
    echo "'${c_value}, ${type}, ${avg}' exists in ${avg_output}"
else
    # pattern not found
    echo "append '${c_value}, ${type}, ${avg}'"
    echo "${c_value}, ${type}, ${avg}" >> $avg_output
fi
