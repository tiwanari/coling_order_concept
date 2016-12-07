#!/bin/sh
find ./feed_neologd/2012 -type f |  xargs xz -dc | ruby -Ku -pe '$_ = $_ =~ /^(#|\*|E)/ ? ($_ =~ /^E/ ? "\n" : "") : $_.split("\t")[0] + " "' >> feed-2012.MRF &
find ./tweet_neologd/2012 -type f |  xargs xz -dc | ruby -Ku -pe '$_ = $_ =~ /^(#|\*|E)/ ? ($_ =~ /^E/ ? "\n" : "") : $_.split("\t")[0] + " "' >> tweet-2012.MRF &
