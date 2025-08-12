
#include "EqualPriceFunction.h"

EqualPriceFunction::EqualPriceFunction(int** schedule, int numberOfTeams) : PriceFunction(schedule, numberOfTeams) {
}

long double* EqualPriceFunction::assignPrice(int** scenario, int start, int end, std::vector<int> startingScore) {
    auto* price = new long double[numberOfTeams];

    for (size_t i = 0; i < numberOfTeams; i++) {
        price[i] = 1.0 / numberOfTeams;
    }

    return price;
}

PriceFunction* EqualPriceFunction::clone() {
    return new EqualPriceFunction(*this);
}

EqualPriceFunction::~EqualPriceFunction() {
}