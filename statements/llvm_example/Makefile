CC				= clang
CXX 			= clang++
LLVM_CONFIG		= llvm-config

CXXFLAGS 		= -Wall $(shell ${LLVM_CONFIG} --cxxflags)

LDFLAGS			= $(shell ${LLVM_CONFIG} --ldflags --system-libs --libs all)


all: main

main: main.o
	$(CXX) -o $@ $(LDFLAGS) main.o

.PHONY: clean

clean:
	rm main main.o
