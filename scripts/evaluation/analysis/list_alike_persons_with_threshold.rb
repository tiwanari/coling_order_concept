#!/usr/bin/env ruby
require 'set'
require 'yaml'
require 'csv'
require_relative "../util/spearman"
require_relative "../util/print"
require_relative "../util/count"

def path_to_person(path_prefix, i)
    "#{path_prefix}/rank#{i}.csv"
end

def load_user_infos(csv_file)
    data = {}
    CSV.read(csv_file, headers: true).each do |row|
        data[row["person"]] = {:age => row["age"].to_i,
                                :gender => row["gender"],
                                :region => row["region"],
                                :sns => row["sns"]}
    end
    return data
end

def puts_graphvis_header(graph_name)
header = <<"EOS"
digraph #{graph_name} {
\tgraph [
\t];
EOS
    puts header
end

def puts_graphvis_footer
footer = <<"EOS"
}
EOS
    puts footer
end

def age_to_gradation_red(age)
    max = 65
    min = 19
    other = ( 230 - (age - min).to_f / (max - min).to_f * 230 + 20 ).to_i.to_s(16)
    return "#FF#{other}#{other}"
end

def gender_to_color(gender)
    case gender
    when "男性" then
        return "blue"
    when "女性" then
        return "green"
    else
        STDERR.puts "ERROR: #{gender} is not a gender!"
        exit 1
    end
end

def region_to_shpe(region)
    case region
    when "北海道" then
        return "ellipse"
    when "東北" then
        return "polygon"
    when "関東" then
        return "egg"
    when "中部" then
        return "diamond"
    when "近畿" then
        return "pentagon"
    when "中国" then
        return "house"
    when "四国" then
        return "trapezium"
    when "九州" then
        return "box"
    end
end

def job_to_bar(job)
    return ""
end

def sns_to_foo(sns)
    return ""
end

def puts_graphvis_nodes(num_of_nodes, user_info_csv_file, path_to_persons)
    user_infos = load_user_infos(user_info_csv_file)

    num_of_nodes.times.with_index do |r|
        index = r

        file_path = path_to_person(path_to_persons, r)
        if File.symlink?(file_path) then
            index = File.readlink(file_path).gsub(/[^0-9]/,"").to_i
            STDERR.puts "map rank#{r} -> rank#{index}"
        end

node_info = <<"EOS"
\t#{r} [
\t\tlabel = "#{r} (#{user_infos[index.to_s][:sns]})",
\t\tstyle = "solid,filled",
\t\tshape = #{region_to_shpe(user_infos[index.to_s][:region])},
\t\tcolor = "#{gender_to_color(user_infos[index.to_s][:gender])}",
\t\tfillcolor = "#{age_to_gradation_red(user_infos[index.to_s][:age])}",
\t];
EOS
        puts node_info
    end
end

def puts_graphvis_edge(from, to, label)
    puts "\t#{from} -> #{to} ["
    puts "\t\tlabel = \"#{label}\";"
    puts "\t];"
end




config = YAML.load(File.open("../config.yml").read)
QUERIES_FOLDER = "../#{config["directory"]["queries"]}"

require_params = 3
if ARGV.length < require_params
    puts "This script needs at least #{require_params} parameters"
    puts "Usage: #{__FILE__} user_infos_csv path_to_persons_data threshold"
    puts "\t- path_to_persons_data should be in #{QUERIES_FOLDER})"
    puts "e.g. #{__FILE__} queries9/gold/human/all/analysis/infos.csv queries9/gold/human/all 0.65"
    exit 1
end


user_infos_csv = "#{QUERIES_FOLDER}/#{ARGV[0]}"
path_to_persons = "#{QUERIES_FOLDER}/#{ARGV[1]}"
threshold = ARGV[2].to_f

num_of_person_data = count_files(path_to_persons, "rank*") # count ranks (human data)

STDERR.puts "path_to_persons: #{path_to_persons}"
STDERR.puts "num_of_person_data: #{num_of_person_data}"
STDERR.puts "threshold: #{threshold}"
STDERR.puts "user infos: #{user_infos_csv}"

persons = []
num_of_person_data.times do |i|
    x = path_to_person(path_to_persons, i)
    xss = avg_ranks(read_ranks_from_file(x))
    persons << xss
end


puts_graphvis_header("alike_more_than_threshold")
puts_graphvis_nodes(num_of_person_data, user_infos_csv, path_to_persons)

persons.each_with_index do |center, p|
    rhos = []
    invalid_memo = Set.new # memorize invalid data

    persons.each_with_index do |other, q|
        if p == q then
            rhos[q] = -2.0
            next
        end
        num_of_valid_data = 0

        rho = 0.0
        center.size.times do |i|
            begin
                r = spearman(center[i], other[i])
                raise "r = Nan" if r.nan?
            rescue => ex
                invalid_memo << "rank#{p} & rank#{q}: query#{i}, #{ex.message}"
                next
            end
            num_of_valid_data += 1
            rho += r
        end
        rho /= num_of_valid_data.to_f

        rhos[q] = rho
    end

    STDERR.puts "more than the threshold for rank#{p} are ..."
    STDERR.puts "center, nearest no., other, ave. rho"

    rhos.map.with_index              # add index as [rho, index]
                .sort_by {|v, i| -v} # sort by rhos
                .select {|v, i| v >= threshold} # choose over the threshold
                .each.with_index(1) do |(v, i), r|
        # puts "#{p},#{r},#{i},#{v.round(3)}"
        puts_graphvis_edge(p, i, "#{r}, #{v.round(3)}")
    end

    unless invalid_memo.empty?
        STDERR.puts "invalid data for rank#{p}"
        invalid_memo.each do |memo|
            STDERR.puts memo
        end
    end
end

puts_graphvis_footer
