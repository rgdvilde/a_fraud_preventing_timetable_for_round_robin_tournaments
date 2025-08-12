#ifndef THESIS_INVERSEEXPONENTIALPRICEFUNCTION_H
#define THESIS_INVERSEEXPONENTIALPRICEFUNCTION_H

#include <cmath>

#include "./PriceFunction.h"

class InverseExponentialPriceFunction : public PriceFunction  {
public:
    InverseExponentialPriceFunction(int **schedule, int numberOfTeams);
    long double* assignPrice(int** scenario, int start, int end, std::vector<int> startingScore);
    ~InverseExponentialPriceFunction() override;
    PriceFunction* clone() override;
};


#endif //THESIS_INVERSEEXPONENTIALPRICEFUNCTION_H
