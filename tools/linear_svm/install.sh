#!/bin/sh
# @(#) this is based on the procedure written in
# @(#) https://www.csie.ntu.edu.tw/~cjlin/libsvmtools
echo "----- preparation -----"
if [ -f liblinear-ranksvm-2.1.zip ]; then
    echo "liblinear-ranksvm-2.1.zip exists"
else
    echo "liblinear-ranksvm-2.1.zip does not exist"
    echo "----- downloading -----"
    wget https://www.csie.ntu.edu.tw/~cjlin/libsvmtools/ranksvm/liblinear-ranksvm-2.1.zip
    unzip liblinear-ranksvm-2.1.zip
fi
echo "----- making -----"
patch -d liblinear-ranksvm-2.1 < ignore_comment_line.patch
patch -d liblinear-ranksvm-2.1 < add_ablation_test.patch
cd liblinear-ranksvm-2.1
make
