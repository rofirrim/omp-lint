GCCDIR = /home/roger/soft/gcc/gcc-5.2/bin

CXX = $(GCCDIR)/g++
# Flags for the C++ compiler: C++11 and all the warnings
CXXFLAGS = -std=c++11 -Wall -fno-rtti
# Workaround for an issue of -std=c++11 and the current GCC headers
CXXFLAGS += -Wno-literal-suffix

# Determine the plugin-dir and add it to the flags
PLUGINDIR=$(shell $(CXX) -print-file-name=plugin)
CXXFLAGS += -I$(PLUGINDIR)/include

LDFLAGS = -std=c++11

# top level goal: build our plugin as a shared library
all: omp-lint.so

omp-lint.so: omp-lint.o
	$(CXX) $(LDFLAGS) -shared -o $@ $<

omp-lint.o : omp-lint.cc
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

clean:
	rm -f omp-lint.o omp-lint.so

.PHONY: all clean check
