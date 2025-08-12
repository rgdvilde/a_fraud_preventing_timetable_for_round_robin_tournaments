#ifndef THESIS_ELOPREDICTOR_H
#define THESIS_ELOPREDICTOR_H

#include "./Predictor.h"

class ELOPredictor : public Predictor {
public:
    vector<double> elo;
    ELOPredictor(vector<double> elo);
    int predict(float rng, const vector<int>& index, map<vector<int>, tuple<float, float, float, int>> adaptations) override;
    Predictor* clone() override;
    map<vector<int>, tuple<float, float, float, int>>  TransformToTeams();
    ELOPredictor(const ELOPredictor& other);
    float cutoff = 2.77;
};



#endif //THESIS_ELOPREDICTOR_H
