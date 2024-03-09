#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

class Config{
    private:
        unsigned N; //num of base stations
        unsigned R; //num of resources
        unsigned sim_time; //time of sim
    public:
        Config(){
            this->N = 3;
            this->R = 273;
            this->sim_time = 800;
        }
        Config(unsigned N, unsigned R, unsigned sim_time){
            this->N = N;
            this->R = R;
            this->sim_time = sim_time;
        }
        unsigned num_of_stations();
        unsigned num_of_resources();
        unsigned get_sim_end_time();
};

unsigned Config::num_of_stations(){
    return N;
}

unsigned Config::num_of_resources(){
    return R;
}

unsigned Config::get_sim_end_time(){
    return sim_time;
}

class Simulation: public Config{
    public:
        bool dbg;
        unsigned time;
        float intensity;
        Simulation(){
            this->time = 0;
            this->dbg = false;
        }
        bool run();
        void debug();
        void incrementTime();
        void incrementTime(unsigned);
};

void Simulation::debug(){
    this->dbg = true;
}

bool Simulation::run(){
    if (time<=get_sim_end_time())
        return true;
    return false;
}

void Simulation::incrementTime(){
    time+=10;
}

void Simulation::incrementTime(unsigned increment){
    if(dbg){
        std::cout<<"Incremented sim time by "<<increment<<"."
        <<std::endl<<"Current sim time: "<<time<<"."<<std::endl;
    }
    time+=increment;
}

class User {
    public:
        unsigned service_time;
        User(){
            this->service_time = 1+rand()%30;
        }
};

class BaseStation: public Simulation{
    public:
        unsigned id,resources,power;
        unsigned users_rejected;
        bool hibernate;
        std::vector<User> users_list;
        BaseStation(){
            this->resources = num_of_resources();
            this->power = 0;
            this->users_rejected = 0;
            this->hibernate = false;    
            users_list.clear();
        }   
        unsigned num_of_users();
        bool isFull();
        bool isEmpty();
};

bool BaseStation::isEmpty(){
    if(users_list.empty())
        return true;
    return false;
}

bool BaseStation::isFull(){
    if(num_of_users()==num_of_resources())
        return true;
    return false;
}

unsigned BaseStation::num_of_users(){
    return users_list.size();
}

class Network: public Simulation{
    public:
        std::vector<BaseStation> stations;
        unsigned next_event_time;
        Network(){
            for(unsigned i=0;i<num_of_stations();i++){
                BaseStation station;
                station.id = i;
                stations.push_back(station);
            }   
        }
        void config();
        void addUser();
        void getNextEvent();
        bool all_stations_empty();
        bool all_stations_full();
};

void Network::getNextEvent(){
    incrementTime(next_event_time);
}

bool Network::all_stations_full(){
    for(unsigned i=0;i<num_of_stations();i++){
        if(stations[i].isFull())
            continue;
        return false;
    }
    return true;
}

bool Network::all_stations_empty(){
    for(unsigned i=0;i<num_of_stations();i++){
        if(stations[i].isEmpty())
            continue;
        return false;
    }
    return true;
}

void Network::config(){ 
    std::cout<<"--- NETWORK CONFIG ---"<<std::endl;
    std::cout<<"N:"<<num_of_stations()<<std::endl;
    std::cout<<"R:"<<num_of_resources()<<std::endl;
    std::cout<<"Sim_time:"<<get_sim_end_time()<<std::endl;
    for(unsigned i=0;i<num_of_stations();i++)
        std::cout<<"STATION:"<<stations[i].id<<
        " RESOURCES:"<<stations[i].resources<<std::endl;
    std::cout<<"----------------------"<<std::endl;
}

void Network::addUser(){
    //Create a new user and attach it to BS
    User usr;
    if (stations.empty()){
        std::cout<<"FATAL: no stations exist";
    }
    unsigned idx = rand()%num_of_stations();
    stations[idx].users_list.push_back(usr);
    std::cout<<"Added user to station "<<idx<<"."<<std::endl;
    std::cout<<"Station "<<idx<<" has "<<stations[idx].users_list.size()<<" users."<<std::endl;
}   

void simulate(Simulation sim, Network net){
    net.config();
    while (sim.run()){
        std::cout<<"------------------"<<std::endl;
        if(net.all_stations_empty())
            std::cout<<"ALL STATIONS ARE EMPTY.";
        net.addUser(); 
        sim.incrementTime(10);
    }
}

int main(){
    srand(time(NULL)); 

    Config cfg; //Create config for simulation
    Simulation sim; //Init simulation with given config
    sim.debug();
    Network net; //Create the network

    simulate(sim,net);
    return 0;
}