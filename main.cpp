#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iterator>

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
    if (time!=get_sim_end_time())
        return true;
    return false;
}

void Simulation::incrementTime(){
    time+=10;
}

void Simulation::incrementTime(unsigned increment){
    //if((time+increment)<=get_sim_end_time())
    time+=increment;
    if(dbg){
        std::cout<<"Incremented sim time by "<<increment<<"."
        <<std::endl<<"Current sim time: "<<time<<"."<<std::endl;
    }
}

class User: public Simulation {
    public:
        unsigned service_time;
        unsigned service_time_end;
        int service_time_left;
        bool marked; //mark to destroy
        User(){
            this->service_time = 1+rand()%30;
            this->service_time_end = time + service_time;
            this->service_time_left = service_time;
            this->marked = false;
        }
        void recalcServiceTimeLeft();
};

void User::recalcServiceTimeLeft(){
    service_time_left = service_time_end - time;
    if (service_time_left < 0){
        std::cout<<"Something went wrong, user should already be gone."<<std::endl;
    } 
}

class BaseStation: public Simulation{
    public:
        unsigned id,resources,power;
        unsigned users_rejected;
        unsigned next_user_time;
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
        void getNextUserArrivalTime();
        void addUser();
        bool isFull();
        bool isEmpty();
};

void BaseStation::addUser(){
    User usr;
    users_list.push_back(usr);
}

void BaseStation::getNextUserArrivalTime(){
    next_user_time = 1+rand()%10;
}

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
        std::vector<unsigned> smallest_arrival_time_ids;
        bool init;
        unsigned next_event_time;
        unsigned next_user;
        Network(){
            stations.clear();
            smallest_arrival_time_ids.clear();
            init = true;
            for(unsigned i=0;i<num_of_stations();i++){
                BaseStation station;
                station.id = i;
                stations.push_back(station);
            }   
        }
        void config();
        void addUser();
        void addAllUsers();
        void getNextEvent();
        void recalcTimeLeftForAllUsers();
        void calcNextUserArrivalTimeStations();
        void findSmallestArrivalTime(std::vector<unsigned>);
        void decideCase();
        void markUsers(unsigned);
        unsigned userEndsBeforeNextUser();
        unsigned getSmallestServiceTimeUser();
        bool all_stations_empty();
        bool all_stations_full();
};

void Network::markUsers(unsigned min_service_time){
    unsigned marked = 0;
    for(int i=0;i<num_of_stations();i++){
        for(int j=0;j<stations[i].num_of_users();j++){
            if(min_service_time==stations[i].users_list[j].service_time){
                stations[i].users_list[j].marked = true;
                marked++;
            }
        }
    }
    std::cout<<"Marked "<<marked<<" users."<<std::endl;
}

unsigned Network::getSmallestServiceTimeUser(){
    std::vector<unsigned> service_times;
    for(unsigned i=0;i<num_of_stations();i++){
        for(unsigned j=0;j<stations[i].num_of_users();j++){
            service_times.push_back(stations[i].users_list[j].service_time);
        }
    }
    unsigned min = *std::min_element(service_times.begin(), service_times.end());
    return min;
}

void Network::recalcTimeLeftForAllUsers(){
    for(unsigned i=0;i<num_of_stations();i++){
        for(unsigned j=0;j<stations[i].users_list.size();j++){
            stations[i].users_list[j].recalcServiceTimeLeft();
        }
    }
}

unsigned Network::userEndsBeforeNextUser(){
    /*check if a user from any station finishes before
    next user arrives at given station.
    There are 3 possiblities:
    1: Any user doesnt end before next arrives
    2: User arrives at the same time user ends
    3: User ends before next arrives*/
    recalcTimeLeftForAllUsers();
    for(unsigned i=0;i<num_of_stations();i++){
        for(unsigned j=0;j<stations[i].users_list.size();j++){
            if(stations[i].users_list[j].service_time<next_user)
                return 3;
        }
    }
}

void Network::findSmallestArrivalTime(std::vector<unsigned> arrival_times){
    unsigned min = *std::min_element(arrival_times.begin(), arrival_times.end());
    next_user = min;
    //check if some stations have the same arrival time
    for(unsigned i=0;i<num_of_stations();i++){
        if (min == stations[i].next_user_time)
            smallest_arrival_time_ids.push_back(stations[i].id);
    }
    if(smallest_arrival_time_ids.size()>1){
        std::cout<<"There are stations with same arrival time ("<<min<<")."<<std::endl;
        std::cout<<"Those are stations:";
        for(unsigned i=0;i<smallest_arrival_time_ids.size();i++){
            std::cout<<smallest_arrival_time_ids[i]<<" ";
        }
        std::cout<<std::endl;
    } else{
        std::cout<<"Station with smallest arrival time:"<<smallest_arrival_time_ids[0];
        std::cout<<std::endl;
    }
}

void Network::calcNextUserArrivalTimeStations(){
    std::vector<unsigned> arrival_times;
    for(unsigned i=0;i<num_of_stations();i++){
        stations[i].getNextUserArrivalTime();
        arrival_times.push_back(stations[i].next_user_time);
    }
    next_user = *std::min_element(arrival_times.begin(), arrival_times.end());
    //now check if stations there are more stations with smalles arrival time for user.
    //std::vector<unsigned> smallest_arrival_time_ids;
    //check if some stations have the same arrival time
    for(unsigned i=0;i<num_of_stations();i++){
        if (next_user == stations[i].next_user_time)
            smallest_arrival_time_ids.push_back(stations[i].id);
    }
    if(smallest_arrival_time_ids.size()>1){
        std::cout<<"There are stations with same arrival time ("<<next_user<<")."<<std::endl;
        std::cout<<"Those are stations: ";
        for(unsigned i=0;i<smallest_arrival_time_ids.size();i++){
            std::cout<<smallest_arrival_time_ids[i]<<" ";
        }
        std::cout<<std::endl;
    } else{
        std::cout<<"Station with smallest arrival time ("<<next_user<<")->"<<smallest_arrival_time_ids[0];
        std::cout<<std::endl;
    }
    next_event_time = next_user;
    if(all_stations_empty()){ //all stations are empty
       addAllUsers();
    }
    else{ //check if any user/users finish before next arrives
        decideCase();
        //must check if station is full etc.
    }
}

void Network::addAllUsers(){
    for(unsigned i=0;i<smallest_arrival_time_ids.size();i++){
            stations[smallest_arrival_time_ids[i]].addUser();
            std::cout<<"Added user to station "<<smallest_arrival_time_ids[i]<<"."
            <<" Service time for user:"<<stations[smallest_arrival_time_ids[i]].users_list.back().service_time
            <<std::endl;
        }
    smallest_arrival_time_ids.clear();
}
void Network::decideCase(){
    /*There are 3 possiblities:
    1: Any user doesnt end before next arrives
    2: User arrives at the same time user ends
    3: User ends before next arrives*/
    unsigned min_service_time = getSmallestServiceTimeUser();
    if (min_service_time>next_user){
        //then next event is next user arriving
        //just add user
        next_event_time = next_user;
        addAllUsers();
        return;
    }
    else if (min_service_time==next_user){
        //next event is next user arriving and user ending
        next_event_time = next_user;
        //destroy user and add new one
        markUsers(min_service_time);
        addAllUsers();
        //get the id of users that finish and destory them
    }
    else {
        markUsers(min_service_time);
        //user ends before next arrives
    }
}

void Network::getNextEvent(){
    calcNextUserArrivalTimeStations();
    smallest_arrival_time_ids.clear();
    //now check if before next user comes, some user ends.
    //incrementTime(next_event_time); 
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
            std::cout<<"ALL STATIONS ARE EMPTY."<<std::endl;
        net.getNextEvent();
        std::cout<<"NEXT EVENT TIME:"<<net.next_event_time<<std::endl;
        sim.incrementTime(net.next_event_time);
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