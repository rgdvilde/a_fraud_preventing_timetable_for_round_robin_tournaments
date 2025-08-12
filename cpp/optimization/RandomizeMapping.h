
#ifndef THESIS_RANDOMIZEMAPPING_H
#define THESIS_RANDOMIZEMAPPING_H

#include "./Optimization.h"
#include <random>
#include <algorithm>

class RandomizeMapping: public Optimization {
public:
    RandomizeMapping();
    RandomizeMapping(RandomizeMapping& other);
    Configuration optimize(Configuration config);
};

#endif //THESIS_RANDOMIZEMAPPING_H
