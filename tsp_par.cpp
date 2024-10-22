// C++ Threads implementation of TSP with GA
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <future>
#include <random>
#include <vector>
#include <chrono>
#include <math.h>
#include <string>
#include <ctime>
#include <mutex>
using namespace std;
using namespace std::chrono;

#include "tsp.hpp"
#include "utils/barrier.hpp"


/**
 * Computes sub-populations.
 * 
 * @param population is the population to be splitted.
 * @param nw is the number of workers/chunks.
 * @return the sub-populations.
*/
vector<vector<vector<int>>> getSubPopulations(vector<vector<int>> population, int nw) {
    vector<vector<vector<int>>> subPopulations;
    int n = population.size();
    int delta = n / nw;
    for (int i = 0; i < nw; i++) {
        int start = i * delta;
        int end = (i != (nw - 1) ? (i + 1) * delta : n);
        subPopulations.push_back(
            vector<vector<int>>(population.begin()+start, population.begin()+end));
    }
    return subPopulations;
}


int main(int argc, char* argv[]) {
    if (argc == 1)
        cerr<<"Usage: ./tsp_par 'num.cities' 'num.population' 'num.generations' 'num.workers'"<<endl;
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
        {
            #if TRACE_PAR
            utimer c("Creation");
            #endif
            population  = create_population(numCities, numPopulation);
            coordinates = create_coordinates(numCities);
            distances   = create_distances_matrix(coordinates);
        }
        vector<thread>      workers;
        vector<vector<int>> newPopulation;
        vector<vector<vector<int>>> subPopulations;
        {
            #if TRACE_PAR
            utimer s("Subpopulation");
            #endif
            subPopulations = getSubPopulations(population, nw);
        }
        for (size_t i = 0; i < nw; i++) {
            workers.push_back(thread(
                    [&subPopulations,distances,numGenerations,i]() {
                #if TRACE_PAR
                utimer w("Worker");
                #endif
                int size = subPopulations[i].size();
                for (size_t g = 0; g < numGenerations; g++) {
                    vector<vector<int>> childrens;
                    for (size_t j = 0; j < size-1; j++) {
                        vector<int> child = crossover(subPopulations[i][j], subPopulations[i][j+1]);
                        mutation(child);
                        childrens.push_back(child);
                    }
                    subPopulations[i].insert(subPopulations[i].end(), childrens.begin(), childrens.end());
                    ranking(subPopulations[i], distances);
                    subPopulations[i].erase(subPopulations[i].begin()+size, subPopulations[i].end()); 
                }
            }));
        }
        {
            #if TRACE_PAR
            utimer t("Join");
            #endif
            for (thread &t : workers) t.join();
        }
        
        {
            #if TRACE_PAR
            utimer t("Merging");
            #endif
            bool done = false;
            while (!done) {
                int zero_sizes = 0; // number of processed vectors
                // update population with the sorted one
                for (size_t i = 0; i < subPopulations.size(); i++) {
                    if (subPopulations[i].size()) {
                        newPopulation.push_back(subPopulations[i].front());
                        subPopulations[i].erase(subPopulations[i].begin());
                    }
                    else
                        zero_sizes++;
                }
                done = (zero_sizes == subPopulations.size());
            }
        }
        population = newPopulation;
    }

    return 0;
}
