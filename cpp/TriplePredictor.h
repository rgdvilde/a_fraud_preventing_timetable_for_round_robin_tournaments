#ifndef THESIS_TRIPLEPREDICTOR_H
#define THESIS_TRIPLEPREDICTOR_H

#include "./Predictor.h"

class TriplePredictor : public Predictor {
public:
    map<vector<int>, tuple<float, float, float, int>> teams;
    TriplePredictor(map<vector<int>, tuple<float, float, float, int>> teams, int i);
    TriplePredictor(TriplePredictor& other);
    int predict(float rng, const vector<int>& index, map<vector<int>, tuple<float, float, float, int>> adaptations) override;
    Predictor* clone() override;
    ~TriplePredictor() override;
};


#endif //THESIS_TRIPLEPREDICTOR_H
