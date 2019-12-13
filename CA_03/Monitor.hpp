#include <iostream>
#include <vector>
#include <string>

class Road{
    public:
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

    public:
        Monitor(std::vector<Road> _roads){
            roads = _roads;
        }
        void get_path_access();
        void realease_path_access();
};