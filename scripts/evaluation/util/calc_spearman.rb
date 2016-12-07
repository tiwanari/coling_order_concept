#!/usr/bin/env ruby
require_relative "spearman"
require_relative 'print'

def read_adjs_from_file(filename)
    adjs = []
    # read each line
    File.foreach(filename).each do |line|
        # skip real value line
        unless line.start_with?("#")
            next
        end
        # split line into items
        items = line.strip.split(" ")
        # insert adjective (in person's data, items[1] may be comment)
        adjs << items[1] if items[1].split(",").size == 1 # comment check
    end
    return adjs
end

require_params = 2
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} ranking1 ranking2 [show_annotate (any value is ok)]"
    puts "e.g. #{__FILE__} ./gold/gold0.csv ../../result/gather/ver1/svm_result.txt"
    exit 1
end

OUT_LENGTH = 40

ranking0 = ARGV[0]
ranking1 = ARGV[1]
STDERR.puts "ranking0: #{ranking0}"
STDERR.puts "ranking1: #{ranking1}"

adj_ranking0 = read_adjs_from_file(ranking0)
adj_ranking1 = read_adjs_from_file(ranking1)
show_annotate = (ARGV.length == require_params + 1)

if adj_ranking0 == adj_ranking1
    puts "OK: they are same order"
    puts "rank: #{adj_ranking0.to_s}"
else
    puts "NG: they are NOT same order"
    puts "rank0: #{adj_ranking0.to_s}"
    puts "rank1: #{adj_ranking1.to_s}"
    exit 1
end

ranking0 = avg_ranks(read_ranks_from_file(ranking0))
ranking1 = avg_ranks(read_ranks_from_file(ranking1))


rhos = spearmans(ranking0, ranking1)
puts_line("rhos", OUT_LENGTH)
rhos.size.times do |i|
    if show_annotate
        puts "#{rhos[i]} -> #{adj_ranking0[i]}"
    else
        puts "#{i} #{rhos[i]}"
    end
end
puts_line("average", OUT_LENGTH)
puts rhos.reduce(:+) / rhos.size.to_f
