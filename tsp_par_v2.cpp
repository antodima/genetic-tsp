// C++ Threads v2 implementation of TSP with GA
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
#include <ctime>
#include <mutex>
#include <future>
using namespace std;
using namespace std::chrono;

#include "tsp.hpp"
#include "utils/barrier.hpp"


vector<pair<int,int>> getChunks(int n, int nw) {
    vector<pair<int,int>> chunks;
    int delta = n / nw;
    for (int i = 0; i < nw; i++) {
        int start = i * delta;
        int end = (i != (nw - 1) ? (i + 1) * delta : n);
        chunks.push_back(make_pair(start,end));
    }
    return chunks;
}


int main(int argc, char* argv[]) {
    if (argc == 1)
        cerr<<"Usage: ./tsp_par_v2 'num.cities' 'num.population' 'num.generations' 'num.workers'"<<endl;
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
            population  = create_population(numCities, numPopulation);
            coordinates = create_coordinates(numCities);
            distances   = create_distances_matrix(coordinates);
        }
        while (numGenerations--) {
            population.erase(population.begin()+numPopulation, population.end());
            
            Barrier barrier(nw);
            vector<future<vector<vector<int>>>> futures;
            vector<pair<int,int>> chunks = getChunks(population.size(), nw);

            for (size_t i = 0; i < nw; i++) {
                int start = chunks[i].first;
                int end = chunks[i].second;
                vector<vector<int>> subPopulation(population.begin()+start, population.begin()+end);

                futures.push_back(std::async(std::launch::async, 
                    [&barrier](vector<vector<int>> sp, vector<vector<double>> d) {
                        vector<vector<int>> childrens;
                        for (size_t i = 0; i < sp.size()-1; i++) {
                            vector<int> child = crossover(sp[i], sp[i+1]);
                            mutation(child);
                            childrens.push_back(child);
                        }
                        sp.insert(sp.end(), childrens.begin(), childrens.end());
                        ranking(sp, d);
                        return sp;
                }, subPopulation, distances ));
            }

            vector<vector<int>>         newPopulation;
            vector<vector<vector<int>>> subPopulations;
            for (auto &f : futures) subPopulations.push_back(f.get());
            // merge sub-populations
            int idx = 0;
            bool done = false;
            while (!done) {
                int zero_sizes = 0; // number of processed vectors
                // update population with the sorted one
                for (size_t i = 0; i < subPopulations.size(); i++) {
                    if (idx < subPopulations[i].size())
                        newPopulation.push_back(subPopulations[i][idx]);
                    else
                        zero_sizes++;
                }
                done = (zero_sizes == subPopulations.size());
                idx++;
            }
            population = newPopulation;
        }
    }

    return 0;
}
