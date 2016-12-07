g++ -I. -O2 -Wunused mkindex.cc -o mkindex; g++ -I. -DUSE_FCGI -O2 -Wunused search.cc -lfcgi -o search.fcgi
