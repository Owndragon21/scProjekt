#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iterator>

class Config{ /* Configuration */
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
        unsigned currSimTime();
        void incrementTime();
        void incrementTime(unsigned);
};

unsigned Simulation::currSimTime(){
    return time;
}

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
        unsigned service_start;
        unsigned service_time;
        unsigned service_end;
        User(unsigned stime){
            this->service_start = stime;
            this->service_time = 1+rand()%30;
            this->service_end = stime+service_time;
            print();
        }
        void print();
};

void User::print(){
    std::cout<<"--- USER CREATED ---"<<std::endl
    <<"service_start: "<<service_start<<std::endl
    <<"service_time: "<<service_time<<std::endl
    <<"service_end: "<<service_end<<std::endl
    <<"--------------------"<<std::endl;
}

class Event: public Simulation {
    public: /* Event calendar */
        unsigned time;
        unsigned eid,sid,uid;
        char type; /* 'A' - User arrives */
                   /* 'D' - User departs */
        Event(unsigned time, unsigned sid, char type){
            this->sid = sid;
            this->time = time;
            this->type = type;
            if (!(type == 'A' || type == 'D'))
                abort();
        } /* event_time, station id, event_type */
};

bool compareEvents(Event e1, Event e2){
    return (e1.time > e2.time);
}

bool compareUsers(User u1, User u2){
    return (u1.service_time > u2.service_time);
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
        bool executeEvent(Event,std::vector<Event>*,unsigned);
        Event scheduleEvent(unsigned);
        void getNextUserArrivalTime();
        void printUsersList();
        bool isFull();
        bool isEmpty();
};

void BaseStation::printUsersList(){
    for (unsigned i=0;i<users_list.size();i++){
        std::cout<<users_list[i].service_time<<" ";
    }
}

bool BaseStation::executeEvent(Event event, std::vector<Event>* event_list, unsigned stime){   
    std::cout<<"Event type:"<<event.type<<" time:"<<event.time<<std::endl; 
    if (event.type == 'A'){
        // Schedule user departure event if user is added
        if (!isFull()){
            User usr(stime);
            Event event(usr.service_end,id,'D');
            std::cout<<"Added user to station:"<<id<<" User end time:"<<usr.service_end<<std::endl;
            users_list.push_back(usr);
            event_list->push_back(event);
            return true;
        }
        else //Handover to another station
            return false;
    }
    else if (event.type == 'D'){
        // User departs
        sort(users_list.begin(), users_list.end(), compareUsers);
        printUsersList();
        users_list.pop_back();
        return true;
    }
    else //Unknown event type
        abort();
}

Event BaseStation::scheduleEvent(unsigned stime){
    getNextUserArrivalTime();
    std::cout<<"STIME:"<<stime<<std::endl;
    Event event(next_user_time + stime, id, 'A');
    return event;
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
        std::vector<Event> events_list;
        bool init;
        unsigned next_event_time;
        unsigned next_user;
        Network(){
            stations.clear();
            events_list.clear();
            init = true;
            for(unsigned i=0;i<num_of_stations();i++){
                BaseStation station;
                station.id = i;
                stations.push_back(station);
            }   
        }
        void methodABC(unsigned&);
        void printEventList();
        void printUsersAtStations();
        void config();
        void next_user_time_all_stations();
        unsigned get_id_station_least_users();
        bool all_stations_empty();
        bool all_stations_full();
};

void Network::printUsersAtStations(){
    std::cout<<"------------------"<<std::endl;
    for(unsigned i=0;i<num_of_stations();i++){
        std::cout<<"Station"<<i<<" has "<<stations[i].users_list.size()<<" users."<<std::endl;
    }
    std::cout<<"------------------"<<std::endl;
}

unsigned Network::get_id_station_least_users(){
    unsigned min = stations[0].users_list.size();
    unsigned sid = 0;
    for(unsigned i=0;i<num_of_stations();i++){
        if (stations[i].users_list.size()<min){
            min = stations[i].users_list.size();
            sid = i;
        }
    }
    return sid;
}

void Network::printEventList(){
    for(unsigned i=0;i<events_list.size();i++)
        std::cout<<"E:"<<i<<" time:"<<events_list[i].time
        <<" type:"<<events_list[i].type<<std::endl
        <<"----------------"<<std::endl;
}

void Network::next_user_time_all_stations(){
    for(unsigned i=0;i<num_of_stations();i++)
        stations[i].getNextUserArrivalTime();
}

void Network::methodABC(unsigned &stime){
    /* Phase A: Event Scheduling  */
    for (unsigned i=0;i<num_of_stations();i++){
        // Schedule arrival event
        events_list.push_back(stations[i].scheduleEvent(stime));
        //std::cout<<"E"<<i<<": "<<events_list[i].time<<std::endl;
    }
    std::cout<<"---"<<std::endl;
    // Sort event list
    sort(events_list.begin(), events_list.end(), compareEvents);
    printEventList();
    // Pop next event from calendar 
    Event next_event = events_list.back();
    // Remove event from calendar
    events_list.pop_back();
    // Increment sim time to nearest event
    stime=next_event.time;
    incrementTime(next_event.time);
    // Execute nearest event
    if(!stations[next_event.sid].executeEvent(next_event, &events_list, stime)){
        // If station is FULL, handover event to non full station
        // OR drop it.
        if ( all_stations_full() ){
            //if all stations are full, drop
            std::cout<<"User dropped for station:"<<next_event.sid<<std::endl;
        }
        else{ //Try adding user to station with smallest amount of users
            next_event.sid = get_id_station_least_users();
            if (!stations[next_event.sid].executeEvent(next_event, &events_list, stime)){
                abort(); //This acually should never happen
            }
        }
    }
    // Sort events again
    sort(events_list.begin(), events_list.end(), compareEvents);
    printEventList();
    std::cout<<"DONE: Phase A -> SIM TIME = "<<stime<<std::endl;
    /*****************************/
    /* Phase B */
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
    std::cout<<"SIM_END_TIME:"<<get_sim_end_time()<<std::endl;
    for(unsigned i=0;i<num_of_stations();i++)
        std::cout<<"STATION:"<<stations[i].id<<
        " RESOURCES:"<<stations[i].resources<<std::endl;
    std::cout<<"----------------------"<<std::endl;
}

void simulate(Simulation sim, Network net, unsigned iters){
    net.config();
    unsigned i = 0;
    unsigned stime = 0;
    while ( sim.run() && (i<=iters) ){
        std::cout<<"----------------------"<<std::endl;
        net.methodABC(stime);
        net.printUsersAtStations();
        std::cout<<"SID:"<<net.get_id_station_least_users()<<std::endl;
        i++;
    }
}

int main(){
    //srand(time(NULL)); 
    srand(0);
    
    Config cfg; //Create config for simulation
    Simulation sim; //Init simulation with given config
    sim.debug();
    Network net; //Create the network

    simulate(sim, net, 5);
    
    return 0;
}