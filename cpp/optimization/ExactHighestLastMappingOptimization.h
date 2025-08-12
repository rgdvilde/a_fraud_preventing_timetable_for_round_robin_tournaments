
#ifndef THESIS_EXACTHIGHESTLASTMAPPINGOPTIMIZATION_H
#define THESIS_EXACTHIGHESTLASTMAPPINGOPTIMIZATION_H

#include "./Optimization.h"

class ExactHighestLastMappingOptimization: public Optimization {
public:
    ExactHighestLastMappingOptimization();
    ExactHighestLastMappingOptimization(ExactHighestLastMappingOptimization& other);
    Configuration optimize(Configuration config);
};


#endif //THESIS_EXACTHIGHESTLASTMAPPINGOPTIMIZATION_H
