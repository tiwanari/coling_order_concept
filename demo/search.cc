// -*- coding: utf-8 -*-
// #define DEBUG

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <sstream>
#include <queue>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <fstream>
#include "cedar.h"
#ifdef USE_FCGI
#include <fcgi_stdio.h>
#endif

const char* JDEPP_PATH = "/path/to/jdepp/trained/with/neologd";
const char* PYTHON_PATH = "/path/to/python";
const char* TWEET_TO_SENT_SCRIPT_PATH = "./tweet2sent.py";
const char* COUNTER_PROGRAM_PATH = "/path/to/order_concept/src/main";
const char* COUNTER_PROGRAM_OUTPUT_DIR_PATH = "/path/to/tmp";
const char* COUNTER_PROGRAM_PATTTERN_DIR_PATH = "/path/to/order_concept/dataset/ja/count_patterns/ipa";
const char* COUNTER_PROGRAM_OUTPUT_PATH = "/path/to/tmp/format/output.txt";
const char* SVM_PATH = "/path/to/order_concept/tools/linear_svm/liblinear-ranksvm-2.1";
const char* RUN_SVM_SCRIPT_PATH = "/path/to/order_concept/scripts/demo/run_svm_validation.sh";
const char* SVM_MODEL_PATH = "./model";
const char* SVM_OUTPUT_PATH = "./prediction";
const char* RUBY_PATH = "/path/to/ruby";
const char* ORDER_WITH_SVM_RESULT_SCRIPT_PATH
    = "/path/to/order_concept/scripts/demo/concat_svm_results.rb";
const char* REASONS_PATH
    = "/path/to/reasons_file/out";
const char* LOG_FILE = "/path/to/log.txt";
std::string INDEX_DIR_PATH = "/path/to/index/";

static const size_t BUFFER_SIZE = 1 << 21;
static const size_t KEY_SIZE = 8;

std::queue <int> die_queue;

typedef cedar::da <int> trie_t;

char* decode_query (char* sin, size_t lin = 0) {
  // URL decode
  if (! lin) lin = std::strlen (sin);

  char* p     = &sin[0];
  char* p_end = &sin[lin];
  char* q     = &sin[0];
  for(; p != p_end; ++p, ++q) {
    if (*p == '+') { *q = ' '; continue; }
    if (*p != '%') { *q = *p;  continue; }
    char tmp = (*++p >= 'A' ? *p - 'A' + 10 : *p - '0');
    tmp *= 16;
    tmp += (*++p >= 'A' ? *p - 'A' + 10 : *p - '0');
    *q = tmp;
  }
  *q = '\0';
  return sin;
}

void split (const char* s, char delim, std::vector <std::string>& ret) {
  ret.clear ();
  char s_[BUFFER_SIZE];
  std::strcpy (s_, s);
  decode_query (s_);
  std::stringstream ss (s_);
  std::string item;
  while (std::getline (ss, item, delim))
    if (!item.empty ())
      ret.push_back (item);
}

std::string join_string(const std::vector <std::string>& items, const std::string& delim) {
  std::string joined (items[0]);
  for (size_t i = 1; i < items.size (); ++i)
    joined += delim + items[i];
  return joined;
}

void parseQS (
    char* q,
    std::vector <std::string>& periods,
    size_t& max,
    std::vector <std::string>& prefs,
    std::vector <std::string>& genders,
    std::vector <std::string>& adjs,
    std::vector <std::string>& nouns,
    int& num_of_sentences) {
  char * qstr = getenv("QUERY_STRING");
  if (! qstr) return ;
  char qs[BUFFER_SIZE];
  // parse query string
  std::strcpy (&qs[0], qstr);
  for (char* p = std::strtok (qs, "&"); p != NULL; p = std::strtok (NULL, "&")) {
    if (std::strncmp (p, "periods=", 8) == 0) {
      split (p + 8, ',', periods);
    } else if (std::strncmp (p, "max=", 4) == 0) {
      max = std::strtol (p + 4, NULL, 10);
    } else if (std::strncmp (p, "prefs=", 6) == 0) {
      split (p + 6, ',', prefs);
    } else if (std::strncmp (p, "genders=", 8) == 0) {
      split (p + 8, ',', genders);
    } else if (std::strncmp (p, "adjs=", 5) == 0) {
      split (p + 5, ',', adjs);
    } else if (std::strncmp (p, "nouns=", 6) == 0) {
      split (p + 6, ',', nouns);
    } else if (std::strncmp (p, "nsent=", 6) == 0) {
      num_of_sentences = std::strtol (p + 6, NULL, 10);
    }
  }
}

class byte_encoder {
public:
  byte_encoder () : _len (0), _key () {}
  byte_encoder (unsigned int i) : _len (0), _key () { encode (i); }
  unsigned int encode  (unsigned int i, unsigned char* const key_) const {
    unsigned int len_ = 0;
    for (key_[len_] = (i & 0x7f); i >>= 7; key_[++len_] = (i & 0x7f))
      key_[len_] |= 0x80;
    return ++len_;
  }
  void encode (const unsigned int i) { _len = encode (i, _key); }
  unsigned int decode (unsigned int& i, const unsigned char* const
                       key_) const {
    unsigned int b (0), len_ (0);
    for (i = key_[0] & 0x7f; key_[len_] & 0x80; i += (key_[len_] & 0x7fu) << b)
      ++len_, b += 7;
    return ++len_;
  }
  unsigned int        len () { return _len; }
  const char* key () { return reinterpret_cast <const char*> (&_key[0]); }
private:
  unsigned int  _len;
  unsigned char _key[KEY_SIZE];
};

int popen2 (int *fd_r, int *fd_w, const char* command, const char* argv[], bool will_die) {
    int pipe_child2parent[2];
    int pipe_parent2child[2];
    int pid;
    if (::pipe (pipe_child2parent) < 0) return -1;
    if (::pipe (pipe_parent2child) < 0) {
      ::close (pipe_child2parent[0]);
      ::close (pipe_child2parent[1]);
      return -1;
    }
    // fork
    if ((pid = fork ()) < 0) {
      ::close (pipe_child2parent[0]);
      ::close (pipe_child2parent[1]);
      ::close (pipe_parent2child[0]);
      ::close (pipe_parent2child[1]);
      return -1;
    }
    if (pid == 0) { // child process
      ::close (pipe_parent2child[1]);
      ::close (pipe_child2parent[0]);
      ::dup2 (pipe_parent2child[0], 0);
      ::dup2 (pipe_child2parent[1], 1);
      ::close (pipe_parent2child[0]);
      ::close (pipe_child2parent[1]);
      if (::execv (command, const_cast <char* const*> (argv)) < 0) {
        ::perror ("popen2");
        ::exit (1);
        ::close (pipe_parent2child[0]);
        ::close (pipe_child2parent[1]);
        return -1;
      }
    }

    if (will_die) {
        die_queue.push(pid);
    }
    ::fprintf (stderr, "%s (pid = %d) spawn.\n", command, pid);

    ::close (pipe_parent2child[0]);
    ::close (pipe_child2parent[1]);

    *fd_r = pipe_child2parent[0];
    *fd_w = pipe_parent2child[1];

    return pid;
}

void read_keys_from_a_flie (const std::string& filename, std::vector <std::string>& keys) {
  char line[BUFFER_SIZE];
  FILE* fp = ::fopen (filename.c_str (), "r");
  while (::fgets (line, BUFFER_SIZE, fp))
    keys.push_back (std::string (line, std::strlen (line) - 1));
  ::fclose (fp);
}

void join_keys (
  const std::vector <std::string>& periods,
  const std::vector <std::string>& prefs,
  const std::vector <std::string>& genders,
  std::vector <std::string>& labels) {

  for (size_t i = 0; i < periods.size (); ++i)
    for (size_t j = 0; j < genders.size (); ++j)
      for (size_t k = 0; k < prefs.size (); ++k)
        labels.push_back (periods[i] + "/" + genders[j] + "_" + prefs[k]);
}

void genearte_labels(std::vector <std::string>& labels) {
  std::vector <std::string> periods, prefs, genders;

  read_keys_from_a_flie (INDEX_DIR_PATH + "periods", periods);
  read_keys_from_a_flie (INDEX_DIR_PATH + "prefs", prefs);
  read_keys_from_a_flie (INDEX_DIR_PATH + "genders", genders);

  join_keys (periods, prefs, genders, labels);
}

int load_index (
    const std::vector <std::string>& labels,
    std::tr1::unordered_map <std::string, std::vector <unsigned char*> >& index,
    std::tr1::unordered_map <std::string, unsigned char*>& buf) {

  ::fprintf (stderr, "reading index..");
  for (size_t i = 0; i < labels.size (); ++i) {
    size_t size = 0;
    FILE* fp = ::fopen ((INDEX_DIR_PATH + labels[i] + ".index").c_str (), "rb");
    if (! fp) return -1;
    // get size
    if (::fseek (fp, 0, SEEK_END) != 0) return -1;
    size = static_cast <size_t> (::ftell (fp));
    if (::fseek (fp, 0, SEEK_SET) != 0) return -1;

    unsigned char* buf_
        = buf[labels[i]]
        = static_cast <unsigned char*>  (std::malloc (sizeof (unsigned char) * size));
    if (size != ::fread (buf_, sizeof (unsigned char), size, fp)) return -1;
    ::fclose (fp);

    index.insert (std::make_pair (labels[i], std::vector <unsigned char*> ()));
    std::vector <unsigned char*>& index_ = index[labels[i]];
    for (unsigned char* p = &buf_[0]; p != &buf_[size]; ++p) {
      index_.push_back (p);
      while (*p) ++p;
    }
  }
  ::fprintf (stderr, "done.\n");
  return 0;
}

int load_sent (
    const std::vector <std::string>& labels,
    std::tr1::unordered_map <std::string, char*>& sent) {

  ::fprintf (stderr, "reading sent..");
  for (size_t i = 0; i < labels.size (); ++i) {
    size_t size = 0;
    FILE* fp = ::fopen ((INDEX_DIR_PATH + labels[i] + ".sent").c_str (), "rb");
    if (! fp) return -1;
    // get size
    if (::fseek (fp, 0, SEEK_END) != 0) return -1;
    size = static_cast <size_t> (::ftell (fp));
    if (::fseek (fp, 0, SEEK_SET) != 0) return -1;

    sent.insert (std::make_pair (labels[i], static_cast <char*>  (std::malloc (sizeof (char) * size))));
    char* sent_ = sent[labels[i]];
    if (size != ::fread (sent_, sizeof (char), size, fp)) return -1;
    ::fclose (fp);
  }
  ::fprintf (stderr, "done.\n");
  return 0;
}

void search_lines_as_offsets (
    const trie_t& keys,
    const std::vector <std::string>& adjs,
    const std::vector <std::string>& nouns,
    const std::vector <unsigned char*>& index_,
    std::tr1::unordered_set <int>& offsets,
    std::set <int>& offsets_) {

  // TODO: remove duplicated codes
  byte_encoder encoder;

  for (size_t l = 0; l < adjs.size (); ++l) {
    const char* adj = adjs[l].c_str ();
    int n = keys.exactMatchSearch <int> (adj);
    if (n != trie_t::CEDAR_NO_VALUE && n != trie_t::CEDAR_NO_PATH) {
      int i = 0;
      // ::fprintf (stderr, "%s found.\n", adj);
      while (*(index_[n] + i)) {
        unsigned int offset = 0;
        i += encoder.decode (offset, index_[n] + i);
        offsets.insert (offset);
      }
    } else {
      // ::fprintf (stderr, "%s not found.\n", adj);
    }
  }
  for (size_t l = 0; l < nouns.size (); ++l) {
    const char* noun = nouns[l].c_str ();
    int n = keys.exactMatchSearch <int> (noun);
    if (n != trie_t::CEDAR_NO_VALUE && n != trie_t::CEDAR_NO_PATH) {
      int i = 0;
      // ::fprintf (stderr, "%s found.\n", noun);
      while (*(index_[n] + i)) {
        unsigned int offset = 0;
        i += encoder.decode (offset, index_[n] + i);
        if (offsets.find (offset) != offsets.end ())
          offsets_.insert (offset);
      }
    } else {
      // ::fprintf (stderr, "[%s] not found.\n", noun);
    }
  }
}

// run liblinear's ranking svm
void run_svm () {
  ::fprintf (stderr, "try to run svm...\n");
  // Usage: ./run_svm_validation.sh path_to_svm_src path_to_test path_to_model path_to_prediction(output)
  const char* svm_args[]
      = {RUN_SVM_SCRIPT_PATH, SVM_PATH, COUNTER_PROGRAM_OUTPUT_PATH, SVM_MODEL_PATH, SVM_OUTPUT_PATH, 0};
  int fd_r, fd_w;
  ::popen2 (&fd_r, &fd_w, RUN_SVM_SCRIPT_PATH, svm_args, true);
  ::fprintf (stderr, "run svm done.\n");
}

void order_concepts (int &fd_r, int &fd_w, int num_of_sentences) {
  ::fprintf (stderr, "try to order concept.\n");
  std::stringstream ss;
  ss << num_of_sentences;
  // Usage: ./concat_svm_results.rb path_to_test_data path_to_prediction
  const char* orderer_args[]
      = {RUBY_PATH, ORDER_WITH_SVM_RESULT_SCRIPT_PATH, COUNTER_PROGRAM_OUTPUT_PATH,
                SVM_OUTPUT_PATH, REASONS_PATH, ss.str ().c_str (), 0};
  ::popen2 (&fd_r, &fd_w, RUBY_PATH, orderer_args, true);
  ::fprintf (stderr, "order concept done.\n");
}

void wait_all() {
  while (!die_queue.empty()) {
    int pid = die_queue.front(); die_queue.pop();
    int status;

    pid_t w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
    if (w == -1) { perror("waitpid"); exit(EXIT_FAILURE); }

    if (WIFEXITED(status)) {
      ::fprintf(stderr, "pid (%d) exited, status=%d\n", pid, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      ::fprintf(stderr, "***ERROR!***: pid (%d) was killed by signal %d\n", pid, WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
      ::fprintf(stderr, "pid (%d) was stopped by signal %d\n", pid, WSTOPSIG(status));
    } else if (WIFCONTINUED(status)) {
      ::fprintf(stderr, "pid (%d) continued\n", pid);
    }
    if (!WIFEXITED(status) && !WIFSIGNALED(status)) {
      die_queue.push(pid);
    }
  }
}

int main (int argc, char** argv) {
  trie_t keys;
  if (keys.open ((INDEX_DIR_PATH + "keys.trie").c_str()) == -1)
    std::exit (1);

  std::vector <std::string> labels;
  genearte_labels (labels); // for loading every index and sent

  std::tr1::unordered_map <std::string, std::vector <unsigned char*> > index;
  std::tr1::unordered_map <std::string, unsigned char*> buf;
  std::tr1::unordered_map <std::string, char*> sent;
  if (load_index (labels, index, buf) != 0) return -1;
  if (load_sent (labels, sent) != 0) return -1;

  char q[256] = "*";
  size_t max  = 100000;
  int num_of_sentences  = 10;
  std::vector <std::string> periods, prefs, genders, adjs, nouns;

  // default values
  split ("2013/01,2013/03", ',', periods);
  split ("東京", ',', prefs);
  split ("F,M", ',', genders);
  split ("暑い,寒い", ',', adjs);
  split ("東京,シドニー", ',', nouns);

  int fd_r1 (0), fd_r2 (0), fd_r3 (0), fd_r4(0),
      fd_w1 (0), fd_w2 (0), fd_w3 (0), fd_w4(0);
  const char* jdepp_args[] = {JDEPP_PATH, 0};
  ::popen2 (&fd_r2, &fd_w2, JDEPP_PATH, jdepp_args, false); // run new jdepp

  std::ofstream log_file(LOG_FILE);
#ifdef USE_FCGI
  while (FCGI_Accept() >= 0)
#else
#endif
  {
    // ::fprintf (stdout, "Content-type: text/html; charset=UTF-8\r\n\r\n");
    ::fprintf (stdout, "Content-type: appliation/json; charset=UTF-8\r\n\r\n");
    log_file << "wrote header" << std::endl;

    // parse query
    parseQS (&q[0], periods, max, prefs, genders, adjs, nouns, num_of_sentences);
    log_file << "read query" << std::endl;

    std::string nouns_s = join_string (nouns, ","); // nouns_s = "CONCEPT1,CONCEPT2,..."

    // run new counter program
    if (adjs.size () <= 1)
      adjs.push_back ("xxx");
    if (adjs[1] == "")
      adjs[1] = "xxx";

    // run counter program
    // Usage: ./main adjective antonym concepts(e.g., A,B,C)
    //                  output_dir pattern_files_dir < sentences_processed_with_jdepp
    const char* counter_args[]
        = {COUNTER_PROGRAM_PATH, adjs[0].c_str (), adjs[1].c_str (), nouns_s.c_str (),
            COUNTER_PROGRAM_OUTPUT_DIR_PATH, COUNTER_PROGRAM_PATTTERN_DIR_PATH, 0};
    ::popen2 (&fd_r3, &fd_w3, COUNTER_PROGRAM_PATH, counter_args, true);

    std::vector <std::string> labels;
    join_keys (periods, prefs, genders, labels); // labels = {period1/gender1/prefs1, ...}

    // read tweets and convert for jdepp
    const char* tweet2sent_args[] = {PYTHON_PATH, TWEET_TO_SENT_SCRIPT_PATH, nouns_s.c_str (), 0};
    ::popen2 (&fd_r1, &fd_w1, PYTHON_PATH, tweet2sent_args, true);

    // // search for 'max' lines per label
    // search for 'max' lines per query
    int n = 0;
    for (size_t i = 0; i < labels.size (); ++i) {
      std::vector <unsigned char*>& index_ = index[labels[i]];
      std::tr1::unordered_set <int> offsets;
      std::set <int> offsets_;
      // process query
      search_lines_as_offsets (keys, adjs, nouns, index_, offsets, offsets_);

      char* sent_ = sent[labels[i]];
      for (std::set <int>::iterator it = offsets_.begin ();
           it != offsets_.end () && ++n <= max; ++it) {
        // write a sent and read a tweet
        char buf[BUFFER_SIZE];
        ::write (fd_w1, &sent_[*it], std::strlen (&sent_[*it]));
        ::write (fd_w1, "\n", 1);
#ifdef DEBUG
        log_file << "raw(" << n << "): " << &sent_[*it] << std::endl << std::flush;
#endif

        size_t read = ::read (fd_r1, buf, sizeof (char) * BUFFER_SIZE);

#ifdef DEBUG
        buf[read] = '\0';
        log_file << "tweet(" << n << "): " << buf << std::endl << std::flush;
#endif

        // input a tweet to jdepp and read result
        ::write (fd_w2, buf, read);
        // ::write (fd_w2, "\n", 1);

        // count breaks
        int num_break = 0;
        for (int k = 0; k < read; k++) if (buf[k] == '\n') num_break++;

        int checked_break = 0;
        while (checked_break < num_break) {
            read = ::read (fd_r2, buf, sizeof (char) * BUFFER_SIZE);
            buf[read] = '\0';

            // count EOS
            char *sp = buf;
            while ((sp = ::strstr(sp, "EOS\n")) != NULL) {
                checked_break++;
                sp++;
            }
#ifdef DEBUG
            log_file << "jdepp(" << n << "): " << buf << std::endl << std::flush;
#endif
            // write the jdepp result to counter
            ::write (fd_w3, &buf, read);
        }
      }
      ::fflush (stdout);
    }
    ::fprintf (stderr, "%d lines found.\n", n);

    // kill tweet2sent script
    ::write (fd_w1, "EOF\n", 4);
    ::fflush (stdout);
    ::close (fd_r1);
    ::close (fd_w1);

    ::fprintf (stderr, "count done.\n");
    ::fflush (stdout);

    // kill counter script
    ::write (fd_w3, "EOF\n", 4);
    ::close (fd_r3);
    ::close (fd_w3);
    log_file << "counted concepts" << std::endl;

    // run liblinear's ranking svm
    run_svm ();
    log_file << "ran svm" << std::endl;

    // order concepts based on svm result and output the result
    order_concepts (fd_r4, fd_w4, num_of_sentences);
    char buf[BUFFER_SIZE];
    size_t read = ::read (fd_r4, buf, sizeof (char) * BUFFER_SIZE);
    ::fwrite (buf, sizeof (char), read, stdout);
    log_file << "ordered concepts" << std::endl;

    ::fflush (stdout);
    ::close (fd_r4);
    ::close (fd_w4);

    // wait child processes
    wait_all();
  }

  // free some variables
  for (size_t i = 0; i < labels.size (); ++i) {
    std::free (buf[labels[i]]);
    std::free (sent[labels[i]]);
  }
}
