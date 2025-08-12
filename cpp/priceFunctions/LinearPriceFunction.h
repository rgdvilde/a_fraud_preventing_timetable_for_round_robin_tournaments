#ifndef THESIS_LINEARPRICEFUNCTION_H
#define THESIS_LINEARPRICEFUNCTION_H

#include "./PriceFunction.h"

class LinearPriceFunction : public PriceFunction  {
public:
    LinearPriceFunction(int **schedule, int numberOfTeams);
    long double* assignPrice(int** scenario, int start, int end, std::vector<int> startingScore);
    ~LinearPriceFunction() override;
    PriceFunction* clone() override;
};


#endif //THESIS_LINEARPRICEFUNCTION_H
