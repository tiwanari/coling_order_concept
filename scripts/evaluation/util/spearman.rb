# read ranks from a file as an array of array of pair
# e.g.
# クジラ,1,キリン,2,ゾウ,2
# 飛行機,1,新幹線,2,ヘリコプター,3
# ->
# [[["クジラ", "1"], ["キリン", "2"], ["ゾウ", "2"]],
#  [["飛行機", "1"], ["新幹線", "2"], ["ヘリコプター", "3"]]]
def read_ranks_from_file(filename)
    rankings = []
    # read each line
    File.foreach(filename).each do |line|
        # skip comment line
        if line.start_with?("#")
            next
        end
        # split line into items
        items = line.strip.split(",")
        # make pairs in every 2 items
        # e.g. ["クジラ", "1", "キリン", "2", "ゾウ", "2"]
        # -> [["クジラ", "1"], ["キリン", "2"], ["ゾウ", "2"]]
        rankings << items.each_slice(2).to_a
    end
    return rankings
end

# calculate the average of sequence
# e.g. from: 8, to: 9 -> (8 + 9) / 2 = 8.5
def avg(from, to)
    range = (from..to)
    return range.reduce(:+).to_f / range.size
end

# calculate average rankings
# note: it also convert array of array of pair into
#       array of hash (:string => int)
# e.g.
# [[["クジラ", "1"], ["キリン", "2"], ["ゾウ", "2"]],
#  [["飛行機", "1"], ["新幹線", "2"], ["ヘリコプター", "3"]]]
# ->
# [{:クジラ => 1.0, :キリン => 2.5, :ゾウ => 2.5},
#  {:飛行機 => 1.0, :新幹線 => 2.0, :ヘリコプター => 3.0}]
# (I don't know the order of hash elements)
def avg_ranks(xs)
    rankings = []
    # check each ranking
    xs.each do |ranking|
        new_ranking = {}
        # traverse reverse order (ranking.length-1 -> 0)
        cur_rank = ranking.length
        while cur_rank > 0 do
            (concept, r) = ranking[cur_rank - 1]
            # check the current item has a correct value
            # e.g. #K has rank K (r.to_i == cur_rank) or not
            # if not, replace these values with their average
            # e.g. 1,2,2,4
            # (reversal traverse)
            # -> 4: OK, 2: NG
            # -> replace 2s with (2 + 3) / 2 = 2.5
            if r.to_i < cur_rank
                # we know how many items should be replaced by checking r.to_i
                # e.g. 1,2,2,4 -> replace #3 to #2 (r.to_i)
                to = r.to_i
                avg_rank = avg(to, cur_rank)
                # replace values with their average
                while to <= cur_rank
                    (concept, r) = ranking[cur_rank - 1]
                    new_ranking[concept] = avg_rank
                    cur_rank = cur_rank - 1
                end
            else
                # if it is correct, just add it into new_ranking
                new_ranking[concept] = r.to_f
                cur_rank = cur_rank - 1
            end
        end
        rankings << new_ranking
    end
    return rankings
end

# calculate Spearman's rho
def spearman(xs, ys)
    # d2 = 0
    # # sum d^2
    # xs.each do |key, val|
    #     d2 = d2 + (val - ys[key])**2
    # end
    # # NOTE: Only if all n ranks are distinct integers, it can be computed using the popular formula
    # # see https://en.wikipedia.org/wiki/Spearman%27s_rank_correlation_coefficient
    # # 1 - (6 * sum d^2) / (n * (n^2 - 1))
    # return 1 - (6 * d2) / (xs.length * (xs.length**2 - 1))
    xlist = []
    ylist = []
    xs.each do |key, val|
        xlist << val.to_f
        if ys[key].nil?
            throw Exception.new("no such key: #{key}")
        end
        ylist << ys[key].to_f
    end
    return spearman2(xlist, ylist)
end

# calculate Spearman's rho
def spearman2(xs, ys)
    x_avg = xs.reduce(:+) / xs.length
    y_avg = ys.reduce(:+) / ys.length
    numerator = xs.zip(ys).inject(0.0) {|sum, (x, y)| sum += (x - x_avg) * (y - y_avg)}
    denominator = Math.sqrt(xs.inject(0.0) {|sum, x| sum += (x - x_avg) ** 2}) \
                    * Math.sqrt(ys.inject(0.0) {|sum, y| sum += (y - y_avg) ** 2})
    rho = numerator / denominator
    return rho
end

# calculate Spearman's rhos
def spearmans(xss, yss)
    rhos = []
    # check each ranking
    # NOTE: this assumes 2 ranking sets has the same order of rankings
    for i in 0...xss.length
        xs = xss[i]
        ys = yss[i]
        rhos << spearman(xs, ys)
    end
    return rhos
end
