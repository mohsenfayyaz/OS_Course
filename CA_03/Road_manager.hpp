#ifndef __ROADMANAGER__
#define __ROADMANAGER__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include "Monitor.hpp"

#define SEPERATOR "#"
#define DELIMITER "-"
#define OUTPUT_DELIMITER ","

class Road_manager{
    private:
        std::vector<Road> roads;  // all the roads we have between 2 nodes
        std::vector<Path> paths;  // pathes containing a subset of roads
        std::vector<std::string> split(std::string s, std::string delimiter);
        static Monitor* my_monitor;
        static uint64_t timeSinceEpochMillisec();
        static void write_to_file(std::string s, std::string file_path);

    public:
        Road_manager(){
            
        }
        void parse_input_file(std::string file_path);
        void print_roads();
        void print_paths();
        void start_simulation();
        static void run_car(int car_id, int path_id, Path path);
};


#endif