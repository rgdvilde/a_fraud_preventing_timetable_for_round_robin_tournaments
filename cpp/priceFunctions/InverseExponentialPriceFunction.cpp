#include "InverseExponentialPriceFunction.h"

InverseExponentialPriceFunction::InverseExponentialPriceFunction(int** schedule, int numberOfTeams) : PriceFunction(schedule, numberOfTeams) {
}

long double* InverseExponentialPriceFunction::assignPrice(int** scenario, int start, int end, std::vector<int> startingScore) {
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

    for (int i = 0; i < indices.size(); i++) {
        int team = indices[i];
        double thisPrice = (double)(pow(2,-i) * (2-1)) / (double)(2*(1 - pow(2,-numberOfTeams)));
        price[team] = thisPrice;
    }

    return price;
}

PriceFunction* InverseExponentialPriceFunction::clone() {
    return new InverseExponentialPriceFunction(*this);
}

InverseExponentialPriceFunction::~InverseExponentialPriceFunction() {
}