#include "Monitor.hpp"

Monitor::Monitor(std::vector<Road> _roads){
    roads = _roads;
    sem_init(&aggregate_pollution_mutex, 0, 1);
    for(int i = 0; i < roads.size(); i++){
        sem_t mutex; 
        sem_init(&mutex, 0, 1);
        semaphores.push_back(mutex);
    }
}

Monitor::~Monitor(){
    sem_destroy(&aggregate_pollution_mutex); 
    for(int i = 0; i < semaphores.size(); i++){
        sem_destroy(&semaphores[i]); 
    }
}

std::pair<double, double> Monitor::drive_to_road(Road road, double car_pollution){
    int road_index = find_road_index(road);
    //wait 
    sem_wait(&(semaphores[road_index]));
    double pollution = calculate_pollution(car_pollution, roads[road_index].hardness);
    //signal
    sem_post(&(semaphores[road_index]));

    //wait 
    int save_aggregate_pollution;
    sem_wait(&aggregate_pollution_mutex);
    aggregate_pollution += pollution;
    save_aggregate_pollution = aggregate_pollution;
    //signal
    sem_post(&aggregate_pollution_mutex);

    return std::make_pair(pollution, save_aggregate_pollution);
}

int Monitor::find_road_index(Road road){
    for(int i = 0; i < roads.size(); i++){
        if(roads[i].start == road.start && roads[i].end == road.end){
            return i;
        }
    }
}

double Monitor::calculate_pollution(double car_pollution, double hardness){
    double pollution = 0;
    for(long int k = 0; k < 10000000; k++){
        pollution += k / (10*10*10*10*10*10*car_pollution*hardness);
    }
    return pollution;
}