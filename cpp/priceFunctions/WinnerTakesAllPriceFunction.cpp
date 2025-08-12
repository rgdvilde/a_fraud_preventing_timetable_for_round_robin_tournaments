#include "WinnerTakesAllPriceFunction.h"

WinnerTakesAllPriceFunction::WinnerTakesAllPriceFunction(int** schedule, int numberOfTeams) : PriceFunction(schedule, numberOfTeams) {
}

long double* WinnerTakesAllPriceFunction::assignPrice(int** scenario, int start, int end, std::vector<int> startingScore) {
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
    int _index = 0;
    int _score = -1;
    for(int i = 0; i < numberOfTeams; i++) {
        if(score[i] > _score) {
            _score = score[i];
            _index = i;
        }
    }
    price[_index] = 1.0;
    return price;
}

PriceFunction* WinnerTakesAllPriceFunction::clone() {
    return new WinnerTakesAllPriceFunction(*this);
}

WinnerTakesAllPriceFunction::~WinnerTakesAllPriceFunction() {
}