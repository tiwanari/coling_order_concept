# Kotonush: Understanding Concepts Based on Values behind Social Media
- **This is a snapshot of the back-end system for [COLING 16 System Demonstrations](http://coling2016.anlp.jp/), an international conference.**
- **This has not been refactored and will not be maintained.**
- Please see [the front-end system](https://github.com/tiwanari/coling_kotonush).

authors: Tatsuya Iwanari, Naoki Yoshinaga, Kohei Ohara

# Structure
This repository includes the files used in our demo system. The structure is like this;

```
.
├── README.md
├── dataset
├── demo
├── scripts
├── src
├── tools
├── googletest
└── xz
```

## Submodules
The following two submodules are used in our counter program.

- `googletest`
    - testing
- `xz`
    - unarchiving .xz files.

## Tools
`tools` has some programs like `svr` or `svm` as tools to order concepts.

## Dataset
`dataset` has the evaluation dataset for Coling 2016.

## Scripts
`scripts` contains some evaluation scripts and helper demo scripts for Coling 2016.

## Demo
`demo` is the folder that contains programs for accessing to our back-end system (i.e., endpoints) and making indices.

## Evidence Counter
`src` keeps the codes of the evidence counter.


### Make
Move to _src_ folder and run a command ``make``.

### Usage
In _src_ folder, type the following command
```
./main adjective antonym concepts(e.g., A,B,C) output_dir pattern_files_dir < sentences_processed_with_jdepp
```

#### output\_dir
_output\_dir_ is a string parameter to specify the output folder.

#### pattern\_files\_dir
_pattern\_files\_dir_ is a string parameter to specify the path to pattern files.

(e.g., `../dataset/ja/count_patterns/ipa`)

#### Dependency
This program has a depency on liblzma for decoding xz files and it
needs the following programs:

- Autoconf 2.64
- Automake 1.12
- gettext 0.18 (Note: autopoint depends on cvs!)
- libtool 2.2
