UNAME = ${shell uname}

h_files  := $(wildcard *.h)
h_files  += $(wildcard util/*.h)
cc_files := $(wildcard *.cc)
cc_files += $(wildcard util/*.cc)
objs = $(cc_files:.cc=.o)

opts :=
opts += -O3
opts += -std=c++11
opts += -Wall
opts += -Werror
opts += -Wno-sign-compare
opts += -Dlzma_attr_warn_unused_result=
ifeq ($(UNAME),Linux)
opts += -lpthread
endif

debug := -g

LIBRARY_DIR = ../lib/gen

XZ_DIR = ../xz
XZ_STATIC_LIBRARY = $(LIBRARY_DIR)/liblzma.a
XZ_INCLUDE_DIR = $(XZ_DIR)/src/liblzma/api
XZ_INCLUDE_SUBDIR = $(XZ_INCLUDE_DIR)/lzma
XZ_TEST_DIR = $(XZ_DIR)/tests

GTEST_DIR = ../googletest/googletest
TEST_INCLUDE_DIR = $(GTEST_DIR)/include
TEST_STATIC_LIBRARY = $(LIBRARY_DIR)/libgtest.a

test_cc_files = $(filter-out main.cc, $(cc_files))
test_cc_files += $(wildcard unittest/*.cc)
test_objs += $(test_cc_files:.cc=.o)
test_opts =-g

CXX = g++
CXXFLAGS = $(opts)
INCLUDES = -I.
INCLUDES += -I$(XZ_INCLUDE_DIR)
INCLUDES += -I$(XZ_INCLUDE_SUBDIR)
INCLUDES += -I$(TEST_INCLUDE_DIR)
LIBS = $(XZ_STATIC_LIBRARY)
TEST_LIBS = $(LIBS)
TEST_LIBS += $(TEST_STATIC_LIBRARY)

OBJDIR = ./obj
objs = $(addprefix $(OBJDIR)/, $(cc_files:.cc=.o))
test_objs = $(addprefix $(OBJDIR)/, $(test_cc_files:.cc=.o))

main : $(LIBS) $(objs) $(h_files)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(objs) $(LIBS)

$(OBJDIR)/%.o : %.cc
	@mkdir -p $(OBJDIR)/unittest
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ -c $<

$(XZ_STATIC_LIBRARY):
	cd ..; git submodule init; git submodule update
	cd $(XZ_DIR); ./autogen.sh; ./configure; make && make check
	mkdir -p $(LIBRARY_DIR)
	cp $(XZ_DIR)/src/liblzma/.libs/liblzma.a $(XZ_STATIC_LIBRARY)

.PHONY: run
run:
	./main

.PHONY: t
t: $(TEST_LIBS) $(test_objs) $(h_files)
	$(CXX) $(CXXFLAGS) $(test_opts) $(TEST_INCLUDES) -o $@ $(test_objs) $(TEST_LIBS)
	./$@

$(TEST_STATIC_LIBRARY):
	cd ..; git submodule init; git submodule update
	$(CXX) -c -I$(TEST_INCLUDE_DIR) -I$(GTEST_DIR) $(GTEST_DIR)/src/gtest-all.cc -o $(GTEST_DIR)/gtest-all.o
	mkdir -p $(LIBRARY_DIR)
	ar -rv $(TEST_STATIC_LIBRARY) $(GTEST_DIR)/gtest-all.o

debug: $(LIBS) $(objs) $(h_files)
	$(CXX) $(debug) $(CXXFLAGS) $(INCLUDES) -o $@ $(objs) $(LIBS)
	gdb ./$@

.PHONY: clean
clean :
	-rm -f ./obj/*.o
	-rm -f ./obj/unittest/*.o
	-rm -f main
	-rm -f t
	-rm -f debug
	-rm -rf unittest/out/*
	-rm -r *.dSYM
