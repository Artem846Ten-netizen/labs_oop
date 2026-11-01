#ifndef GAME_H
#define GAME_H

#include <fstream>
#include <iostream>
#include "Factory.h"
#include "Strategy.h"

// Returns index of the biggest score, or -1 if the biggest score is shared
inline int findWinner(const std::vector<int>& scores) {
    int best = 0;
    for (size_t i = 1; i < scores.size(); i++) {
        if (scores[i] > scores[best]) {
            best = (int)i;
        }
    }
    for (size_t i = 0; i < scores.size(); i++) {
        if ((int)i != best && scores[i] == scores[best]) {
            return -1;
        }
    }
    return best;
}

// Points for every combination of moves.
// Row order: CCC, CCD, CDC, CDD, DCC, DCD, DDC, DDD
class GameMatrix {
private:
    int table[8][3];

    static int rowIndex(char m1, char m2, char m3) {
        int index = 0;
        if (m1 == DEFECT) {
            index += 4;
        }
        if (m2 == DEFECT) {
            index += 2;
        }
        if (m3 == DEFECT) {
            index += 1;
        }
        return index;
    }

public:
    GameMatrix() {
        int defaults[8][3] = {
            {7, 7, 7},
            {3, 3, 9},
            {3, 9, 3},
            {0, 5, 5},
            {9, 3, 3},
            {5, 0, 5},
            {5, 5, 0},
            {1, 1, 1}
        };
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 3; j++) {
                table[i][j] = defaults[i][j];
            }
        }
    }

    // File has 8 lines with 3 numbers in each
    explicit GameMatrix(const std::string& path) {
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file.open(path.c_str());
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 3; j++) {
                file >> table[i][j];
            }
        }
    }

    int getScore(char m1, char m2, char m3, int player) const {
        return table[rowIndex(m1, m2, m3)][player];
    }
};

// One game of three strategies
class Game {
private:
    Strategy* players[3];
    GameMatrix matrix;
    History histories[3];
    int totals[3];
    char lastMoves[3];
    int lastScores[3];
    int rounds;

public:
    Game(Strategy* a, Strategy* b, Strategy* c, const GameMatrix& gameMatrix) {
        players[0] = a;
        players[1] = b;
        players[2] = c;
        matrix = gameMatrix;
        rounds = 0;
        for (int i = 0; i < 3; i++) {
            totals[i] = 0;
            lastMoves[i] = COOPERATE;
            lastScores[i] = 0;
        }
    }

    void playRound() {
        for (int i = 0; i < 3; i++) {
            lastMoves[i] = players[i]->choose(histories[i]);
        }
        for (int i = 0; i < 3; i++) {
            lastScores[i] = matrix.getScore(lastMoves[0], lastMoves[1], lastMoves[2], i);
            totals[i] += lastScores[i];
        }
        histories[0].add(lastMoves[0], lastMoves[1], lastMoves[2]);
        histories[1].add(lastMoves[1], lastMoves[0], lastMoves[2]);
        histories[2].add(lastMoves[2], lastMoves[0], lastMoves[1]);
        rounds++;
    }

    void play(int steps) {
        for (int i = 0; i < steps; i++) {
            playRound();
        }
    }

    int getRounds() const {
        return rounds;
    }

    int getTotal(int player) const {
        return totals[player];
    }

    int getLastScore(int player) const {
        return lastScores[player];
    }

    char getLastMove(int player) const {
        return lastMoves[player];
    }

    int getWinner() const {
        std::vector<int> scores(totals, totals + 3);
        return findWinner(scores);
    }
};

// Plays every triple of strategies (without repeats) and sums the points
class Tournament {
private:
    std::vector<std::string> names;
    std::vector<int> totals;
    StrategyFactory factory;
    GameMatrix matrix;
    int steps;

public:
    Tournament(const std::vector<std::string>& strategyNames, const StrategyFactory& strategyFactory,
               const GameMatrix& gameMatrix, int stepCount) {
        names = strategyNames;
        totals = std::vector<int>(names.size(), 0);
        factory = strategyFactory;
        matrix = gameMatrix;
        steps = stepCount;
    }

    void run(std::ostream& out) {
        int gameNumber = 0;
        for (size_t i = 0; i < names.size(); i++) {
            for (size_t j = i + 1; j < names.size(); j++) {
                for (size_t k = j + 1; k < names.size(); k++) {
                    std::unique_ptr<Strategy> a = factory.create(names[i]);
                    std::unique_ptr<Strategy> b = factory.create(names[j]);
                    std::unique_ptr<Strategy> c = factory.create(names[k]);
                    Game game(a.get(), b.get(), c.get(), matrix);
                    game.play(steps);

                    totals[i] += game.getTotal(0);
                    totals[j] += game.getTotal(1);
                    totals[k] += game.getTotal(2);

                    gameNumber++;
                    out << "Game " << gameNumber << ": "
                        << names[i] << " = " << game.getTotal(0) << ", "
                        << names[j] << " = " << game.getTotal(1) << ", "
                        << names[k] << " = " << game.getTotal(2) << std::endl;
                }
            }
        }

        out << std::endl << "Final protocol:" << std::endl;
        for (size_t i = 0; i < names.size(); i++) {
            out << names[i] << ": " << totals[i] << std::endl;
        }
        int winner = getWinner();
        if (winner == -1) {
            out << "Draw" << std::endl;
        } else {
            out << "Winner: " << names[winner] << std::endl;
        }
    }

    int getTotal(int index) const {
        return totals[index];
    }

    int getWinner() const {
        return findWinner(totals);
    }
};

#endif
