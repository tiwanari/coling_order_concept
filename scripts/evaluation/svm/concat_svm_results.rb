#!/usr/bin/env ruby
require 'find'
require_relative '../util/print'

require_params = 2
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} path_to_prediction_and_test output_path"
    puts "e.g. #{__FILE__} ../../result/gather/ver16/queries4/svm ../../result/gather/ver16/svm_result_c0.01_k0.txt"
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
        File.open(annotate_file) do |af|
            # ====== from here, svm specific codes =====

            # test (annotate) file looks like this:
            # # 大きい
            # # イヌ
            # 4 qid:1 1:-0.428144 2:-0.0173882 3:0.385219 4:0.333333 5:0.333333
            # # ウシ
            # ...

            # adjective
            # e.g.
            # # 大きい
            top_line = af.gets
            contents.concat(top_line)
            adj = top_line.strip.split(" ").last
            # concept (from test) and values (from prediction)
            ranking = {}
            af.readlines.each_slice(2) do |concept, hint|
                # concept from test
                # e.g.
                # # イヌ
                c = concept.split(" ").last.strip
                # value from prediction
                # e.g.
                # 0.22592685
                v = pf.gets.strip.to_f
                # concept => value
                ranking[c] = v
            end
            # sort with values in descending ("-" means it) order (svm result)
            sorted = ranking.sort_by {|(_, v)| -v }
            # concatenate results
            cur_rank = 1

            sorted_concepts_str = ""
            sorted_values_str = ""
            sorted.each.with_index do |(k, v), j|
                unless j == 0
                    contents.concat(",")
                    sorted_concepts_str += " > "
                    sorted_values_str += " > "
                    cur_rank = j + 1 # new rank
                end
                # check values
                sorted_concepts_str += k
                sorted_values_str += sprintf("%+.2f", v)

                contents.concat("#{k},#{cur_rank}")
            end
            puts "#{adj}: #{sorted_concepts_str}"
            puts "\t-> #{sorted_values_str}"
            contents.concat("\n")

            # ====== to here, svm specific codes =====
        end
    end
end

# write result
File.write(path_to_output, contents)

puts_line("params (show again)", OUT_LENGTH)
puts "prediction path: #{path_to_prediction}"
puts "test path: #{path_to_test}"
puts "output path: #{path_to_output}"
