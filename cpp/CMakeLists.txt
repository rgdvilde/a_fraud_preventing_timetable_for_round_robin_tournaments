cmake_minimum_required(VERSION 3.27)
project(thesis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add nlohmann/json
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

add_executable(thesis main.cpp
        Configuration.cpp
        Configuration.h
        ELOPredictor.cpp
        ELOPredictor.h
        Predictor.cpp
        Predictor.h
        TriplePredictor.cpp
        TriplePredictor.h
        calculation/FVCalculation.cpp
        calculation/FVCalculation.h
        calculation/SimulatedFVCalculation.cpp
        calculation/SimulatedFVCalculation.h
        helpers/Data.cpp
        helpers/Data.h
        helpers/JSON.cpp
        helpers/JSON.h
        optimization/Optimization.cpp
        optimization/Optimization.h
        optimization/ExactHigestFirstMappingOptimization.cpp
        optimization/ExactHigestFirstMappingOptimization.h
        optimization/ExactHighestLastMappingOptimization.cpp
        optimization/ExactHighestLastMappingOptimization.h
        optimization/RandomizeMapping.cpp
        optimization/RandomizeMapping.h
        priceFunctions/PriceFunction.cpp
        priceFunctions/PriceFunction.h
        priceFunctions/LinearPriceFunction.cpp
        priceFunctions/LinearPriceFunction.h
        priceFunctions/WinnerTakesAllPriceFunction.cpp
        priceFunctions/WinnerTakesAllPriceFunction.h
        priceFunctions/TopThreePriceFunction.cpp
        priceFunctions/TopThreePriceFunction.h
        priceFunctions/InverseExponentialPriceFunction.cpp
        priceFunctions/InverseExponentialPriceFunction.h
        priceFunctions/EqualPriceFunction.cpp
        priceFunctions/EqualPriceFunction.h
        priceFunctions/DropPriceFunction.cpp
        priceFunctions/DropPriceFunction.h)

target_link_libraries(thesis PRIVATE
        nlohmann_json::nlohmann_json
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
