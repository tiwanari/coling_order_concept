# About this folder
This folder contains files for our demo system.

authors: Tatsuya Iwanari, Naoki Yoshinaga, Kohei Ohara


# Files

```
.
├── cedar.h
├── index/
├── make.sh
├── mkindex.cc
├── model
├── prefix_synonym.cc
├── search.cc
└── tweet2sent.py
```

This folder originally contained the following files
but they were removed because they are out of focus of our study.
The top 3 are used to make indices and the others are used
to help inputting concepts by suggesting concepts based on the hyper/hyponymy.

```
.
├── ContentWAuto.csv -> a dictionary file to keep conjugated forms of adjectives
├── id2gender -> a map from ids to genders
├── id2pref -> a map from ids to prefs
├── entities.count -> the counts of Wikipedia entities
└── hyper_hypo_count -> the coutns of Wikipdeida hyper/hyponym entities
```


# Workflow

These files are used in the following order;

1. `mkindex.cc` makes indices with the keys files in `index` folder. The keys are something like `M` for male, `F` for female in `index/genders`.
    1. We use `twweet2sent.py` to normalize tweets for POS tagging or other analyzer.
    1. We use `id2gender` and `id2pref` to map Twitter users to their gender/prefecture respectively.
    1. We make indices for adjectives with `ContentWAuto.csv`.
    1. We use `index/nouns` as the list of concepts to be indexed.
    1. `ceder.h` is the data structure to keep our indices.
2. `prefix_synonym.cc` is used to suggest some concepts to users when they input concepts. It uses `entities.count` and `hyper_hypo_count` to search the frequent (thus, may be useful) occurred entities.
3. `search.cc` does search for the tweets contain the target keys (adjectives, concepts, genders, prefectures, periods, etc.)
