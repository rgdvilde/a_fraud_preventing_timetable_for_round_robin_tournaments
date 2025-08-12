
#ifndef THESIS_OPTIMIZATION_H
#define THESIS_OPTIMIZATION_H


#include "../Configuration.h"

class Optimization {
public:
    Optimization();
    Optimization(Optimization& other) noexcept;
    virtual Configuration optimize(Configuration config) = 0;
};


#endif //THESIS_OPTIMIZATION_H
