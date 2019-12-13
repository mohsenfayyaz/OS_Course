#include <iostream>
#include "Road_manager.hpp"

#define INPUT_PATH "input.txt"

int main(){
    Road_manager my_road_manager = Road_manager();
    try{
        my_road_manager.parse_input_file(INPUT_PATH);

    }catch(std::string e){
        std::cerr << e << std::endl;
    }
}