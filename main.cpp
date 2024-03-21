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
            this->sim_time = 1*60*60*1000;
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
        unsigned time,end_time;
        unsigned event_counter;
        float intensity;
        float factor;
        Simulation(){
            this->time = 0;
            this->end_time = get_sim_end_time();
            this->dbg = false;
            factor = 0.5;
            this->intensity = factor*1.0;
            this->event_counter = 0;
        }
        bool run();
        void debug();
        void changeIntensity();
        float getIntensity();
        unsigned currSimTime();
        void incrementTime();
        void incrementTime(unsigned);
};

float Simulation::getIntensity(){
    return intensity;
}

void Simulation::changeIntensity(){ 
    /* [Hours of sim] |  [Factor]   */
    /*      0 - 8     |    0.5      */
    /*      8 - 14    |    0.75     */ 
    /*      14 - 18   |     1       */
    /*      18 - 24   |    0.75     */   
    // convert to hours and if stime>=24 [h] reset to 0.
    if ( (time >= 0)&&(time <= 8)&&(factor!=0.5) ){ // 0 - 8 [h] 
        factor = 0.5;
        float old_intensity = intensity;
        intensity = factor*(1);
        std::cout << "Intensity changed from:"<<old_intensity<<" to:"<<intensity<<std::endl;
    }
    else if ((time >= 8)&&(time <= 14)&&(factor!=0.75)){ // 8 - 14 [h]
        factor = 0.75;
        float old_intensity = intensity;
        intensity = factor*(1);
        std::cout << "Intensity changed from:"<<old_intensity<<" to:"<<intensity<<std::endl;
    }
    else if ((time >= 14)&&(time <= 18)&&(factor!=1.0)){ // 14-18 [h]
        factor = 1;
        float old_intensity = intensity;
        intensity = factor*(1);
        std::cout << "Intensity changed from:"<<old_intensity<<" to:"<<intensity<<std::endl;
    }
    else if ((time >= 18)&&(time <= 24)&&(factor!=0.75)){ // 18-24 [h]
        factor = 0.75;
        float old_intensity = intensity;
        intensity = factor*(1);
        std::cout << "Intensity changed from:"<<old_intensity<<" to:"<<intensity<<std::endl;
    }
    else //Should never get here
        return;
        //abort();
}

unsigned Simulation::currSimTime(){
    return time;
}

void Simulation::debug(){
    this->dbg = true;
}

bool Simulation::run(){
    if (time<get_sim_end_time())
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

class User {
    public:
        unsigned service_start;
        unsigned service_time;
        unsigned service_end;
        User(unsigned stime){
            this->service_start = stime;
            this->service_time = 1000+rand()%30000;
            this->service_end = stime+service_time;
            //print();
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

class Event {
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
class BaseStation: public Config{
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
        bool executeEvent(Event,std::vector<Event>*,Simulation*);
        Event scheduleEvent(Simulation*);
        void getNextUserArrivalTime(Simulation*);
        void printUsersList();
        bool isFull();
        bool isEmpty();
};

void BaseStation::printUsersList(){
    for (unsigned i=0;i<users_list.size();i++){
        std::cout<<users_list[i].service_time<<" ";
    }
}

bool BaseStation::executeEvent(Event event, std::vector<Event>* event_list, Simulation* sim){   
    //std::cout<<"Event type:"<<event.type<<" time:"<<event.time<<std::endl; 
    if (event.type == 'A'){
        // Schedule user departure event if user is added
        if (!isFull()){
            User usr(sim->time);
            Event event(usr.service_end,id,'D');
            //std::cout<<"Added user to station:"<<id<<" User end time:"<<usr.service_end<<std::endl;
            users_list.push_back(usr);
            event_list->push_back(event);
            /* Now schedule next user arriving at that station */
            getNextUserArrivalTime(sim);
            Event eve(sim->time + next_user_time,id,'A');
            event_list->push_back(eve);
            sim->event_counter+=2;
            // -> schedule another arrival at station <-
            return true;
        }
        else //Handover to another station
            return false;
    }
    else if (event.type == 'D'){
        // User departs
        sort(users_list.begin(), users_list.end(), compareUsers);
        //printUsersList();
        users_list.pop_back();
        return true;
    }
    else //Unknown event type
        abort();
}

Event BaseStation::scheduleEvent(Simulation* sim){
    getNextUserArrivalTime(sim);
    //std::cout<<"STIME:"<<sim->time<<std::endl;
    Event event(next_user_time + sim->time, id, 'A');
    return event;
}

void BaseStation::getNextUserArrivalTime(Simulation* sim){
    //sim->changeIntensity(); 
    next_user_time = 1000+rand()%1000;
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

class Network: public Config{
    public:
        std::vector<BaseStation> stations;
        std::vector<Event> events_list;
        unsigned total_users;
        unsigned next_event_time;
        unsigned next_user;
        Network(){
            total_users = 0;
            stations.clear();
            events_list.clear();
            for(unsigned i=0;i<num_of_stations();i++){
                BaseStation station;
                station.id = i;
                stations.push_back(station);
            }   
        }
        void methodABC(Simulation*);
        unsigned get_id_station_least_users();
        bool all_stations_empty();
        bool all_stations_full();
        /* Utility */
        void printEventList();
        void printUsersAtStations();
        void config();
        /**********/
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

void Network::methodABC(Simulation* sim){
    /* Phase A: Event Scheduling  */
    if ( events_list.empty() ){
        for (unsigned i=0;i<num_of_stations();i++){
            // Schedule arrival event
            events_list.push_back(stations[i].scheduleEvent(sim));
            //std::cout<<"E"<<i<<": "<<events_list[i].time<<std::endl;
            sim->event_counter++;
        }
    }
    //std::cout<<"---"<<std::endl;
    // Sort event list
    sort(events_list.begin(), events_list.end(), compareEvents);
    //printEventList();
    // Pop next event from calendar 
    Event next_event = events_list.back();
    // Remove event from calendar
    events_list.pop_back();
    // Increment sim time to nearest event
    if (next_event.time > get_sim_end_time()){
        sim->time = get_sim_end_time();
        return;
    }
    sim->time = next_event.time;
    // Execute nearest event
    if(!stations[next_event.sid].executeEvent(next_event, &events_list, sim)){
        // If station is FULL, handover event to non full station
        // OR drop it.
        if ( all_stations_full() ){
            //if all stations are full, drop
            //std::cout<<"User dropped for station:"<<next_event.sid<<std::endl;
        }
        else{ //Try adding user to station with smallest amount of users
            next_event.sid = get_id_station_least_users();
            if (!stations[next_event.sid].executeEvent(next_event, &events_list, sim)){
                abort(); //This acually should never happen
            }
        }
    } 
    else
        total_users++;
    // Sort events again
    sort(events_list.begin(), events_list.end(), compareEvents);
    //printEventList();
    std::cout<<"DONE: Phase A -> SIM TIME = "<<sim->time<<" [ms]"<<std::endl;
    //std::cout<<"TOTAL USERS AT STATIONS: "<<total_users<<std::endl;
    //std::cout<<"TOTAL EVENTS: "<<sim->event_counter<<std::endl;
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

void simulate(Simulation sim, Network net, unsigned iters=1000000){
    net.config();
    unsigned i = 0;
    while ( sim.run() /* && (i<=iters) */ ){
        //std::cout<<"----------------------"<<std::endl;
        net.methodABC(&sim);
        //net.printUsersAtStations();
        //std::cout<<"SID:"<<net.get_id_station_least_users()<<std::endl;
        i++;
    }
    std::cout<<"TOTAL EVENTS:"<<sim.event_counter;
}

int main(){
    srand(time(NULL)); 

    Config cfg; //Create config for simulation
    Simulation sim; //Init simulation with given config
    sim.debug();
    Network net; //Create the network

    simulate(sim, net);
    
    return 0;
}