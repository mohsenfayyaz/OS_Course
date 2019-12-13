#include "Road_manager.hpp"

void t(){

}
void Road_manager::start_simulation(){
    std::vector<std::thread> threads;
    int car_id = 0;
    for(int i = 0; i < paths.size(); i++){
        int path_id = i;
        for(int j = 0; j < paths[i].init_num_of_cars; j++){
            // Constructs the new thread and runs it. Does not block execution.
            threads.push_back(std::thread(run_car, car_id, path_id, paths[i]));
            car_id++;
        }
    }

    for(int i = 0; i < threads.size(); i++){
        threads[i].join();
    }
}

void Road_manager::run_car(int car_id, int path_id, Path path){
    std::cout << car_id << std::endl;
}

void Road_manager::parse_input_file(std::string file_path){
    std::ifstream infile;
    infile.open(file_path);
    if (infile.is_open()){
        std::string in1, in2, dash;
        double in3;
        while (!infile.eof()){
            infile >> in1;
            if(in1 == SEPERATOR){
                break;
            }
            infile >> dash >> in2 >> dash >> in3;
            Road new_road = Road(in1, in2, in3); //start - end - hardness
            roads.push_back(new_road);
        }
        // -------#------
        std::getline(infile, in1, '\n');
        while (std::getline(infile, in1)){
            Path new_path = Path();
            std::vector<std::string> line = split(in1, DELIMITER);

            for(int i = 0; i < line.size()-1; i++){
                Road new_road = Road(line[i], line[i+1], -1); //start - end - hardness
                new_path.add_road(new_road);
            }

            std::getline(infile, in1);
            new_path.init_num_of_cars = std::stoi(in1);

            paths.push_back(new_path);
        }
        infile.close();
    }
    else{
        throw("Error opening file");
    }
    print_roads();
    print_paths();
}

void Road_manager::print_roads(){
    for(int i = 0; i < roads.size(); i++){
        roads[i].print();
    }
}

void Road_manager::print_paths(){
    for(int i = 0; i < paths.size(); i++){
        paths[i].print();
    }
}

std::vector<std::string> Road_manager::split(std::string s, std::string delimiter){
    std::vector<std::string> splitted;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        // std::cout << token << std::endl;
        splitted.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    splitted.push_back(s);
    return splitted;
    // std::cout << s << std::endl;
}