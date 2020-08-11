// Travelling Salesman Problem
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include <math.h>
#include <string>
#include <cmath>
#include <ctime>
#include <mutex>
using namespace std;


/**
 * Creates the set of chromosome/paths.
 * 
 * @param numCities is the number of cities.
 * @param dimension of the population.
 * @return the population/list of chromosomes.
*/
vector<vector<int>> create_population(int numCities, int dimension) {
    vector<vector<int>> population;
    vector<int> chromosome;
    for (size_t i = 0; i < numCities; i++) chromosome.push_back(i);
    chromosome.push_back(chromosome[0]);
    population.push_back(chromosome);
    for (size_t j = 0; j < dimension-1; j++) {
        std::random_shuffle(chromosome.begin()+1, chromosome.end()-1);
        population.push_back(chromosome);
    }
    return population;
}

void display_population(vector<vector<int>> population) {
    cout<<"\nPopulation:\n";
    int idx = 1;
    for (vector<int> chromosome : population) {
        string s = to_string(idx) + "\t";
        for (int gene : chromosome) s += to_string(gene) + " ";
        cout<<s<<endl;
        idx++;
    }
}

/**
 * Creates the coordinates of the cities in the plane.
 * 
 * @param numCities is the number of cities.
 * @return the coordinates of the cities/list of (x,y) pairs.
*/
vector<pair<double,double>> create_coordinates(int numCities) {
    srand(time(0));
    vector<pair<double,double>> coordinates;
    for (size_t i = 0; i < numCities; i++) {
        double x = (double)rand() / (double)(RAND_MAX/numCities);
        double y = (double)rand() / (double)(RAND_MAX/numCities);
        coordinates.push_back(make_pair(x,y));
    }
    return coordinates;
}

void display_coordinates(vector<pair<double,double>> coordinates) {
    cout<<"\nCoordinates:\n";
    for (int i = 0; i < coordinates.size(); i++)
        cout<<i<<"\t("<<coordinates[i].first<<", "
            <<coordinates[i].second<<")"<<endl;
}

/**
 * Creates the matrix of the distances between cities.
 * 
 * @param coordinates are the (x,y) coordinates of the cities.
 * @return the list of distances of the cities.
*/
vector<vector<double>> create_distances_matrix(
                                    vector<pair<double,double>> coordinates) {
    int numCities = coordinates.size();
    vector<vector<double>> matrix(numCities, vector<double>(numCities));
    for (size_t row = 0; row < numCities; row++) {
        for (size_t col = 0; col < numCities; col++) {
            double x1 = coordinates[row].first;
            double y1 = coordinates[row].second;
            double x2 = coordinates[col].first;
            double y2 = coordinates[col].second;
            int xd = abs((int)(x1 - x2)); // x distance
            int yd = abs((int)(y1 - y2)); // y distance
            matrix[row][col] = sqrt(pow(xd,2) + pow(yd,2));
        }
    }
    return matrix;
}

void display_distances_matrix(vector<vector<double>> distances) {
    cout<<"\nDistances:\n";
    for (size_t i = 0; i < distances.size(); i++) cout<<"\t"<<i;
    cout<<"\n"<<endl;
    for (size_t row = 0; row < distances.size(); row++) {
        cout<<row;
        for (size_t col = 0; col < distances[row].size(); col++) 
            cout<<"\t"<<distances[row][col];
        cout<<endl;
    }
}

/**
 * Computes fitness function.
 * 
 * @param chromosome is the path containing cities to be crossed.
 * @param distances is the matrix containing the distances between cities.
 * @return the fitness of the chromosome.
*/
double fitness(vector<int> chromosome, vector<vector<double>> distances) {
    double distance = .0;
    for (size_t i = 0; i < chromosome.size()-1; i++) {
        int c1 = chromosome[i];     // first city
        int c2 = chromosome[i+1];   // second city
        distance += distances[c1][c2];
    }
    return (double)1 / (distance + 1.0e-9);
}

/**
 * Computes the fitness function among all the population's members.
 * 
 * @param population is the population to be processed.
 * @param distances is the matrix of distances.
 * @return the fitnesses of the individuals of the population.
*/
vector<double> fitness_population(vector<vector<int>> population,
                                 vector<vector<double>> distances) {
    vector<double> fitnesses;
    for (size_t i = 0; i < population.size(); i++) 
        fitnesses.push_back(fitness(population[i], distances));
    return fitnesses;
}

void display_fitness_population(vector<double> fitnesses) {
    cout<<"\nFitnesses:\n";
    for (size_t i = 0; i < fitnesses.size(); i++) 
        cout<<i+1<<"\t"<<fitnesses[i]<<endl;
}

/**
 * Computes crossover among two parents.
 * 
 * @param parent1 is the chromosome of the first parent.
 * @param parent2 is the chromosome of the second parent.
 * @return the children chromosome.
*/
vector<int> crossover(vector<int> parent1, vector<int> parent2) {
    int len = parent1.size();
    int start = rand() % (len - 1) + 1;
    int end = rand() % (len - 1) + 1;
    if (start > end) swap(start,end);

    vector<int> child; child.push_back(parent1[0]);
    vector<int> gene;
    for (size_t i = start; i < end; i++) {
        gene.push_back(parent1[i]);
        child.push_back(parent1[i]);
    }
    for (size_t i = 1; i < parent2.size()-1 && child.size() <= len; i++) {
        int c = parent2[i];
        if (find(gene.begin(), gene.end(), c) == gene.end()) child.push_back(c);
    }
    child.push_back(parent1[0]);

    #if TRACE_TSP
    cout<<"\nCrossover:\n";
    cout<<"\tStart="<<start<<", End="<<end<<"\n";
    cout<<"\tParent1:\t"; for (int e : parent1) cout<<e<<" "; cout<<"\n";
    cout<<"\tParent2:\t"; for (int e : parent2) cout<<e<<" "; cout<<"\n";
    cout<<"\tChildren:\t"; for (int e : child) cout<<e<<" "; cout<<endl;
    #endif

    return child;
}

/**
 * Performs mutation on the chromosome.
 * 
 * @param chromosome is the chromosome to be mutated.
*/
void mutation(vector<int> &chromosome) {
    double p = 0.1;
    double r = ((double) rand() / RAND_MAX);
    if (r <= p) {
        int len = chromosome.size()-1;
        int p1 = rand() % (len - 1) + 1;
        int p2 = rand() % (len - 1) + 1;
        #if TRACE_TSP
        cout<<"Mutation:\n"
            <<"\tp1="<<p1<<", p2="<<p2<<"\n\t";
        for (int c : chromosome) cout<<c<<" "; cout<<"\n\t";
        #endif
        swap(chromosome[p1], chromosome[p2]);
        #if TRACE_TSP
        for (int c : chromosome) cout<<c<<" "; cout<<endl;
        #endif
    }
}

/**
 * Sorts the population by fitness.
 * 
 * @param population is the list of chromosome.
 * @param distances the list of distances of the cities.
*/
void ranking(vector<vector<int>> &population, vector<vector<double>> distances) {
    std::sort(population.begin(), population.end(), 
            [&](vector<int> a, vector<int> b) {
                double fa = fitness(a, distances);
                double fb = fitness(b, distances);
                return fa > fb;
    });
}

template <typename Iterator>
void ranking_iter(Iterator start, Iterator end, vector<vector<double>> distances) {
    std::sort(start, end, [&](vector<int> a, vector<int> b) {
        double fa = fitness(a, distances);
        double fb = fitness(b, distances);
        return fa > fb;
    });
}