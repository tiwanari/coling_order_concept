#!/usr/bin/env ruby
require_relative "print"
require 'fileutils'
require 'yaml'

# This script was updated after ver17 results
# if you want to run evaluations on old results
# you should care about 'epsilon'
# e.g., old files have names like
# svm_result_ep0_c0.1_k0.txt
# but, now we get
# svm_result_c0.1_k0.txt
# (ep = epsilon)

config = YAML.load(File.open("../config.yml").read)
RESULTS_FOLDER = config["directory"]["results"]

OUT_LENGTH = 80

require_params = 2
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} path_to_data kernel|type"
    puts "\t-path_to_data should be in #{RESULTS_FOLDER})"
    puts "e.g. #{__FILE__} ver17/join4/svm k0|t11"
    exit 1
end


path_to_data = ARGV[0]
type = ARGV[1]
classifier = File.basename(path_to_data)
classifier = "svm" if classifier =~ /.*svm.*/
classifier = "svr" if classifier =~ /.*svr.*/

path_to_tuned_cs_file = "#{path_to_data}/tuned_cs.txt"
path_to_tuned_rhos_file = "#{path_to_data}/tuned_rhos.txt"
path_to_tuned_ranking_file = "#{path_to_data}/tuned_ranking.txt"

RANKING_FILE_TEMPLATE = "#{classifier}_result_c%s_#{type}.txt"
RHOS_FILE_TEMPLATE = "rhos_c%s_#{type}.txt"

STDERR.puts "path_to_data: #{path_to_data}"
STDERR.puts "type: #{type}"
STDERR.puts "classifier: #{classifier}"
STDERR.puts "path_to_tuned_cs_file: #{path_to_tuned_cs_file}"
STDERR.puts "path_to_output: (rhos: #{path_to_tuned_rhos_file}, ranking: #{path_to_tuned_ranking_file})"

query_cs = []
File.open(path_to_tuned_cs_file) do |file|
    file.each_line do |line|
        # each line looks like this:
        # best (i, c, rho) = (0, 100.0, 0.46766513056835635)
        data = line.strip.split(") = (").last # e.g. 0, 100.0, 0.46766513056835635)
        query_id, c_value, _ = data.split(", ")
        c_str = sprintf("%g", c_value.to_f)
        # convert 10e-2 -> 0.01
        # NOTE: this assumes the suffix number is a decimal digit (i.e., 1-9 not 10 or more)
        if c_str.include?("e")
            c_str = "0." + ("0" * (c_value[c_value.length-1].to_i - 1)) + c_value[0]
        end
        query_cs[query_id.to_i] = c_str
    end
end

rhos_body = ""
ranking_body = ""
query_cs.each.with_index do |c, i|
    # TODO: run script
    unless File.exist?("#{path_to_data}/#{sprintf(RHOS_FILE_TEMPLATE, c)}")
        STDERR.puts "error: #{sprintf(RHOS_FILE_TEMPLATE, c)} is not found. c = #{c} not evaluated!"
        next
    end

    # puts_line("reading rho file...", OUT_LENGTH, STDERR)
    File.open("#{path_to_data}/#{sprintf(RHOS_FILE_TEMPLATE, c)}") do |file|
        # skip 3 lines (they are a kind of notifications)
        file.gets
        file.gets
        file.gets
        file.each_line do |line|
            query_id, rho = line.split(" ")
            if query_id.to_i == i
                rhos_body.concat("#{query_id} #{rho}\n")
                break
            end
        end
    end

    # puts_line("reading ranking file...", OUT_LENGTH, STDERR)
    File.open("#{path_to_data}/#{sprintf(RANKING_FILE_TEMPLATE, c)}") do |file|
        file.readlines.each_slice(2).with_index do |(adj, ranking), j|
            if i == j
                ranking_body.concat("#{adj}#{ranking}")
                break
            end
        end
    end
end

File.write(path_to_tuned_rhos_file, rhos_body)
File.write(path_to_tuned_ranking_file, ranking_body)
