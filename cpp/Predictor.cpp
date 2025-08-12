
#include "Predictor.h"

Predictor::Predictor() {
    this->number = 0;
}

Predictor::Predictor(Predictor& other) noexcept  {
    this->number = other.number;
}