#include <iostream>
#include <fstream>
#include <iomanip>
#include <cassert>
#include <random>
#include <vector>
#include <chrono>
#include <ctime>
using namespace std;
using namespace std::chrono;

#include "tsp.hpp"


int main(int argc, char* argv[]) {
    if (argc == 1)
        cerr<<"Usage: ./tsp_seq 'num.cities' 'num.population' 'num.generations'"<<endl;
    int numCities = atoi(argv[1]);
    cout<<"Num. cities: "<<numCities<<endl;
    int numPopulation = atoi(argv[2]);
    cout<<"Num. population: "<<numPopulation<<endl;
    int numGenerations = atoi(argv[3]);
    cout<<"Num. generations: "<<numGenerations<<"\n"<<endl;
    srand(time(0));
    auto now = system_clock::to_time_t(system_clock::now());
    cout<<"Time: "<<std::put_time(std::localtime(&now), "%H:%M:%S")<<endl;

    vector<vector<int>>         population;
    vector<pair<double,double>> coordinates;
    vector<vector<double>>      distances;
    {
        utimer t("Program");
        {
            // utimer t("Creation"); // average time = 92 microseconds
            population  = create_population(numCities, numPopulation);
            coordinates = create_coordinates(numCities);
            distances   = create_distances_matrix(coordinates);
        }
        while (numGenerations--) {
            population.erase(population.begin()+numPopulation, population.end());
            {
                // utimer b("Breeding"); // average time = 1793 microseconds
                for (size_t i = 0; i < numPopulation-1; i++) {
                    vector<int> child;
                    {
                        // utimer t("Crossover"); // average time = 1 microseconds
                        child = crossover(population[i], population[i+1]);
                    }
                    {
                        // utimer t("Mutation"); // average time = 0.1 microseconds
                        mutation(child);
                    }
                    {
                        // utimer t("Adding"); // average time = 0.2 microseconds
                        population.push_back(child); 
                    }
                }
            }
            { 
                // utimer t("Ranking"); // average time = 3294 microseconds
                ranking(population, distances);
            }
        }
    }
    
    return 0;
}
