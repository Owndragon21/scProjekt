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

enum Phase{ phase1 = 1, phase2 = 2, phase3 = 3, phase4 = 4, unknown = -1 };

class Simulation: public Config{
    public:
        bool dbg, disable_sleep;
        unsigned time, end_time;
        unsigned event_counter, users_counter;
        unsigned hours;
        double intensity, factor, lambda;
        float L, H;
        Phase phase;
        Simulation(double lambda = 0.00){
            time = 0; end_time = get_sim_end_time();
            dbg = false; disable_sleep = false;
            factor = 0.5; this->lambda = lambda /*5*/; intensity = factor*lambda;
            event_counter = 0; users_counter = 0;
            hours = 60*60*1000;
            L = getL(); H = getH();
            phase = Phase::phase1;
        }
        bool run();
        void debug();
        void changeIntensity();
        void cI();
};

Phase getPhase(unsigned stime, unsigned hours){
    stime = stime % (24*hours);
    if ((stime >= 0)&&(stime <= 8*hours))
        return phase1;
    if ((stime >= 8)&&(stime <= 14*hours))
        return phase2;
    if ((stime >= 14)&&(stime <= 18*hours))
        return phase3;
    if ((stime >= 18)&&(stime <= 24*hours))
        return phase4;
    abort();
    return unknown;
}

void Simulation::changeIntensity(){ 
    /********************************/
    /* [Hours of sim] |  [Factor]   */
    /*      0 - 8     |    0.5      */
    /*      8 - 14    |    0.75     */ 
    /*      14 - 18   |     1       */
    /*      18 - 24   |    0.75     */   
    /********************************/
    Phase phase = getPhase(time, hours);
    switch (phase){
        case phase1:{ // 0 - 8 [h]
            if (factor == 0.5)
                break;
            factor = 0.5;
            std::cout << "Intensity changed from:"<<intensity;
            intensity = factor*lambda;
            std::cout<<" to:"<<intensity<<std::endl;
            break;
        }
        case phase2:{ // 8 - 14 [h]
            if (factor == 0.75)
                break;
            factor = 0.75;
            std::cout << "Intensity changed from:"<<intensity;
            intensity = factor*lambda;
            std::cout<<" to:"<<intensity<<std::endl;
            break;
        }
        case phase3:{ // 14 - 18 [h]
            if (factor == 1)
                break;
            factor = 1;
            std::cout << "Intensity changed from:"<<intensity;
            intensity = factor*lambda;
            std::cout<<" to:"<<intensity<<std::endl;
            break;;
        }
        case phase4:{ // 18 - 24 [h]
            if (factor == 0.75)
                break;
            factor = 0.75;
            std::cout << "Intensity changed from:"<<intensity;
            intensity = factor*lambda;
            std::cout<<" to:"<<intensity<<std::endl;
            break;
        }
        default:
            break;
    }
    return;
}

void Simulation::cI(){
    switch(phase){
        case phase1:
            phase = Phase::phase2;
            std::cout << "Intensity changed from:"<<intensity;
            intensity = 0.75*lambda;
            std::cout<<" to:"<<intensity<<std::endl;
            break;

        case phase2:
            phase = Phase::phase3;
            std::cout << "Intensity changed from:"<<intensity;
            intensity = 1.00*lambda;
            std::cout<<" to:"<<intensity<<std::endl;
            break;
        
        case phase3:
            phase = Phase::phase4;
            std::cout << "Intensity changed from:"<<intensity;
            intensity = 0.75*lambda;
            std::cout<<" to:"<<intensity<<std::endl;
            break;
        
        case phase4:
            phase= Phase::phase1;
            std::cout << "Intensity changed from:"<<intensity;
            intensity = 0.50*lambda;
            std::cout<<" to:"<<intensity<<std::endl;
            break;
        
        default:
            abort();
    }
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
            this->service_time = 1000+rand()%29001; // 1-30[s]
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

enum class eventType{
    Arrival,
    Departure,
    ActivateStation,
    HibernateStation,
    Intensity
};

class Event {
    public: /* Event calendar */
        unsigned time;
        unsigned sid;
        eventType type;
        Event(unsigned time, unsigned sid, eventType type){
            this->sid = sid;
            this->time = time;
            this->type = type;
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
        unsigned id, resources;
        unsigned users_rejected;
        unsigned next_user_time;
        double power;
        bool hibernate, scheduled_state_change, hit_20_before;
        bool overloaded, triggered;
        std::vector<User> users_list;
        BaseStation(){
            this->resources = num_of_resources();
            this->power = 0.00;
            this->users_rejected = 0;
            this->hibernate = true;
            this->scheduled_state_change = false;
            this->hit_20_before = false;    
            this->overloaded = false;
            this->triggered = false;
            users_list.clear();
        }   
        unsigned num_of_users();
        double currentUsage();
        double currentPower(unsigned);
        bool executeEvent(Event, std::vector<Event>*, Simulation*, bool);
        Event scheduleEvent(Simulation*);
        void getNextUserArrivalTime(Simulation*);
        void printUsersList();
        void activate();
        void deactivate();
        bool isHibernated();
        bool isFull();
        bool isEmpty();
        bool isHeavyLoaded();

};

bool BaseStation::isHeavyLoaded(){
    return (currentUsage() > getH() ? true : false);
}

double BaseStation::currentPower(unsigned duration){
    return static_cast<double>(duration*(isHibernated() ? 1 : 200));
}

void BaseStation::activate(){
    hibernate = false;
}

void BaseStation::deactivate(){
    hibernate = true;
}

bool BaseStation::isHibernated(){
    return hibernate;
}

double BaseStation::currentUsage(){
    return static_cast<double>(num_of_users())/resources;
}

void BaseStation::printUsersList(){
    for (unsigned i=0;i<users_list.size();i++){
        std::cout<<users_list[i].service_time<<" ";
    }
}

bool BaseStation::executeEvent(Event event, std::vector<Event>* event_list, Simulation* sim, bool redirect){
    //std::cout<<"Event type:"<<event.type<<" time:"<<event.time<<std::endl; 
    switch (event.type){
        case eventType::Arrival: 
            if (!redirect){ // Schedule next user arriving at that station
                sim->users_counter++;
                getNextUserArrivalTime(sim);
                Event eve(sim->time + next_user_time, id, eventType::Arrival);
                insertEvent(event_list, eve);
            }
            if (!isFull() && !isHibernated() && !scheduled_state_change){
                User usr(sim->time);
                users_list.push_back(usr);
                Event event(usr.service_end, id, eventType::Departure); // Schedule user departure event if user is added
                insertEvent(event_list, event);
                //std::cout<<"Added user to station:"<<id<<" User end time:"<<usr.service_end<<std::endl;
                break;
            }
            else
                return false; // Handover to another station

        case eventType::Departure: // User departs
            users_list.pop_back();
            break;
        /*
        case eventType::ActivateStation: // Activate hiberanted station event
            activate();
            scheduled_state_change = false;
            hit_20_before = false;
            break;  */
        /*
        case eventType::HibernateStation: // Hibernate active station event
            deactivate();
            scheduled_state_change = false;
            break;  */ 
        case eventType::Intensity:
            sim->cI();
            break;

        default: //Unknown event type
            abort();
    }
    return true; // Event executed
}


Event BaseStation::scheduleEvent(Simulation* sim){
    getNextUserArrivalTime(sim);
    Event event(next_user_time + sim->time, id, eventType::Arrival);
    return event;
}

void BaseStation::getNextUserArrivalTime(Simulation* sim){
    std::mt19937 gen(rand());
    //sim->changeIntensity(); 
    std::exponential_distribution<double> exp_dist(sim->intensity);
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
        double total_usage, total_power;
        unsigned next_event_time, next_user;
        int triggered_station_id;
        Network(){
            total_usage = 0.00; total_power = 0.00;
            triggered_station_id = -1;
            stations.clear(); events_list.clear();
            for(unsigned i=0;i<num_of_stations();i++){
                BaseStation station;
                station.id = i;
                stations.push_back(station);
            }
        }
        /* MAIN FUNCTIONS */
        void methodABC(Simulation*);

        void changeStationStatesIfNeeded(Simulation*);
        void scheduleIntensityChange(Simulation*);
        void stationManagement(Simulation*);
        void executeStationEvent(Event, Simulation*);
        unsigned get_id_station_least_users();
        unsigned get_id_station_not_hibernated();
        unsigned get_number_of_heavy_loaded_stations();

        bool all_stations_empty();
        bool all_stations_full();
        bool allStationsActive();
        bool isOnlyActiveStation(unsigned);
        bool isStationEvent(Event);
        bool neighbourStationHeavyLoaded(unsigned);
        bool anyStationToHibernate();
        /*****************/

        /* Statistics */
        void calcStationsUsage(unsigned);
        void calcStationsPower(unsigned,bool);
        double getTotalPowerUsage();
        unsigned calcTotalUsersRejected();
        /**************/
        
        /* Utility */
        void printEventList();
        void printStateOfAllStations();
        void printUsersAtStations();
        void printUsageOfAllStations();
        void config();
        /**********/
};

void Network::executeStationEvent(Event event, Simulation* sim){
    if (event.type == eventType::ActivateStation && triggered_station_id == -1)
        abort();
    if (event.type == eventType::ActivateStation){
        stations[event.sid].activate();
        unsigned num_of_users_to_transition = (stations[triggered_station_id].num_of_users())/2;
        unsigned counter = 0;
        for (auto &e : events_list){
            if(counter == num_of_users_to_transition)
                break;
            if(e.sid == triggered_station_id && e.type == eventType::Departure){
                e.sid = event.sid;
                stations[event.sid].users_list.push_back(stations[triggered_station_id].users_list.back());
                stations[triggered_station_id].users_list.pop_back();
                counter++;
            }
        }
        //stations[event.sid].activate();
        stations[event.sid].scheduled_state_change = false;
        stations[triggered_station_id].triggered = false;
        printUsersAtStations();
        printStateOfAllStations();
    }
    else if (event.type == eventType::HibernateStation){
        printUsersAtStations();
        std::vector<unsigned> active_stations_ids = {};
        for (auto s : stations){
            if((!s.isHibernated()) && (s.id != event.sid) /*DO NOT CONSIDER STATION ITSELF*/){
                active_stations_ids.push_back(s.id);
            }
        }
        unsigned num_of_active_stations = active_stations_ids.size();
        if (num_of_active_stations == 0){
            abort();
        }
        unsigned users_to_transfer = stations[event.sid].num_of_users();
        unsigned division = users_to_transfer/num_of_active_stations;
        std::vector<unsigned> users_per_station = {};
        if (num_of_active_stations > 1){
            std::cout<<"There is more than 1 station active ("<<num_of_active_stations<<")"<<std::endl;
            std::cout<<"Trying to split users evenly..."<<std::endl;
            if (users_to_transfer%num_of_active_stations){
                std::cout<<"Warn: The number of users to transfer("<<
                users_to_transfer<<") is not divisible by num of active stations."<<std::endl;
            }
            unsigned loc_counter = 0;
            unsigned last_station_id = active_stations_ids.back();
            for (auto idx : active_stations_ids){
                if (idx == last_station_id){ //Transfer all remaining users to last station
                    users_per_station.push_back(users_to_transfer-loc_counter);
                    break;
                }
                users_per_station.push_back(division);
                loc_counter += division;
            }
            if (users_per_station.size() != active_stations_ids.size()){
                abort();
            }
            std::cout<<"Evaluated divison..."<<std::endl;
            for (unsigned i=0;i<active_stations_ids.size();i++){
                std::cout<<"Station"<<active_stations_ids[i]<<" -> "<<users_per_station[i]<<std::endl;
            }
        } 
        else{
            users_per_station.push_back(division);
        }
        if (users_per_station.empty()){
            abort();
        }
        std::cout<<"division:"<<division<<std::endl;
        std::cout<<"users to transfer:"<<users_to_transfer<<std::endl;
        std::cout<<"station/stations ID to transfer:";
        for (auto id : active_stations_ids){
            std::cout<<id<<" ";
        }
        std::cout<<std::endl;
        if (users_per_station.size() != active_stations_ids.size()){
            abort();
        }
        unsigned users_transfered = 0;
        for (unsigned i=0;i<active_stations_ids.size();i++){
            if (users_transfered == users_to_transfer){break;}
            unsigned station_transfers = 0;
            for (auto &e : events_list){
                if (station_transfers == users_per_station[i]){break;}
                if ((e.sid == event.sid) &&(e.type == eventType::Departure)){
                    // Transfer departure event of that user to active station
                    e.sid = active_stations_ids[i];
                    stations[active_stations_ids[i]].users_list.push_back(stations[event.sid].users_list.back());
                    stations[event.sid].users_list.pop_back();
                    users_transfered++;
                    station_transfers++;
                }
            }
        }
       
        if (!stations[event.sid].users_list.empty()){
            std::cout<<users_transfered;
            std::cout<<stations[event.sid].users_list.size();
            abort(); //Users list of station to hibernate must be empty
        }
        
        printUsersAtStations();
        stations[event.sid].deactivate();
        stations[event.sid].scheduled_state_change = false;
        //stations[event.sid].triggered = false;
        printStateOfAllStations();
    }
}

bool Network::anyStationToHibernate(){
    for (auto s : stations){
        if(s.currentUsage()<getL())
            return true;
    }
    return false;
}

bool Network::allStationsActive(){
    for (auto s : stations){
        if (!s.isHibernated())
            continue;
        return false;
    }
    return true;
}

void Network::stationManagement(Simulation* sim){
    /* Case 1: Wake up hibernated station if needed */
    bool wake_up_station = false;
    unsigned hib_station_id = -1;
    unsigned overload_station_id = -1;
    if(!allStationsActive()){
        unsigned loaded_stations = get_number_of_heavy_loaded_stations();
        if (loaded_stations != 0 && loaded_stations != num_of_stations()){
            for (auto &s : stations){
                if (s.isHeavyLoaded() && !s.triggered){
                    overload_station_id = s.id;
                    for (auto i : stations){
                        if (i.isHibernated() && !i.scheduled_state_change /*AND not scheduled to activate*/){
                            hib_station_id = i.id;
                        }
                    }
                    if (hib_station_id != -1){
                        s.triggered = true;
                        wake_up_station = true;
                    }
                }
                if(wake_up_station)
                    break;
            }
        }
    }
    if (wake_up_station){ //Schedule an event to wake up this station
        std::cout<<"Overloaded station ID:"<<overload_station_id
        <<" USAGE:"<<100*stations[overload_station_id].currentUsage()<<"% ["<<stations[overload_station_id].num_of_users()<<"]"<<std::endl;
        std::cout<<"Hibernated station to activate ID:"<<hib_station_id<<std::endl;
        stations[hib_station_id].scheduled_state_change = true;
        triggered_station_id = overload_station_id;
        Event event(sim->time+50, hib_station_id, eventType::ActivateStation); // Turning station ON/OFF takes 50 [ms]
        std::cout<<"Scheduling event to activate station"<<hib_station_id<<"."<<std::endl;
        std::cout<<"SIM TIME:"<<sim->time<<std::endl;
        insertEvent(&events_list, event);
    }
    /****************************************/
    /* Case 2: Hibernated station if needed */
    bool hibernate_station = false;
    unsigned station_to_hib_id = -1;
    if (!anyStationToHibernate())
        return;
    for (auto s : stations){
        if(!s.isHibernated() && !isOnlyActiveStation(s.id)){
            if(s.currentUsage()<getL() && !s.scheduled_state_change){
                hibernate_station = true;
                station_to_hib_id = s.id;
            }
        }
    }
    if (hibernate_station){
        // Active station should be hibernated (schedule event)
        stations[station_to_hib_id].scheduled_state_change = true;
        Event event(sim->time+50, stations[station_to_hib_id].id, eventType::HibernateStation); // Turning station ON/OFF takes 50 [ms]
        std::cout<<"Scheduling event to deactivate station"<<stations[station_to_hib_id].id
        <<". USAGE: "<<100*stations[station_to_hib_id].currentUsage()<<" ["<<stations[station_to_hib_id].num_of_users()<<"]"<<std::endl;
        std::cout<<"SIM TIME:"<<sim->time<<std::endl;
        insertEvent(&events_list, event);
    }
    /****************************************/
}

unsigned Network::get_number_of_heavy_loaded_stations(){
    unsigned res = 0;
    for (auto s : stations){
        if (s.currentUsage() > getH())
            res++;
    }
    return res;
}

void Network::scheduleIntensityChange(Simulation* sim){
    unsigned day = 1 + (sim->time)/(24*60*60*1000);
    const unsigned h[4] = {8,14,18,24};
    for (unsigned i=1;i<=day;i++){
        for (unsigned j=0;j<=3;j++){
            Event event(day*h[j]*60*60*1000, 0, eventType::Intensity);
            insertEvent(&events_list, event);
        }
    }
}

bool Network::isStationEvent(Event event){
    return (event.type == eventType::ActivateStation || event.type == eventType::HibernateStation);
}

double Network::getTotalPowerUsage(){
    for (auto x : stations){
        total_power+=x.power;
    }
    return total_power;
}

void Network::calcStationsPower(unsigned duration, bool transition = false){
    for (unsigned i=0;i<num_of_stations();i++){
        if (transition)
            stations[i].power += static_cast<double>(duration*1000);
        else
            stations[i].power += stations[i].currentPower(duration);
    }
}

void Network::printUsageOfAllStations(){
    std::cout<<"Usage of all stations:"<<std::endl;
    for (auto s : stations)
        std::cout<<"Station"<<s.id<<": "<<100*s.currentUsage()<<"%"<<std::endl;
    std::cout<<"----------------------"<<std::endl;
}

void Network::printStateOfAllStations(){
    std::string state = "";
    for (auto x : stations){
        if (x.isHibernated())
            state = "OFF";
        else
            state = "ON";
        std::cout<<"Station"<<x.id<<" is "<<state<<std::endl;
    }
}

bool Network::isOnlyActiveStation(unsigned sid){
    for (auto station : stations){
        if (station.id == sid)
            continue; // Do not consider station itself
        if (!station.isHibernated() && !station.scheduled_state_change)
            return false;
    }
    return true;
}

unsigned Network::get_id_station_not_hibernated(){
   bool at_least_one_station_active = false;
    for (auto station : stations){
        if (!station.isHibernated()){
            at_least_one_station_active = true;
            break;
        }
    }
    if (!at_least_one_station_active){
        //printStateOfAllStations();
        abort();
    }
    unsigned sid = 0; 
    double minUsage = 1.00;
    for (auto station : stations){
        if (!(station.isHibernated()) && (station.currentUsage() < minUsage) && !station.scheduled_state_change){
            sid = station.id;
            minUsage = station.currentUsage();
        }
    }
    return sid;
} 

bool Network::neighbourStationHeavyLoaded(unsigned sid){
    for (auto station : stations){
        if (station.id == sid)
            continue; // Do not consider station itself
        if (station.currentUsage() > getH())
            return true;
    }
    return false;
}

void Network::changeStationStatesIfNeeded(Simulation* sim){
    for (unsigned i=0;i<num_of_stations();i++){
        if (!stations[i].isHibernated() && !stations[i].hit_20_before)
            stations[i].hit_20_before = stations[i].currentUsage() > getL() ? true : false;
        if ((stations[i].isHibernated()) && (neighbourStationHeavyLoaded(stations[i].id)) && (!stations[i].scheduled_state_change)){
            // Hibernated station should be activated (schedule event)
            stations[i].scheduled_state_change = true;
            Event event(sim->time+50, stations[i].id, eventType::ActivateStation); // Turning station ON/OFF takes 50 [ms]
            std::cout<<"Scheduling event to activate station"<<stations[i].id<<"."<<std::endl;
            std::cout<<"SIM TIME:"<<sim->time<<std::endl;
            insertEvent(&events_list, event);
        }
        else if (!(stations[i].isHibernated()) && (stations[i].currentUsage() < getL()) && !(isOnlyActiveStation(stations[i].id)) && (stations[i].hit_20_before) && (!stations[i].scheduled_state_change)){ // Never deactivate only active station
            // Active station should be hibernated (schedule event)
            stations[i].scheduled_state_change = true;
            Event event(sim->time+50, stations[i].id, eventType::HibernateStation); // Turning station ON/OFF takes 50 [ms]
            std::cout<<"Scheduling event to deactivate station"<<stations[i].id<<". USAGE: "<<100*stations[i].currentUsage()<<std::endl;
            std::cout<<"SIM TIME:"<<sim->time<<std::endl;
            insertEvent(&events_list, event);
        }
        else
            continue; // Do not change state of that station
        
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
        <<" type:"<<int(events_list[i].type)<<std::endl
        <<"----------------"<<std::endl;
}

void Network::methodABC(Simulation* sim){
    /* Phase A: Event Scheduling  */
    if (events_list.empty()){ // Initial state of network
        for (unsigned i=0;i<num_of_stations();i++){
            if (sim->disable_sleep) stations[i].activate();
            if (!i) stations[i].activate(); //Activate only one station at first
            events_list.push_back(stations[i].scheduleEvent(sim)); // Schedule first arrival events at all stations
        }
        scheduleIntensityChange(sim);
        sort(events_list.begin(), events_list.end(), compareEvents);
    }

    Event next_event = events_list.back(); // Pop next event from calendar 
    events_list.pop_back(); // Remove event from calendar
    sim->event_counter++;

    if (next_event.time > sim->end_time){ //If next event is after sim end time
        calcStationsUsage(sim->end_time - sim->time);
        calcStationsPower(next_event.time - sim->time, isStationEvent(next_event));
        sim->time = sim->end_time;
        return;
    }
    calcStationsUsage(next_event.time - sim->time);
    calcStationsPower(next_event.time - sim->time, isStationEvent(next_event));

    sim->time = next_event.time; // Increment sim time to nearest event

    bool dropped = false;

    if (isStationEvent(next_event)){
        executeStationEvent(next_event,sim);    
        printUsageOfAllStations();
    }
    else{
        bool redirect = false;
        if(!stations[next_event.sid].executeEvent(next_event, &events_list, sim, redirect)){ // Execute nearest event
            if (all_stations_full()){ // If station is FULL, handover event to non full station OR drop it.
                //std::cout<<"User dropped for station:"<<next_event.sid<<std::endl;
                //stations[next_event.sid].users_rejected++;
                dropped = true;
            }
            else{ //Try adding user to station with smallest amount of users
                //next_event.sid = get_id_station_least_users();
                redirect = true;
                next_event.sid = get_id_station_not_hibernated();
                if (!stations[next_event.sid].executeEvent(next_event, &events_list, sim, redirect)){
                    std::cout<<"x";
                    abort(); //This actually should never happen
                }
            }
        }
    }
    //std::cout<<"DONE: Phase A -> SIM TIME = "<<sim->time<<" [ms]"<<std::endl;
    /*****************************/

    /* Phase B: Conditional Events */
    //if (!sim->disable_sleep) changeStationStatesIfNeeded(sim);
    if (!sim->disable_sleep) stationManagement(sim);
    if (dropped) stations[next_event.sid].users_rejected++;
    //sim->changeIntensity();
    /*******************************/
    
    //printUsageOfAllStations();
    //double u = 0.0;
    //for (auto x : stations) u += x.currentUsage();
    //std::cerr<<"sim time:"<<sim->time<<" usage:"<<100*(u/num_of_stations())<<"%"<<std::endl;

    //printEventList();
    //std::cout<<"DONE: Phase A -> SIM TIME = "<<sim->time<<" [ms]"<<std::endl;
    //std::cout<<"TOTAL USERS AT STATIONS: "<<total_users<<std::endl;
    //std::cout<<"TOTAL EVENTS: "<<sim->event_counter<<std::endl;
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

void simulate(Simulation* sim, Network* net, unsigned iters=10000){
    net->config();
    //sim.disable_sleep = true;
    unsigned i = 0;
    while ( sim->run() /*&& (i<=iters)*/ ){
        //std::cout<<"----------------------"<<std::endl;
        net->methodABC(sim);
        //net.printUsersAtStations();
        //i++;
    }   
    net->printUsersAtStations();
    std::cout<<"TOTAL EVENTS: "<<sim->event_counter<<std::endl;
    std::cout<<"AVERAGE USAGE: "<<100*(net->total_usage/sim->get_sim_end_time())<<"%"<<std::endl;
    std::cout<<"TOTAL POWER USAGE: "<<net->getTotalPowerUsage()/sim->get_sim_end_time()<<" W"<<std::endl;
    std::cout<<"TOTAL USERS CREATED: "<<sim->users_counter<<std::endl;
    std::cout<<"USERS REJECTED: "<<100*(static_cast<double>(net->calcTotalUsersRejected())/sim->users_counter)<<"% ["<<net->calcTotalUsersRejected()<<"]"<<std::endl;
}

int main(void){
    srand(time(NULL)); 
    std::clock_t c_start = std::clock();
    freopen("simlog.txt","w",stderr);

    Config cfg; //Create config for simulation
    const int num_of_simulations = 1;
    double loc_lambda = 0.00;
    std::vector<unsigned> dropped_users = {};
    for (int i=0;i<num_of_simulations;i++){
        loc_lambda = 10+0.5*static_cast<double>(i);
        Simulation sim(loc_lambda); //Init simulation with given config
        //sim.disable_sleep = true;
        sim.debug();
        Network net; //Create the network
        simulate(&sim, &net);
        dropped_users.push_back(net.calcTotalUsersRejected());
        std::cerr<<"lambda: "<<sim.lambda<<" users_dropped: "<<net.calcTotalUsersRejected()<<std::endl;
        delete &sim, net;
    }
    for(auto d : dropped_users){
        std::cout<<d<<" ";
    }

    std::clock_t c_end = std::clock();
    long double time_elapsed_ms = 1000.0 * (c_end-c_start) / CLOCKS_PER_SEC;
    std::cout <<std::endl<< "CPU time used: " << time_elapsed_ms << " ms\n";
    return 0;
}