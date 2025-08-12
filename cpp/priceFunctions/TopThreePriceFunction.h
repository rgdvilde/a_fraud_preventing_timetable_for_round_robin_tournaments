#ifndef THESIS_TOPTHREEPRICEFUNCTION_H
#define THESIS_TOPTHREEPRICEFUNCTION_H

#include "./PriceFunction.h"

class TopThreePriceFunction : public PriceFunction   {
public:
    TopThreePriceFunction(int **schedule, int numberOfTeams);
    long double* assignPrice(int** scenario, int start, int end, std::vector<int> startingScore);
    ~TopThreePriceFunction() override;
    PriceFunction* clone() override;
};


#endif //THESIS_TOPTHREEPRICEFUNCTION_H
