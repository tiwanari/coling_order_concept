#!/usr/bin/env ruby
require 'set'
require 'yaml'
require_relative "../util/spearman"
require_relative "../util/print"
require_relative "../util/count"

def path_to_person(path_prefix, i)
    "#{path_prefix}/rank#{i}.csv"
end

config = YAML.load(File.open("../config.yml").read)
QUERIES_FOLDER = "../#{config["directory"]["queries"]}"

OUT_LENGTH = 80

require_params = 2
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} path_to_persons_data path_to_gold"
    puts "\t-path_to_* should be in #{QUERIES_FOLDER})"
    puts "e.g. #{__FILE__} queries9/gold/human/all queries9/gold/golds/all/"
    exit 1
end


GOLD_CSV = "gold.csv"

path_to_persons = "#{QUERIES_FOLDER}/#{ARGV[0]}"
path_to_gold = "#{QUERIES_FOLDER}/#{ARGV[1]}/#{GOLD_CSV}"

num_of_person_data = count_files(path_to_persons, "rank*") # count ranks (human data)

STDERR.puts "path_to_persons: #{path_to_persons}"
STDERR.puts "num_of_person_data: #{num_of_person_data}"
STDERR.puts "path_to_gold: #{path_to_gold}"

puts_line("read gold data", OUT_LENGTH, STDERR)
gold = avg_ranks(read_ranks_from_file(path_to_gold))
puts_line("!read gold data", OUT_LENGTH, STDERR)


puts_line("read persons data", OUT_LENGTH, STDERR)
persons = []
num_of_person_data.times do |i|
    x = path_to_person(path_to_persons, i)
    xss = avg_ranks(read_ranks_from_file(x))
    persons << xss
end
puts_line("!read persons data", OUT_LENGTH, STDERR)


puts_line("calc spearman against gold", OUT_LENGTH, STDERR)
rhos = []
invalid_memo = Set.new # memorize invalid data


gold.size.times do |i|
    num_of_valid_data = 0
    rho = 0.0
    persons.each_with_index do |rankings, p|
        begin
            r = spearman(rankings[i], gold[i])
            raise "r = Nan" if r.nan?
        rescue => ex
            invalid_memo << "query#{i}: #{p}, #{ex.message}"
            next
        end
        num_of_valid_data += 1
        rho += r
    end
    rho /= num_of_valid_data # average
    STDERR.puts "query#{i} (ave. rho): #{rho}"
    rhos << rho
end
puts_line("!calc spearman against gold", OUT_LENGTH, STDERR)


puts "rho avg: #{rhos.reduce(:+) / rhos.size}"

puts_line("invalid data lists", OUT_LENGTH, STDERR)
# invalid data lists
invalid_memo.each do |memo|
    STDERR.puts memo
end
