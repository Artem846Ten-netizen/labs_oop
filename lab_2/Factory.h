#ifndef FACTORY_H
#define FACTORY_H

#include <fstream>
#include <sstream>
#include <stdexcept>
#include "Strategy.h"

// Reads config files. Streams throw exceptions on any error.
class ConfigReader {
public:
    static int readInt(const std::string& path) {
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file.open(path.c_str());
        int value;
        file >> value;
        return value;
    }

    static std::vector<std::string> readWords(const std::string& path) {
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file.open(path.c_str());
        std::string line;
        std::getline(file, line);

        std::vector<std::string> words;
        std::istringstream stream(line);
        std::string word;
        while (stream >> word) {
            words.push_back(word);
        }
        return words;
    }
};

// Object factory: creates a strategy by its name
class StrategyFactory {
private:
    std::string configDir;

    std::string configPath(const std::string& name) const {
        return configDir + "/" + name + ".txt";
    }

public:
    StrategyFactory(const std::string& dir = "") {
        configDir = dir;
    }

    std::unique_ptr<Strategy> create(const std::string& name) const {
        if (name == "AlwaysCooperate") {
            return std::unique_ptr<Strategy>(new AlwaysCooperate());
        }
        if (name == "AlwaysDefect") {
            return std::unique_ptr<Strategy>(new AlwaysDefect());
        }
        if (name == "Random") {
            return std::unique_ptr<Strategy>(new RandomChoice());
        }
        if (name == "Alternating") {
            return std::unique_ptr<Strategy>(new Alternating());
        }
        if (name == "TitForTat") {
            return std::unique_ptr<Strategy>(new TitForTat());
        }
        if (name == "Grudger") {
            return std::unique_ptr<Strategy>(new Grudger());
        }
        if (name == "Majority") {
            return std::unique_ptr<Strategy>(new Majority());
        }
        if (name == "Punisher") {
            int rounds = 2;
            if (configDir != "") {
                rounds = ConfigReader::readInt(configPath(name));
            }
            return std::unique_ptr<Strategy>(new Punisher(rounds));
        }
        if (name == "Probabilistic") {
            int percent = 50;
            if (configDir != "") {
                percent = ConfigReader::readInt(configPath(name));
            }
            return std::unique_ptr<Strategy>(new Probabilistic(percent));
        }
        if (name == "Voting") {
            std::vector<std::string> memberNames;
            if (configDir != "") {
                memberNames = ConfigReader::readWords(configPath(name));
            } else {
                memberNames.push_back("TitForTat");
                memberNames.push_back("Grudger");
                memberNames.push_back("AlwaysDefect");
            }
            std::unique_ptr<Voting> voting(new Voting());
            for (size_t i = 0; i < memberNames.size(); i++) {
                if (memberNames[i] == "Voting") {
                    throw std::invalid_argument("Voting can not contain itself");
                }
                voting->add(create(memberNames[i]));
            }
            return std::unique_ptr<Strategy>(voting.release());
        }
        throw std::invalid_argument("Unknown strategy: " + name);
    }

    static std::vector<std::string> allNames() {
        std::vector<std::string> names;
        names.push_back("AlwaysCooperate");
        names.push_back("AlwaysDefect");
        names.push_back("Random");
        names.push_back("Alternating");
        names.push_back("TitForTat");
        names.push_back("Grudger");
        names.push_back("Majority");
        names.push_back("Punisher");
        names.push_back("Probabilistic");
        names.push_back("Voting");
        return names;
    }
};

#endif
