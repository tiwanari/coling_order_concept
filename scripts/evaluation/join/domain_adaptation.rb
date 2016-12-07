#!/usr/bin/env ruby
require 'find'
require 'fileutils'
require_relative '../util/print'
require 'parallel'

# reset random seed
srand(703266108192116103993606214654)
N_SAMPLE = 120 # = 5!

module Classifier
    SVM = 0
    SVR = 1
end

def adjust_hint_numbers(hints, skip, hint_offset)
    # split hints and convert hint_list to hash
    hint_array = []
    row_id = 0
    before_hints = ""
    hints.strip.split(" ").each_with_index do |hint, i|
        # just save the value
        if i < skip
            before_hints.concat("#{hint} ")
            next
        end
        # check values
        id, value = hint.split(":")
        if id.to_i == row_id + 1
            hint_array[row_id] = value
            row_id += 1
        else
            STDERR.puts "something wrong!!"
        end
    end
    hint_str = ""
    hint_array.each.with_index(0) do |value, i|
        hint_str.concat("#{i + hint_offset}:#{value} ")
    end

    return before_hints, hint_str, hint_offset + row_id
end

def join_domains(contents0, contents1, classifier)
    body = ""
    contents0.each do |key, hint|
        if classifier == Classifier::SVM
            body.concat("# #{key}\n")
            before_hints, hint_str, offset = adjust_hint_numbers(hint, 2, 1)
            body.concat(before_hints) # add domain0 hints
            body.concat(hint_str) # add domain0 hints
            _, hint_str, _ = adjust_hint_numbers(contents1[key], 2, offset)
            body.concat(hint_str) # add domain1 hints
            body.concat("\n")
        elsif classifier == Classifier::SVR
            before_hints, hint_str, offset = adjust_hint_numbers(hint, 1, 1)
            body.concat(before_hints) # add domain0 hints
            body.concat(hint_str) # add domain0 hints
            _, hint_str, _ = adjust_hint_numbers(contents1[key], 1, offset)
            body.concat(hint_str) # add domain1 hints
            body.concat("##{key}\n")
        end
    end
    return body
end

def read_output_file(filename, method)
    adj = ""
    content = {}
    puts "reading #{filename}"
    File.open(filename) do |file|
        if method == Classifier::SVM
            adj = file.gets.split(" ").last.strip
            file.readlines.each_slice(2) do |lines|
                # "lines" looks like this:
                # # うどん
                # qid:1 1:0 2:3 ...
                # use the first line
                concept = lines.first.split(" ").last.strip
                # use the second line
                content[concept] = lines.last.strip
            end
        elsif method == Classifier::SVR
            file.each_line do |line|
                # each line looks like this:
                # rho 1:val_1 2:val_2 ... # adj concept0, concept1, ...
                # e.g. 0.1 1:0.1 2:0.3 ... # 大きい イヌ,ネコ,...
                hint, query = line.strip.split("#")
                content[query] = hint
            end
        end
    end
    return adj, content
end

puts "This script is not completed"
exit 1

require_params = 3
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} path_to_domain0_queries path_to_domain1_queries path_to_output"
    puts "e.g. #{__FILE__} ../../../result/gather/ver9/feed/queries4 ../../../result/gather/ver9/tweet/queries4 ../../../result/gather/ver9/join4"
    exit 1
end

OUT_LENGTH = 80
PATH_TO_SVM_DATA_SUFFIX = "svm"
PATH_TO_SVR_DATA_SUFFIX = "svr"
TRAIN_PATH_SUFFIX = "train_data"
TEST_PATH_SUFFIX = "test_data"
TUNE_PATH_SUFFIX = "tune_data"

path_to_domain0_queries = ARGV[0]
path_to_domain1_queries = ARGV[1]
path_to_output = ARGV[2]

puts "path_to_domain0_queries: #{path_to_domain0_queries}"
puts "path_to_domain1_queries: #{path_to_domain1_queries}"
puts "path_to_output: (svm: #{path_to_output}/#{PATH_TO_SVM_DATA_SUFFIX}, svr: #{path_to_output}/#{PATH_TO_SVR_DATA_SUFFIX})"


FileUtils.mkdir_p("#{path_to_output}/#{PATH_TO_SVM_DATA_SUFFIX}")
FileUtils.mkdir_p("#{path_to_output}/#{PATH_TO_SVR_DATA_SUFFIX}")

cases = [\
[Classifier::SVM, [0, 1]],\
[Classifier::SVR, [0, 1]]]

# create data
cases.each do |classifier, domains|
    # gather objective
    adjs = []
    contents = []
    domains.each do |domain|
        adjs[domain] = []
        contents[domain] = []
        # set target/output file path based on the domain and classifier
        path_to_target_dir = ""
        if domain == 0
            path_to_target_dir.concat(path_to_domain0_queries)
        elsif domain == 1
            path_to_target_dir.concat(path_to_domain1_queries)
        end
        if classifier == Classifier::SVM
            path_to_target_dir.concat("/#{PATH_TO_SVM_DATA_SUFFIX}/test_data")
        elsif classifier == Classifier::SVR
            path_to_target_dir.concat("/#{PATH_TO_SVR_DATA_SUFFIX}/test_data")
        end

        # read svm or svr results and extend their hint values,
        # and write the extended results into files
        files = Dir.glob("#{path_to_target_dir}/test*.txt")
        files.size.times do |i|
            filename = "#{path_to_target_dir}/test#{i}.txt"
            adj, content = read_output_file(filename, classifier)
            unless content.nil?
                adjs[domain] << adj
                contents[domain] << content
            else
                STDERR.puts "error!: #{filename}"
            end
        end
    end

    file_contents = []
    adjs[0].size.times do |query_id|
        body = ""
        if adjs[0][query_id] == adjs[1][query_id] && contents[0][query_id].size == contents[1][query_id].size
            body.concat("# #{adjs[0][query_id]}\n") if classifier == Classifier::SVM
            body.concat(join_domains(contents[0][query_id], contents[1][query_id], classifier))
            unless body.nil?
                file_contents << body
            else
                STDERR.puts "error!: something wrong on query #{query_id}"
            end
        else
            STDERR.puts "error!: something wrong on query #{query_id}"
            STDERR.puts "error!: domain 0 differs from domain 1"
            exit 1
        end
    end

    path_to_output_dir = ""
    if classifier == Classifier::SVM
        path_to_output_dir = "#{path_to_output}/#{PATH_TO_SVM_DATA_SUFFIX}"
    elsif classifier == Classifier::SVR
        path_to_output_dir = "#{path_to_output}/#{PATH_TO_SVR_DATA_SUFFIX}"
    end

    # write the joined data
    train_data_path = "#{path_to_output_dir}/#{TRAIN_PATH_SUFFIX}"
    test_data_path = "#{path_to_output_dir}/#{TEST_PATH_SUFFIX}"
    tune_data_path = "#{path_to_output_dir}/#{TUNE_PATH_SUFFIX}"
    FileUtils.mkdir_p(train_data_path)
    FileUtils.mkdir_p(test_data_path)
    FileUtils.mkdir_p(tune_data_path)

    # create train/test_data with combination
    puts_line("create train/test_data", OUT_LENGTH)
    Parallel.each(0...file_contents.size, in_processes: 40) do |i|
        for_test = file_contents[i]
        train_data = "#{train_data_path}/train#{i}.txt"
        test_data = "#{test_data_path}/test#{i}.txt"
        tune_data_sub_path = "#{tune_data_path}/tune#{i}"
        tune_train_data_path = "#{tune_data_sub_path}/#{TRAIN_PATH_SUFFIX}"
        tune_test_data_path = "#{tune_data_sub_path}/#{TEST_PATH_SUFFIX}"
        FileUtils.mkdir_p(tune_train_data_path)
        FileUtils.mkdir_p(tune_test_data_path)

        puts "train: #{train_data}, test: #{test_data}, tune: #{tune_data_sub_path}/"

        # ----- train data -----
        # create train data without a test data (N - 1)
        for_train = file_contents - [for_test]
        # delete old data
        File.delete(train_data) if File.exist?(train_data)
        for_train.each do |content|
            if classifier == Classifier::SVR
                content = content.split("\n").sample(N_SAMPLE).join("\n").concat("\n")
            end
            File.open(train_data, "a") do |f|
                f.write(content)
            end
        end

        # ----- test data ------
        File.write(test_data, for_test) # just write it

        # ----- tune data -----
        for_train.each.with_index do |for_tune_test, j|
            tune_train_data = "#{tune_train_data_path}/train#{j}.txt"
            tune_test_data = "#{tune_test_data_path}/test#{j}.txt"

            # create train data without a test and a train data (N - 1 - 1)
            for_tune_train = for_train - [for_tune_test]

            # delete old data
            File.delete(tune_train_data) if File.exist?(tune_train_data)
            for_tune_train.each do |content|
                if classifier == Classifier::SVR
                    content = content.split("\n").sample(N_SAMPLE).join("\n").concat("\n")
                end
                File.open(tune_train_data, "a") do |f|
                    f.write(content)
                end
            end
            # === test data ===
            File.write(tune_test_data, for_tune_test) # just write it
        end
    end
    puts_line("", OUT_LENGTH)
end
