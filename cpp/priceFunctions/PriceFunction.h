#ifndef THESIS_PRICEFUNCTION_H
#define THESIS_PRICEFUNCTION_H

#include <vector>
#include <iostream>

using namespace std;

class PriceFunction {
public:
    int** schedule;
    int numberOfTeams;
    PriceFunction(int** schedule, int numberOfTeams);
    virtual long double* assignPrice(int** scenario, int start, int end, vector<int> startingScore) = 0;
    virtual ~PriceFunction() {};
    virtual PriceFunction* clone() = 0;
};


#endif //THESIS_PRICEFUNCTION_H
