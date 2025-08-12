#ifndef THESIS_SIMULATEDFVCALCULATION_H
#define THESIS_SIMULATEDFVCALCULATION_H

#include "./FVCalculation.h"

class SimulatedFVCalculation: public FVCalculation {
private:
    int*** run(int _runs, map<vector<int>, tuple<float, float, float, int>> adaptations = {}, int start=0, int end = -1);
    map<vector<int>, map<vector<int>, long double>> calculateTable(int*** scenarios, int amount) const;
    long double* calculateEV(int*** scenarios, int amount, int start = 0, int end = -1, vector<int> staringScore = {}) const;
public:
    Configuration config;
    int runs = 1000;
    SimulatedFVCalculation(const Configuration& config);
    map<map<string,string>, list<vector<long double>>> calculate() override;
};


#endif //THESIS_SIMULATEDFVCALCULATION_H
