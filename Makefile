# https://github.com/fastflow/fastflow
FF_ROOT = /usr/local/include/fastflow

CXX = g++ -std=c++17
INCLUDES = -I $(FF_ROOT)
CXXFLAGS = -g #-DTRACE_FF #-DTRACE_TSP

LDFLAGS = -pthread
OPTFLAGS = -O3 -finline-functions -DNDEBUG -ftree-vectorize #-fopt-info-vec-missed -fopt-info-vec

TARGETS = tsp_seq tsp_par tsp_ff

.PHONY: all clean cleanall
.SUFFIXES: .cpp

%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS)

all : $(TARGETS)
clean :
	rm -f $(TARGETS)
cleanall : clean
	\rm -f *.o *~ *.csv *txt