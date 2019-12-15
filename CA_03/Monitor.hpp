#ifndef __MONITOR__
#define __MONITOR__

#include <iostream>
#include <vector>
#include <string>
#include <semaphore.h> 
#include <unistd.h> 
#include <math.h>
#include<tuple>
#include <chrono>

class Road{
    public:
        int id;
        std::string start, end;
        double hardness;
        
        Road(std::string _start, std::string _end, double _hardness){
            start = _start;
            end = _end;
            hardness = _hardness;
        }

        void print(){
            std::cout << start << "|" << end << "|" << hardness << std::endl;
        }
};

class Path{
    public:
        std::vector<Road> path;
        int init_num_of_cars;

        Path(){;}
        void add_road(Road r){
            path.push_back(r);
        }

        void print(){
            for(int i = 0; i < path.size(); i++){
                std::cout << path[i].start << "-" << path[i].end << " | ";
            }
            std::cout << init_num_of_cars << std::endl;
        }
};

class Monitor{
    private:
        std::vector<Road> roads;  //Each road is a waitable condition
        std::vector<sem_t> semaphores;
        double aggregate_pollution = 0;
        sem_t aggregate_pollution_mutex;
        int find_road_index(Road road);
        double calculate_pollution(double car_pollution, double hardness);

    public:
        Monitor(std::vector<Road> _roads);
        ~Monitor();

        std::tuple<double, double, uint64_t, uint64_t> drive_to_road(Road road, double car_pollution);  // Returns calculated pollution
        void realease_path_access();
};


#endif