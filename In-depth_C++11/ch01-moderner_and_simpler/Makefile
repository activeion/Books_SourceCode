CXXFLAGS = -O0 -g3  -I ../.. -pthread #-g3可以让gdb展开宏, print命令
LDFLAGS = -lpthread 
BASE_SRC = 
LIB_SRC =
BINARIES = 1-01 1-02 1-03 1-04 1-05 1-06 1-07 1-08 1-09 1-10 1-13 1-16 1-17 1-19 1-20 1-21 1-22 1-23 1-24
HEADERS=$(wildcard *.h)

all: $(BINARIES)
$(BINARIES): $(HEADERS)
$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) $(LDFLAGS)

clean:
	rm -f $(BINARIES) core

1-01:1-01_auto.cc
1-02:1-02_auto.cc
1-03:1-03_decltype.cc
1-04:1-04_decltype.cc
1-05:1-05_template.cc
1-06:1-06_》.cc
1-07:1-07_init.cc
1-08:1-08_init.cc
1-09:1-09_init.cc
1-10:1-10_init_narrow.cc
1-13:1-11_1-15_for_range/Test.cc
1-16:1-16_functor.cc
1-17:1-17_std_function.cc
1-19:1-19.cc
1-20:1-20.cc
1-21:1-21.cc
1-22:1-22.cc
1-23:1-23.cc
1-24:1-24.cc
