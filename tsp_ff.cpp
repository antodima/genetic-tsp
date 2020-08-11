// FastFlow implementation of TSP with GA
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <string>
#include <math.h>
#include <ctime>
#include <mutex>
using namespace std;
using namespace std::chrono;

#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include <ff/utils.hpp>
#include <ff/pipeline.hpp>
using namespace ff;

#include "tsp.hpp"
#include "utils/utimer.hpp"
using path  = vector<int>;
using coord = pair<double,double>;

mutex lk;


struct Master : ff_node_t<path> {
    Master(vector<path> p) : population(p), max_size(p.size()) {}

    path* svc(path *task) {
        if (isStart) {
            for (path p : population) ff_send_out(new path(p));
            #if TRACE_FF
            { lk.lock(); cout<<"Master sended "<<population.size()<<" tasks"<<endl; lk.unlock(); }
            #endif
            population.clear();
            isStart = false;
            return EOS;
        }
        if (task != nullptr) {
            ntasks++;
            #if TRACE_FF
            { lk.lock(); cout<<"Master received task "<<ntasks<<endl; lk.unlock(); }
            #endif
            path &t = *task;
            population.push_back(t);
            delete task;
        }
        return GO_ON;
    }

    void eosnotify(ssize_t) {
        if (population.size() >= max_size) {
            for (size_t i = 0; i < max_size; i++) ff_send_out(new path(population[i]));
            #if TRACE_FF
            { lk.lock(); cout<<"Master sended in `eosnotify` "<<max_size<<" tasks"<<endl; lk.unlock(); }
            #endif
            population.clear();
            ntasks = 0;
        }
        ff_send_out((path*)EOS);
    }

    vector<path>    population;
    size_t          max_size;
    size_t          ntasks;
    bool            isStart = true;
};

struct Worker: ff_node_t<path> {
    Worker(vector<vector<double>> d) : distances(d) {}

    path* svc(path *task) {
        #if TRACE_FF
        { lk.lock(); cout<<"Worker "<<get_my_id()<<" received a task"<<endl; lk.unlock(); }
        #endif
        path &t = *task;
		if (task != nullptr) subPopulation.push_back(t);
		return GO_ON;
    }

    void eosnotify(ssize_t) {
        vector<path> childrens;
        for (size_t i = 0; i < subPopulation.size()-1; i++) {
            path child = crossover(subPopulation[i], subPopulation[i+1]);
            mutation(child);
            childrens.push_back(child);
        }
        for (path c : childrens) subPopulation.push_back(c);
        ranking(subPopulation, distances);
        for (path p : subPopulation) ff_send_out(new path(p));
        #if TRACE_FF
        { lk.lock(); cout<<"Worker "<<get_my_id()<<" sended "<<subPopulation.size()<<" elements ("<<childrens.size()<<" childrens)"<<endl; lk.unlock(); }
        #endif
    }

    void svc_end() { subPopulation.clear(); }

    vector<path>            subPopulation;
    vector<vector<double>>  distances;
};


int main(int argc, char* argv[]) {
    if (argc == 1)
        cerr<<"Usage: ./tsp_ff 'num.cities' 'num.population' 'num.generations' 'num.workers'"<<endl;
    int numCities = atoi(argv[1]);
    cout<<"Num. cities: "<<numCities<<endl;
    int numPopulation = atoi(argv[2]);
    cout<<"Num. population: "<<numPopulation<<endl;
    int numGenerations = atoi(argv[3]);
    cout<<"Num. generations: "<<numGenerations<<endl;
    int nw = atoi(argv[4]);
    cout<<"Num. workers: "<<nw<<"\n"<<endl;
    srand(time(0));
    auto now = system_clock::to_time_t(system_clock::now());
    cout<<"Time: "<<std::put_time(std::localtime(&now), "%H:%M:%S")<<endl;

    vector<vector<int>>         population;
    vector<pair<double,double>> coordinates;
    vector<vector<double>>      distances;
    {
        utimer t("Program");

        population  = create_population(numCities, numPopulation);
        coordinates = create_coordinates(numCities);
        distances   = create_distances_matrix(coordinates);

        vector<ff_node*> workers;
        for (size_t i = 0; i < nw; i++)
            workers.push_back(new Worker(distances));//(new ff_comb(new Breeder(), new Ranker(distances)));
        ff_farm farm;
        farm.add_emitter(new Master(population));
        farm.add_workers(workers);
        // farm.cleanup_workers();
        farm.remove_collector();
        farm.wrap_around();

        for (size_t i = 0; i < numGenerations; i++) {
            #if TRACE_FF
            cout<<"--Generation "<<(i+1)<<endl; 
            #endif
            if (farm.run_and_wait_end() < 0) error("running farm");
        }
    }

    return 0;
}
