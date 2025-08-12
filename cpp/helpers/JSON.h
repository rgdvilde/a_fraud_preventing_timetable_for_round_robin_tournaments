
#ifndef THESIS_JSON_H
#define THESIS_JSON_H

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <sstream>


// Function to parse a simple JSON string into a map
std::map<std::string, std::string> parseJSON(const std::string& jsonString);

// Function to trim leading and trailing whitespace
std::string trim(const std::string& str);

// Function to remove surrounding quotes from a string
std::string removeQuotes(const std::string& str);

// Function to parse a JSON object (simple key-value pairs)
std::map<std::string, std::string> parseJSONObject(const std::string& jsonString);

// Function to parse a top-level JSON array of objects
std::vector<std::map<std::string, std::string>> parseJSONArray(const std::string& jsonString);

// Function to read a JSON file and return its content as a string
std::string readJSONFile(const std::string& filename);

// Function to process JSON and determine whether it's an object or an array
void processJSON(const std::string& jsonString);

bool isJSONArray(const std::string& jsonString);

bool isJSONObject(const std::string& jsonString);

#endif //THESIS_JSON_H
