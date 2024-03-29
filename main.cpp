#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iterator>
#include <random>
class Config{ /* Configuration */
    private:
        unsigned N; //num of base stations
        unsigned R; //num of resources
        unsigned sim_time; //time of sim
        float L;
        float H;
    public:
        Config(){
            this->N = 3;
            this->R = 273;
            this->sim_time = 24*60*60*1000;
            this->L=0.2;
            this->H=0.8;
        }
        Config(unsigned N, unsigned R, unsigned sim_time, float L, float H){
            this->N = N;
            this->R = R;
            this->sim_time = sim_time;
            this->L=0.2;
            this->H=0.8;
        }
        unsigned num_of_stations();
        unsigned num_of_resources();
        unsigned get_sim_end_time();
        float getL();
        float getH();
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

float Config::getL(){
    return L;
}

float Config::getH(){
    return H;
}
class Simulation: public Config{
    public:
        bool dbg;
        unsigned time,end_time;
        unsigned event_counter,users_counter;
        unsigned hours;
        double intensity,factor,lambda;
        float L,H;
        Simulation(){
            this->time = 0;
            this->end_time = get_sim_end_time();
            this->dbg = false;
            this->factor = 0.5;
            this->lambda = 20;
            this->intensity = factor*lambda;
            this->event_counter = 0;
            this->hours = 60*60*1000;
            this->L = getL();
            this->H = getH();   
        }
        bool run();
        void debug();
        void changeIntensity();
        double getIntensity();
};

double Simulation::getIntensity(){
    return intensity;
}

void Simulation::changeIntensity(){ 
    /* [Hours of sim] |  [Factor]   */
    /*      0 - 8     |    0.5      */
    /*      8 - 14    |    0.75     */ 
    /*      14 - 18   |     1       */
    /*      18 - 24   |    0.75     */   
    // convert to hours and if stime>=24 [h] reset to 0.
    if ( (time >= 0)&&(time <= 8*hours)&&(factor!=0.5) ){ // 0 - 8 [h] 
        factor = 0.5;
        float old_intensity = intensity;
        intensity = factor*lambda;
        std::cout << "Intensity changed from:"<<old_intensity<<" to:"<<intensity<<std::endl;
    }
    else if ((time >= 8*hours)&&(time <= 14*hours)&&(factor!=0.75)){ // 8 - 14 [h]
        factor = 0.75;
        float old_intensity = intensity;
        intensity = factor*lambda;
        std::cout << "Intensity changed from:"<<old_intensity<<" to:"<<intensity<<std::endl;
    }
    else if ((time >= 14*hours)&&(time <= 18*hours)&&(factor!=1.0)){ // 14-18 [h]
        factor = 1;
        float old_intensity = intensity;
        intensity = factor*lambda;
        std::cout << "Intensity changed from:"<<old_intensity<<" to:"<<intensity<<std::endl;
    }
    else if ((time >= 18*hours)&&(time <= 24*hours)&&(factor!=0.75)){ // 18-24 [h]
        factor = 0.75;
        float old_intensity = intensity;
        intensity = factor*lambda;
        std::cout << "Intensity changed from:"<<old_intensity<<" to:"<<intensity<<std::endl;
    }
    else 
        return;
}

void Simulation::debug(){
    this->dbg = true;
}

bool Simulation::run(){
    if (time<get_sim_end_time())
        return true;
    return false;
}

class User {
    public:
        unsigned service_start;
        unsigned service_time;
        unsigned service_end;
        User(unsigned sim_time){
            this->service_start = sim_time;
            this->service_time = 1000+rand()%30000; // 1-30[s]
            this->service_end = sim_time+service_time;
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
        char type; /* 'A' - User arrives  */
                   /* 'D' - User departs  */
                   /* 'S' - Station event */
        Event(unsigned time, unsigned sid, char type){
            this->sid = sid;
            this->time = time;
            this->type = type;
            if (!(type == 'A' || type == 'D' || type == 'S'))
                abort();
        } /* event_time, station id, event_type */
};

bool compareEvents(Event e1, Event e2){
    return (e1.time > e2.time);
}

bool compareUsers(User u1, User u2){
    return (u1.service_time > u2.service_time);
}

void insertEvent(std::vector<Event>* events_list, Event event){
    //std::cout<<"Size: "<<events_list->size()<<std::endl;
    if (events_list->size() < 2)
        abort();
    for (unsigned i=0;i<events_list->size();i++){
        if ((*events_list)[i].time<=event.time){
            events_list->insert(events_list->begin()+i,event);
            return;
        }
    }
    events_list->push_back(event);
    return;
}

class BaseStation: public Config{
    public:
        unsigned id,resources;
        unsigned users_rejected;
        unsigned next_user_time;
        double power;
        bool hibernate;
        std::vector<User> users_list;
        BaseStation(){
            this->resources = num_of_resources();
            this->power = 0.00;
            this->users_rejected = 0;
            this->hibernate = true;    
            users_list.clear();
        }   
        unsigned num_of_users();
        double currentUsage();
        double currentPower(unsigned);
        bool executeEvent(Event,std::vector<Event>*,Simulation*,bool);
        Event scheduleEvent(Simulation*);
        void getNextUserArrivalTime(Simulation*);
        void printUsersList();
        void activate();
        void deactivate();
        bool isHibernated();
        bool isFull();
        bool isEmpty();
};

void BaseStation::activate(){
    hibernate = false;
}

void BaseStation::deactivate(){
    hibernate = true;
}

bool BaseStation::isHibernated(){
    return hibernate;
}

double BaseStation::currentPower(unsigned duration){
    return static_cast<double>(duration*200);
}

double BaseStation::currentUsage(){
    return static_cast<double>(num_of_users())/resources;
}

void BaseStation::printUsersList(){
    for (unsigned i=0;i<users_list.size();i++){
        std::cout<<users_list[i].service_time<<" ";
    }
}

bool BaseStation::executeEvent(Event event, std::vector<Event>* event_list, Simulation* sim, bool handover){   
    //std::cout<<"Event type:"<<event.type<<" time:"<<event.time<<std::endl; 
    if (event.type == 'A'){
        // Schedule next user arriving at that station
        if (!handover){
            sim->users_counter++;
            getNextUserArrivalTime(sim);
            Event eve(sim->time + next_user_time,id,'A');
            insertEvent(event_list, eve);
            //event_list->push_back(eve);
        }
        // Schedule user departure event if user is added
        if (!isFull()&&!isHibernated()){
            User usr(sim->time);
            users_list.push_back(usr);
            Event event(usr.service_end,id,'D');
            insertEvent(event_list, event);
            //event_list->push_back(event);
            //std::cout<<"Added user to station:"<<id<<" User end time:"<<usr.service_end<<std::endl;
            return true;
        }
        else //Handover to another station
            return false;
    }
    else if (event.type == 'D'){
        // User departs
        //sort(users_list.begin(), users_list.end(), compareUsers);
        //printUsersList();
        users_list.pop_back();
        return true;
    }
    else if (event.type == 'S'){
        // Handle events like turning station on/off
    }
    else //Unknown event type
        abort();
}

Event BaseStation::scheduleEvent(Simulation* sim){
    getNextUserArrivalTime(sim);
    Event event(next_user_time + sim->time, id, 'A');
    return event;
}

void BaseStation::getNextUserArrivalTime(Simulation* sim){
    std::mt19937 gen(rand());
    sim->changeIntensity(); 
    std::exponential_distribution<double> exp_dist(sim->intensity);
    // Generate a random value
    next_user_time = 1000*exp_dist(gen);

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
        double total_usage;
        double total_power;
        unsigned next_event_time;
        unsigned next_user;
        Network(){
            total_usage = 0;
            total_power = 0;
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
        void calcStationsUsage(unsigned);
        void calcStationsPower(unsigned);
        double getTotalPowerUsage();
        unsigned calcTotalUsersRejected();
        /* Utility */
        void printEventList();
        void printStationsState();
        void printUsersAtStations();
        void config();
        /**********/
};

double Network::getTotalPowerUsage(){
    for (auto x : stations)
        total_power+=x.power;
    return total_power;
}

void Network::calcStationsPower(unsigned duration){
    for (unsigned i=0;i<num_of_stations();i++){
        stations[i].power += stations[i].currentPower(duration);
    }
}

unsigned Network::calcTotalUsersRejected(){
    unsigned sum = 0;
    for (auto x : stations){
        sum+=x.users_rejected;
    }
    return sum;
}

void Network::calcStationsUsage(unsigned duration){
    double usage = 0.00;
    unsigned N = num_of_resources();
    for (auto x : stations){
        usage += x.currentUsage();
    }
    total_usage+=duration*(usage/num_of_stations());
}

void Network::printUsersAtStations(){
    std::cout<<"------------------"<<std::endl;
    for(unsigned i=0;i<num_of_stations();i++){
        std::cout<<"Station"<<i<<" has "<<stations[i].users_list.size()<<" users."<<std::endl;
    }
}

void Network::printStationsState(){
    std::string state = "";
    for (auto x : stations){
        if (!x.isHibernated())
            state = "ON";
        else
            state = "OFF";
        std::cout<<"Station"<<x.id<<" is "<<state<<std::endl;
    }
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
    if (events_list.empty()){ // Initial state of network
        for (unsigned i=0;i<num_of_stations();i++){
            // Schedule first arrival events at all stations
            if (!i) //Active only 1 station
                stations[i].activate();
            stations[i].activate();
            events_list.push_back(stations[i].scheduleEvent(sim));
            //insertEvent(&events_list, stations[i].scheduleEvent(sim));
        }
        printStationsState();
        sort(events_list.begin(), events_list.end(), compareEvents);
    }
    Event next_event = events_list.back(); // Pop next event from calendar 
    sim->event_counter++;
    events_list.pop_back(); // Remove event from calendar
    // Increment sim time to nearest event
    if (next_event.time > sim->end_time){ //If next event is after sim end time
        calcStationsUsage(sim->end_time - sim->time);
        sim->time = sim->end_time;
        return;
    }
    calcStationsUsage(next_event.time - sim->time);
    calcStationsPower(next_event.time - sim->time);
    sim->time = next_event.time;
    // Execute nearest event
    if(!stations[next_event.sid].executeEvent(next_event, &events_list, sim, false)){
        // If station is FULL, handover event to non full station OR drop it.
        if (all_stations_full()){
            //if all stations are full, drop
            //std::cout<<"User dropped for station:"<<next_event.sid<<std::endl;
            stations[next_event.sid].users_rejected++;
        }
        else{ //Try adding user to station with smallest amount of users
            next_event.sid = get_id_station_least_users();
            stations[next_event.sid].activate();
            if (!stations[next_event.sid].executeEvent(next_event, &events_list, sim, true)){
                abort(); //This actually should never happen
            }
        }
    } 
    // Sort events again
    //sort(events_list.begin(), events_list.end(), compareEvents);
    //printEventList();

    //std::cout<<"DONE: Phase A -> SIM TIME = "<<sim->time<<" [ms]"<<std::endl;
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
    std::cout<<"SIM_END_TIME:"<<get_sim_end_time()<<" [ms]"<<std::endl;
    for(unsigned i=0;i<num_of_stations();i++)
        std::cout<<"STATION:"<<stations[i].id<<
        " RESOURCES:"<<stations[i].resources<<std::endl;
    std::cout<<"----------------------"<<std::endl;
}

void simulate(Simulation sim, Network net, unsigned iters=10000){
    net.config();
    unsigned i = 0;
    while ( sim.run() /*&& (i<=iters)*/ ){
        //std::cout<<"----------------------"<<std::endl;
        net.methodABC(&sim);
        //net.printUsersAtStations();
        //std::cout<<"SID:"<<net.get_id_station_least_users()<<std::endl;
        //net.printUsersAtStations();
        i++;
    }   
    net.printUsersAtStations();
    std::cout<<"TOTAL EVENTS: "<<sim.event_counter<<std::endl;
    std::cout<<"AVERAGE USAGE: "<<100*(net.total_usage/sim.get_sim_end_time())<<"%"<<std::endl;
    std::cout<<"TOTAL POWER USAGE: "<<net.getTotalPowerUsage()/sim.get_sim_end_time()<<" W"<<std::endl;
    std::cout<<"TOTAL USERS CREATED: "<<sim.users_counter<<std::endl;
    std::cout<<"USERS REJECTED: "<<100*(static_cast<double>(net.calcTotalUsersRejected())/sim.users_counter)<<"%";
}

int main(void){
    srand(time(NULL)); 
    std::clock_t c_start = std::clock();

    //freopen("simlog.txt","w",stderr);
    Config cfg; //Create config for simulation
    Simulation sim; //Init simulation with given config
    sim.debug();
    Network net; //Create the network
    simulate(sim, net);

    std::clock_t c_end = std::clock();
    long double time_elapsed_ms = 1000.0 * (c_end-c_start) / CLOCKS_PER_SEC;
    std::cout <<std::endl<< "CPU time used: " << time_elapsed_ms << " ms\n";
    return 0;
}