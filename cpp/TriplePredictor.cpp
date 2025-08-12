
#include "TriplePredictor.h"

TriplePredictor::TriplePredictor(map<vector<int>, tuple<float, float, float, int>> teams, int i) : Predictor() {
    this->teams = std::move(teams);
}

TriplePredictor::TriplePredictor(TriplePredictor& other)  : Predictor(other) {
    this->number = other.number;
    this->teams = other.teams;
}

int TriplePredictor::predict(float rng, const vector<int>& index, map<vector<int>, tuple<float, float, float, int>> adaptations) {
    if(adaptations.find(index) != adaptations.end()) {
        if(rng < get<0>(adaptations[index])) {
            return 0;
        }
        else if(rng < (get<0>(adaptations[index]) + get<1>(adaptations[index]))) {
            return 1;
        }
        return 2;
    }
    if(rng < get<0>(teams[index])) {
        return 0;
    }
    else if(rng < (get<0>(teams[index]) + get<1>(teams[index]))) {
        return 1;
    }
    return 2;
}

Predictor* TriplePredictor::clone() {
    return new TriplePredictor(*this);
}

TriplePredictor::~TriplePredictor() {
}