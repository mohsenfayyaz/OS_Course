CXX=g++ -std=c++11
all: main.o Monitor.o
	$(CXX) main.o Monitor.o

main.o: main.cpp
	$(CXX) -c main.cpp

Monitor.o: Monitor.cpp Monitor.h
	$(CXX) -c Monitor.cpp


clean:
	rm *.o