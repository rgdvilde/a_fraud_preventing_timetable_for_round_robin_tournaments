
#ifndef THESIS_DROPPRICEFUNCTION_H
#define THESIS_DROPPRICEFUNCTION_H

#include "./PriceFunction.h"

class DropPriceFunction: public PriceFunction {
public:
    DropPriceFunction(int **schedule, int numberOfTeams);
    long double* assignPrice(int** scenario, int start, int end, std::vector<int> startingScore);
    ~DropPriceFunction() override;
    PriceFunction* clone() override;
};


#endif //THESIS_DROPPRICEFUNCTION_H
