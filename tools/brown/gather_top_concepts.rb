#!/usr/bin/env ruby
require_params = 2
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} input_file top_k"
    puts "e.g. #{__FILE__} ./feed-2009_1_10.c1024 30"
    exit 1
end

input_file = ARGV[0]
top_k = ARGV[1].to_i

clusters = Hash.new
# read a file
File.open(input_file) do |file|
    file.each_line do |line|
        cluster, concept, _ = line.strip.split("\t")
        # puts "#{cluster} #{concept} #{freq}"
        if clusters[cluster].nil?   # no element in the cluster
            clusters[cluster] = [concept]
        elsif clusters[cluster].size() < top_k
            clusters[cluster] << concept
        end
    end
end

# output results
clusters.each do |cluster, arr|
    print "#{cluster}\t"
    arr.each do |concept|
        print " #{concept},"
    end
    print "\n"
end
