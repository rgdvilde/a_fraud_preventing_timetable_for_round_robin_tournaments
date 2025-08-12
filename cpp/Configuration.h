#ifndef THESIS_CONFIGURATION_H
#define THESIS_CONFIGURATION_H

#include <random>
#include <iostream>
#include <map>
#include <string>
#include "./Predictor.h"
#include "./priceFunctions/PriceFunction.h"
using namespace std;


class Configuration {
private:

public:
    map<string, bool> pre = {
            {"exactHighestLast", false},
            {"exactHighestFirst", false}
    };
    Configuration& operator=(const Configuration& other);
    Predictor* predictor;
    PriceFunction* priceFunction;
    vector<double> elo;
    map<vector<int>, tuple<float, float, float, int>> teams;
    int** schedule;
    int numberOfTeams;
    mt19937* rng;
    int* selection;
    vector<int> mapping;
    bool roundSwitchingOptimization;
    map<int, string> naming;
    string name;
    string basePath;
    map<string, string> fileContent;
    Configuration(mt19937* _rng, int** _schedule,map<vector<int>, tuple<float, float, float, int>> _teams);
    Configuration(mt19937* _rng, const string& jsonString, string _basePath);
    Configuration(mt19937* _rng, map<string, string> jsonMap, string _basePath);
    Configuration();
    static vector<Configuration> loadConfigurations(mt19937* _rng, const std::string &fileName);
    static vector<Configuration> generateConfigurations(mt19937* _rng, vector<string> schedule, vector<string> teams, vector<string> elo, vector<string> selection, vector<string> mapping, vector<string> naming, vector<string> priceFunction, vector<string> runs, vector<string> preRuns, vector<string> postRuns, string _basePath);
    static Configuration loadConfiguration(mt19937* _rng, const std::string &fileName);
    bool mirrorSchedule;
    int rounds;
    float cutoff = 2.0;
    ~Configuration();
    Configuration(const Configuration& other);
    string id;
    int runs = 10;  // Default number of runs for simulations
    int preRuns = 10;  // Default number of pre-runs for table calculation (will be set to runs if not specified)
    int postRuns = 10;  // Default number of post-runs for EV calculation (will be set to runs if not specified)
};


#endif //THESIS_CONFIGURATION_H
