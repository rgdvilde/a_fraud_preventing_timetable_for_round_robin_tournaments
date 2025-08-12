#include "RandomizeMapping.h"
#include <random>
#include <algorithm>

RandomizeMapping::RandomizeMapping() : Optimization() {
}

RandomizeMapping::RandomizeMapping(RandomizeMapping& other) : Optimization(other) {
}

Configuration RandomizeMapping::optimize(Configuration config) {
    vector<int> permutation(config.numberOfTeams);
    for (int i = 0; i < config.numberOfTeams; i++) {
        permutation[i] = i;
    }
    
    if (config.rng != nullptr) {
        shuffle(permutation.begin(), permutation.end(), *config.rng);
    } else {
        random_device rd;
        mt19937 gen(rd());
        shuffle(permutation.begin(), permutation.end(), gen);
    }
    
    Configuration optimizedConfig(config);
    for (int i = 0; i < config.rounds; i++) {
        for (int j = 0; j < config.numberOfTeams; j++) {
            optimizedConfig.schedule[i][j] = permutation[config.schedule[i][j]];
        }
    }
    
    for (int i = 0; i < config.numberOfTeams; i++) {
        optimizedConfig.mapping[i] = permutation[config.mapping[i]];
    }
    
    return optimizedConfig;
}
