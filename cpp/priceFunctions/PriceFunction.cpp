#include "PriceFunction.h"

PriceFunction::PriceFunction(int** schedule, int numberOfTeams){
    this->schedule = schedule;
    this->numberOfTeams = numberOfTeams;
}