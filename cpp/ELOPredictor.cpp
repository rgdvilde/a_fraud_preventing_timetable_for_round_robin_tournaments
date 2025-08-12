//
// Created by rgdv on 12/02/2025.
//

#include "ELOPredictor.h"

ELOPredictor::ELOPredictor(vector<double> elo) : Predictor(){
    this->elo = elo;
}

int ELOPredictor::predict(float rng, const vector<int>& index, map<vector<int>, tuple<float, float, float, int>> adaptations) {
    if(adaptations.find(index) != adaptations.end()) {
        if(rng < get<0>(adaptations[index])) {
            return 0;
        }
        else if(rng < (get<0>(adaptations[index]) + get<1>(adaptations[index]))) {
            return 1;
        }
        return 2;
    }
    if (elo.empty()) {
        throw runtime_error("Error: elo map is not defined!");
    }
    double alpha = pow(10,-(elo[index[0]] + 100 - elo[index[1]])/200);
    double theta = exp(cutoff);

    vector<double> odds = {
            1 / (theta * alpha + 1),
            alpha / (theta + alpha),
            (alpha * (pow(theta, 2) - 1)) / ((alpha + theta) * (alpha * theta + 1))
    };

    if(rng < odds[0]) {
        return 0;
    }
    else if(rng < odds[0] + odds[1]) {
        return 1;
    }
    return 2;
}

Predictor* ELOPredictor::clone() {
    return new ELOPredictor(*this);
}

ELOPredictor::ELOPredictor(const ELOPredictor& other) {
    this->elo = other.elo;
}

map<vector<int>, tuple<float, float, float, int>> ELOPredictor::TransformToTeams() {
    map<vector<int>, tuple<float, float, float, int>> teams = {};
    for(int i = 0; i < elo.size(); i++) {
        for(int j = 0; j < elo.size(); j++) {
            vector<int> index = {i,j};
            if(i==j) {
                teams[index] = make_tuple(0.0,0.0,0.0,0);
            }
            else {
                double alpha = pow(10,-(elo[index[0]] + 100 - elo[index[1]])/200);
                double theta = exp(cutoff);

                vector<double> odds = {
                        1 / (theta * alpha + 1),
                        alpha / (theta + alpha),
                        (alpha * (pow(theta, 2) - 1)) / ((alpha + theta) * (alpha * theta + 1))
                };
                teams[index] = make_tuple(odds[0],odds[1],odds[2],0);
            }
        }
    }
    return teams;
}