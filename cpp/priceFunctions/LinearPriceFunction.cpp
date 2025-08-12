#include "LinearPriceFunction.h"

LinearPriceFunction::LinearPriceFunction(int** schedule, int numberOfTeams) : PriceFunction(schedule, numberOfTeams) {
}

long double* LinearPriceFunction::assignPrice(int** scenario, int start, int end, std::vector<int> startingScore) {
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
        int team = indices[i];
        double thisPrice = 2 * (double)(indices.size() - i) / (double)(indices.size() * (indices.size() + 1));
        price[team] = thisPrice;
    }

    return price;
}

PriceFunction* LinearPriceFunction::clone() {
    return new LinearPriceFunction(*this);
}

LinearPriceFunction::~LinearPriceFunction() {
}