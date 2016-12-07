# About
This folder contains some scripts to evaluate the method.


# Evaluations

## Gold Standard
We use human annotated rankings to create gold standard.
Use the following script to create gold standard rankings.

Just run [create\_gold\_standard.rb](./create_gold_standard.rb),
a script to create gold standard, which uses [spearman.rb](./util/spearman.rb) as a module.


## Baseline
We use raw values in the counter result files as a baseline.
See result#.txt in [results folder](../../results/gather).
After making a gold standard into [dataset](../../dataset)'s gold folder,
just run `./run_baseline_evaluation.sh`. It shows parameters which are necessary in it.

### Flow
The [run\_baseline\_evaluation.sh](./run_baseline_evaluation.sh) uses scripts in the following order:

1. sort.rb
1. calc\_spearman.rb



## SVM
After making a gold standard into [dataset](../../dataset)'s gold folder,
Just run `./run_svm_evaluation.sh`. It shows parameters which are necessary in it.

### Flow
The [run\_svm\_evaluation.sh](./run_svm_evaluation.sh) uses scripts in the following order:

1. create\_svm\_validation\_data.rb
1. run\_svm\_validation.sh
1. concat\_svm\_results.rb
1. calc\_spearman.rb



## SVR
**The following description is as same as SVM.**

After making a gold standard into [dataset](../../dataset)'s gold folder,
Just run `./run_svr_evaluation.sh`. It shows parameters which are necessary in it.

### Flow
The [run\_svr\_evaluation.sh](./run_svr_evaluation.sh) uses scripts in the following order:

1. create\_svr\_validation\_data.rb
1. run\_svr\_validation.sh
1. concat\_svr\_results.rb
1. calc\_spearman.rb




# Folders
## gold
This folder contains gold standards created by [create\_gold\_standard.rb](./create_gold_standard.rb).

**The folder was moved to [dataset](../../dataset)'s gold folder.**

### gold*.txt
This is a gold standard file in a format
`rho adj concepts`. It looks like

```
0.9893939393939393 大きい クジラ,キリン,ゾウ,クマ,ウシ,ウマ,イヌ,サル,ネコ,ネズミ
0.9333333333333333 安い ハンバーガー,パン,焼きそば,チャーハン,カレー,パスタ,ピザ,ステーキ,寿司
...
```

### gold*.csv
This is a gold standard file in the same format of human annotated files. It looks like:

```
# 大きい
クジラ,1,キリン,2,ゾウ,3,クマ,4,ウシ,5,ウマ,6,イヌ,7,サル,8,ネコ,9,ネズミ,10
# 安い
ハンバーガー,1,パン,2,焼きそば,3,チャーハン,4,カレー,5,パスタ,6,ピザ,7,ステーキ,8,寿司,9
...
```


## util
This folder contains some helper scripts.

### [print.rb](./util/print.rb)
A module to help to print something.

### [count.rb](./util/count.rb)
A module to help to count something.

### [parse\_yaml.sh](./util/parse_yaml.sh)
A module to load config as yaml in shell script.

### [spearman.rb](./util/spearman.rb)
A module to calculate speraman's rho (coefficient value)

### [calc\_spearman.rb](./util/calc_spearman.rb)
A script to calculate Spearman's rho between 2 files (set of ranking).

### [gather\_results\_based\_on\_cs.rb](./util/gather_results_based_on_cs.rb)
A script to gather results base on super parameter C.





## baseline
This folder keeps some scripts to do baseline evaluation.

### [sort.rb](./baseline/sort.rb)
A script to sort concepts with raw values of counter (use normalized values).

### [sort\_with\_compounded\_value.rb](./baseline/sort_with_compounded_value.rb)
A script to sort concepts with the combined comparative values [Comp (+) - Comp (-)] of counter (use normalized values).



## svm
This folder keeps some scripts to do SVM evaluation.
NOTE:
These scripts use liblinear as an implementation of ranking SVM
but you can also use svm\_rank.
To use them, look into run\_svm\_validation.sh
and you'll find there are commented out lines for svm\_rank.
This also can be applied to tune\_param.rb and specify
`../tools/rank_svm/src` as the path of ranking svm.

### [create\_svm\_validation\_data.rb](./svm/create_svm_validation_data.rb)
A script to create svm cross validation data.

### [run\_svm\_validation.sh](./svm/run_svm_validation.sh)
A script to run svm cross validation.

### [concat\_svm\_results.rb](./svm/concat_svm_results.rb)
A script to concatenate svm results and format for spearman calculation.

### [tune\_param.rb](./svm/tune_param.rb)
A script to tune C parameter of SVM.



## svr
**The following description is as same as svm.**

This folder keeps some scripts to do svr evaluation.

### [create\_svr\_validation\_data.rb](./svr/create_svr_validation_data.rb)
A script to create svr cross validation data.

### [run\_svr\_validation.sh](./svr/run_svr_validation.sh)
A script to run svr cross validation.

### [concat\_svr\_results.rb](./svr/concat_svr_results.rb)
A script to concatenate svr results and format for spearman calculation.

### [tune\_param.rb](./svr/tune_param.rb)
A script to tune C parameter of SVM.



## join
This folder has a script to join 2 domain results into a folder.

### [join\_queries.rb](./join/join_queries.rb)
A script to gather 2 queries results (objective/subjective) into a folder.

### [domain\_data\_simple\_join.rb](./join/domain_data_simple_join.rb)
A script to gather 2 domain results (feed/tweet) into a file.



## analysis
This folder is used to keep some scripts to analysis users' data.



## test
This folder is used to keep script tests.



# NOTE
Some scripts are used directly in sub folders such as `svr/tune_param.rb`. I know some of them are not good and I will fix. For the future refactoring, I leave some commands to find codes to be fixed.

```
# find .  -type f -print | xargs grep '../parse_yaml'
# find .  -type f -print | xargs grep '../config'
```
