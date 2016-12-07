#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <string>
#include <set>
#include <tr1/unordered_map>
#include "cedar.h"

static const size_t BUFFER_SIZE = 1 << 21;
static const size_t KEY_SIZE = 8;

class byte_encoder {
public:
  byte_encoder () : _len (0), _key () {}
  byte_encoder (unsigned int i) : _len (0), _key () { encode (i); }
  unsigned int encode (unsigned int i, unsigned char* const key_) const {
    unsigned int len_ = 0;
    for (key_[len_] = (i & 0x7f); i >>= 7; key_[++len_] = (i & 0x7f))
      key_[len_] |= 0x80;
    return ++len_;
  }
  void encode (const unsigned int i) { _len = encode (i, _key); }
  unsigned int decode (unsigned int& i, const unsigned char* const
                       key_) const {
    unsigned int b (0), len_ (0);
    for (i = key_[0] & 0x7f; key_[len_] & 0x80; i += (key_[len_] &
                                                      0x7fu) << b)
      ++len_, b += 7;
    return ++len_;
  }
  unsigned int        len () { return _len; }
  const char* key () { return reinterpret_cast <const char*> (&_key[0]); }
private:
  unsigned int  _len;
  unsigned char _key[KEY_SIZE];
};

int main (int argc, char** argv) {
  if (argc < 4) {
    std::fprintf (stderr, "Usage: %s noun adj dir < file\n", argv[0]);
    std::exit (1);
  }
  std::tr1::unordered_map <std::string, std::vector <std::string> > index;
  // std::vector <std::pair <std::string, size_t> > counter;

  static const size_t BUFFER_SIZE = 1 << 21;

  typedef cedar::da <int> trie_t;
  trie_t keys;

  // register k keywords into list
  char line[BUFFER_SIZE];
  size_t nnouns (0), nadjs (0);
  std::tr1::unordered_map <size_t, std::string> id2pref, id2gender;
  std::set <std::string> prefs, genders;
  {
    // reading prefs
    std::fprintf (stderr, "reading prefs..");
    {
      FILE* reader = std::fopen ("id2pref", "r");
      char line[1 << 21];
      while (fgets (&line[0], 1 << 21, reader) != NULL) {
        char*  p = &line[0];
        size_t id   = std::strtol (p, &p, 10);
        char*  pref = ++p; while (*p != '\n') ++p; *p = '\0';
        id2pref.insert (std::pair <size_t, std::string> (id, pref));
        prefs.insert (pref);
      }
      std::fclose (reader);
      std::fprintf (stderr, "done; %ld prefs\n", prefs.size ());
    }
    // reading gender
    std::fprintf (stderr, "reading gender..");
    {
      FILE* reader = std::fopen ("id2gender", "r");
      char line[1 << 21];
      while (fgets (&line[0], 1 << 21, reader) != NULL) {
        char* p = &line[0];
        size_t id     = std::strtol (p, &p, 10);
        char*  gender = ++p; while (*p != '\t') ++p; *p = '\0';
        id2gender.insert (std::pair <size_t, std::string> (id, gender));
        genders.insert (gender);
      }
      std::fclose (reader);
      std::fprintf (stderr, "done\n");
    }
    // reading concepts
    std::fprintf (stderr, "reading concepts..");
    FILE* fp = std::fopen (argv[1], "r");
    while (std::fgets (line, BUFFER_SIZE, fp) != NULL) {
      size_t len = std::strlen (line) - 1;
      keys.update (line, len, nnouns++);
      // counter.push_back (std::pair <std::string, size_t> (std::string (line, len), 0));
    }
    std::fclose (fp);
    std::fprintf (stderr, "done; #concepts = %ld\n", nnouns);
    std::fprintf (stderr, "reading attributes..");
    //
    // reading adjetives
    fp = std::fopen (argv[2], "r");
    std::tr1::unordered_map <std::string, size_t> adjs;
    while (std::fgets (line, BUFFER_SIZE, fp) != NULL) {
      size_t len = std::strlen (line) - 1;
      adjs.insert (std::pair <std::string, size_t> (std::string (line, len), nadjs++));
      // counter.push_back (std::pair <std::string, size_t> (std::string (line, len), 0));
    }
    std::fclose (fp);
    // register inflexted forms for given adjectives
#ifdef USE_JUMAN
    if (! (fp = std::fopen ("ContentWAuto.csv", "r")))
#else
    if (! (fp = std::fopen ("AdjW.csv", "r")))
#endif
      std::exit (1);
    while (std::fgets (line, BUFFER_SIZE, fp) != NULL) {
      char* p = &line[0];
      char* surf = p;
      while (*p != ',') ++p; *p = '\0';
      size_t len = p - surf;
      ++p;
      for (size_t i = 0; i < 3; ++i)
        { while (*p != ',') ++p; ++p; }
      // std::fprintf (stderr, "%s\n", p);
      if (std::strncmp (p, "形容詞,", std::strlen ("形容詞,")) == 0) {
        for (size_t i = 0; i < 3; ++i)
          { while (*p != ',') ++p; ++p; }
#ifndef USE_JUMAN
        for (size_t i = 0; i < 2; ++i)
          { while (*p != ',') ++p; ++p; }
#endif
#ifdef USE_JUMAN
        if (std::strncmp (p, "語幹,", std::strlen ("語幹,")) != 0 &&
            std::strncmp (p, "文語",  std::strlen ("文語"))  != 0)
#else
        if (std::strncmp (p, "ガル接続,", std::strlen ("ガル接続,")) != 0 &&
            std::strncmp (p, "文語",  std::strlen ("文語"))  != 0)
#endif
        {
          while (*p != ',') ++p; ++p;
          char* fin = p;
          while (*p != ',') ++p; *p = '\0';
          std::tr1::unordered_map <std::string, size_t>::iterator it = adjs.find (fin);
          if (it != adjs.end ())
            if (keys.exactMatchSearch <int> (surf, len) == -1)
              keys.update (surf, len, nnouns + it->second);
        }
      }
    }
    std::fclose (fp);
    std::fprintf (stderr, "done; #attributes = %ld\n", nadjs);
    keys.save ((std::string (argv[1]) + "_" + std::string (argv[2]) + ".trie").c_str ());
  }
  {
    std::fprintf (stderr, "indexing input..");
    std::tr1::unordered_map <std::string, FILE*> sent;
    std::tr1::unordered_map <std::string, int> offset;
    for (std::set <std::string>::iterator it = genders.begin ();
         it != genders.end (); ++it)
      for (std::set <std::string>::iterator jt = prefs.begin ();
           jt != prefs.end (); ++jt) {
        const std::string label = *it + "_" + *jt;
        FILE* writer = std::fopen ((std::string (argv[3]) + "/" + label + ".sent").c_str (), "wb");
        std::fwrite (" ", sizeof (char), 1, writer);
        sent.insert (std::pair <std::string, FILE*> (label, writer));
        index.insert (std::pair <std::string, std::vector <std::string> >
                      (label, std::vector <std::string> (nnouns + nadjs)));
        offset.insert (std::pair <std::string, int> (label, 1));
      }
    byte_encoder encoder;
    std::set <int> found;
    while (std::fgets (line, BUFFER_SIZE, stdin) != NULL) {
      found.clear ();
      char* q = &line[0];
      size_t id = std::strtol (q, &q, 10); ++q;
      if (*q != 'T') continue;
      std::tr1::unordered_map <size_t, std::string>::iterator it
        = id2gender.find (id);
      if (it == id2gender.end ()) continue;
      std::tr1::unordered_map <size_t, std::string>::iterator jt
        = id2pref.find (id);
      if (jt == id2pref.end ()) continue;
      // switch
      const std::string label = it->second + "_" + jt->second;
      FILE* sent_ = sent[label];
      std::vector <std::string>& index_ = index[label];
      int& offset_ = offset[label];
      //
      q += 2;
      char* s = q;
      bool found_noun (false), found_adj (false);
      for (size_t i = 0; q[i] != '\n'; ++i) {
        const char* p = &q[i];
        for (size_t from (0), pos (0); p[pos] != '\n'; ) {
          int n = keys.traverse (p, from, pos, pos + 1);
          if (n == trie_t::CEDAR_NO_VALUE) continue;
          if (n == trie_t::CEDAR_NO_PATH)  break;
          assert (n < nnouns + nadjs);
          found.insert (n);
          if (n < nnouns)
            found_noun = true;
          else
            found_adj  = true;
        }
      }
      if (found_noun && found_adj) {
        encoder.encode (offset_);
        for (std::set <int>::iterator kt = found.begin ();
             kt != found.end (); ++kt) {
          assert (*kt < index_.size ());
          index_[*kt] += std::string (encoder.key (), encoder.len ());
          // ++counter[*it].second;
        }
        size_t len = std::strlen (s) - 1;
        offset_ += std::fwrite (s,    sizeof (char), len, sent_);
        offset_ += std::fwrite ("\0", sizeof (char), 1,   sent_);
      }
    }
    for (std::tr1::unordered_map <std::string, FILE*>::iterator it = sent.begin ();
         it != sent.end (); ++it)
      std::fclose (it->second);
    std::fprintf (stderr, "done.\n");
  }
  {
    std::fprintf (stderr, "saving index..");
    for (std::tr1::unordered_map <std::string, std::vector <std::string> >::iterator it = index.begin (); it != index.end (); ++it) {
      FILE* writer = std::fopen ((std::string (argv[3]) + "/" + it->first + ".index").c_str (), "wb");
      for (std::vector <std::string>::iterator jt = it->second.begin ();
           jt != it->second.end (); ++jt) {
        std::fwrite (jt->c_str (), sizeof (char), jt->size (), writer);
        std::fwrite ("\0", sizeof (char), 1, writer);
      }
      std::fclose (writer);
    }
    std::fprintf (stderr, "done.\n");
  }
  // {
  //   for (std::vector <std::pair <std::string, size_t> >::iterator it = counter.begin ();
  //        it != counter.end (); ++it)
  //     if (it->second)
  //       std::fprintf (stderr, "%s %d\n", it->first.c_str (), it->second);
  // }
}
