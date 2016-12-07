#!/usr/bin/env ruby
require 'json'

require_params = 3
if ARGV.length < require_params
    STDERR.puts "This script needs at least #{require_params} parameters"
    STDERR.puts "Usage: #{__FILE__} path_to_test_data path_to_prediction"
    exit 1
end

path_to_test = ARGV[0]
path_to_prediction = ARGV[1]
path_to_reasons = ARGV[2]
num_of_sentences = ARGV[3].to_i

STDERR.puts "prediction path: #{path_to_prediction}"
STDERR.puts "test path: #{path_to_test}"
STDERR.puts "reasons path: #{path_to_reasons}"

contents = {}
File.open(path_to_prediction, "r:utf-8") do |pf|
    # prediction file looks like this:
    # 0.22592685
    # 0.53208453
    # ...
    File.open(path_to_test, "r:utf-8") do |tf|
        # test file looks like this:
        # # 大きい
        # # イヌ
        #  qid:1 1:-0.428144 2:-0.0173882 3:0.385219 4:0.333333 5:0.333333
        # # ウシ
        # ...

        # the first line is an adjective with #
        # e.g.
        # # 大きい
        adj = tf.gets.strip.split(" ").last
        contents[:adj] = adj

        # pairs of a concept (from test) and a value (from prediction)
        pairs = {}
        contents[:ranking] = {}
        tf.readlines.each_slice(2) do |concept, hint|
            # a concept from test
            # e.g.
            # # イヌ
            c = concept.strip.split(" ").last

            # value from prediction
            # e.g.
            # 0.22592685
            v = pf.gets.strip.to_f

            # concept => value
            pairs[c] = v
        end
        # sort concepts with values in descending ("-" means it) order (svm result)
        sorted = pairs.sort_by {|(_, v)| -v }

        # concatenate results
        sorted.each.with_index do |(k, v), j|
            contents[:ranking][(j+1).to_s] = {:concept => k, :value => v}
        end
    end
end

# read ids of concepts
require 'csv'
ID_TO_CONCEPT = "concepts"
id_to_concept = {}
CSV.read(path_to_reasons + "/" + ID_TO_CONCEPT + ".txt", "r:utf-8").each do |row|
    id_to_concept[row[0]] = row[1].chomp
end

CO_OCCURENCE = "co_occurrences"
DEPENDENCE = "dependencies"
SIMILE = "similia"
COMPARATIVE = "comparatives"

files = [CO_OCCURENCE, \
DEPENDENCE, \
SIMILE, \
COMPARATIVE]

reasons = {:max_sentences => num_of_sentences}
files.each do |filename|
    path = path_to_reasons + "/" + filename + ".txt"
    # init sentences
    sentences = {}
    id_to_concept.each do |id, concept|
        sentences[concept] = {:pos_count => 0, :neg_count => 0, \
                              :pos_sentences => [], :neg_sentences => []}
    end
    # read TSV file
    # e.g., id0[TAB]id1[TAB]...[TAB]sentence
    CSV.read(path, "r:utf-8", :col_sep => "\t").shuffle.each do |row|
        # e.g., row = [id0,id1,...,sentence]
        sentence = row.last
        row.pop # sentence should be dropped
        row.each do |id|
            concept = id_to_concept[id]
            # coloring concept
            sentence.sub!(concept, "<span style=\"color: red;\">#{concept}</span>")
            # inserting sentence (pos/neg)
            if sentence[1] == '+'
                sentences[concept][:pos_count] += 1
                next if sentences[concept][:pos_count] > num_of_sentences # limit lines
                sentences[concept][:pos_sentences] << sentence.slice(3..-1)
            elsif sentence[1] == '-'
                sentences[concept][:neg_count] += 1
                next if sentences[concept][:neg_count] > num_of_sentences # limit lines
                sentences[concept][:neg_sentences] << sentence.slice(3..-1)
            end
        end
    end
    reasons[filename.to_sym] = sentences
end
contents[:reasons] = reasons

# write the result in JSON
puts JSON.pretty_generate(contents)
STDOUT.flush
