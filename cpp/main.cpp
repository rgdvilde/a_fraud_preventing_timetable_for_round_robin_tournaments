#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>
#include "./helpers/JSON.h"
#include "./calculation/SimulatedFVCalculation.h"
#include "./helpers/Data.h"

using namespace std;
using json = nlohmann::json;

uint32_t seed_val = 0;           // populate somehow
mt19937 rng(seed_val);                   // e.g. keep one global instance (per thread)

std::string getCurrentDateTimeFolderName() {
    // Get current time
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    // Format time as YYYYMMDD_HHMM
    std::ostringstream oss;
    oss << (tm.tm_year + 1900)    // Year
        << (tm.tm_mon + 1)        // Month (1-based)
        << tm.tm_mday << "_"
        << tm.tm_hour
        << tm.tm_min;

    return oss.str();
}


// Function to determine the next available letter based on all values in the map
char getNextAvailableLetter(const std::map<std::string, std::string>& m) {
    std::set<char> usedLetters;

    // Collect all last letters from the existing values
    for (const auto& pair : m) {
        if (!pair.second.empty()) {
            usedLetters.insert(pair.second.back());
        }
    }

    // Find the first unused letter in the range 'A' to 'Z'
    for (char letter = 'A'; letter <= 'Z'; ++letter) {
        if (usedLetters.find(letter) == usedLetters.end()) {
            return letter;
        }
    }

    std::cout << "Warning: No available letters left!" << std::endl;
    return '\0'; // Indicate that no letters are available
}

// Function to add a key with the next available letter
std::string addKeyWithNextLetter(const std::string& c, std::map<std::string, std::string>* m, const std::string& key) {
    char nextLetter = getNextAvailableLetter(*m);
    if (nextLetter == '\0') return ""; // No available letters

    // Assign the new value
    std::string combination = c + nextLetter;
    (*m)[key] = combination;
    return combination;
}

// Function to print term translations in a readable format
void printTermTranslations(const map<string, map<string, string>>& termTranslation) {
    std::cout << "\n=== TERM TRANSLATIONS ===" << std::endl;
    for (const auto& category : termTranslation) {
        std::cout << "\n" << category.first << ":" << std::endl;
        for (const auto& mapping : category.second) {
            std::cout << "  " << mapping.second << " -> " << mapping.first << std::endl;
        }
    }
    std::cout << "========================\n" << std::endl;
}

map<string,map<string,string>> processResults(vector<tuple<Configuration, map<map<string,string>, list<vector<long double>>>>> rrr, const std::string& outputPath) {
    vector<vector<string>> out = {{
                                          "round",
                                          "game",
                                          "numberOfTeams",
                                          "numberOfRounds",
                                          "mirrored",
                                          "homeTeam",
                                          "awayTeam",
                                          "eloDifference",
                                          "homeElo",
                                          "awayElo",
                                          "rankDifference",
                                          "mappingName",
                                          "priceFunctionName",
                                          "PreName",
                                          "TM",
                                          "PR",
                                          "PC",
                                          "NT",
                                          "MR",
                                          "P(s)",
                                          "Eg_max(p)",
                                          "Eg(p|e0)",
                                          "Eg(p|e1)",
                                          "Eg(p|e2)"}};
    map<string,map<string,string>> termTranslation = {};
    for (const auto& outer_tuple : rrr) {
        const Configuration& config = std::get<0>(outer_tuple);
        const auto& inner_map = std::get<1>(outer_tuple);
        for (const auto& [key_map, value_list] : inner_map) {
            for (const auto& vec : value_list) {
                int numberOfTeams = config.numberOfTeams;
                int numberOfRounds = config.rounds;
                
                // Check if elo is set otherwise set to NA
                string homeEloStr, awayEloStr, eloDifferenceStr;
                if (!config.elo.empty() && stoi(key_map.at("homeTeam")) < config.elo.size() && stoi(key_map.at("awayTeam")) < config.elo.size()) {
                    double homeElo = config.elo[stoi(key_map.at("homeTeam"))];
                    double awayElo = config.elo[stoi(key_map.at("awayTeam"))];
                    double eloDifference = abs(homeElo - awayElo);
                    homeEloStr = to_string(homeElo);
                    awayEloStr = to_string(awayElo);
                    eloDifferenceStr = to_string(eloDifference);
                } else {
                    homeEloStr = "NA";
                    awayEloStr = "NA";
                    eloDifferenceStr = "NA";
                }
                
                int rankDifference = abs(stoi(key_map.at("homeTeam")) - stoi(key_map.at("awayTeam")));
                // Generate initial line of data for the 'out' vector
                vector<string> lineBase = {
                        key_map.at("round"),
                        key_map.at("game"),
                        to_string(numberOfTeams),
                        to_string(numberOfRounds),
                        to_string(config.mirrorSchedule),
                        key_map.at("homeTeam"),
                        key_map.at("awayTeam"),
                        eloDifferenceStr,
                        homeEloStr,
                        awayEloStr,
                        to_string(rankDifference),
                        config.fileContent.at("mapping"),
                        config.fileContent.at("priceFunction")
                };
                // Generate the pre-mapping string
                string pre = "";
                if(config.pre.at("exactHighestLast")) {
                    pre += "exactHighestLast";
                }
                if(config.pre.at("exactHighestFirst")) {
                    pre += "exactHighestFirst";
                }
                if(config.pre.at("random")) {
                    pre += "random";
                }
                lineBase.push_back(pre);
                // Generate the mapping string
                {
                        if (termTranslation.find("selection (TM)") == termTranslation.end()) {
                            termTranslation["selection (TM)"] = {};
                        }
                        string mappingKey;
                        if (config.fileContent.at("mapping").find("linear") != std::string::npos) {
                            mappingKey = "linear";
                        } else if (config.fileContent.at("mapping").find("2_groups") != std::string::npos) {
                            mappingKey = "2 groups";
                        } else if (config.fileContent.at("mapping").find("same") != std::string::npos) {
                            mappingKey = "same";
                        } else {
                            mappingKey = config.fileContent.at("mapping");
                        }

                        if(termTranslation["selection (TM)"].find(mappingKey) != termTranslation["selection (TM)"].end()) {
                            lineBase.push_back(termTranslation["selection (TM)"].find(mappingKey)->second);
                        }
                        else {
                            string s2 = addKeyWithNextLetter("", &termTranslation["selection (TM)"], mappingKey);
                            lineBase.push_back(s2);
                        }
                        if(config.pre.at("exactHighestLast")) {
                            if (termTranslation.find("pre (PR)") == termTranslation.end()) {
                                termTranslation["pre (PR)"] = {};
                            }
                            if(termTranslation["pre (PR)"].find("exactHighestLast") != termTranslation["pre (PR)"].end()) {
                                lineBase.push_back(termTranslation["pre (PR)"].find("exactHighestLast")->second);
                            }
                            else {
                                string s2 = addKeyWithNextLetter("", &termTranslation["pre (PR)"], "exactHighestLast");
                                lineBase.push_back(s2);
                            }
                        }
                        else if(config.pre.at("exactHighestFirst")) {
                            if (termTranslation.find("pre (PR)") == termTranslation.end()) {
                                termTranslation["pre (PR)"] = {};
                            }
                            if(termTranslation["pre (PR)"].find("exactHighestFirst") != termTranslation["pre (PR)"].end()) {
                                lineBase.push_back(termTranslation["pre (PR)"].find("exactHighestFirst")->second);
                            }
                            else {
                                string s2 = addKeyWithNextLetter("", &termTranslation["pre (PR)"], "exactHighestFirst");
                                lineBase.push_back(s2);
                            }
                        }
                        else if(config.pre.at("random")) {
                            if (termTranslation.find("pre (PR)") == termTranslation.end()) {
                                termTranslation["pre (PR)"] = {};
                            }
                            if(termTranslation["pre (PR)"].find("random") != termTranslation["pre (PR)"].end()) {
                                lineBase.push_back(termTranslation["pre (PR)"].find("random")->second);
                            }
                            else {
                                string s2 = addKeyWithNextLetter("", &termTranslation["pre (PR)"], "random");
                                lineBase.push_back(s2);
                            }
                        }
                        else {
                            if (termTranslation.find("pre (PR)") == termTranslation.end()) {
                                termTranslation["pre (PR)"] = {};
                            }
                            if(termTranslation["pre (PR)"].find("none") != termTranslation["pre (PR)"].end()) {
                                lineBase.push_back(termTranslation["pre (PR)"].find("none")->second);
                            }
                            else {
                                string s2 = addKeyWithNextLetter("", &termTranslation["pre (PR)"], "none");
                                lineBase.push_back(s2);
                            }
                        }
                        // Generate the price function string
                        if (termTranslation.find("priceFunction (PC)") == termTranslation.end()) {
                            termTranslation["priceFunction (PC)"] = {};
                        }
                        if(termTranslation["priceFunction (PC)"].find(config.fileContent.at("priceFunction")) != termTranslation["priceFunction (PC)"].end()) {
                            lineBase.push_back(termTranslation["priceFunction (PC)"].find(config.fileContent.at("priceFunction"))->second);
                        }
                        else {
                            string s2 = addKeyWithNextLetter("", &termTranslation["priceFunction (PC)"], config.fileContent.at("priceFunction"));
                            lineBase.push_back(s2);
                        }                        
                        // Add numberOfTeams categorical variable (NT)
                        if (termTranslation.find("numberOfTeams (NT)") == termTranslation.end()) {
                            termTranslation["numberOfTeams (NT)"] = {};
                        }
                        string numberOfTeamsStr = to_string(numberOfTeams);
                        if(termTranslation["numberOfTeams (NT)"].find(numberOfTeamsStr) != termTranslation["numberOfTeams (NT)"].end()) {
                            lineBase.push_back(termTranslation["numberOfTeams (NT)"].find(numberOfTeamsStr)->second);
                        }
                        else {
                            string s2 = addKeyWithNextLetter("", &termTranslation["numberOfTeams (NT)"], numberOfTeamsStr);
                            lineBase.push_back(s2);
                        }
                        
                        // Add mirrored categorical variable (MR)
                        if (termTranslation.find("mirrored (MR)") == termTranslation.end()) {
                            termTranslation["mirrored (MR)"] = {};
                        }
                        string mirroredStr = to_string(config.mirrorSchedule);
                        if(termTranslation["mirrored (MR)"].find(mirroredStr) != termTranslation["mirrored (MR)"].end()) {
                            lineBase.push_back(termTranslation["mirrored (MR)"].find(mirroredStr)->second);
                        }
                        else {
                            string s2 = addKeyWithNextLetter("", &termTranslation["mirrored (MR)"], mirroredStr);
                            lineBase.push_back(s2);
                        }
                }

                vector<string> line = lineBase;
                line.push_back(to_string(vec[0]));
                line.push_back(to_string(std::max({vec[1], vec[2], vec[3]})));
                line.push_back(to_string(vec[1]));
                line.push_back(to_string(vec[2]));
                line.push_back(to_string(vec[3]));
                out.push_back(line);
            }
        }
    }
    Data::writeCSV(outputPath, out);
    return termTranslation;
}


vector<string> splitString(const string& str, char delim) {
    vector<string> result;
    stringstream ss(str);
    string item;
    while (getline(ss, item, delim)) {
        result.push_back(trim(item));
    }
    return result;
}


int runMain(const std::string& configPath, const std::string& outputPath, bool benchmarkMode = false) {
        // Read configurations from the provided config file
        std::ifstream f(configPath);
        if (!f.is_open()) {
            std::cerr << "Error: Could not open config file at " << configPath << std::endl;
            return 1;
        }
        json configs_json = json::parse(f);
        vector<Configuration> all_configs;
        
        // Process each configuration from the JSON array
        for (const auto& config_json : configs_json) {
            vector<string> schedule;
            vector<string> teams;
            vector<string> elo;
            vector<string> selection;
            vector<string> naming;
            vector<string> mapping;
            vector<string> price;
            string path;
            
            // Check for each field and provide default values if not found
            if (config_json.contains("schedule")) {
                schedule = config_json["schedule"].get<vector<string>>();
            } else {
                std::cout << "schedule not found" << std::endl;
                schedule = {};
            }
            
            if (config_json.contains("teams")) {
                teams = config_json["teams"].get<vector<string>>();
            } else {
                std::cout << "teams not found" << std::endl;
                teams = {};
            }
            
            if (config_json.contains("elo")) {
                elo = config_json["elo"].get<vector<string>>();
            } else {
                std::cout << "elo not found" << std::endl;
                elo = {};
            }
            
            if (config_json.contains("selection")) {
                selection = config_json["selection"].get<vector<string>>();
            } else {
                std::cout << "selection not found" << std::endl;
                selection = {};
            }
            
            if (config_json.contains("naming")) {
                naming = config_json["naming"].get<vector<string>>();
            } else {
                std::cout << "naming not found" << std::endl;
                naming = {};
            }
            
            if (config_json.contains("mapping")) {
                mapping = config_json["mapping"].get<vector<string>>();
            } else {
                std::cout << "mapping not found" << std::endl;
                mapping = {};
            }
            
            if (config_json.contains("price")) {
                price = config_json["price"].get<vector<string>>();
            } else {
                std::cout << "price not found" << std::endl;
                price = {};
            }
            
            if (config_json.contains("path")) {
                path = config_json["path"].get<string>();
            } else {
                std::cout << "path not found" << std::endl;
                path = "";
            }
            
            int runs = config_json.value("runs", 100); // Get runs from JSON, default to 100 if not present
            int preRuns = config_json.value("preRuns", runs); // Get preRuns from JSON, default to runs if not present
            int postRuns = config_json.value("postRuns", runs); // Get postRuns from JSON, default to runs if not present

            // Convert int values to string vectors for generateConfigurations
            vector<string> runs_str = {to_string(runs)};
            vector<string> preRuns_str = {to_string(preRuns)};
            vector<string> postRuns_str = {to_string(postRuns)};

            vector<Configuration> generated_configs = Configuration::generateConfigurations(
                &rng,
                schedule,
                teams,
                elo,
                mapping,
                selection,
                naming,
                price,
                runs_str,
                preRuns_str,
                postRuns_str,
                path
            );
            all_configs.insert(all_configs.end(), generated_configs.begin(), generated_configs.end());
        }

        vector<tuple<Configuration, map<map<string,string>, list<vector<long double>>>>> rrr = {};
        vector<tuple<Configuration, map<map<string,string>, list<vector<long double>>>>> rrr_average = {};

        using TupleType = std::tuple<Configuration, std::map<std::vector<int>, std::list<std::vector<long double>>>>;
        std::vector<double> iteration_times; // Store per-iteration durations
        
        // Start total program timer
        auto program_start = std::chrono::high_resolution_clock::now();

        std::vector<TupleType> myTupleVector;
        int count = 0;
        for (const Configuration& currentConfig : all_configs) {
            auto start = std::chrono::high_resolution_clock::now();
            cout << count << "/" << all_configs.size() << endl;
            count++;
            vector<string> line = {};
            Configuration thisConfig = Configuration(currentConfig);
            SimulatedFVCalculation calc = SimulatedFVCalculation(thisConfig);

            // Set runs from config
            calc.runs = thisConfig.runs;

            map<map<string,string>, list<vector<long double>>> r = calc.calculate();

            // Iterate over the map
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::nano> iteration_duration = end - start;
            double duration_in_minutes = iteration_duration.count() / (60.0 * 1e9);

            iteration_times.push_back(duration_in_minutes);
            std::cout << "Iteration " << count << " time: " << duration_in_minutes << " minutes" << std::endl;

            rrr.emplace_back(thisConfig, r);
        }
        
        // End total program timer
        auto program_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::nano> total_duration = program_end - program_start;
        double total_time_minutes = total_duration.count() / (60.0 * 1e9);
        
        // If benchmark mode is enabled, write timing CSV
        if (benchmarkMode) {
            std::string timingOutputPath = outputPath.substr(0, outputPath.find_last_of('.')) + "_benchmark.csv";
            std::ofstream timingFile(timingOutputPath);
            
            if (timingFile.is_open()) {
                // Write header
                timingFile << "iteration,time_minutes,preruns,postruns,numberOfTeams,mirror\n";
                
                // Write iteration times
                for (size_t i = 0; i < iteration_times.size(); ++i) {
                    if (i < all_configs.size()) {
                        const Configuration& config = all_configs[i];
                        timingFile << i + 1 << "," << iteration_times[i] << "," 
                                 << config.preRuns << "," << config.postRuns << "," 
                                 << config.numberOfTeams << "," << config.mirrorSchedule << "\n";
                    } else {
                        timingFile << i + 1 << "," << iteration_times[i] << ",NA,NA,NA,NA\n";
                    }
                }
                
                // Write total time
                timingFile << "total," << total_time_minutes << ",NA,NA,NA,NA\n";
                timingFile.close();
                
                std::cout << "Benchmark results written to: " << timingOutputPath << std::endl;
                std::cout << "Total program runtime: " << total_time_minutes << " minutes" << std::endl;
            } else {
                std::cerr << "Warning: Could not open benchmark output file: " << timingOutputPath << std::endl;
            }
        }
        
        auto termTranslation = processResults(rrr, outputPath);
        printTermTranslations(termTranslation);
        return 0;
}


int*** run(Configuration config, int _runs, map<vector<int>, tuple<float, float, float, int>> adaptations, int start, int end) {
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

long double* calculateEV(Configuration config, int*** scenarios, int amount, int start, int end, vector<int> staringScore) {
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

//./thesis -c ./in.json -o ./out.csv
int main(int argc, char* argv[]) {
    // Set default values for configPath and outputPath
    std::string configPath = "./in_default.json";
    std::string outputPath = "./out_default.csv"; // Default output path
    bool benchmarkMode = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-c" && i + 1 < argc) {
            configPath = argv[i + 1];
            i++; // Skip the next argument since we've used it
        } else if (arg == "-o" && i + 1 < argc) {
            outputPath = argv[i + 1];
            i++; // Skip the next argument since we've used it
        } else if (arg == "-b") {
            benchmarkMode = true;
        }
    }
    
    if (configPath.empty()) {
        std::cerr << "Error: No config file specified. Please use -c flag to specify the config file path." << std::endl;
        std::cerr << "Example: " << argv[0] << " -c ./in.json -o ./lineout.csv" << std::endl;
        std::cerr << "Use -b flag to enable benchmark mode (outputs timing CSV)" << std::endl;
        return 1;
    }
    
    return runMain(configPath, outputPath, benchmarkMode);
}
