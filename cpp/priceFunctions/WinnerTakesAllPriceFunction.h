
#ifndef THESIS_WINNERTAKESALLPRICEFUNCTION_H
#define THESIS_WINNERTAKESALLPRICEFUNCTION_H

#include "./PriceFunction.h"

class WinnerTakesAllPriceFunction : public PriceFunction {
public:
    WinnerTakesAllPriceFunction(int **schedule, int numberOfTeams);
    long double* assignPrice(int** scenario, int start, int end, std::vector<int> startingScore);
    ~WinnerTakesAllPriceFunction() override;
    PriceFunction* clone() override;
};


#endif //THESIS_WINNERTAKESALLPRICEFUNCTION_H
