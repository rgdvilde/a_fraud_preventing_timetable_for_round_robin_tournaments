#include "SimulatedFVCalculation.h"

SimulatedFVCalculation::SimulatedFVCalculation(const Configuration& config) : FVCalculation() {
    this->config = config;
}

map<map<string,string>, list<vector<long double>>> SimulatedFVCalculation::calculate() {
    map<map<string,string>, list<vector<long double>>> results;
    map<vector<int>, long double> opportunityCosts;
    int*** initialRuns = run(config.preRuns > 0 ? config.preRuns : runs);
    map<vector<int>, map<vector<int>, long double>> table = calculateTable(initialRuns, config.preRuns);

    for(int i = 0; i < config.rounds; i++) {
        vector<int> index = {0, i};
        map<vector<int>, long double> dist  = table[index];
    }

    for(int i = 0; i < config.rounds; i++) {
        for(int j = 0; j < config.numberOfTeams / 2; j++) {
            map<string, string> key = {};
            key["round"] = to_string(i);
            key["homeTeam"] = to_string(config.schedule[i][2*j]);
            key["awayTeam"] = to_string(config.schedule[i][2*j+1]);
            key["game"] = to_string(j);
            results[key] = list<vector<long double>>{};
            auto start = std::chrono::high_resolution_clock::now();
            int*** _runs[] = {
                    run(config.postRuns, {
                            {
                                    {
                                            config.schedule[i][2*j],
                                            config.schedule[i][2*j+1]
                                    },
                                    make_tuple(1.0,0.0,0.0,0)
                            }
                    }, i, config.rounds),
                    run(config.postRuns, {
                            {
                                    {
                                            config.schedule[i][2*j],
                                            config.schedule[i][2*j+1]
                                    },
                                    make_tuple(0.0,1.0,0.0,0)
                            }
                    }, i, config.rounds),
                    run(config.postRuns, {
                            {
                                    {
                                            config.schedule[i][2*j],
                                            config.schedule[i][2*j+1]
                                    },
                                    make_tuple(0.0,0.0,1.0,0)
                            }
                    }, i, config.rounds),
                    run(config.postRuns, {}, i, config.rounds)
            };
            map<vector<int>, long double> dist;
            // If the round is the first round, set the distribution to 100% for the initial score of 0 for all teams
            if(i < 1) {
                dist = {
                        {
                                vector<int>(config.numberOfTeams, 0),
                                1.0
                        }
                };
            }
            // If the round is not the first round, set the distribution to the previous round's distribution
            else {
                vector<int> index = {0,i-1};
                dist = table[index];
            }
            
            // Check that sum of probabilities equals 1.0
            long double sum = 0.0;
            for (const auto& pair : dist) {
                sum += pair.second;
            }
            if (abs(sum - 1.0) > 1e-6) {
                throw runtime_error("Sum of probabilities in dist is not 1.0. Sum = " + to_string(sum) + 
                                   " for Round " + to_string(i) + ", Game " + to_string(j));
            }
            // Calculate the EV for each team for each scenario
            auto** ev = new long double*[config.numberOfTeams];
            for(int k = 0; k < config.numberOfTeams; k++) {
                ev[k] = new long double[4];
            }
            long double PFV = 0.0;
            auto* SEV = new long double[4];
            for (const auto& pair : dist) {
                // Calculate the change in EV for each team for each scenario and each collusion strategy
                auto _t = new long double*[4];
                _t[0] = calculateEV(_runs[0], config.postRuns, i, config.rounds, pair.first);
                _t[1] = calculateEV(_runs[1], config.postRuns, i, config.rounds, pair.first);
                _t[2] = calculateEV(_runs[2], config.postRuns, i, config.rounds, pair.first);
                _t[3] = calculateEV(_runs[3], config.postRuns, i, config.rounds, pair.first);
                long double MPh = _t[0][config.schedule[i][2*j]] - _t[3][config.schedule[i][2*j]];
                long double MGh = _t[3][config.schedule[i][2*j]] - _t[1][config.schedule[i][2*j]];
                long double MPa = _t[1][config.schedule[i][2*j+1]] - _t[3][config.schedule[i][2*j+1]];
                long double MGa = _t[3][config.schedule[i][2*j+1]] - _t[0][config.schedule[i][2*j+1]];
                long double FVh = MPh - MGa;
                long double FVa = MPa - MGh;
                long double FVd = _t[2][config.schedule[i][2*j]] - _t[3][config.schedule[i][2*j]] + _t[2][config.schedule[i][2*j+1]] - _t[3][config.schedule[i][2*j+1]];

                map<string, string> key3 = {};
                key3["round"] = to_string(i);
                key3["homeTeam"] = to_string(config.schedule[i][2*j]);
                key3["awayTeam"] = to_string(config.schedule[i][2*j+1]);
                key3["game"] = to_string(j);
                results[key3].push_back({
                    pair.second, FVh, FVa, FVd
                });

                for(int k = 0; k < config.numberOfTeams; k++) {
                    for(int l = 0; l < 4; l++) {
                        ev[k][l] += pair.second * _t[l][k];
                    }
                }
            }
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::nano> iteration_duration = end - start;
            double duration_in_minutes = iteration_duration.count() / (60.0 * 1e9);
            vector<int> index = {i,j};
            opportunityCosts[index] = PFV;
            for(int _i = 0; _i < 4; _i++) {
                for(int _j = 0; _j < config.postRuns; _j++) {
                    for(int _k = 0; _k < config.rounds - i; _k++) {
                        delete _runs[_i][_j][_k];
                    }
                    delete _runs[_i][_j];
                }
                delete _runs[_i];
            }
        }
    }
    
    return results;
}

// Run the simulation and get a number of scenarios
int*** SimulatedFVCalculation::run(int _runs, map<vector<int>, tuple<float, float, float, int>> adaptations, int start, int end) {
    int _end = end;
    if(_end < 0) _end = config.rounds;
    uniform_real_distribution<float> uniform(0, 1);
    int*** scenario = new int**[_runs];
    for(int i = 0; i < _runs; i++) {
        scenario[i] = new int*[_end - start];
        for(int j = start; j < _end; j++) {
            scenario[i][j - start] = new int[config.numberOfTeams / 2];
            for(int k = 0; k < config.numberOfTeams / 2; k++) {
                float randomNumber = uniform(*config.rng);
                int homeTeam = config.schedule[j][2*k];
                int awayTeam = config.schedule[j][2*k+1];
                vector<int> index = {homeTeam, awayTeam};
                float homeWinOdds = get<0>(config.teams[index]);
                float awayWinnOdds = get<1>(config.teams[index]);
                scenario[i][j - start][k] = config.predictor->predict(randomNumber, index, adaptations);
            }
        }
    }
    return scenario;
}

// Calculate the table of probabilities of each score distribution
map<vector<int>, map<vector<int>, long double>> SimulatedFVCalculation::calculateTable(int*** scenarios, int amount) const {
    map<vector<int>, map<vector<int>, long double>> table;
    auto* ev = new double[config.numberOfTeams];
    for(int i = 0; i < amount; i++) {
        vector<int> points(config.numberOfTeams, 0);
        for(int j = 0; j < config.rounds; j++) {
            for(int k = 0; k < config.numberOfTeams / 2; k++) {
                if(scenarios[i][j][k] == 0) {
                    points[config.schedule[j][2*k]] += 3;
                }
                else if(scenarios[i][j][k] == 1) {
                    points[config.schedule[j][2*k+1]] += 3;
                }
                else if(scenarios[i][j][k] == 2) {
                    points[config.schedule[j][2*k]] += 1;
                    points[config.schedule[j][2*k+1]] += 1;
                }
            }
            vector<int> index = {0,j};
            if(table.find(index) == table.end()) {
                table[index] = {};
            }
            if(table[index].find(points) == table[index].end()) {
                table[index][points] = 0.0;
            }
            table[index][points] += 1 / (float) amount;
        }
    }
    return table;
}

// Calculate the Expected value for each team for each scenario
long double* SimulatedFVCalculation::calculateEV(int*** scenarios, int amount, int start, int end, vector<int> staringScore) const {
    int _end = end;
    if(_end < 0) _end = config.rounds;
    vector<int> zeros(config.numberOfTeams, 0);
    if(staringScore.empty()) staringScore = zeros;
    auto* ev = new long double[config.numberOfTeams];
    for(int i = 0; i < amount; i++) {
        long double* scenarioPrice = config.priceFunction->assignPrice(scenarios[i], start, _end, staringScore);
        for(int j = 0; j < config.numberOfTeams; j++) {
            ev[j] += scenarioPrice[j] / (double) amount;
        }
        delete scenarioPrice;
    }
    return ev;
}