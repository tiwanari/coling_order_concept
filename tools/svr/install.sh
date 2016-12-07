#!/bin/sh
# @(#) this is based on the procedure written in
# @(#) https://www.csie.ntu.edu.tw/~cjlin/libsvmtools
echo "----- preparation -----"
if [ -d liblinear ]; then
    echo "liblinear exists"
else
    echo "liblinear does not exist"
    echo "----- downloading -----"
    git submodule init; git submodule update
fi
echo "----- making -----"
patch -d liblinear < add_ablation_test.patch
cd liblinear
make
