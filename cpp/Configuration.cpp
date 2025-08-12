
#include "Configuration.h"
#include "./helpers/Json.h"
#include <string>
#include "./helpers/Data.h"
#include "./TriplePredictor.h"
#include "./ELOPredictor.h"
#include "./priceFunctions/WinnerTakesAllPriceFunction.h"
#include "./priceFunctions/LinearPriceFunction.h"
#include "./priceFunctions/TopThreePriceFunction.h"
#include "./priceFunctions/InverseExponentialPriceFunction.h"
#include "./priceFunctions/EqualPriceFunction.h"
#include "./priceFunctions/DropPriceFunction.h"
#include "./optimization/ExactHighestLastMappingOptimization.h"
#include "./optimization/ExactHigestFirstMappingOptimization.h"
#include "./optimization/RandomizeMapping.h"
#include <utility>

Configuration::Configuration(){
    predictor = nullptr;
    priceFunction = nullptr;
}

Configuration::Configuration(mt19937* _rng, int** _schedule,map<vector<int>, tuple<float, float, float, int>> _teams){
    rng = _rng;
    schedule = _schedule;
    teams = std::move(_teams);
}
namespace fs = std::filesystem;
std::string normalizePath(const std::string& path) {
    fs::path p(path);
    fs::path resolvedPath = fs::canonical(p);

    return resolvedPath.string();
}

string generateHexID(mt19937* _rng) {
    std::stringstream ss;
    std::random_device rd;
    std::uniform_int_distribution<int> dis(0, 255);

    int random = dis(*_rng);
    for (int i = 0; i < 3; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << random;
    }

    return ss.str();
}

Configuration::Configuration(mt19937* _rng, map<string, string> jsonMap, string _basePath){
    rng = _rng;
    basePath = _basePath;
        roundSwitchingOptimization = false;
        this->id = generateHexID(_rng);
        if(jsonMap.find("mapping") != jsonMap.end()) {
            mapping = Data::readMapping(normalizePath(basePath + jsonMap["mapping"]));
            this->fileContent["mapping"] = jsonMap["mapping"];
        }
        if(jsonMap.find("teams") != jsonMap.end()) {
            this->fileContent["teams"] = jsonMap["teams"];
            if(!mapping.empty()) {
                map<vector<int>, tuple<float, float, float, int>> tempTeams = Data::readTeams(basePath + jsonMap["teams"]);
                for (int i = 0; i < mapping.size(); ++i) {
                    for (int j = 0; j < mapping.size(); ++j) {
                        teams[{
                                i,
                                j
                        }] = tempTeams[{
                                mapping[i],
                                mapping[j]
                        }];
                    }
                }
                numberOfTeams = (int)(mapping.size());
                predictor = new TriplePredictor(teams, 0);
            }
            else {
                teams = Data::readTeams(basePath + jsonMap["teams"]);
                numberOfTeams = (int)(pow(teams.size(),0.5)+0.05);
                predictor = new TriplePredictor(teams, 0);
            }
        }
        if(jsonMap.find("elo") != jsonMap.end()) {
            this->fileContent["elo"] = jsonMap["elo"];
            if(!mapping.empty()) {
                numberOfTeams = (int)(mapping.size());
                vector<double> tempElo = Data::readElo(basePath + jsonMap["elo"]);
                for (int i = 0; i < mapping.size(); ++i) {
                    elo.push_back(tempElo[mapping[i]]);
                }
                ELOPredictor* _t = new ELOPredictor(elo);
                predictor = _t;
                teams = _t->TransformToTeams();
            }
            else {
                elo = Data::readElo(basePath + jsonMap["elo"]);
                ELOPredictor* _t = new ELOPredictor(elo);
                predictor = _t;
                teams = _t->TransformToTeams();
                numberOfTeams = (int)(elo.size());
            }
        }
        if(jsonMap.find("selection") != jsonMap.end()) {
            this->fileContent["selection"] = jsonMap["selection"];
            string part1, part2;
            stringstream ss(jsonMap["selection"]);
            if (std::getline(ss, part1, ';') && std::getline(ss, part2)) {
                selection = Data::readSelection(basePath + part2);
                if(part1 == "exactHighestLast") {
                    pre["exactHighestLast"] = true;
                }
                if(part1 == "exactHighestFirst") {
                    pre["exactHighestFirst"] = true;
                }
                if(part1 == "random") {
                    pre["random"] = true;
                }
            } else {
                selection = Data::readSelection(basePath + jsonMap["selection"]);
            }
        }
        if(jsonMap.find("naming") != jsonMap.end()) {
            this->fileContent["naming"] = jsonMap["naming"];
            naming = Data::readNaming(basePath + jsonMap["naming"]);
        }
        mirrorSchedule = (jsonMap["mirrorSchedule"] == "true");
        if(!mirrorSchedule) {
            tuple<int, int**> loadedSchedule = Data::loadSchedule(basePath + jsonMap["schedule"]);
            rounds = get<0>(loadedSchedule);
            int** _schedule = get<1>(loadedSchedule);

            schedule = new int*[rounds];
            for(int i = 0; i < rounds; i++) {
                schedule[i] = new int[numberOfTeams];
                for(int j = 0; j < numberOfTeams; j++) {
                    schedule[i][j] = selection[_schedule[i][j]];
                }
            }
        }
        else {
            tuple<int, int**> loadedSchedule = Data::loadSchedule(basePath + jsonMap["schedule"]);
            this->fileContent["schedule"] = jsonMap["schedule"];
            rounds = 2*get<0>(loadedSchedule);
            int** singleSchedule = get<1>(loadedSchedule);
            schedule = new int*[rounds];
            for(int i = 0; i < rounds; i++) {
                schedule[i] = new int[numberOfTeams];
                for(int j = 0; j < numberOfTeams; j++) {
                    if(i < numberOfTeams - 1) {
                        schedule[i][j] = singleSchedule[selection[i]][selection[j]];
                    }
                    else {
                        if(j % 2 == 0) {
                            schedule[i][j+1] = singleSchedule[selection[i-numberOfTeams+1]][selection[j]];
                        }
                        else {
                            schedule[i][j-1] = singleSchedule[selection[i-numberOfTeams+1]][selection[j]];
                        }
                    }
                }
            }
        }
        if(jsonMap.find("priceFunction") != jsonMap.end()) {
            this->fileContent["priceFunction"] = jsonMap["priceFunction"];
            if(jsonMap["priceFunction"] == "linear") {
                priceFunction = new LinearPriceFunction(schedule, numberOfTeams);
            }
            if(jsonMap["priceFunction"] == "winnerTakesAll") {
                priceFunction = new WinnerTakesAllPriceFunction(schedule, numberOfTeams);
            }
            if(jsonMap["priceFunction"] == "topThree") {
                priceFunction = new TopThreePriceFunction(schedule, numberOfTeams);
            }
            if(jsonMap["priceFunction"] == "inverseExponential") {
                priceFunction = new InverseExponentialPriceFunction(schedule, numberOfTeams);
            }
            if(jsonMap["priceFunction"] == "equal") {
                priceFunction = new EqualPriceFunction(schedule, numberOfTeams);
            }
            if(jsonMap["priceFunction"] == "drop") {
                priceFunction = new DropPriceFunction(schedule, numberOfTeams);
            }
        }
        else {
            priceFunction = new WinnerTakesAllPriceFunction(schedule, numberOfTeams);
        }
        
        if(jsonMap.find("runs") != jsonMap.end()) {
            this->runs = stoi(jsonMap["runs"]);
            this->fileContent["runs"] = jsonMap["runs"];
        }
        
        if(jsonMap.find("preRuns") != jsonMap.end()) {
            this->preRuns = stoi(jsonMap["preRuns"]);
            this->fileContent["preRuns"] = jsonMap["preRuns"];
        } else {
            this->preRuns = this->runs;
        }
        if(jsonMap.find("postRuns") != jsonMap.end()) {
            this->postRuns = stoi(jsonMap["postRuns"]);
            this->fileContent["postRuns"] = jsonMap["postRuns"];
        } else {
            this->postRuns = this->runs;
        }
}


Configuration::Configuration(mt19937* _rng, const string& jsonString, string _basePath) :
        Configuration(_rng, parseJSONObject(jsonString),_basePath){
}


Configuration Configuration::loadConfiguration(mt19937* _rng, const std::string &fileName) {
    string jsonString = readJSONFile(fileName);
    size_t pos = fileName.find_last_of('/');
    string basePath = fileName.substr(0, pos+1);
    Configuration _t = Configuration(_rng, jsonString, basePath);
    if(_t.pre["exactHighestLast"]) {
        ExactHighestLastMappingOptimization opt = ExactHighestLastMappingOptimization();
        _t = opt.optimize(_t);
    }
    if(_t.pre["exactHighestFirst"]) {
        ExactHigestFirstMappingOptimization opt = ExactHigestFirstMappingOptimization();
        _t = opt.optimize(_t);
    }
    return _t;
}

vector<Configuration> Configuration::loadConfigurations(mt19937* _rng, const std::string &fileName) {
    vector<Configuration> configVector = {};
    size_t pos = fileName.find_last_of('/');
    string basePath = fileName.substr(0, pos+1);
    string jsonString = readJSONFile(fileName);
    if(isJSONArray(jsonString)) {
        vector<map<string, string>> jsonArray = parseJSONArray(jsonString);
        for (const auto& jsonObject : jsonArray) {
            Configuration _t = Configuration(_rng, jsonObject, basePath);
            if(_t.pre["exactHighestLast"]) {
                ExactHighestLastMappingOptimization opt = ExactHighestLastMappingOptimization();
                _t = opt.optimize(_t);
            }
            if(_t.pre["exactHighestFirst"]) {
                ExactHigestFirstMappingOptimization opt = ExactHigestFirstMappingOptimization();
                _t = opt.optimize(_t);
            }
            if(_t.pre["random"]) {
                int originalRuns = _t.runs;
                _t.preRuns = 1;
                for (int i = 0; i < originalRuns; ++i) {
                    Configuration randomizedConfig = _t;
                    RandomizeMapping randomizer;
                    randomizedConfig = randomizer.optimize(randomizedConfig);
                    configVector.push_back(randomizedConfig);
                }
            }
            else {
                configVector.push_back(_t);
            }
        }
    }
    else if(isJSONObject(jsonString)) {
        map<std::string, std::string> jsonObject = parseJSONObject(jsonString);
        Configuration _t = Configuration(_rng, jsonObject, basePath);
        if(_t.pre["exactHighestLast"]) {
            ExactHighestLastMappingOptimization opt = ExactHighestLastMappingOptimization();
            _t = opt.optimize(_t);
        }
        if(_t.pre["exactHighestFirst"]) {
            ExactHigestFirstMappingOptimization opt = ExactHigestFirstMappingOptimization();
            _t = opt.optimize(_t);
        }
        if(_t.pre["random"]) {
            int originalRuns = _t.runs;
            _t.preRuns = 1;
            for (int i = 0; i < originalRuns; ++i) {
                Configuration randomizedConfig = _t;
                RandomizeMapping randomizer;
                randomizedConfig = randomizer.optimize(randomizedConfig);
                configVector.push_back(randomizedConfig);
            }
        }
        else {
            configVector.push_back(_t);
        }
    }
    return configVector;
}

std::string generateJsonString(
        const std::vector<std::string>& schedule,
        const std::vector<std::string>& teams,
        const std::vector<std::string>& elo,
        const std::vector<std::string>& selection,
        const std::vector<std::string>& mapping,
        const std::vector<std::string>& naming,
        const std::vector<std::string>& priceFunction,
        const std::vector<std::string>& runs,
        const std::vector<std::string>& preRuns,
        const std::vector<std::string>& postRuns)
{
    std::ostringstream oss;
    oss << "[";

    bool first = true;
            for (const auto& _s : schedule) {
        size_t semicolon_pos = _s.find(';');
        string s = _s;
        bool mirrorSchedule = false;
        if (semicolon_pos != std::string::npos) {
            s = _s.substr(semicolon_pos + 1);       // Part after semicolon
            mirrorSchedule = true;
        }
        
        // Handle teams and elo separately
        std::vector<std::string> dataSources;
        dataSources.insert(dataSources.end(), teams.begin(), teams.end());
        dataSources.insert(dataSources.end(), elo.begin(), elo.end());
        
        cout << "Data sources: " << dataSources.size() << endl;

        for (const auto& dataSource : dataSources) {
                    for (const auto& m : selection) {
            for(const auto& o : mapping) {
                        for (const auto& p : priceFunction) {
                            for (const auto& r : runs) {
                                for (const auto& pr : preRuns) {
                                    for (const auto& por : postRuns) {
                            if (!first) oss << ",";
                            first = false;

                            oss << "{" << std::endl
                                << "\"schedule\":\"" << s << "\"," << std::endl;

                            // Dynamically decide the key based on value
                            if (dataSource.find("teams") != std::string::npos) {
                                oss << "\"teams\":\"" << dataSource << "\"," << std::endl;
                            } else if (dataSource.find("elo") != std::string::npos) {
                                oss << "\"elo\":\"" << dataSource << "\"," << std::endl;
                            } else {
                                oss << "\"teams\":\"" << dataSource << "\"," << std::endl;  // Default case
                            }

                            if (mirrorSchedule) {
                                oss << "\"mirrorSchedule\":\"" << "true" << "\"," << std::endl;
                            }

                            oss << "\"selection\":\"" << m << "\"," << std::endl
                                << "\"mapping\":\"" << o << "\"," << std::endl
                                << "\"priceFunction\":\"" << p << "\"," << std::endl
                                << "\"runs\":\"" << r << "\"," << std::endl
                                << "\"preRuns\":\"" << pr << "\"," << std::endl
                                << "\"postRuns\":\"" << por << "\"" << std::endl
                                << "}";
                                    }
                                }
                            }
                        }
                }
            }
        }
    }

    oss << "]";
    return oss.str();
}

vector<Configuration> Configuration::generateConfigurations(mt19937* _rng, vector<string> schedule, vector<string> teams, vector<string> elo, vector<string> selection, vector<string> mapping, vector<string> naming, vector<string> priceFunction, vector<string> runs, vector<string> preRuns, vector<string> postRuns, string _basePath) {
    string jsonString = generateJsonString(schedule, teams, elo, selection, mapping, naming, priceFunction, runs, preRuns, postRuns);

    vector<map<string, string>> jsonArray = parseJSONArray(jsonString);
    vector<Configuration> configVector = {};

    int count = 0;
    for (const auto& jsonObject : jsonArray) {
        count++;
        Configuration _t = Configuration(_rng, jsonObject, _basePath);
        for (const auto& pair : jsonObject) {
        }
        if(_t.pre["exactHighestLast"]) {
            ExactHighestLastMappingOptimization opt = ExactHighestLastMappingOptimization();
            _t = opt.optimize(_t);
        }
        if(_t.pre["exactHighestFirst"]) {
            ExactHigestFirstMappingOptimization opt = ExactHigestFirstMappingOptimization();
            _t = opt.optimize(_t);
        }
        if(_t.pre["random"]) {
            int originalRuns = _t.runs;
            _t.preRuns = 1;
            for (int i = 0; i < originalRuns; ++i) {
                Configuration randomizedConfig = _t;
                RandomizeMapping randomizer;
                randomizedConfig = randomizer.optimize(randomizedConfig);
                configVector.push_back(randomizedConfig);
            }
        }
        else {
            configVector.push_back(_t);
        }
    }
    return configVector;
}

Configuration::Configuration(const Configuration& other) {
    this->id = other.id;
    this->predictor = other.predictor->clone();
    this->priceFunction = other.priceFunction->clone();
    this->elo = other.elo;
    this->teams = other.teams;
    this->schedule = other.schedule;
    this->numberOfTeams = other.numberOfTeams;
    this->rng = other.rng;
    this->mapping = other.mapping;
    this->name = other.name;
    this->mirrorSchedule = other.mirrorSchedule;
    this->rounds = other.rounds;
    this->fileContent = other.fileContent;
    this->roundSwitchingOptimization = other.roundSwitchingOptimization;
    this->pre = other.pre;
    this->runs = other.runs;
    this->preRuns = other.preRuns;
    this->postRuns = other.postRuns;
}

Configuration& Configuration::operator=(const Configuration& other) {
    if (this == &other)
        return *this;
    delete this->predictor;
    this->id = other.id;

    this->predictor = other.predictor->clone();
    this->priceFunction = other.priceFunction->clone();
    this->elo = other.elo;
    this->teams = other.teams;
    this->schedule = other.schedule;
    this->numberOfTeams = other.numberOfTeams;
    this->rng = other.rng;
    this->mapping = other.mapping;
    this->name = other.name;
    this->mirrorSchedule = other.mirrorSchedule;
    this->rounds = other.rounds;
    this->fileContent = other.fileContent;
    this->roundSwitchingOptimization = other.roundSwitchingOptimization;
    this->pre = other.pre;
    this->runs = other.runs;
    this->preRuns = other.preRuns;
    this->postRuns = other.postRuns;
    return *this;
}

Configuration::~Configuration() {
    delete predictor;
    predictor = nullptr;
    delete priceFunction;
    priceFunction = nullptr;
}