
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include "Data.h"

void Data::writeCSV(const string& filename, const vector<vector<string>>& data) {
    ofstream file(filename);
    if (!file.is_open()) {
        return;
    }
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i != row.size() - 1) {
                file << ",";
            }
        }
        file << endl;
    }
    file.close();
}

int Data::readCSV(const string& filename, vector<vector<string>>& data) {
    ifstream file(filename);
    if (!file.is_open()) {
        return 1;
    }
    string line;
    while (getline(file, line, '\n')) {
        vector<string> dataline = {};
        istringstream iss(line);
        string token;
        while (getline(iss, token, ',')) {
            token.erase(remove(token.begin(),token.end(),' '),token.end());
            token.erase(remove(token.begin(),token.end(),'\n'),token.end());
            token.erase(remove(token.begin(),token.end(),'\r'),token.end());
            dataline.push_back(token);
        }
        data.push_back(dataline);
    }
    file.close();
    return 0;
}

map<vector<int>, tuple<float, float, float, int>> Data::transformAmountToUniform(const string& homeWinsFilename, const string& awayWinsFilename, const string& amountFilename, int teams) {
    vector<vector<string>> homeWins = {};
    vector<vector<string>> awayWins = {};
    vector<vector<string>> amount = {};
    Data::readCSV(homeWinsFilename, homeWins);
    Data::readCSV(awayWinsFilename, awayWins);
    Data::readCSV(amountFilename, amount);
    map<vector<int>, tuple<float, float, float, int>> information;
    for(int i = 1; i < teams+1; i++) {
        for(int j = 1; j < teams+1; j++) {
            vector<int> index = {
                    i-1, j-1
            };
            int indexAmount = int(stof(amount[i][j])+0.05);
            int indexHomeWins = int(stof(homeWins[i][j])+0.05);
            int indexHomeLosses = int(stof(awayWins[j][i])+0.05);
            if(indexAmount > 0) {
                tuple<float, float, float, int> content = {
                        ((float) indexHomeWins / (float) indexAmount),
                        ((float) indexHomeLosses / (float) indexAmount),
                        ((float) (indexAmount - indexHomeWins - indexHomeLosses) / (float) indexAmount),
                        indexAmount,
                };
                information[index] = content;
            }
            else {
                tuple<float, float, float, int> content = {
                        0.0f,0.0f,0.0f,0
                };
                information[index] = content;
            }

        }
    }
    return information;
}

tuple<int, int**> Data::loadSchedule(const string& fileName) {
    vector<vector<string>> _t;
    Data::readCSV(fileName, _t);
    int** schedule = new int*[_t.size()];
    int rounds = _t.size();
    for(int i = 0; i < _t.size(); i++) {
        schedule[i] = new int[_t[0].size()];
        for(int j = 0; j < _t[0].size(); j++) {
            schedule[i][j] = stoi(_t[i][j]);
        }
    }
    return make_tuple(rounds, schedule);
}

int* Data::loadMapping(const string& fileName) {
    vector<vector<string>> _t;
    Data::readCSV(fileName, _t);
    int* mapping = new int[_t.size()];
    for(int i = 0; i < _t.size(); i++) {
        mapping[i] = stoi(_t[i][1]);
    }
    return mapping;
}

map<vector<int>, tuple<float, float, float, int>> Data::generateTeams(int amount) {
    map<vector<int>, tuple<float, float, float, int>> teams;
    for(int i = 0; i < amount; i++) {
        for(int j = 0; j < amount; j++) {
            if(i==j) {
                vector<int> index = {i,j};
                tuple<float, float, float, int> content = {
                        0.0f,0.0f,0.0f,0
                };
                teams[index] = content;
            }
            else {
                vector<int> index = {i,j};
                tuple<float, float, float, int> content = {
                        0.8*(float)(1+i)/(float)(i+j+2),
                        0.8*(float)(1+j)/(float)(i+j+2),
                        0.2f,
                        0
                };
                teams[index] = content;
            }

        }
    }
    return teams;
}

int Data::writeTeams(const map<vector<int>, tuple<float, float, float, int>>& teams, const string& fileName) {
    vector<vector<string>> lines = {};
    for (const auto& pair : teams) {
        vector<string> line = {};
        line.push_back(to_string(pair.first[0]));
        line.push_back(to_string(pair.first[1]));
        line.push_back(to_string(get<0>(pair.second)));
        line.push_back(to_string(get<1>(pair.second)));
        line.push_back(to_string(get<2>(pair.second)));
        line.push_back(to_string(get<3>(pair.second)));
        lines.push_back(line);
    }
    Data::writeCSV(fileName, lines);
    return 0;
}

map<vector<int>, tuple<float, float, float, int>> Data::readTeams(const string& fileName) {
    vector<vector<string>> data = {};
    map<vector<int>, tuple<float, float, float, int>> teams = {};
    Data::readCSV(fileName, data);
    for (int i = 0; i < data.size(); ++i) {
        vector<string> line = data[i];
        vector<int> index = {stoi(line[0]),stoi(line[1])};
        teams[index] = {
                stof(line[2]),
                stof(line[3]),
                stof(line[4]),
                stoi(line[5]),
        };
    }
    return teams;
}

int* Data::readSelection(const string& fileName) {
    vector<vector<string>> data = {};
    Data::readCSV(fileName, data);
    int* selection = new int[data.size()];
    for (int i = 0; i < data.size(); ++i) {
        selection[stoi(data[i][0])] = stoi(data[i][1]);
    }
    return selection;
}

vector<int> Data::readMapping(const string& fileName) {
    vector<vector<string>> data = {};
    Data::readCSV(fileName, data);
    vector<int> mapping(data.size());
    for (int i = 0; i < data.size(); ++i) {
        mapping[stoi(data[i][0])] = stoi(data[i][1]);
    }
    return mapping;
}

map<int, string> Data::readNaming(const string& fileName) {
    vector<vector<string>> data = {};
    map<int, string> names = {};
    Data::readCSV(fileName, data);
    for (int i = 0; i < data.size(); ++i) {
        names[stoi(data[i][0])] = data[i][1];
    }
    return names;
}

vector<double> Data::readElo(const string& fileName) {
    vector<vector<string>> data = {};
    Data::readCSV(fileName, data);
    vector<double> elo(data.size());
    for (int i = 0; i < data.size(); ++i) {
        string _i = data[i][0];
        string _elo = data[i][1];
        elo[stoi(_i)] = stod(_elo);
    }
    return elo;
}
