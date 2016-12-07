#!/bin/sh
set -eu

# include parse_yaml function
. util/parse_yaml.sh

# read yaml file
eval $(parse_yaml config.yml "config_")

require_params=3
if [ $# -lt $require_params ]; then
    echo "need at least $require_params argument"
    echo "Usage: $0 eval_data_path gold_data_path target_column(-1: comparative compounding baseline)"
    echo "\t- eval_data_path should be located in ${config_directory_results}"
    echo "\t- gold_data_path should be located in ${config_directory_queries}"
    echo "e.g. $0 ver16/feed/queries4/ queries4/gold/ 2"
    exit 1
fi

GOLD_CSV="gold.csv"

eval_data_path="${config_directory_results}/$1"
gold_data_path="${config_directory_queries}/$2/${GOLD_CSV}"
target_column=$3

# count result* files (e.g., result0.txt, result1.txt, ...)
num_of_ranks=`find  ${eval_data_path} -type f -name "result*" | wc -l`

baseline_output_dir=""
if [ $target_column = "-1" ]; then
    baseline_output_dir="${eval_data_path}/comparative_baseline"
else
    baseline_output_dir="${eval_data_path}/baseline_row${target_column}"
fi
baseline_rankings="${baseline_output_dir}/rankings.csv"
baseline_rhos="${baseline_output_dir}/rhos.txt"

mkdir -p $baseline_output_dir


echo "eval_data_path: $eval_data_path"
echo "gold_data_path: $gold_data_path"

if [ $target_column = "-1" ]; then
    echo "target_column: comparative (4 & 5)"
else
    echo "target_column: $target_column"
fi

echo "num_of_ranks: $num_of_ranks"

echo "out: baseline's rankings -> $baseline_rankings"
echo "out: baseline's rhos -> $baseline_rhos"


echo "############### ./baseline/sort.rb ###############"
> $baseline_rankings
for i in `seq 1 $num_of_ranks`
do
    if [ $target_column = "-1" ]; then
        # Usage: ./sort_with_compounded_value.rb path_to_file
        # e.g. ./sort_with_compounded_value.rb ../../result/gather/ver6/result1.txt

        ./baseline/sort_with_compounded_value.rb ${eval_data_path}/result${i}.txt >> $baseline_rankings

    else
        # Usage: ./sort.rb path_to_file target_column
        # e.g. ./sort.rb ../../result/gather/ver6/result1.txt 2

        ./baseline/sort.rb ${eval_data_path}/result${i}.txt $target_column >> $baseline_rankings

    fi
done
echo "############### !./baseline/sort.rb ###############"


echo "############### ./util/calc_spearman.rb ###############"
# Usage: ./calc_spearman.rb ranking1 ranking2 [show_annotate (any value is ok)]
# e.g. ./calc_spearman.rb ./gold/gold0.csv ../../result/gather/ver1/svm_result.txt

final_result="`./util/calc_spearman.rb $gold_data_path $baseline_rankings`"

echo "$final_result" > $baseline_rhos
echo "$final_result"

echo "############### !./util/calc_spearman.rb ###############"


echo "-------------- params (show again) -----------------"
echo "eval_data_path: $eval_data_path"
echo "gold_data_path: $gold_data_path"

if [ $target_column = "-1" ]; then
    echo "target_column: comparative (4 & 5)"
else
    echo "target_column: $target_column"
fi

echo "num_of_ranks: $num_of_ranks"

echo "out: baseline's rankings -> $baseline_rankings"
echo "out: baseline's rhos -> $baseline_rhos"
