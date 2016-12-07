#!/usr/bin/env ruby
require 'find'
require_relative '../util/print'

require_params = 2
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} path_to_prediction_and_test output_path"
    puts "e.g. #{__FILE__} ../../result/gather/ver16/queries4/svr ../../result/gather/ver16/svr_result_c0.01_t11.txt"
    exit 1
end

OUT_LENGTH = 80

path_to_prediction = "#{ARGV[0]}/validation/prediction"
path_to_test = "#{ARGV[0]}/test_data"
path_to_output = ARGV[1]

puts "prediction path: #{path_to_prediction}"
puts "test path: #{path_to_test}"
puts "output path: #{path_to_output}"

predict_files = []
Find.find(path_to_prediction) do |file|
    next if file == path_to_prediction
    Find.prune if File.directory?(file)
    predict_files << file
end

contents = ""
# concatenate prediction files
puts_line("concatenating files", OUT_LENGTH)
predict_files.size.times do |i|
    predict_file = "#{path_to_prediction}/prediction#{i}.txt"
    annotate_file = "#{path_to_test}/test#{i}.txt"
    puts "predict file: #{predict_file}"
    puts "annotate file: #{annotate_file}"
    File.open(predict_file) do |pf|
        # prediction file looks like this:
        # 0.22592685
        # 0.53208453
        # ...
        File.open(annotate_file) do |tf|
            # ====== from here, svr specific codes =====

            # test file looks like this:
            # 0.11363636363636363 1:0.6071428571428571 ...\
            #   5:0.5357142857142857 # 綺麗だ ひまわり,サクラ,椿,ラベンダー,バラ,桔梗,菊,百合
            # ...

            val_rankings = []
            adj = ""
            # adjective and rankings (from test), and values (from prediction)
            tf.each_line.with_index do |line, j|
                items = line.strip.split(" ")

                # adjective from test
                if j == 0
                    adj = items[items.size - 2]
                    contents.concat("# #{adj}\n")
                end

                # rankings from test
                # e.g.
                # ひまわり,サクラ,椿,ラベンダー,バラ,桔梗,菊,百合
                ranking = items.last

                # value from prediction
                # e.g.
                # 0.22592685
                v = pf.gets.strip.to_f

                # concept => value
                val_rankings << [ranking, v]
            end
            # sort with values in descending ("-" means that) order (svr result)
            best_rank, val = val_rankings.max_by {|(_, v)| v }
            puts val
            rank_with_index = Hash[best_rank.split(",").map.with_index {|concept, j| [concept, j + 1]}]
            contents.concat("#{rank_with_index.flatten.join(",")}\n")

            # ====== to here, svr specific codes =====
        end
    end
end

# write result
File.write(path_to_output, contents)

puts_line("params (show again)", OUT_LENGTH)
puts "prediction path: #{path_to_prediction}"
puts "test path: #{path_to_test}"
puts "output path: #{path_to_output}"
