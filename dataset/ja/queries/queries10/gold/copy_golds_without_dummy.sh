#!/bin/sh

src="../../queries9/gold/golds"
dst="./golds"

for d in `cat ./golds/lists.txt`
do
    mkdir -p ${dst}/${d}
    sed -e '59,60d' ${src}/${d}/gold.csv        | sed -e '35,36d'    > ${dst}/${d}/gold.csv
    sed -e '$d'     ${src}/${d}/gold.txt        | sed -e '18d'        > ${dst}/${d}/gold.txt
    sed -e '$d'     ${src}/${d}/for_plot.gpdata | sed -e '18d' > ${dst}/${d}/for_plot.gpdata

    cat ${src}/${d}/invalid_data_lists.txt | grep -v 'query18:' | grep -v 'query30:' \
        > ${dst}/${d}/invalid_data_lists.txt
done
