
#ifndef THESIS_EXACTHIGESTFIRSTMAPPINGOPTIMIZATION_H
#define THESIS_EXACTHIGESTFIRSTMAPPINGOPTIMIZATION_H

#include "./Optimization.h"

class ExactHigestFirstMappingOptimization: public Optimization {
public:
    ExactHigestFirstMappingOptimization();
    ExactHigestFirstMappingOptimization(ExactHigestFirstMappingOptimization& other);
    Configuration optimize(Configuration config);
};


#endif //THESIS_EXACTHIGESTFIRSTMAPPINGOPTIMIZATION_H
