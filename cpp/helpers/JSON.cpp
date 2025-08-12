#include "JSON.h"

std::map<std::string, std::string> parseJSON(const std::string& jsonString) {
    std::map<std::string, std::string> jsonMap;
    std::string key, value;
    std::stringstream ss(jsonString);
    std::string line;

    while (std::getline(ss, line, ',')) {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            key = trim(line.substr(0, colonPos));
            value = trim(line.substr(colonPos + 1));

            // Remove quotes from key and value if they exist
            key = removeQuotes(key);
            value = removeQuotes(value);

            // Insert the key-value pair into the map
            jsonMap[key] = value;
        }
    }
    return jsonMap;
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::string removeQuotes(const std::string& str) {
    if (!str.empty() && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

std::map<std::string, std::string> parseJSONObject(const std::string& jsonString) {
    std::map<std::string, std::string> jsonMap;

    std::string temp = jsonString;

    if (!temp.empty() && temp.front() == '{' && temp.back() == '}') {
        temp = temp.substr(1, temp.size() - 2);
    }

    std::stringstream ss(temp);
    std::string line;

    while (std::getline(ss, line, ',')) {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = trim(line.substr(0, colonPos));
            std::string value = trim(line.substr(colonPos + 1));

            key = removeQuotes(key);
            value = removeQuotes(value);

            jsonMap[key] = value;
        }
    }

    return jsonMap;
}

std::vector<std::map<std::string, std::string>> parseJSONArray(const std::string& jsonString) {
    std::vector<std::map<std::string, std::string>> jsonArray;
    std::string temp = jsonString;

    if (!temp.empty() && temp.front() == '[' && temp.back() == ']') {
        temp = temp.substr(1, temp.size() - 2);
    }

    size_t start = 0, end;
    while ((end = temp.find("},{", start)) != std::string::npos) {
        std::string objectStr = temp.substr(start, end - start + 1);
        jsonArray.push_back(parseJSONObject(objectStr));
        start = end + 2;
    }

    if (start < temp.size()) {
        std::string objectStr = temp.substr(start);
        jsonArray.push_back(parseJSONObject(objectStr));
    }

    return jsonArray;
}

std::string readJSONFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string jsonString = buffer.str();
    jsonString = trim(jsonString);

    return jsonString;
}

void processJSON(const std::string& jsonString) {
    if (jsonString.empty()) {
        std::cerr << "Error: JSON string is empty!" << std::endl;
        return;
    }

    if (jsonString.front() == '{' && jsonString.back() == '}') {
        std::map<std::string, std::string> jsonObject = parseJSONObject(jsonString);
    } else if (jsonString.front() == '[' && jsonString.back() == ']') {
        std::vector<std::map<std::string, std::string>> jsonArray = parseJSONArray(jsonString);
    } else {
        std::cerr << "Invalid JSON format!" << std::endl;
    }
}

bool isJSONArray(const std::string& jsonString) {
    std::string trimmed = trim(jsonString);
    return (!trimmed.empty() && trimmed.front() == '[' && trimmed.back() == ']');
}

bool isJSONObject(const std::string& jsonString) {
    std::string trimmed = trim(jsonString);
    return (!trimmed.empty() && trimmed.front() == '{' && trimmed.back() == '}');
}