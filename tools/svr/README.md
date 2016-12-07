# SVR
You can use [liblinear](https://github.com/cjlin1/liblinear) to order concepts.
Here, we describe how to use it for this purpose.
Please refer the site above for more detail.

## Install
Please run `./install.sh` to install it.

## How to use it
liblinear has many options. Please run `./train -h` for more information.

Some main options are showed below:

```
-s type : set type of solver (default 1)
for regression
    11 -- L2-regularized L2-loss support vector regression (primal)
    12 -- L2-regularized L2-loss support vector regression (dual)
    13 -- L2-regularized L1-loss support vector regression (dual)
-c cost : set the parameter C (default 1)
-v n: n-fold cross validation mode
```

### Training
commands:

```
Usage: train [options] training_set_file [model_file]
```

### Predicting
commands:

```
Usage: predict [options] test_file model_file output_file
```

## Data format
The format of data is same as libsvm.

Data looks like:

```
3 1:1 2:1 3:0 4:0.2 5:0
2 1:0 2:0 3:1 4:0.1 5:1
1 1:0 2:1 3:0 4:0.4 5:0
1 1:0 2:0 3:1 4:0.3 5:0
```
