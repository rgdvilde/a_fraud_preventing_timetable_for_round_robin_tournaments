#ifndef THESIS_FVCALCULATION_H
#define THESIS_FVCALCULATION_H

#include "list"
#include "../Configuration.h"

class FVCalculation {
    virtual map<map<string,string>, list<vector<long double>>> calculate() = 0;
};


#endif //THESIS_FVCALCULATION_H
