
#ifndef THESIS_PREDICTOR_H
#define THESIS_PREDICTOR_H


#include <vector>
#include <utility>
#include <iostream>
#include <map>

using namespace std;

class Configuration;

class Predictor {
private:
public:
    int number = 0;
    virtual int predict(float rng, const vector<int>& index, map<vector<int>, tuple<float, float, float, int>> adaptations) = 0;
    virtual Predictor* clone() = 0;
    Predictor();
    Predictor(Predictor& other) noexcept;
    virtual ~Predictor() {
    }
};

#endif //THESIS_PREDICTOR_H
