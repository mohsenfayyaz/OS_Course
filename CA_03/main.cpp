#include <iostream>
#include "Road_manager.hpp"

#define INPUT_PATH "input.txt"

int main(){
    Road_manager my_road_manager = Road_manager();
    try{
        my_road_manager.parse_input_file(INPUT_PATH);
        my_road_manager.start_simulation();

    }catch(std::string e){
        std::cerr << e << std::endl;
    }
}

// Initialize static member of class
Monitor* Road_manager::my_monitor = NULL;