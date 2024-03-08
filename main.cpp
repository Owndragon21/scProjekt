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
        unsigned time;
        float intensity;
        Simulation(){
            this->time = 0;
        }
        bool run();
        void incrementTime();
        void incrementTime(unsigned);
};

bool Simulation::run(){
    if (time<=get_sim_end_time())
        return true;
    return false;
}

void Simulation::incrementTime(){
    time+=10;
}

void Simulation::incrementTime(unsigned increment){
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
        unsigned id,users,resources,power;
        bool hibernate;
        std::vector<unsigned> user_service_time; 
        std::vector<User*> users_list;
        BaseStation(){
            this->resources = num_of_resources();
            this->users = 0;
            this->power = 0;
            this->hibernate = false;    
            user_service_time.clear();
            users_list.clear();
        }   
        void addUser();
        void test();
        unsigned get_user_service_time();
};

unsigned BaseStation::get_user_service_time(){
    return 1+rand()%30;
}

void BaseStation::addUser(){
    users++;
    resources--;
    user_service_time.push_back(get_user_service_time());
}

void BaseStation::test(){
    User* usr = new User();
    users_list.push_back(usr);
}

/*void BaseStation::spawnUser(){
    std::cout<<"----------------------"<<std::endl;
    std::cout<<"Spawing user for station "<<id<<"."<<std::endl;
    users++;
    resources--;
    std::cout<<"Delay for user: "<<getUserDelay()<<"."<<std::endl;
    std::cout<<"Station "<<id<<" has "<<users<<" users and "<<resources
    <<" resources left."<<std::endl;
}*/

class Network: public Simulation{
    public:
        std::vector<BaseStation*> stations;
        Network(){
            for(unsigned i=0;i<num_of_stations();i++){
                BaseStation* station = new BaseStation(); 
                station->id = i;
                stations.push_back(station);
            }   
        }
        void config();
};

void Network::config(){ 
    std::cout<<"--- NETWORK CONFIG ---"<<std::endl;
    std::cout<<"N:"<<num_of_stations()<<std::endl;
    std::cout<<"R:"<<num_of_resources()<<std::endl;
    std::cout<<"Sim_time:"<<get_sim_end_time()<<std::endl;
    for(unsigned i=0;i<num_of_stations();i++)
        std::cout<<"STATION:"<<stations[i]->id<<
        " RESOURCES:"<<stations[i]->resources<<std::endl;
    std::cout<<"----------------------"<<std::endl;
}

void simulate(Simulation* sim, Network* net){
    net->config();
    while (sim->run()){
        std::cout<<"Sim time:"<<sim->time<<std::endl;
        for(unsigned i=0;i<net->num_of_stations();i++){
            
        }
        sim->incrementTime();
    }
}

int main(){
    srand(time(NULL)); 
    Config cfg();
    Simulation* sim = new Simulation();
    Network* net = new Network();
    net->config();
    simulate(sim,net);
    delete net,sim;
    return 0;
}