#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <tr1/unordered_map>

class brown {
private:
  struct eq_ptr {
    bool operator () (const char * const a, const char * const b) const {
      for (size_t i = 0; a[i] == b[i]; ++i) if (! a[i]) return true;
      return false;
    }
  };
  struct hashfun_ptr { // FNV-1
    size_t operator () (const char * const a) const {
      size_t h = 0x811c9dc5;
      for (char const *p = a; *p; ++p) { h *= 0x01000193; h ^= *p; } return h;
    }
  };
  struct count_greater {
    bool operator () (std::pair <size_t, size_t> i, std::pair <size_t, size_t> j)
      const { return i.first > j.first; }
  };
  typedef double  fl_t;
  typedef std::tr1::unordered_map <char *, size_t, hashfun_ptr, eq_ptr>  sbag_t;
  typedef std::vector <std::pair <size_t, size_t> > counter_t;
  typedef std::tr1::unordered_map <size_t, size_t>  ccounter_t;
  size_t  _T;  // length of text
  size_t  _V;  // size of vocabulary
  fl_t  _PMI;
  std::vector <char *>      _str;         // id -> string
  counter_t                 _counter;
  std::vector <size_t>      _text;
  std::vector <ccounter_t>  _left;
  std::vector <ccounter_t>  _right;
  std::vector <size_t>                 _id2cid;
  std::vector <std::vector <size_t> >  _cid2id;
  std::vector <fl_t>                 _p1;
  std::vector <std::vector <fl_t> >  _p2;
  std::vector <std::vector <fl_t> >  _q2;
  std::vector <fl_t>                 _s1;
  std::vector <std::vector <fl_t> >  _l2;
  std::vector <fl_t>                 _cache;
  const size_t                       _precision;
  // internal functions
  union byte_4 {
    unsigned int u;
    float        f;
    explicit byte_4 (unsigned int u_) : u (u_) {}
    byte_4 (float f_)        : f (f_) {}
  };
  inline fl_t _icsi_log (byte_4 b)
  { return _cache[(b.u & 0x7fffff) >> (23 - _precision)] + (b.u >> 23) - 127; }
  inline fl_t _calc_pmi (fl_t p_ij, fl_t p_i, fl_t p_j)
  // { return p_ij == 0 ? 0 : p_ij * (_icsi_log (p_ij) - _icsi_log (p_i) - _icsi_log (p_j)); }
  // { return p_ij == 0 ? 0 : p_ij * (_icsi_log (p_ij) - _icsi_log (p_i * p_j)); }
  { return p_ij == 0 ? 0 : p_ij * (_icsi_log (p_ij / (p_i * p_j))); }
  // { return p_ij == 0 ? 0 : p_ij * (log2 (p_ij) - log2 (p_i * p_j)); }
  // { return p_ij == 0 ? 0 : p_ij * (log2 (p_ij) - log2 (p_i) - log2 (p_j)); }
  // { return p_ij == 0 ? 0 : p_ij * (log2 (p_ij / (p_i * p_j))); }
  inline fl_t _calc_pmi2 (fl_t p_ij, fl_t p_i, fl_t p_j)
  { return p_ij == 0 ? 0 : p_ij * log2 (p_ij / (p_i * p_j)); }
  void _project_bigram (const ccounter_t &bigram, std::vector <size_t> &ccount) {
    std::fill (ccount.begin (), ccount.end (), 0);
    for (ccounter_t::const_iterator it = bigram.begin ();
         it != bigram.end (); ++it)
      ccount[_id2cid[it->first]] += it->second;
  }
  fl_t _calc_loss (size_t i, size_t j) {
    fl_t loss = _s1[i] + _s1[j] - _q2[i][j] - _q2[j][i];
    fl_t p_u  = _p1[i] + _p1[j];
    loss -= _calc_pmi (_p2[i][i] + _p2[i][j] + _p2[j][i] + _p2[j][j], p_u, p_u);
    for (size_t h = 0; h < k; ++h) {
      if (h == i || h == j) continue;
      fl_t q_uh = _calc_pmi (_p2[i][h] + _p2[j][h], p_u, _p1[h]);
      fl_t q_hu = _calc_pmi (_p2[h][i] + _p2[h][j], _p1[h], p_u);
      loss -= q_uh + q_hu;
    }
    return loss;
  }
  fl_t _find_closest_clusters (size_t &u, size_t &e) {
    fl_t loss_min = _l2[u][e];
    for (size_t i = 0; i <= k; ++i)
      for (size_t j = i + 1; j <= k; ++j)
        if (_l2[i][j] < loss_min)
          { loss_min = _l2[i][j]; u = i; e = j; }
    return loss_min;
  }
  void _merge (size_t u, size_t e, fl_t loss_min, size_t id = 0) {
    _PMI -= loss_min;
    std::fprintf (stderr, "merge clusters %ld & %ld (loss: %f) => pmi: %f\n",
                  u, e, loss_min, _PMI);
    // swap e <=> k
    if (e != k) { // k => e
      if (id) _id2cid[id] = e;
      std::swap (_cid2id[e], _cid2id[k]);
      std::swap (_p1[e], _p1[k]);
      std::swap (_p2[e][e], _p2[k][k]); std::swap (_p2[e][k],  _p2[k][e]);
      std::swap (_q2[e][e], _q2[k][k]); std::swap (_q2[e][k],  _q2[k][e]);
      for (size_t i = 0; i < k; ++i) {
        if (i == e) continue;
        std::swap (_p2[i][e], _p2[i][k]); std::swap (_p2[e][i], _p2[k][i]);
        std::swap (_q2[i][e], _q2[i][k]); std::swap (_q2[e][i], _q2[k][i]);
        std::swap (i <  e ? _l2[i][e] : _l2[e][i], _l2[i][k]);
      }
      std::swap (_s1[e], _s1[k]);
    }
    for (size_t i = 0; i < _cid2id[k].size (); ++i) {
      const size_t id_ = _cid2id[k][i];
      _id2cid[id_] = u;
      _cid2id[u].push_back (id_);
    }
    for (size_t i = 0; i < k; ++i)
      if (i != u)
        for (size_t j = i + 1; j < k; ++j)
          if (j != u) {
            const fl_t p_t = _p1[i] + _p1[j];
            _l2[i][j] -= _q2[i][u] + _q2[u][i] + _q2[i][k] + _q2[k][i] + _q2[j][u] + _q2[u][j] + _q2[j][k] + _q2[k][j];
            const fl_t q_tu = _calc_pmi (_p2[i][u] + _p2[j][u], p_t, _p1[u]);
            const fl_t q_ut = _calc_pmi (_p2[u][i] + _p2[u][j], _p1[u], p_t);
            const fl_t q_tk = _calc_pmi (_p2[i][k] + _p2[j][k], p_t, _p1[k]);
            const fl_t q_kt = _calc_pmi (_p2[k][i] + _p2[k][j], _p1[k], p_t);
            _l2[i][j] += q_tu + q_ut + q_tk + q_kt;
          }
    // k => e
    _p1[u] += _p1[k];
    _p2[u][u] += _p2[u][k] + _p2[k][u] + _p2[k][k];
    for (size_t i = 0; i < k; ++i) {
      if (i == u) continue;
      _p2[i][u] += _p2[i][k];
      _p2[u][i] += _p2[k][i];
    }
    _q2[u][u] = _calc_pmi (_p2[u][u], _p1[u], _p1[u]);
    _s1[u] = _q2[u][u];
    for (size_t i = 0; i < k; ++i) {
      if (i == u) continue;
      _s1[i] -= _q2[u][i] + _q2[i][u] + _q2[k][i] + _q2[i][k];
      _q2[u][i] = _calc_pmi (_p2[u][i], _p1[u], _p1[i]);
      _q2[i][u] = _calc_pmi (_p2[i][u], _p1[i], _p1[u]);
      _s1[i] += _q2[u][i] + _q2[i][u];
      _s1[u] += _q2[u][i] + _q2[i][u];
    }
    for (size_t i = 0; i < k; ++i)
      for (size_t j = i + 1; j < k; ++j)
        if (i == u || j == u) {
          _l2[i][j] = _calc_loss (i, j); // most heavy
        } else { // i != u && j != u
          fl_t p_t = _p1[i] + _p1[j];
          _l2[i][j] += _q2[i][u] + _q2[u][i] + _q2[j][u] + _q2[u][j];
          fl_t q_tu = _calc_pmi (_p2[i][u] + _p2[j][u], p_t, _p1[u]);
          fl_t q_ut = _calc_pmi (_p2[u][i] + _p2[u][j], _p1[u], p_t);
          _l2[i][j] -= q_tu + q_ut;
        }
  }
public:
  size_t  k;  // # clusters
  brown (size_t k_, size_t precision) : _T (0), _V (0), _PMI (0), _str (), _counter (), _text (), _left (), _right (), _id2cid (), _cid2id (), _p1 (), _p2 (), _q2 (), _s1 (), _l2 (), _cache (), _precision (precision), k (k_) {
    _cache.resize (1 << _precision);
    byte_4 b (0x3f800000u);
    for (unsigned int i = 0; i < (1u << _precision); ++i, b.u += (1u << (23 - precision)))
      _cache[i] = log2 (b.f);
  }
  void read_text (const char * fn) {
    sbag_t sbag;
    FILE * reader = std::fopen (fn, "r");
    std::fprintf (stderr, "counting words...");
    char * line = 0;
#ifdef __APPLE__
    size_t read = 0;
    while ((line = fgetln (reader, &read)) != NULL) {
#else
    ssize_t read = 0;
    size_t  size = 0;
    while ((read = getline (&line, &size, reader)) != -1) {
#endif
      char * p (line), * p_end (line + read - 1);
      *p_end = ' ';
      do {
        char * word = p; while (*p != ' ') ++p; *p = '\0';
        if (p == word) continue;
        sbag_t::iterator it = sbag.find (word);
        if (it == sbag.end ()) {
          char * copy = new char[p - word + 1];
          std::strcpy (copy, word);
          _str.push_back (copy);
          it = sbag.insert (sbag_t::value_type (copy, _V)).first;
          _counter.push_back (counter_t::value_type (0, _V));
          _left.push_back  (ccounter_t ());
          _right.push_back (ccounter_t ());
          ++_V;
        }
        size_t id = it->second;
        _text.push_back (id);
        ++_counter[id].first;
        ++_T;
      } while (p++ != p_end);
    }
    std::fclose (reader);
    std::fprintf (stderr, "done.\n");
    std::fprintf (stderr, "\tlength of input: %ld\n", _T);
    std::fprintf (stderr, "\t# distinct words: %ld\n", _V);
  }
  void setup () {
    std::fprintf (stderr, "mapping string to id...");
    std::vector <size_t> idmap (_V);
    std::stable_sort (_counter.begin (), _counter.end (), count_greater ());
    for (size_t i = 0; i < _V; ++i)
      idmap[_counter[i].second] = i;
    for (size_t i = 1; i < _T; ++i) { // collocation with more frequent words
      if (idmap[_text[i]] > idmap[_text[i-1]])
        ++_left[_text[i]][_text[i-1]];  // i-1, i: i   > i-1
      else
        ++_right[_text[i-1]][_text[i]]; // i-1, i: i-1 >= i
    }
    std::vector <size_t> ().swap (_text);
    std::fprintf (stderr, "done.\n");
  }
  void set_initial_clusters () {
    std::fprintf (stderr, "building initial cluster...");
    _id2cid.resize (_V);
    _cid2id.resize (k + 1, std::vector <size_t> ());
    _p1.resize (k + 1);
    _p2.resize (k + 1, std::vector <fl_t> (k + 1, 0));
    _q2.resize (k + 1, std::vector <fl_t> (k + 1, 0));
    _s1.resize (k + 1, 0);
    _l2.resize (k + 1, std::vector <fl_t> (k + 1, 0));
    std::vector <size_t> ccount (k + 1, 0);
    for (size_t h = 0; h < k; ++h) {
      const size_t id = _counter[h].second;
      _id2cid[id] = h;
      _cid2id[h].push_back (id);
      _p1[h] = static_cast <fl_t> (_counter[h].first) / _T;
      _project_bigram (_left[id], ccount);
      // compute q2
      for (size_t i = 0; i < k; ++i) {
        if (ccount[i] == 0) { _p2[i][h] = _q2[i][h] = 0; continue; }
        _p2[i][h] = static_cast <fl_t> (ccount[i]) / (_T - 1);
        const fl_t pmi = _q2[i][h] = _calc_pmi (_p2[i][h], _p1[i], _p1[h]);
        _s1[i] += pmi; _s1[h] += pmi; _PMI += pmi;
      }
      _project_bigram (_right[id], ccount);
      for (size_t i = 0; i < k; ++i) {
        if (ccount[i] == 0) { _p2[h][i] = _q2[h][i] = 0; continue; }
        _p2[h][i] = static_cast <fl_t> (ccount[i]) / (_T - 1);
        const fl_t pmi = _q2[h][i] = _calc_pmi (_p2[h][i], _p1[h], _p1[i]);
        _s1[i] += pmi; _s1[h] += pmi; _PMI += pmi;
      }
      _s1[h] -= _q2[h][h];
    }
    std::fprintf (stderr, "done; pmi = %f\n", _PMI);
  }
  void process () {
    // initialize
    for (size_t i = 0; i < k; ++i)
      for (size_t j = i + 1; j < k; ++j)
        _l2[i][j] = _calc_loss (i, j);
    // perform_clustering
    std::fprintf (stderr, "perform brown clustering...\n");
    std::vector <size_t> ccount (k + 1, 0);
    for (size_t h = k; h < _V; ++h) {
      const size_t id = _counter[h].second;
      std::fprintf (stderr, "%ld (%s: %ld): ", h, _str[id], _counter[h].first);
      _id2cid[id] = k;
      _cid2id[k].clear ();
      _cid2id[k].push_back (id);
      _p1[k] = static_cast <fl_t> (_counter[h].first) / _T;
      _s1[k] = 0;
      // squeeze left / right contexts
      _project_bigram (_left[id], ccount);
      // compute q2
      for (size_t i = 0; i < k; ++i) {
        if (ccount[i] == 0) { _p2[i][k] = _q2[i][k] = 0; continue; }
        _p2[i][k] = static_cast <fl_t> (ccount[i]) / (_T - 1);
        const fl_t pmi = _q2[i][k] = _calc_pmi (_p2[i][k], _p1[i], _p1[k]);
        _s1[i] += pmi; _s1[k] += pmi; _PMI += pmi;
      }
      _project_bigram (_right[id], ccount);
      for (size_t i = 0; i <= k; ++i) {
        if (ccount[i] == 0) { _p2[k][i] = _q2[k][i] = 0; continue; }
        _p2[k][i] = static_cast <fl_t> (ccount[i]) / (_T - 1);
        const fl_t pmi = _q2[k][i] = _calc_pmi (_p2[k][i], _p1[k], _p1[i]);
        _s1[i] += pmi; _s1[k] += pmi; _PMI += pmi;
      }
      _s1[k] -= _q2[k][k];
      // compute L2 for k & i
      for (size_t i = 0; i < k;  ++i) {
        for (size_t j = i + 1; j < k; ++j) {
          const fl_t p_u = _p1[i] + _p1[j];
          const fl_t q_uk = _calc_pmi (_p2[i][k] + _p2[j][k], p_u, _p1[k]);
          const fl_t q_ku = _calc_pmi (_p2[k][i] + _p2[k][j], _p1[k], p_u);
          _l2[i][j] += _q2[i][k] + _q2[k][i] + _q2[j][k] + _q2[k][j] - q_uk - q_ku;
        }
        _l2[i][k] = _calc_loss (i, k);
      }
      // find smallest loss
      size_t u (0), e (1);
      fl_t loss_min = _find_closest_clusters (u, e);
      // merge
      _merge (u, e, loss_min, id);
    }
    std::fprintf (stderr, "done.\n");
  }
  void print_pmi () {
    fl_t pmi = 0;
    for (size_t i = 0; i < k; ++i)
      for (size_t j = 0; j < k; ++j)
        pmi += _calc_pmi2 (_p2[i][j], _p1[i], _p1[j]);
    std::fprintf (stderr, "%f %f\n", pmi, _PMI);
  }
  void complete () { // dirty as a hell
    std::fprintf (stderr, "perform bottom-up clustering...\n");
    std::vector <std::pair <size_t, std::string> > node (k); // parent; edge
    std::vector <size_t> cid2node (k);
    std::vector <size_t> id2cid (_id2cid);
    for (size_t i = 0; i < k; ++i) {
      node[i] = std::pair <size_t, std::string> (0, "");
      cid2node[i] = i;
    }
    do {
      std::fprintf (stderr, "k = %2ld: ", --k);
      size_t u (0), e (1); // e => u
      fl_t loss_min = _find_closest_clusters (u, e);
      // merge
      size_t parent = node.size ();
      node.push_back (std::pair <size_t, std::string> (0, ""));
      node[cid2node[u]].second = "0";
      node[cid2node[e]].second = "1";
      cid2node[u] = node[cid2node[u]].first = node[cid2node[e]].first = parent;
      if (e != k) cid2node[e] = cid2node[k];
      _merge (u, e, loss_min);
    } while (k > 1);
    for (int i = node.size () - 2; i >= 0; --i)
      node[i].second = node[node[i].first].second + node[i].second;
    for (size_t i = 0; i < _V; ++i) {
      size_t id = _counter[i].second;
      std::fprintf (stdout, "%s\t%s\t%ld\n",
                    node[id2cid[id]].second.c_str (), _str[id], _counter[i].first);
    }
  }
};

int main (int argc, char ** argv) {
  if (argc < 4)
    { std::fprintf (stderr, "Usage: %s fn k prec\n", argv[0]); std::exit (1); }
  const char * fn   = argv[1];
  const size_t k    = std::strtol (argv[2], NULL, 10);
  const size_t prec = std::strtol (argv[3], NULL, 10);
  brown * b = new brown (k, prec);
  b->read_text (fn);
  b->setup ();
  b->set_initial_clusters ();
  b->process ();
  b->print_pmi ();
  b->complete ();
  delete b;
}
