#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <stdexcept>
#include <string>
#include <vector>

// Command line arguments:
// name1 name2 name3 ... [--mode=detailed|fast|tournament] [--steps=N] [--configs=DIR] [--matrix=FILE]
class Arguments {
private:
    std::vector<std::string> names;
    std::string mode;
    std::string configDir;
    std::string matrixPath;
    int steps;

    static bool startsWith(const std::string& text, const std::string& prefix) {
        return text.compare(0, prefix.size(), prefix) == 0;
    }

public:
    Arguments(int argc, char* argv[]) {
        steps = 100;
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (startsWith(arg, "--mode=")) {
                mode = arg.substr(7);
            } else if (startsWith(arg, "--steps=")) {
                steps = std::stoi(arg.substr(8));
            } else if (startsWith(arg, "--configs=")) {
                configDir = arg.substr(10);
            } else if (startsWith(arg, "--matrix=")) {
                matrixPath = arg.substr(9);
            } else if (startsWith(arg, "--")) {
                throw std::invalid_argument("Unknown option: " + arg);
            } else {
                names.push_back(arg);
            }
        }

        if (names.size() < 3) {
            throw std::invalid_argument("At least three strategies are needed");
        }
        if (mode == "") {
            if (names.size() == 3) {
                mode = "detailed";
            } else {
                mode = "tournament";
            }
        }
        if (mode != "detailed" && mode != "fast" && mode != "tournament") {
            throw std::invalid_argument("Unknown mode: " + mode);
        }
        if (mode != "tournament" && names.size() != 3) {
            throw std::invalid_argument("This mode needs exactly three strategies");
        }
        if (steps <= 0) {
            throw std::invalid_argument("Steps must be positive");
        }
    }

    const std::vector<std::string>& getNames() const {
        return names;
    }

    std::string getMode() const {
        return mode;
    }

    std::string getConfigDir() const {
        return configDir;
    }

    std::string getMatrixPath() const {
        return matrixPath;
    }

    int getSteps() const {
        return steps;
    }
};

#endif
