#include "TopThreePriceFunction.h"

TopThreePriceFunction::TopThreePriceFunction(int** schedule, int numberOfTeams) : PriceFunction(schedule, numberOfTeams) {
}

long double* TopThreePriceFunction::assignPrice(int** scenario, int start, int end, std::vector<int> startingScore) {
    vector<int>score(std::move(startingScore));
    for(int i = start; i < end; i++) {
        for(int j = 0; j < numberOfTeams / 2; j++) {
            if(scenario[i-start][j] == 0) {
                score[schedule[i][2*j]] += 3;
            }
            else if(scenario[i-start][j] == 1) {
                score[schedule[i][2*j+1]] += 3;
            }
            else if(scenario[i-start][j] == 2) {
                score[schedule[i][2*j]] += 1;
                score[schedule[i][2*j+1]] += 1;
            }
        }
    }
    auto* price = new long double[numberOfTeams];
    int _score = -1;

    vector<int> indices(score.size());
    for (size_t i = 0; i < indices.size(); i++) {
        indices[i] = i;
    }

    sort(indices.begin(), indices.end(), [&](int a, int b) {
        return score[a] > score[b];
    });

    for (size_t i = 0; i < indices.size(); i++) {
        if(i == 0) price[indices[i]] = 0.6;
        if(i == 1) price[indices[i]] = 0.3;
        if(i == 2) price[indices[i]] = 0.1;
    }

    return price;
}

PriceFunction* TopThreePriceFunction::clone() {
    return new TopThreePriceFunction(*this);
}

TopThreePriceFunction::~TopThreePriceFunction() {
}