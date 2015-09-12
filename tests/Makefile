GCCDIR = /home/roger/soft/gcc/gcc-5.2/bin

OMP_FLAGS=-fopenmp
PLUGIN_FLAGS=-fplugin=../src/omp-lint.so

CC = $(GCCDIR)/gcc
CFLAGS = $(OMP_FLAGS) $(PLUGIN_FLAGS)

CXX = $(GCCDIR)/g++
CXXFLAGS = $(OMP_FLAGS) $(PLUGIN_FLAGS)

END=
TESTS_C= \
	   test01.c \
	   test02.c \
	   $(END)

all: $(TESTS_C) $(TESTS_CXX)
	for t in $(TESTS_C); do \
		$(MAKE) run-test-c TEST_FILE=$$t ; \
	done
	for t in $(TESTS_CXX); do \
		$(MAKE) run-test-cxx TEST_FILE=$$t ; \
	done

run-test-c:
	$(CC) -c $(CFLAGS) $(TEST_FILE)

run-test-cxx:
	$(CXX) -c $(CFLAGS) $(TEST_FILE)

