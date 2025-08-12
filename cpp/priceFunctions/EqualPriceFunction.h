
#ifndef THESIS_EQUALPRICEFUNCTION_H
#define THESIS_EQUALPRICEFUNCTION_H

#include "./PriceFunction.h"

class EqualPriceFunction: public PriceFunction {
public:
    EqualPriceFunction(int **schedule, int numberOfTeams);
    long double* assignPrice(int** scenario, int start, int end, std::vector<int> startingScore);
    ~EqualPriceFunction() override;
    PriceFunction* clone() override;
};


#endif //THESIS_EQUALPRICEFUNCTION_H
