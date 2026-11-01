#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <iostream>
#include "Game.h"

// Runs the game in one of the three modes and prints the results
class Simulator {
private:
    StrategyFactory factory;
    GameMatrix matrix;
    int steps;

    void printTotals(const Game& game, const std::vector<std::string>& names, std::ostream& out) const {
        out << "Total points:" << std::endl;
        for (int i = 0; i < 3; i++) {
            out << names[i] << ": " << game.getTotal(i) << std::endl;
        }
        int winner = game.getWinner();
        if (winner == -1) {
            out << "Draw" << std::endl;
        } else {
            out << "Winner: " << names[winner] << std::endl;
        }
    }

    std::vector<std::unique_ptr<Strategy> > createThree(const std::vector<std::string>& names) const {
        std::vector<std::unique_ptr<Strategy> > result;
        for (int i = 0; i < 3; i++) {
            result.push_back(factory.create(names[i]));
        }
        return result;
    }

public:
    Simulator(const StrategyFactory& strategyFactory, const GameMatrix& gameMatrix, int stepCount) {
        factory = strategyFactory;
        matrix = gameMatrix;
        steps = stepCount;
    }

    void runDetailed(const std::vector<std::string>& names, std::istream& in, std::ostream& out) const {
        std::vector<std::unique_ptr<Strategy> > players = createThree(names);
        Game game(players[0].get(), players[1].get(), players[2].get(), matrix);

        for (int round = 1; round <= steps; round++) {
            out << "Press Enter for the next step or type quit: ";
            std::string line;
            if (!std::getline(in, line) || line == "quit") {
                break;
            }
            game.playRound();
            out << "Round " << round << ":" << std::endl;
            for (int i = 0; i < 3; i++) {
                out << "  " << names[i] << ": move " << game.getLastMove(i)
                    << ", round points " << game.getLastScore(i)
                    << ", total " << game.getTotal(i) << std::endl;
            }
        }
        printTotals(game, names, out);
    }

    void runFast(const std::vector<std::string>& names, std::ostream& out) const {
        std::vector<std::unique_ptr<Strategy> > players = createThree(names);
        Game game(players[0].get(), players[1].get(), players[2].get(), matrix);
        game.play(steps);
        printTotals(game, names, out);
    }

    void runTournament(const std::vector<std::string>& names, std::ostream& out) const {
        Tournament tournament(names, factory, matrix, steps);
        tournament.run(out);
    }
};

#endif
