#!/usr/bin/env ruby
require 'find'
require 'fileutils'
require_relative '../util/print'
require_relative '../util/count'
require 'parallel'
require_relative '../util/spearman'

def same_query?(query0, query1)
    return false if query0.first != query1.first
    return false if query0.last.sort != query1.last.sort
    return true
end


# ===== from here, svr specific codes =====

# read file as ([adj, concepts], {concept0 => [hint0, hint1, ...], concept1 => [hint1, hint2, ...]})
# reuse svm data
def read_svm_data_file(filename, g_queries)
    content = {}
    concepts = []
    adj = ""
    puts "reading #{filename}"
    File.open(filename) do |file|
        adj = file.gets.split(" ").last.strip
        file.readlines.each_slice(2) do |lines|
            # "lines" looks like this:
            # # うどん
            # qid:1 1:0 2:3 ...
            concept = lines.first.split(" ").last.strip
            # remove qid value
            hint_list = lines.last.strip.sub(/qid:\d+/, "")
            # split hint_list and convert hint_list to hash
            hint_array = []
            hint_list.split(" ").each_with_index do |hint, i|
                id, value = hint.split(":")
                if id.to_i == i + 1
                    hint_array[i] = value.to_f
                else
                    STDERR.puts "something wrong!!"
                end
            end
            content[concept] = hint_array
            concepts << concept
        end
    end

    gold_standard = \
        g_queries.find \
            {|query| same_query?(query, [adj, concepts])}

    return nil, nil if gold_standard.nil?

    return gold_standard, content
end

def make_concepts_orders_for_learning(query, content, sampling_n)
    adj, concepts = query
    combi = concepts.combination(2).to_a
    total = combi.length.to_f
    lines = ""
    gold = Hash[concepts.map.with_index {|concept, j| [concept, j + 1]}]
    concepts.permutation.to_a.sample(sampling_n).each do |sequence|
        test_rank = Hash[sequence.map.with_index {|concept, j| [concept, j + 1]}]
        rho = spearman(gold, test_rank)
        # content[concept][#]
        num_of_correct_co_occurences = 0 # 0
        num_of_correct_dependencies = 0 # 1
        num_of_correct_similia = 0 # 2
        num_of_correct_comparative_win = 0 # 3
        num_of_correct_comparative_lose = 0 # 4
        combi.each do |(a, b)|
            a_rank = test_rank[a]
            b_rank = test_rank[b]
            a_content = content[a]
            b_content = content[b]
            # rank check and count
            if a_rank < b_rank # a is higher than b (e.g. a:1, b:3)
                num_of_correct_co_occurences    += 1 if a_content[0] > b_content[0]
                num_of_correct_dependencies     += 1 if a_content[1] > b_content[1]
                num_of_correct_similia          += 1 if a_content[2] > b_content[2]
                num_of_correct_comparative_win  += 1 if a_content[3] > b_content[3]
                num_of_correct_comparative_lose += 1 if a_content[4] < b_content[4]
            elsif b_rank < a_rank # b is higher than a (e.g. a:3, b:2)
                num_of_correct_co_occurences    += 1 if b_content[0] > a_content[0]
                num_of_correct_dependencies     += 1 if b_content[1] > a_content[1]
                num_of_correct_similia          += 1 if b_content[2] > a_content[2]
                num_of_correct_comparative_win  += 1 if b_content[3] > a_content[3]
                num_of_correct_comparative_lose += 1 if b_content[4] < a_content[4]
            end
        end
        # normalize
        num_of_correct_co_occurences    /= total
        num_of_correct_dependencies     /= total
        num_of_correct_similia          /= total
        num_of_correct_comparative_win  /= total
        num_of_correct_comparative_lose /= total
        lines.concat("#{rho} 1:#{num_of_correct_co_occurences}\
 2:#{num_of_correct_dependencies}\
 3:#{num_of_correct_similia}\
 4:#{num_of_correct_comparative_win}\
 5:#{num_of_correct_comparative_lose}\
 # #{adj} #{sequence.join(",")}\n")
    end
    return lines
end

def fact(n)
    (1..n).to_a.inject(1) {|f, i| f*i}
end

# reset random seed
srand(703266108192116103993606214654)

# ===== to here, svr specific codes =====

require_params = 2
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} path_to_gold path_to_svm_data"
    puts "\tNOTE: this script reuse svm format results (result#.txt)"
    puts "e.g. #{__FILE__} ../../dataset/ja/queries/queries4/gold/gold.txt ../../result/gather/ver16/feed/queries4"
    exit 1
end

OUT_LENGTH = 80
TRAIN_PATH_SUFFIX = "train_data"
TEST_PATH_SUFFIX = "test_data"
TUNE_PATH_SUFFIX = "tune_data"

path_to_gold = ARGV[0]
path_to_svm_data = ARGV[1]
num_of_data = count_files(path_to_output, "result*")

puts "path_to_gold: #{path_to_gold}"
path_to_svr_output = "#{path_to_svm_data}/svr"

train_data_path = "#{path_to_svr_output}/#{TRAIN_PATH_SUFFIX}"
test_data_path = "#{path_to_svr_output}/#{TEST_PATH_SUFFIX}"
tune_data_path = "#{path_to_svr_output}/#{TUNE_PATH_SUFFIX}"

FileUtils.mkdir_p(train_data_path)
FileUtils.mkdir_p(test_data_path)
FileUtils.mkdir_p(tune_data_path)

puts "path_to_output: (train: #{train_data_path}," \
        + " test: #{test_data_path}," \
        + " tune: #{tune_data_path})"
puts "num_of_data: #{num_of_data}"


# in gold, each line consists of max_rho adjective concepts
# 0.9893939393939393 大きい クジラ,キリン,ゾウ,クマ,ウシ,ウマ,イヌ,サル,ネコ,ネズミ

# read gold standard file as
# rhos: [rho0, rho1, ...]
# queries: [adj, [concept0, concept1, ...]]
puts_line("reading gold", OUT_LENGTH, STDERR)
g_rhos = []
g_queries = []
min_num_of_concepts = 100
File.foreach(path_to_gold).each do |line|
    rho, adj, ranking = line.strip.split(" ")
    STDERR.puts "rho: #{rho}, query: (#{adj}, [#{ranking}])"
    g_rhos << rho
    g_ranking = ranking.split(",")
    g_queries << [adj, g_ranking]
    if g_ranking.size < min_num_of_concepts
        min_num_of_concepts = g_ranking.size
    end
end
STDERR.puts "min num of concepts = #{min_num_of_concepts}"
minimum_sampling_n = fact(min_num_of_concepts)
puts_line("", OUT_LENGTH, STDERR)


# list of svm_data files
# output file format looks like this:
# # query 1 美味しい
# # うどん
# qid:1 1:0 2:1 3:1 4:79192 5:16361 6:53
# # そば
# qid:1 1:1 2:0 3:0 4:54470 5:10100 6:8
file_contents = []
queries = []
puts_line("reading svm results and insert gold ranking", OUT_LENGTH, STDERR)
num_ok_files = 0
num_of_data.times do |i|
    path = "#{path_to_svm_data}/result#{i + 1}.txt"
    query, content = read_svm_data_file(path, g_queries)
    unless content.nil?
        num_ok_files += 1
        queries << query # [adj, concepts]
        file_contents << content # {concept0 => [hint0, hint1, ...], concept1 => [hint1, hint2, ...]}
    end
end
STDERR.puts "#{num_ok_files} files are okay!"
puts_line("", OUT_LENGTH, STDERR)



# create train/test_data with permutation
puts_line("create train/test_data", OUT_LENGTH, STDERR)
Parallel.each(0...file_contents.size, in_processes: 8) do |i|
    for_test = file_contents[i]
    for_test_query = queries[i]

    train_data = "#{train_data_path}/train#{i}.txt"
    test_data = "#{test_data_path}/test#{i}.txt"
    tune_data_sub_path = "#{tune_data_path}/tune#{i}"

    STDERR.puts "train: #{train_data}, test: #{test_data}, tune: #{tune_data_sub_path}/"

    # ----- train data -----
    # create train data without a test data (N - 1)
    for_train = file_contents - [for_test]
    for_train_queries = queries - [for_test_query]

    # delete old data
    File.delete(train_data) if File.exist?(train_data)
    for_train.size.times do |j|
        # content = {concept0 => [hint0, hint1, ...], concept1 => [hint1, hint2, ...]}
        content = for_train[j]
        # train_query = [adj, gold_standard_ranking]
        train_query = for_train_queries[j]
        STDERR.puts "train data for test #{i}: concept size = #{train_query.last.size}, sampling = #{minimum_sampling_n}"
        train_data_text = make_concepts_orders_for_learning(train_query, content, minimum_sampling_n)
        File.open(train_data, "a") do |f|
            f.write(train_data_text)
        end
    end

    # ----- test data ------
    sampling_n = fact(for_test_query.last.size)
    STDERR.puts "test data #{i}: concept size = #{for_test_query.last.size}, sampling = #{sampling_n}"
    test_data_text = make_concepts_orders_for_learning(for_test_query, for_test, sampling_n)
    File.write(test_data, test_data_text) # just write it

    tune_train_data_path = "#{tune_data_sub_path}/#{TRAIN_PATH_SUFFIX}"
    FileUtils.mkdir_p(tune_train_data_path)
    tune_test_data_path = "#{tune_data_sub_path}/#{TEST_PATH_SUFFIX}"
    FileUtils.mkdir_p(tune_test_data_path)

    # ----- tune data -----
    FileUtils.mkdir_p(tune_data_sub_path)
    for_train.size.times do |j|
        for_tune_test = for_train[j]
        for_tune_test_query = for_train_queries[j]

        tune_train_data = "#{tune_train_data_path}/train#{j}.txt"
        tune_test_data = "#{tune_test_data_path}/test#{j}.txt"

        # create train data without a test and a train data (N - 1 - 1)
        for_tune_train = for_train - [for_tune_test]
        for_tune_train_queries = for_train_queries - [for_tune_test_query]

        # delete old data
        File.delete(tune_train_data) if File.exist?(tune_train_data)
        for_tune_train.size.times do |k|
            tune_train_content = for_tune_train[k]
            tune_train_query = for_tune_train_queries[k]
            STDERR.puts "tune train: concept size = #{tune_train_query.last.size}, sampling = #{minimum_sampling_n}"
            tune_train_data_text = make_concepts_orders_for_learning(tune_train_query, tune_train_content, minimum_sampling_n)
            File.open(tune_train_data, "a") do |f|
                f.write(tune_train_data_text)
            end
        end

        # === test data ===
        sampling_n = fact(for_tune_test_query.last.size)
        STDERR.puts "tune test: concept size = #{for_tune_test_query.last.size}, sampling = #{sampling_n}"
        tune_test_data_text = make_concepts_orders_for_learning(for_tune_test_query, for_tune_test, sampling_n)
        File.write(tune_test_data, tune_test_data_text) # just write it
    end
end
puts_line("", OUT_LENGTH, STDERR)
