
#include "DropPriceFunction.h"

DropPriceFunction::DropPriceFunction(int** schedule, int numberOfTeams) : PriceFunction(schedule, numberOfTeams) {
}

long double* DropPriceFunction::assignPrice(int** scenario, int start, int end, std::vector<int> startingScore) {
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
        if(i < indices.size() / 2) {
            price[team] = 2.0 / numberOfTeams;
        }
    }

    return price;
}


PriceFunction* DropPriceFunction::clone() {
    return new DropPriceFunction(*this);
}

DropPriceFunction::~DropPriceFunction() {
}