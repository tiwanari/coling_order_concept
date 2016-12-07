#!/usr/bin/env ruby
require 'find'
require 'fileutils'
require_relative '../util/print'
require_relative '../util/count'
require 'parallel'

def same_query?(query0, query1)
    return false if query0.first != query1.first
    return false if query0.last.sort != query1.last.sort
    return true
end


# ===== from here, svm specific codes =====

# read file as {query => {concept0 => hint0, ...}}
def read_output_file(filename, g_queries, rank_id)
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
            # replace qid value
            hint = lines.last.strip.sub(/qid:\d+/, "qid:#{rank_id}")
            content[concept] = hint
            concepts << concept
        end
    end

    gold_standard = \
        g_queries.find \
            {|query| same_query?(query, [adj, concepts])}

    return nil if gold_standard.nil?
    g_concepts = gold_standard.last
    g_concepts.each.with_index do |concept, i|
        rank = g_concepts.size - i
        # add rank based on gold standard
        content[concept] = "#{rank} #{content[concept]}"
    end

    body = "# #{adj}\n"
    content.each do |concept, hint|
        body.concat("# #{concept}\n#{hint}\n")
    end
    return body
end

# ===== to here, svm specific codes =====

require_params = 2
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} path_to_gold path_to_output"
    puts "e.g. #{__FILE__} ../../dataset/ja/queries/queries4/gold/gold.txt ../../result/gather/ver16/feed/queries4"
    exit 1
end

OUT_LENGTH = 80
TRAIN_PATH_SUFFIX = "train_data"
TEST_PATH_SUFFIX = "test_data"
TUNE_PATH_SUFFIX = "tune_data"

path_to_gold = ARGV[0]
path_to_output = ARGV[1]
num_of_data = count_files(path_to_output, "result*")

puts "path_to_gold: #{path_to_gold}"
path_to_svm_output = "#{path_to_output}/svm"

train_data_path = "#{path_to_svm_output}/#{TRAIN_PATH_SUFFIX}"
test_data_path = "#{path_to_svm_output}/#{TEST_PATH_SUFFIX}"
tune_data_path = "#{path_to_svm_output}/#{TUNE_PATH_SUFFIX}"

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
File.foreach(path_to_gold).each do |line|
    rho, adj, ranking = line.strip.split(" ")
    STDERR.puts "rho: #{rho}, query: (#{adj}, [#{ranking}])"
    g_rhos << rho
    g_queries << [adj, ranking.split(",")]
end
puts_line("", OUT_LENGTH, STDERR)

# list of output files
# output file format looks like this:
# # query 1 美味しい
# # うどん
# qid:1 1:0 2:1 3:1 4:79192 5:16361 6:53
# # そば
# qid:1 1:1 2:0 3:0 4:54470 5:10100 6:8
file_contents = []
rank_id = 1
puts_line("reading count results and insert gold ranking", OUT_LENGTH, STDERR)
num_of_data.times do |i|
    path = "#{path_to_output}/result#{i + 1}.txt"
    contents = read_output_file(path, g_queries, rank_id)
    unless contents.nil?
        file_contents << contents
        rank_id = rank_id + 1
    else
        STDERR.puts "erro!: rank#{rank_id}"
    end
end
STDERR.puts "#{rank_id - 1} files are okay!"
puts_line("", OUT_LENGTH, STDERR)


# create train/test_data with combination
puts_line("create train/test_data", OUT_LENGTH, STDERR)
Parallel.each(0...file_contents.size, in_processes: 8) do |i|
    for_test = file_contents[i]

    train_data = "#{train_data_path}/train#{i}.txt"
    test_data = "#{test_data_path}/test#{i}.txt"
    tune_data_sub_path = "#{tune_data_path}/tune#{i}"

    STDERR.puts "train: #{train_data}, test: #{test_data}, tune: #{tune_data_sub_path}/"

    # ----- train data -----
    # create train data without a test data (N - 1)
    for_train = file_contents - [for_test]
    # delete old data
    File.delete(train_data) if File.exist?(train_data)
    for_train.each do |content|
        File.open(train_data, "a") do |f|
            f.write(content)
        end
    end

    # ----- test data ------
    File.write(test_data, for_test) # just write it

    tune_train_data_path = "#{tune_data_sub_path}/#{TRAIN_PATH_SUFFIX}"
    FileUtils.mkdir_p(tune_train_data_path)
    tune_test_data_path = "#{tune_data_sub_path}/#{TEST_PATH_SUFFIX}"
    FileUtils.mkdir_p(tune_test_data_path)

    # ----- tune data -----
    for_train.each.with_index do |for_tune_test, j|
        tune_train_data = "#{tune_train_data_path}/train#{j}.txt"
        tune_test_data = "#{tune_test_data_path}/test#{j}.txt"

        # create train data without a test and a train data (N - 1 - 1)
        for_tune_train = for_train - [for_tune_test]

        # delete old data
        File.delete(tune_train_data) if File.exist?(tune_train_data)
        for_tune_train.each do |content|
            File.open(tune_train_data, "a") do |f|
                f.write(content)
            end
        end

        # === test data ===
        File.write(tune_test_data, for_tune_test) # just write it
    end
end
puts_line("", OUT_LENGTH, STDERR)
