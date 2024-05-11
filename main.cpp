#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iterator>
#include <random>

#define assert(condition) \
    if (!(condition)) { \
        std::cout<<"Assertion failed at line: "<<__LINE__ \
        <<" ("<<#condition<<")"<<std::endl; \
        std::exit(EXIT_FAILURE); \
    }

#define assertNoExit(condition) \
    if (!(condition)) { \
        std::cout<<"Assertion failed at line: "<<__LINE__ \
        <<" ("<<#condition<<")"<<std::endl; \
    }

enum Phase{ phase1 = 1, phase2 = 2, phase3 = 3, phase4 = 4, unknown = -1 };

class Simulation{
    public:
        bool dbg, disable_sleep;
        unsigned time, end_time;
        unsigned event_counter, users_counter;
        unsigned hours;
        double intensity, factor, lambda;
        float L, H;
        unsigned N, R;
        Phase phase;
        Simulation(unsigned N = 3, unsigned R = 273,
                   float L = 0.2, float H = 0.8,
                   unsigned end_time = 24*60*60*1000,
                   double lambda = 5.00) {
            time = 0; this->end_time = end_time;
            dbg = false; disable_sleep = false;
            factor = 0.5; this->lambda = lambda /*5*/; intensity = factor*lambda;
            event_counter = 0; users_counter = 0;
            hours = 60*60*1000;
            this->N = N; this->R = R;
            this->L = L; this->H = H;
            phase = Phase::phase1;
        }

        bool run();
        void debug();
        void changeIntensity();
        void config();
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
    assert(false);
    return unknown;
}

void Simulation::config(){
    std::cout<<"--- SIMULATION CONFIG ---"<<std::endl;
    std::cout<<"LAMBDA:"<<lambda<<std::endl;
    std::cout<<"L:"<<L<<std::endl;
    std::cout<<"H:"<<H<<std::endl;
    std::cout<<"SIM_END_TIME:"<<end_time<<" [ms]"<<std::endl;
    std::cout<<"DISABLE SLEEP:";
    disable_sleep ? std::cout<<"Yes" : std::cout<<"No";
    std::cout<<std::endl;
    std::cout<<"-------------------------"<<std::endl;
}

void Simulation::changeIntensity(){
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
            assert(false);
    }
    return;
}

void Simulation::debug(){
    this->dbg = true;
}

bool Simulation::run(){
    if (time < end_time)
        return true;
    return false;
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

void insertEvent(std::vector<Event>* events_list, Event event){
    assert(!(events_list->size() < 2));
    for (unsigned i=0;i<events_list->size();i++){
        if ((*events_list)[i].time<=event.time){
            events_list->insert(events_list->begin()+i,event);
            return;
        }
    }
    events_list->push_back(event);
    return;
}

class BaseStation{
    public:
        unsigned id, resources, users;
        unsigned users_rejected;
        unsigned next_user_time;
        double power;
        bool hibernate, scheduled_state_change;
        bool overloaded, triggered;
        Simulation* baseSim = nullptr; // scope to all BaseStation methods
        BaseStation(Simulation* sim){
            baseSim = sim;
            assert(baseSim);
            this->resources = baseSim->R;
            this->power = 0.00;
            this->users_rejected = 0;
            this->hibernate = true;
            this->scheduled_state_change = false;
            this->overloaded = false;
            this->triggered = false;
            this->users = 0;
        }   
        double currentUsage();
        double currentPower(unsigned);
        bool executeEvent(Event, std::vector<Event>*, Simulation*, bool);
        Event scheduleEvent(Simulation*);
        void getNextUserArrivalTime(Simulation*);   
        void activate();
        void deactivate();
        bool isHibernated();
        bool isFull();
        bool isEmpty();
        bool isHeavyLoaded();
};

bool BaseStation::isHeavyLoaded(){
    return (currentUsage() > baseSim->H ? true : false);
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
    return static_cast<double>(users)/resources;
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
                users++;
                unsigned user_departure_time = sim->time+1000+rand()%29001; 
                Event event(user_departure_time, id, eventType::Departure); // Schedule user departure event if user is added
                insertEvent(event_list, event);
                break;
            }
            else
                return false; // Redirect to another station

        case eventType::Departure: // User departs
            //users_list.pop_back();
            users--;
            break;
        
        case eventType::Intensity:
            sim->changeIntensity();
            break;

        default: //Unknown event type
            assert(false);
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
    if(users == 0)
        return true;
    return false;
}

bool BaseStation::isFull(){
    if(users == baseSim->R)
        return true;
    return false;
}

class Network{
    public:
        std::vector<BaseStation> stations;
        std::vector<Event> events_list;
        double total_usage, total_power;
        unsigned next_event_time, next_user;
        int triggered_station_id;
        Simulation* netSim = nullptr; // scope to all Network methods
        Network(Simulation* sim){
            netSim = sim;
            assert(netSim);
            total_usage = 0.00; total_power = 0.00;
            triggered_station_id = -1;
            stations.clear(); events_list.clear();
            for(unsigned i=0;i<netSim->N;i++){
                BaseStation station(sim);
                station.id = i;
                stations.push_back(station);
            }
        }
        /* MAIN FUNCTIONS */
        void methodABC(Simulation*);

        void init(Simulation*);
        void scheduleIntensityChange(Simulation*);
        void stationManagement(Simulation*);
        void executeStationEvent(Event, Simulation*);
        unsigned get_id_station_least_users();
        unsigned get_id_station_not_hibernated();
        unsigned get_number_of_heavy_loaded_stations();

        bool all_stations_empty();
        bool all_stations_full();
        bool all_active_stations_full();
        bool allStationsActive();
        bool isOnlyActiveStation(unsigned);
        bool isStationEvent(Event);
        bool neighbourStationHeavyLoaded(unsigned);
        bool anyStationToHibernate();
        /*****************/

        /* Statistics */
        void getNetStats(unsigned);
        void calcStationsUsage(unsigned);
        void calcStationsPower(unsigned);
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

void Network::init(Simulation* sim){ // Initialize network
    for (unsigned i=0;i<sim->N;i++){
        if (sim->disable_sleep) { stations[i].activate(); } // Activate all stations
        if ( i == 0) { stations[i].activate(); } // Activate only one station at first
        events_list.push_back(stations[i].scheduleEvent(sim)); // Schedule first arrival events at all stations
    }
    scheduleIntensityChange(sim);
    sort(events_list.begin(), events_list.end(), compareEvents);
}

void Network::executeStationEvent(Event event, Simulation* sim){
    bool local_debug = false;

    if (event.type == eventType::ActivateStation){
        assert(!(triggered_station_id == -1));

        BaseStation& overloaded_station = stations[triggered_station_id]; 
        BaseStation& wakeup_station = stations[event.sid];

        wakeup_station.activate();
        
        unsigned num_of_users_to_transition = (overloaded_station.users)/2;
        unsigned counter = 0;
        for (auto &e : events_list){
            if(counter == num_of_users_to_transition)
                break;
            if(e.sid == overloaded_station.id && e.type == eventType::Departure){
                e.sid = event.sid; // Reschedule OVERLOAD event to WAKEUP station
                wakeup_station.users++; // OVERLOAD --User--> WAKEUP
                overloaded_station.users--;
                counter++;
            }
        }
        //stations[event.sid].activate();
        wakeup_station.scheduled_state_change = false;
        overloaded_station.triggered = false;
        assert(!(overloaded_station.users > sim->R || wakeup_station.users > sim->R));
        if (local_debug){
            printUsersAtStations();
            printStateOfAllStations();
        }
    }
    else if (event.type == eventType::HibernateStation){
        //printUsersAtStations();
        BaseStation& station_to_hibernate = stations[event.sid];

        std::vector<unsigned> active_stations_ids = {};
        unsigned free_slots = 0; // Check if users will fit to active station/stations
        for (auto s : stations){
            if((!s.isHibernated()) && (s.id != station_to_hibernate.id) /*DO NOT CONSIDER STATION ITSELF*/){
                active_stations_ids.push_back(s.id);
                free_slots += (sim->R - s.users);
            }
        }
        assert(active_stations_ids.size() != 0);

        unsigned num_of_active_stations = active_stations_ids.size();
        unsigned users_to_transfer = station_to_hibernate.users;

        if (local_debug){
            std::cout<<"Users to transfer:"<<users_to_transfer<<" Free slots:"<<free_slots<<std::endl;
        }

        if (users_to_transfer > free_slots){
            unsigned users_to_drop = users_to_transfer - free_slots;

                // Check if amount of events for station is equal users to transfer
                /*unsigned counter = 0;
                for (auto e : events_list){
                    if((e.sid == station_to_hibernate.id) && (e.type == eventType::Departure)){
                        counter++;
                    }
                }
                assert(counter == users_to_transfer);*/

                for (int i=0;i<events_list.size();i++){
                    if (users_to_transfer == free_slots){
                        break;
                    }
                    if ((events_list[i].sid == station_to_hibernate.id) 
                    && (events_list[i].type == eventType::Departure)){
                        events_list.erase(events_list.begin()+i);
                        users_to_transfer--;
                        station_to_hibernate.users--;
                        station_to_hibernate.users_rejected++;
                        i--; // Must have so that we never skip an event after removal
                    }
                }

            if (local_debug){
                std::cout<<users_to_drop<<" users will be dropped."<<std::endl;
                std::cout<<"After dropping users:"<<users_to_transfer<<std::endl;
            }
        }
        assert (users_to_transfer <= free_slots); // Should always work now

        unsigned division = users_to_transfer/num_of_active_stations;
        std::vector<unsigned> users_per_station = {};
        bool split = false;

        if (num_of_active_stations > 1){
            bool even = (users_to_transfer%2 == 0) ? true : false;
            if (local_debug){
                printUsersAtStations();
                std::cout<<"There is more than 1 station active ("<<num_of_active_stations<<")"<<std::endl;
                std::cout<<"Trying to split users evenly..."<<std::endl;
                if (!even){
                    std::cout<<"Warn: The number of users to transfer("<<
                    users_to_transfer<<") is not divisible by num of active stations."<<std::endl;
                }
            }


            // Here eval if users will fit to station and adjust ratio
                // Cases:
                // 0: Fit 1: Fit -> div div
                // 1: Won't fit 1: Fit -> max rest
                // 2: Fit : Won't fit -> rest max
                // 3: Won't fit : Won't fit -> assert 

            bool will_fit1 = ((sim->R - stations[active_stations_ids[0]].users) > division) ? true : false;
            bool will_fit2 = ((sim->R - stations[active_stations_ids[1]].users) > (even ? division : division + 1)) ? true : false;

            if (will_fit1 && will_fit2){
                users_per_station.push_back(division);
                users_per_station.push_back(even ? division : division+1);
                    
            }
            else if (will_fit1 && !will_fit2){
                users_per_station.push_back(users_to_transfer - (sim->R - stations[active_stations_ids[1]].users));
                users_per_station.push_back(sim->R - stations[active_stations_ids[1]].users);
            }
            else if (!will_fit1 && will_fit2){
                users_per_station.push_back(sim->R - stations[active_stations_ids[0]].users);
                users_per_station.push_back(users_to_transfer - (sim->R - stations[active_stations_ids[0]].users));
            }
            else{
                assert(false);
            }

            assert(users_per_station.size() == active_stations_ids.size());
            if(local_debug){
                printUsersAtStations();
                std::cout<<"Evaluated divison..."<<std::endl;
                for (unsigned i=0;i<active_stations_ids.size();i++){
                    std::cout<<"Station"<<active_stations_ids[i]<<" -> "<<users_per_station[i]<<std::endl;
                }
                split = false;
            }
        } 
        else{
            users_per_station.push_back(division);
        }

        assert(!users_per_station.empty());
    
        if (local_debug){
            std::cout<<"division:"<<division<<std::endl;
            std::cout<<"users to transfer:"<<users_to_transfer<<std::endl;
            std::cout<<"station/stations ID to transfer:";
            for (auto id : active_stations_ids){
                std::cout<<id<<" ";
            }
            std::cout<<std::endl;
        }
        
        assert(users_per_station.size() == active_stations_ids.size());
        unsigned users_transfered = 0;

        for (unsigned i=0;i<active_stations_ids.size();i++){

            BaseStation& station_to_transfer = stations[active_stations_ids[i]];
            
            unsigned station_transfers = 0;

            for (auto &e : events_list){
                if (station_transfers == users_per_station[i]){
                    break;
                }
                if ((e.sid == event.sid) && (e.type == eventType::Departure)){
                    // Transfer departure event of that user to active station
                    e.sid = station_to_transfer.id;
                    station_to_transfer.users++;
                    station_to_hibernate.users--;
                    station_transfers++;
                }
            }
        }
        assert(station_to_hibernate.users == 0);
        
        station_to_hibernate.deactivate();
        station_to_hibernate.scheduled_state_change = false;
        //stations[event.sid].triggered = false;
        if (local_debug){
            printUsersAtStations();
            printStateOfAllStations();
        }
        for (auto s : stations){
            if (s.users > sim->R){
                printUsersAtStations();
                std::cout<<"act were:"<<active_stations_ids.size()+1<<std::endl;
                std::cout<<"ids:"<<active_stations_ids.back()<<std::endl;
                for (auto x : users_per_station){
                    std::cout<<x<<" ";
                } std::cout<<std::endl;
                assert(false);
            }
        }
    }
}

bool Network::anyStationToHibernate(){
    for (auto s : stations){
        if(s.currentUsage() < netSim->L)
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
    bool local_debug = false;
    /* Case 1: Wake up hibernated station if needed */
    bool wake_up_station = false;
    unsigned hib_station_id = -1;
    unsigned overload_station_id = -1;
    if(!allStationsActive()){
        unsigned loaded_stations = get_number_of_heavy_loaded_stations();
        if (loaded_stations != 0 && loaded_stations != netSim->N){
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
        stations[hib_station_id].scheduled_state_change = true;
        triggered_station_id = overload_station_id;
        Event event(sim->time+50, hib_station_id, eventType::ActivateStation); // Turning station ON/OFF takes 50 [ms]
        insertEvent(&events_list, event);
        if(local_debug){
            std::cout<<"Overloaded station ID:"<<overload_station_id
            <<" USAGE:"<<100*stations[overload_station_id].currentUsage()<<"% ["<<stations[overload_station_id].users<<"]"<<std::endl;
            std::cout<<"Hibernated station to activate ID:"<<hib_station_id<<std::endl;
            std::cout<<"Scheduling event to activate station"<<hib_station_id<<"."<<std::endl;
            std::cout<<"SIM TIME:"<<sim->time<<std::endl;
        }
    }
    /****************************************/
    /* Case 2: Hibernated station if needed */
    bool hibernate_station = false;
    unsigned station_to_hib_id = -1;
    if (!anyStationToHibernate())
        return;
    for (auto s : stations){
        if(!s.isHibernated() && !isOnlyActiveStation(s.id)){
            if(s.currentUsage() < sim->L && !s.scheduled_state_change){
                hibernate_station = true;
                station_to_hib_id = s.id;
            }
        }
    }
    if (hibernate_station){
        // Active station should be hibernated (schedule event)
        stations[station_to_hib_id].scheduled_state_change = true;
        Event event(sim->time+50, stations[station_to_hib_id].id, eventType::HibernateStation); // Turning station ON/OFF takes 50 [ms]
        insertEvent(&events_list, event);
        if (local_debug){
            std::cout<<"Scheduling event to deactivate station"<<stations[station_to_hib_id].id
            <<". USAGE: "<<100*stations[station_to_hib_id].currentUsage()<<" ["<<stations[station_to_hib_id].users<<"]"<<std::endl;
            std::cout<<"SIM TIME:"<<sim->time<<std::endl;
        }
        
    }
    /****************************************/
}

unsigned Network::get_number_of_heavy_loaded_stations(){
    unsigned res = 0;
    for (auto s : stations){
        if (s.currentUsage() > netSim->H)
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

void Network::getNetStats(unsigned duration){
    calcStationsPower(duration);
    calcStationsUsage(duration);
}

void Network::calcStationsPower(unsigned duration){
    for (unsigned i=0;i<netSim->N;i++){
        if (stations[i].scheduled_state_change) { // Transition
            stations[i].power += static_cast<double>(duration)*(1000/50); 
        }
        else { stations[i].power += stations[i].currentPower(duration); }
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
    assert(at_least_one_station_active);
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
        if (station.currentUsage() > netSim->H)
            return true;
    }
    return false;
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
    for (auto x : stations){
        usage += x.currentUsage();
    }
    total_usage+=duration*(usage/netSim->N);
}

void Network::printUsersAtStations(){
    std::cout<<"------------------"<<std::endl;
    for(unsigned i=0;i<netSim->N;i++){
        std::cout<<"Station"<<i<<" has "<<stations[i].users<<" users."<<std::endl;
    }
}

unsigned Network::get_id_station_least_users(){
    unsigned min = stations[0].users;
    unsigned sid = 0;
    for(unsigned i=0;i<netSim->N;i++){
        if (stations[i].users< min){
            min = stations[i].users;
            sid = i;
        }
    }
    return sid;
}

void Network::printEventList(){
    for(unsigned i=0;i<events_list.size();i++){
        std::cout<<"E:"<<i<<" time:"<<events_list[i].time<<" type:";
        switch(events_list[i].type){
            case eventType::Arrival:
                std::cout<<"Arrival";
                break;
            case eventType::Departure:
                std::cout<<"Departure";
                break;
            case eventType::ActivateStation:
                std::cout<<"ActivateStation";
                break;
            case eventType::HibernateStation:
                std::cout<<"HibernateStation";
                break;
            case eventType::Intensity:
                std::cout<<"Intensity";
                break;
            default:
                assert(false);
        }
        std::cout<<" sid:"<<events_list[i].sid;
        std::cout<<std::endl<<"----------------"<<std::endl;
    }
}

void Network::methodABC(Simulation* sim){
    /*** Phase A: Simulation Advance ***/
    if ( (events_list.back().time > sim->end_time) || (events_list.empty()) ){ // Check sim end conditons
        getNetStats(sim->end_time - sim->time);
        sim->time = sim->end_time;
        return;
    }
    Event next_event = events_list.back(); // Pop next event from calendar 
    events_list.pop_back(); // Remove event from calendar
    
    getNetStats(next_event.time - sim->time); 

    sim->time = next_event.time; // Increment sim time to nearest event
    /**********************************/

    /*** Phase B: Execution of events  ***/
    while (true){ // Execute all events with the same occurance time
        assert(next_event.time == sim->time);
        sim->event_counter++;
        switch(next_event.type){
            case eventType::ActivateStation:
            case eventType::HibernateStation:
                executeStationEvent(next_event, sim);
                break;
            default:
                bool redirect = false;
                if(!stations[next_event.sid].executeEvent(next_event, &events_list, sim, redirect)){
                    if( all_active_stations_full() ){
                        stations[next_event.sid].users_rejected++;
                    }
                    else{
                        redirect = true;
                        next_event.sid = get_id_station_not_hibernated();
                        assert(stations[next_event.sid].executeEvent(next_event, &events_list, sim, redirect));
                    }
                }
                break;
        }
        if(events_list.back().time != sim->time){
            break;
        } else {
            next_event = events_list.back();
            events_list.pop_back();
        }
    }
    /*************************************/
    
    /***  Phase C: Conditional events  ***/
    if (!sim->disable_sleep){
        stationManagement(sim);
    }
    /*************************************/
}

bool Network::all_stations_full(){
    for(unsigned i=0;i<netSim->N;i++){
        if(stations[i].isFull())
            continue;
        return false;
    }
    return true;
}

bool Network::all_active_stations_full(){
    for (auto s : stations){
        if (s.users > netSim->R){
            printUsersAtStations();
            assert(s.users > netSim->R);
        }
        if (!s.isFull() && !s.isHibernated() && !s.scheduled_state_change){
            return false;
        }
    }
    return true;
}

bool Network::all_stations_empty(){
    for(unsigned i=0;i<netSim->N;i++){
        if(stations[i].isEmpty())
            continue;
        return false;
    }
    return true;
}

void Network::config(){ 
    std::cout<<"--- NETWORK CONFIG ---"<<std::endl;
    std::cout<<"N:"<<netSim->N<<std::endl;
    std::cout<<"R:"<<netSim->R<<std::endl;
    //std::cout<<"SIM_END_TIME:"<<get_sim_end_time()<<" [ms]"<<std::endl;
    for(unsigned i=0;i<netSim->N;i++)
        std::cout<<"STATION:"<<stations[i].id<<
        " RESOURCES:"<<stations[i].resources<<std::endl;
    std::cout<<"----------------------"<<std::endl;
}

void simulate(Simulation* sim, Network* net, unsigned iters=10000){
    sim->config();
    net->config();
    net->init(sim);
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
    std::cout<<"AVERAGE USAGE: "<<100*(net->total_usage/sim->end_time)<<"%"<<std::endl;
    std::cout<<"TOTAL POWER USAGE: "<<net->getTotalPowerUsage()/sim->end_time<<" W"<<std::endl;
    std::cout<<"TOTAL USERS CREATED: "<<sim->users_counter<<std::endl;
    std::cout<<"USERS REJECTED: "<<100*(static_cast<double>(net->calcTotalUsersRejected())/sim->users_counter)<<"% ["<<net->calcTotalUsersRejected()<<"]"<<std::endl;
}

int main(int argc, char** argv){
    std::cout << "Have " << argc << " arguments:\n";
    for (int i = 0; i < argc; ++i) {
        std::cout << argv[i] << "\n";
    }
    srand(time(NULL)); // seed
    std::clock_t c_start = std::clock();
    freopen("simlog.txt","w",stderr);

    const int num_of_simulations = 1;
    std::vector<unsigned> dropped_users = {};
    //for (int j=0;j<10;j++){
    //    std::cerr<<"--- RUN "<<j<<" ---"<<std::endl;
    for (int i=0;i<num_of_simulations;i++){
                    // N, R,   L,   H,   sim_end,  lambda
        Simulation sim(3, 273, 0.2, 0.8, 86400000, 10.0); //Init simulation with given config
        //sim.disable_sleep = true;
        Network net(&sim); //Create the network
        simulate(&sim, &net);
        dropped_users.push_back(net.calcTotalUsersRejected());
        std::cerr<<"lambda: "<<sim.lambda<<" users_dropped: "<<net.calcTotalUsersRejected()<<std::endl;
        delete &sim, net;
    }

    std::clock_t c_end = std::clock();
    long double time_elapsed_ms = 1000.0 * (c_end-c_start) / CLOCKS_PER_SEC;
    std::cout <<std::endl<< "CPU time used: " << time_elapsed_ms << " ms\n";
    return 0;
}