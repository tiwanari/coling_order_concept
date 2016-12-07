#!/usr/bin/env ruby
require_relative '../util/print'

# This script do the baseline method by combine
# - the positive comparative (plus) c_plus
# - the negative comparative (minus) c_minus
# as c_plus - c_minus

require_params = 1
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} path_to_file"
    puts "e.g. #{__FILE__} ../../result/gather/ver6/result1.txt"
    exit 1
end

OUT_LENGTH = 80
path_to_file = ARGV[0]

STDERR.puts "read file: #{path_to_file}"

puts_line("sorting", OUT_LENGTH, STDERR)
contents = {}
adj = ""
File.open(path_to_file) do |file|
    # ranking file looks like this:
    # # 大きい
    # # イヌ
    # 4 qid:1 1:-0.428144 2:-0.0173882 3:0.385219 4:0.333333 5:0.333333
    # # ウシ
    # ...

    # adj
    # e.g.
    # # 大きい
    adj = file.gets.split(" ").last
    file.readlines.each_slice(2) do |lines|
        # lines looks like this (2 lines -> first, last):
        # # イヌ
        # 4 qid:1 1:-0.428144 2:-0.0173882 3:0.385219 4:0.333333 5:0.333333
        first_line = lines.first.strip
        second_line = lines.last.strip
        # e.g.
        # # イヌ
        concept = first_line.split(" ").last
        # e.g.
        # 4 qid:1 1:-0.428144 2:-0.0173882 3:0.385219 4:0.333333 5:0.333333
        values = second_line.split(" ")
        # positive - negative
        target_column = values[4].sub(/\d+:/, "").to_f - values[5].sub(/\d+:/, "").to_f
        # concept => value
        contents[concept] = target_column
    end
end

# sort with values in descending ("-" means it) order
sorted = contents.sort_by {|k, v| -v }


contents = "# #{adj}\n"

# concatenate results
cur_rank = 1

sorted_concepts_str = ""
sorted_values_str = ""
sorted.each.with_index do |(k, v), i|
    unless i == 0
        contents.concat(",")
        sorted_concepts_str += " > "
        sorted_values_str += " > "
        cur_rank = i + 1 # new rank
    end
    # check values
    sorted_concepts_str += k
    sorted_values_str += sprintf("%+.2f", v)

    contents.concat("#{k},#{cur_rank}")
end
STDERR.puts "#{adj}: #{sorted_concepts_str}"
STDERR.puts "\t-> #{sorted_values_str}"

puts_line("result", OUT_LENGTH, STDERR)
puts contents
