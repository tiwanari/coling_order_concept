#!/usr/bin/env ruby
require 'open3'
require 'fileutils'
require 'yaml'
require_relative '../util/print'
require_relative '../util/count'
require "parallel"
require 'thread'

config = YAML.load(File.open("../config.yml").read)
QUERIES_FOLDER = "../#{config["directory"]["queries"]}"
RESULTS_FOLDER = "../#{config["directory"]["results"]}"

SVM_PATH = "../#{config["tool"]["svm"]}"

OUT_LENGTH = 80

require_params = 2
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} path_to_eval_data path_to_gold [ablation columns (default: 0 = do not)]"
    puts "\t-path_to_eval_data should be in #{RESULTS_FOLDER})"
    puts "\t-path_to_gold should be in #{QUERIES_FOLDER})"
    puts "e.g. #{__FILE__} ver16/feed/queries4 queries4/gold [\"1&2\"]"
    exit 1
end

c_params = \
[\
0.000001, 0.000002, 0.000005, \
0.00001, 0.00002, 0.00005, \
0.0001, 0.0002, 0.0005, \
0.001, 0.002, 0.005, \
0.01, 0.02, 0.05, \
0.1, 0.2, 0.5, \
1.0, 2.0, 5.0, \
10.0, 20.0, 50.0, \
100.0, 200.0, 500.0, \
1000.0, 2000.0, 5000.0, \
10000.0]

GOLD_CSV = "gold.csv"
TEST_PATH_SUFFIX = "test_data"
TUNE_PATH_SUFFIX = "tune_data"

path_to_eval_data = "#{RESULTS_FOLDER}/#{ARGV[0]}"
path_to_gold = "#{QUERIES_FOLDER}/#{ARGV[1]}"
num_of_data = count_files(path_to_eval_data, "result*")
ablation = ARGV.length < require_params + 1 ? "0" : ARGV[require_params]

output_path = "#{path_to_eval_data}/svm"
tune_data_path = "#{output_path}/#{TUNE_PATH_SUFFIX}"
test_data_path = "#{output_path}/#{TEST_PATH_SUFFIX}"

best_output_path = "#{output_path}/tuned_cs.txt"

svm_path = SVM_PATH
gold_path = "#{path_to_gold}/#{GOLD_CSV}"

kernel = 0

puts "path to eval data: #{path_to_eval_data}"
puts "num of data (rankings): #{num_of_data}"

puts "path to gold: #{path_to_gold}"

puts "ablation: #{ablation}"

puts "test data path: #{test_data_path}"
puts "tune data path: #{tune_data_path}"
puts "svm path (check its existence): #{svm_path}"

puts "out: best cs: #{test_data_path}"

puts_line("running ", OUT_LENGTH)

gold_standard = []
File.open(gold_path) do |f|
    f.readlines.each_slice(2) do |adj, ranking|
        # # 大きい
        # クジラ,1,キリン,2,ゾウ,3,クマ,4,ウシ,5,ウマ,6,イヌ,7,サル,8,ネコ,9,ネズミ,10
        # # 安い
        # ...
        gold_standard << "#{adj}#{ranking}"
    end
end

mutex = Mutex.new
bests = []
Parallel.each(0...num_of_data, in_processes: 8) do |i|
    puts "tuning for test#{i}"

    test_data = "#{test_data_path}/test#{i}.txt"

    max_rho = -1.0
    max_c = 0
    c_params.each do |c_value|
        tune_data_sub_path = "#{tune_data_path}/tune#{i}"

        puts_line("run a cross validation", OUT_LENGTH, STDERR)
        STDERR.puts "c_value: #{c_value}"
        STDERR.puts "tune_data_sub_path: #{tune_data_sub_path}"
        STDERR.puts "training on #{tune_data_sub_path} for test: #{test_data}"


        # Usage: $0 train_test_output_path svm_path
        #           [c_value (default: 0.01)] [kernel (default: 0)] [ablation (default: 0 = do not)]
        # e.g. $0 ../../result/gather/ver1/svm ../../tools/svm_rank/src [0.01] [0] [0]
        STDERR.puts "./run_svm_validation.sh #{tune_data_sub_path} #{svm_path} #{c_value} #{kernel} \"#{ablation}\""
        puts_line("svm_validation", OUT_LENGTH, STDERR)
        `./run_svm_validation.sh #{tune_data_sub_path} #{svm_path} #{c_value} #{kernel} \"#{ablation}\"`


        # Usage: #{__FILE__} path_to_prediction_and_test output_path
        # e.g. #{__FILE__} ../../result/gather/ver16/queries4/svm ../../result/gather/ver16/svm_result_c0.01_k0.txt
        svm_rank_output = "#{tune_data_sub_path}/eval/svm_result_c#{c_value}_k#{kernel}.csv"
        FileUtils.mkdir_p(File.dirname(svm_rank_output))

        STDERR.puts "./concat_svm_results.rb #{tune_data_sub_path} #{svm_rank_output}"
        puts_line("concate_svm_results", OUT_LENGTH, STDERR)
        `./concat_svm_results.rb #{tune_data_sub_path} #{svm_rank_output}`


        # create test gold (N - 1) results
        test_gold = gold_standard - [gold_standard[i]]

        # make some temporary files in #{path_to_eval_data}/svm/tmp/
        test_gold_data = "#{output_path}/tmp/test#{i}/test_gold.csv"
        FileUtils.mkdir_p(File.dirname(test_gold_data))
        # write test gold
        File.write(test_gold_data, test_gold.join())


        # Usage: #{__FILE__} ranking1 ranking2 [show_annotate (any value is ok)]
        # e.g. #{__FILE__} ./gold/gold0.csv ../../result/gather/ver1/svm_result.txt
        STDERR.puts "./calc_spearman.rb #{svm_rank_output} #{test_gold_data}"
        puts_line("calc_spearman", OUT_LENGTH, STDERR)
        out_spearman = `../util/calc_spearman.rb #{svm_rank_output} #{test_gold_data}`

        rho = out_spearman.split("\n").last.to_f
        if rho > max_rho
            max_rho = rho
            max_c = c_value
        end
        puts "checked (i, c, rho) = (#{i}, #{c_value}, #{rho})"
    end

    b = "best (i, c, rho) = (#{i}, #{max_c}, #{max_rho})"
    puts b
    mutex.synchronize do
        bests[i] = b
    end
end

File.write(best_output_path, bests.join("\n"))
