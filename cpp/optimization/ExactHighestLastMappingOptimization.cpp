
#include "ExactHighestLastMappingOptimization.h"

ExactHighestLastMappingOptimization::ExactHighestLastMappingOptimization() : Optimization() {
}

ExactHighestLastMappingOptimization::ExactHighestLastMappingOptimization(ExactHighestLastMappingOptimization& other)  : Optimization(other) {
}

Configuration ExactHighestLastMappingOptimization::optimize(Configuration config) {
    vector<int> ranking(config.numberOfTeams);
    for (int i = 0; i < config.numberOfTeams; i++) ranking[i] = i;
    sort(ranking.begin(), ranking.end(), [&](int a, int b) {
        return config.elo[a] > config.elo[b];
    });


    vector<int> perm(config.numberOfTeams);
    for (int i = 0; i < config.numberOfTeams; i++) perm[i] = i;
    vector<vector<int>> all_permutations;
    do {
        all_permutations.push_back(perm);
    } while (next_permutation(perm.begin(), perm.end()));

    vector<int> bestPermutation = all_permutations[0];
    double bestAverageRankDifference = numeric_limits<double>::max();

    vector<double> roundWeight(config.rounds, 0.0);

    int half = (config.rounds + 1) / 2;

    for (int i = 0; i < half; i++) {
        roundWeight[i] = half - i;
    }

    double sum = accumulate(roundWeight.begin(), roundWeight.end(), 0.0);
    if (sum > 0) {
        for (int i = 0; i < half; i++) {
            roundWeight[i] /= sum;
        }
    }

    reverse(roundWeight.begin(), roundWeight.end());

    for (const auto& p : all_permutations) {
        double averageRankDifference = 0;

        for(int i = 0; i < config.rounds; i++) {
            for(int j = 0; j < config.numberOfTeams / 2; j++) {
                averageRankDifference += roundWeight[i] * (double)abs(ranking[p[config.schedule[i][2*j]]] - ranking[p[config.schedule[i][2*j + 1]]]) / (double)(config.numberOfTeams * config.rounds);
            }
        }
        if(averageRankDifference < bestAverageRankDifference) {
            bestPermutation = p;
            bestAverageRankDifference = averageRankDifference;
        }
    }
    Configuration optimizedConfig(config);
    for(int i = 0; i < config.rounds; i++) {
        for(int j = 0; j < config.numberOfTeams; j++) {
            config.schedule[i][j] = bestPermutation[config.schedule[i][j]];
        }
    }
    for(int i = 0; i < config.numberOfTeams; i++) {
        optimizedConfig.mapping[i] = bestPermutation[optimizedConfig.mapping[i]];
    }
    return optimizedConfig;
}