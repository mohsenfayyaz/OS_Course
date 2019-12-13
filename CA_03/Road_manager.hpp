#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include "Monitor.hpp"

#define SEPERATOR "#"
#define DELIMITER " - "

class Road_manager{
    private:
        std::vector<Road> roads;  // all the roads we have between 2 nodes
        std::vector<Path> paths;  // pathes containing a subset of roads
        std::vector<std::string> split(std::string s, std::string delimiter);
        Monitor* my_monitor;

    public:
        Road_manager(){
            
        }
        void parse_input_file(std::string file_path);
        void print_roads();
        void print_paths();
        void start_simulation();
        static void run_car(int car_id, int path_id, Path path);
};