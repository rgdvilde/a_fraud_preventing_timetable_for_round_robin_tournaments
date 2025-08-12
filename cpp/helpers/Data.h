
#ifndef THESIS_DATA_H
#define THESIS_DATA_H

#include <string>
#include <map>

using namespace std;

class Data {
public:
    static void writeCSV(const string& filename, const vector<vector<string>>& data);
    static int readCSV(const string& filename, vector<vector<string>>& data);
    static map<vector<int>, tuple<float, float, float, int>> transformAmountToUniform(const string& homeWinsFilename, const string& awayWinsFilename, const string& amountFilename, int teams);
    static tuple<int, int**> loadSchedule(const string& fileName);
    static int* loadMapping(const string& fileName);
    static map<vector<int>, tuple<float, float, float, int>> generateTeams(int amount);
    static int writeTeams(const map<vector<int>, tuple<float, float, float, int>>& teams, const string& fileName);
    static map<vector<int>, tuple<float, float, float, int>> readTeams(const string& fileName);
    static int* readSelection(const string& fileName);
    static vector<int> readMapping(const string& fileName);
    static vector<double> readElo(const string& fileName);
    static map<int, string> readNaming(const string& fileName);
};


#endif //THESIS_DATA_H
