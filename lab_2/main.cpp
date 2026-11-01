#include <ctime>
#include <iostream>
#include "Arguments.h"
#include "Game.h"
#include "Simulator.h"

int main(int argc, char* argv[]) {
    srand((unsigned)time(0));
    try {
        Arguments args(argc, argv);

        GameMatrix matrix;
        if (args.getMatrixPath() != "") {
            matrix = GameMatrix(args.getMatrixPath());
        }
        StrategyFactory factory(args.getConfigDir());
        Simulator simulator(factory, matrix, args.getSteps());

        if (args.getMode() == "detailed") {
            simulator.runDetailed(args.getNames(), std::cin, std::cout);
        } else if (args.getMode() == "fast") {
            simulator.runFast(args.getNames(), std::cout);
        } else {
            simulator.runTournament(args.getNames(), std::cout);
        }
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Usage: prisoners name1 name2 name3 [more names] [--mode=detailed|fast|tournament] "
                  << "[--steps=N] [--configs=DIR] [--matrix=FILE]" << std::endl;
        std::cerr << "Strategies:";
        std::vector<std::string> all = StrategyFactory::allNames();
        for (size_t i = 0; i < all.size(); i++) {
            std::cerr << " " << all[i];
        }
        std::cerr << std::endl;
        return 1;
    }
    return 0;
}
