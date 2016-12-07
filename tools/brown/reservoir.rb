#!/usr/bin/env ruby

# initialize random
SEED = 52486726322433269207697849189932
srand(SEED)

require_params = 1
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} num_of_samples"
    puts "e.g. #{__FILE__} 10 < lines.txt"
    exit 1
end

num_of_samples = ARGV[0].to_i

n = 0
res = []

# initialize the result array with the first k elements
while line = STDIN.gets do
    res[n] = line.chomp
    n += 1
    break if n == num_of_samples
end

# sampling
while line = STDIN.gets do
    r = rand(0..n)
    res[r] = line.chomp if r < num_of_samples
    n += 1
end

# write results
res.each do |line|
    puts line
end
