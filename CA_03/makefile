CXX=g++ -std=c++11 -pthread
all: main.o Monitor.o Road_manager.o
	$(CXX) main.o Monitor.o Road_manager.o

main.o: main.cpp
	$(CXX) -c main.cpp

Road_manager.o: Road_manager.cpp Road_manager.hpp
	$(CXX) -c Road_manager.cpp

Monitor.o: Monitor.cpp Monitor.hpp
	$(CXX) -c Monitor.cpp


clean:
	rm *.o