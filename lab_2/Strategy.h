#ifndef STRATEGY_H
#define STRATEGY_H

#include <cstdlib>
#include <memory>
#include <string>
#include <utility>
#include <vector>

const char COOPERATE = 'C';
const char DEFECT = 'D';

// History of one prisoner: own moves and moves of two opponents
class History {
private:
    std::vector<char> own;
    std::vector<char> opp1;
    std::vector<char> opp2;

public:
    void add(char myMove, char move1, char move2) {
        own.push_back(myMove);
        opp1.push_back(move1);
        opp2.push_back(move2);
    }

    int size() const {
        return (int)own.size();
    }

    bool anyOpponentDefected(int round) const {
        return opp1[round] == DEFECT || opp2[round] == DEFECT;
    }

    int countOpponentDefections() const {
        int result = 0;
        for (int i = 0; i < size(); i++) {
            if (opp1[i] == DEFECT) {
                result++;
            }
            if (opp2[i] == DEFECT) {
                result++;
            }
        }
        return result;
    }

    int countOpponentCooperations() const {
        return size() * 2 - countOpponentDefections();
    }
};

// Common interface of all strategies
class Strategy {
public:
    virtual ~Strategy() {}
    virtual char choose(const History& history) const = 0;
    virtual std::string name() const = 0;
};

// ---------- trivial strategies ----------

class AlwaysCooperate : public Strategy {
public:
    char choose(const History&) const {
        return COOPERATE;
    }
    std::string name() const {
        return "AlwaysCooperate";
    }
};

class AlwaysDefect : public Strategy {
public:
    char choose(const History&) const {
        return DEFECT;
    }
    std::string name() const {
        return "AlwaysDefect";
    }
};

class RandomChoice : public Strategy {
public:
    char choose(const History&) const {
        if (rand() % 2 == 0) {
            return COOPERATE;
        }
        return DEFECT;
    }
    std::string name() const {
        return "Random";
    }
};

class Alternating : public Strategy {
public:
    char choose(const History& history) const {
        if (history.size() % 2 == 0) {
            return COOPERATE;
        }
        return DEFECT;
    }
    std::string name() const {
        return "Alternating";
    }
};

class TitForTat : public Strategy {
public:
    char choose(const History& history) const {
        if (history.size() == 0) {
            return COOPERATE;
        }
        if (history.anyOpponentDefected(history.size() - 1)) {
            return DEFECT;
        }
        return COOPERATE;
    }
    std::string name() const {
        return "TitForTat";
    }
};

// ---------- non-trivial strategies ----------

// Cooperates until somebody defects, then defects forever
class Grudger : public Strategy {
public:
    char choose(const History& history) const {
        if (history.countOpponentDefections() > 0) {
            return DEFECT;
        }
        return COOPERATE;
    }
    std::string name() const {
        return "Grudger";
    }
};

// Defects for punishRounds rounds after any defection of opponents
class Punisher : public Strategy {
private:
    int punishRounds;

public:
    Punisher(int rounds) {
        punishRounds = rounds;
    }

    char choose(const History& history) const {
        int start = history.size() - punishRounds;
        if (start < 0) {
            start = 0;
        }
        for (int i = start; i < history.size(); i++) {
            if (history.anyOpponentDefected(i)) {
                return DEFECT;
            }
        }
        return COOPERATE;
    }
    std::string name() const {
        return "Punisher";
    }
};

// Does what the majority of opponents' moves was
class Majority : public Strategy {
public:
    char choose(const History& history) const {
        if (history.countOpponentDefections() > history.countOpponentCooperations()) {
            return DEFECT;
        }
        return COOPERATE;
    }
    std::string name() const {
        return "Majority";
    }
};

// Cooperates with the given probability (in percent)
class Probabilistic : public Strategy {
private:
    int percent;

public:
    Probabilistic(int cooperatePercent) {
        percent = cooperatePercent;
    }

    char choose(const History&) const {
        if (rand() % 100 < percent) {
            return COOPERATE;
        }
        return DEFECT;
    }
    std::string name() const {
        return "Probabilistic";
    }
};

// Meta strategy: asks other strategies and does what most of them say
class Voting : public Strategy {
private:
    std::vector<std::unique_ptr<Strategy> > members;

public:
    void add(std::unique_ptr<Strategy> strategy) {
        members.push_back(std::move(strategy));
    }

    char choose(const History& history) const {
        int defects = 0;
        int cooperates = 0;
        for (size_t i = 0; i < members.size(); i++) {
            if (members[i]->choose(history) == DEFECT) {
                defects++;
            } else {
                cooperates++;
            }
        }
        if (defects > cooperates) {
            return DEFECT;
        }
        return COOPERATE;
    }
    std::string name() const {
        return "Voting";
    }
};

#endif
