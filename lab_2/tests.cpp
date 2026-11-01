#include <fstream>
#include <iostream>
#include <sstream>
#include "Arguments.h"
#include "Factory.h"
#include "Game.h"
#include "Simulator.h"
#include "Strategy.h"

using namespace std;

int passed = 0;
int failed = 0;

void check(bool condition, string name) {
    if (condition) {
        passed++;
    } else {
        failed++;
        cout << "FAILED: " << name << endl;
    }
}

void writeFile(string path, string text) {
    ofstream file(path.c_str());
    file << text;
}

void testHistory() {
    History h;
    check(h.size() == 0, "history: empty");
    h.add('C', 'C', 'D');
    h.add('D', 'C', 'C');
    check(h.size() == 2, "history: size");
    check(h.anyOpponentDefected(0), "history: defection in round 0");
    check(!h.anyOpponentDefected(1), "history: no defection in round 1");
    check(h.countOpponentDefections() == 1, "history: count defections");
    check(h.countOpponentCooperations() == 3, "history: count cooperations");
}

void testTrivialStrategies() {
    History h;
    AlwaysCooperate ac;
    AlwaysDefect ad;
    check(ac.choose(h) == COOPERATE, "AlwaysCooperate");
    check(ad.choose(h) == DEFECT, "AlwaysDefect");

    RandomChoice r;
    bool sawC = false;
    bool sawD = false;
    for (int i = 0; i < 200; i++) {
        if (r.choose(h) == COOPERATE) {
            sawC = true;
        } else {
            sawD = true;
        }
    }
    check(sawC && sawD, "Random gives both moves");

    Alternating alt;
    check(alt.choose(h) == COOPERATE, "Alternating: first move");
    h.add('C', 'C', 'C');
    check(alt.choose(h) == DEFECT, "Alternating: second move");

    TitForTat t;
    History h2;
    check(t.choose(h2) == COOPERATE, "TitForTat: first move");
    h2.add('C', 'C', 'C');
    check(t.choose(h2) == COOPERATE, "TitForTat: after cooperation");
    h2.add('C', 'C', 'D');
    check(t.choose(h2) == DEFECT, "TitForTat: after defection");
    h2.add('D', 'C', 'C');
    check(t.choose(h2) == COOPERATE, "TitForTat: forgives");
}

void testNonTrivialStrategies() {
    Grudger g;
    History h;
    check(g.choose(h) == COOPERATE, "Grudger: first move");
    h.add('C', 'D', 'C');
    h.add('C', 'C', 'C');
    check(g.choose(h) == DEFECT, "Grudger: never forgives");

    Punisher p(2);
    History h2;
    h2.add('C', 'D', 'C');
    check(p.choose(h2) == DEFECT, "Punisher: punishes");
    h2.add('D', 'C', 'C');
    check(p.choose(h2) == DEFECT, "Punisher: still punishes");
    h2.add('D', 'C', 'C');
    check(p.choose(h2) == COOPERATE, "Punisher: stops after punishRounds");

    Majority m;
    History h3;
    check(m.choose(h3) == COOPERATE, "Majority: empty history");
    h3.add('C', 'D', 'D');
    check(m.choose(h3) == DEFECT, "Majority: defectors win");
    h3.add('C', 'C', 'C');
    h3.add('C', 'C', 'C');
    check(m.choose(h3) == COOPERATE, "Majority: cooperators win");

    History empty;
    Probabilistic always(100);
    Probabilistic never(0);
    check(always.choose(empty) == COOPERATE, "Probabilistic 100%");
    check(never.choose(empty) == DEFECT, "Probabilistic 0%");

    Voting v;
    v.add(unique_ptr<Strategy>(new AlwaysDefect()));
    v.add(unique_ptr<Strategy>(new AlwaysDefect()));
    v.add(unique_ptr<Strategy>(new AlwaysCooperate()));
    check(v.choose(empty) == DEFECT, "Voting: majority defects");

    Voting v2;
    v2.add(unique_ptr<Strategy>(new AlwaysCooperate()));
    v2.add(unique_ptr<Strategy>(new AlwaysCooperate()));
    v2.add(unique_ptr<Strategy>(new AlwaysDefect()));
    check(v2.choose(empty) == COOPERATE, "Voting: majority cooperates");
}

void testMatrix() {
    GameMatrix m;
    check(m.getScore('C', 'C', 'C', 0) == 7, "matrix: CCC");
    check(m.getScore('C', 'C', 'D', 2) == 9, "matrix: CCD traitor");
    check(m.getScore('C', 'C', 'D', 0) == 3, "matrix: CCD victim");
    check(m.getScore('C', 'D', 'C', 1) == 9, "matrix: CDC");
    check(m.getScore('D', 'C', 'D', 1) == 0, "matrix: DCD");
    check(m.getScore('D', 'D', 'C', 2) == 0, "matrix: DDC");
    check(m.getScore('D', 'D', 'D', 1) == 1, "matrix: DDD");

    writeFile("test_matrix.txt", "1 1 1\n2 2 2\n3 3 3\n4 4 4\n5 5 5\n6 6 6\n7 7 7\n8 8 8\n");
    GameMatrix fromFile("test_matrix.txt");
    check(fromFile.getScore('C', 'C', 'C', 0) == 1, "matrix from file: first row");
    check(fromFile.getScore('D', 'D', 'D', 2) == 8, "matrix from file: last row");

    bool thrown = false;
    try {
        GameMatrix bad("no_such_file.txt");
    } catch (exception&) {
        thrown = true;
    }
    check(thrown, "matrix: missing file throws");

    writeFile("bad_matrix.txt", "1 2 3\n");
    thrown = false;
    try {
        GameMatrix bad("bad_matrix.txt");
    } catch (exception&) {
        thrown = true;
    }
    check(thrown, "matrix: short file throws");
}

void testGame() {
    AlwaysDefect a;
    AlwaysDefect b;
    AlwaysDefect c;
    Game game(&a, &b, &c, GameMatrix());
    game.playRound();
    check(game.getRounds() == 1, "game: rounds");
    check(game.getLastMove(0) == DEFECT, "game: last move");
    check(game.getLastScore(1) == 1, "game: last score");
    game.play(9);
    check(game.getRounds() == 10, "game: play");
    check(game.getTotal(2) == 10, "game: total");
    check(game.getWinner() == -1, "game: draw");

    AlwaysCooperate x;
    AlwaysCooperate y;
    AlwaysDefect z;
    Game game2(&x, &y, &z, GameMatrix());
    game2.play(5);
    check(game2.getTotal(0) == 15, "game2: victim total");
    check(game2.getTotal(2) == 45, "game2: traitor total");
    check(game2.getWinner() == 2, "game2: winner");

    TitForTat t1;
    TitForTat t2;
    TitForTat t3;
    Game game3(&t1, &t2, &t3, GameMatrix());
    game3.play(10);
    check(game3.getTotal(0) == 70, "game3: TitForTat cooperates together");

    vector<int> scores;
    scores.push_back(3);
    scores.push_back(8);
    scores.push_back(5);
    check(findWinner(scores) == 1, "findWinner");
}

void testFactory() {
    StrategyFactory factory;
    vector<string> names = StrategyFactory::allNames();
    check(names.size() == 10, "factory: number of strategies");
    bool allCreated = true;
    for (size_t i = 0; i < names.size(); i++) {
        unique_ptr<Strategy> s = factory.create(names[i]);
        if (s->name() != names[i]) {
            allCreated = false;
        }
    }
    check(allCreated, "factory: creates every strategy");

    bool thrown = false;
    try {
        factory.create("NoSuchStrategy");
    } catch (invalid_argument&) {
        thrown = true;
    }
    check(thrown, "factory: unknown name throws");

    system("mkdir test_configs 2> nul");
    writeFile("test_configs/Punisher.txt", "3");
    writeFile("test_configs/Probabilistic.txt", "100");
    writeFile("test_configs/Voting.txt", "AlwaysDefect AlwaysDefect AlwaysCooperate");
    StrategyFactory withConfigs("test_configs");

    unique_ptr<Strategy> punisher = withConfigs.create("Punisher");
    History h;
    h.add('C', 'D', 'C');
    h.add('C', 'C', 'C');
    h.add('C', 'C', 'C');
    check(punisher->choose(h) == DEFECT, "factory: Punisher uses config");

    unique_ptr<Strategy> prob = withConfigs.create("Probabilistic");
    check(prob->choose(h) == COOPERATE, "factory: Probabilistic uses config");

    unique_ptr<Strategy> voting = withConfigs.create("Voting");
    check(voting->choose(h) == DEFECT, "factory: Voting uses config");

    writeFile("test_configs/Voting.txt", "Voting");
    thrown = false;
    try {
        withConfigs.create("Voting");
    } catch (invalid_argument&) {
        thrown = true;
    }
    check(thrown, "factory: Voting inside Voting throws");

    StrategyFactory missing("no_such_dir");
    thrown = false;
    try {
        missing.create("Punisher");
    } catch (exception&) {
        thrown = true;
    }
    check(thrown, "factory: missing config throws");

    writeFile("test_configs/Punisher.txt", "abc");
    thrown = false;
    try {
        withConfigs.create("Punisher");
    } catch (exception&) {
        thrown = true;
    }
    check(thrown, "factory: broken config throws");
}

void testTournament() {
    vector<string> names;
    names.push_back("AlwaysCooperate");
    names.push_back("AlwaysDefect");
    names.push_back("TitForTat");
    names.push_back("Grudger");

    Tournament t(names, StrategyFactory(), GameMatrix(), 10);
    ostringstream out;
    t.run(out);
    string text = out.str();

    int games = 0;
    size_t pos = text.find("Game ");
    while (pos != string::npos) {
        games++;
        pos = text.find("Game ", pos + 1);
    }
    check(games == 4, "tournament: 4 triples from 4 strategies");
    check(t.getTotal(0) == 76, "tournament: AlwaysCooperate total");
    check(t.getTotal(1) == 126, "tournament: AlwaysDefect total");
    check(t.getTotal(2) == 130 && t.getTotal(3) == 130, "tournament: TitForTat and Grudger totals");
    check(t.getWinner() == -1, "tournament: draw");
    check(text.find("Draw") != string::npos, "tournament: protocol says draw");

    vector<string> three;
    three.push_back("AlwaysCooperate");
    three.push_back("AlwaysDefect");
    three.push_back("TitForTat");
    Tournament t2(three, StrategyFactory(), GameMatrix(), 10);
    ostringstream out2;
    t2.run(out2);
    check(t2.getWinner() == 1, "tournament: AlwaysDefect wins");
    check(out2.str().find("Winner: AlwaysDefect") != string::npos, "tournament: protocol has winner");
}

void testSimulator() {
    vector<string> names;
    names.push_back("AlwaysDefect");
    names.push_back("AlwaysCooperate");
    names.push_back("AlwaysCooperate");

    Simulator sim(StrategyFactory(), GameMatrix(), 5);
    ostringstream out;
    sim.runFast(names, out);
    check(out.str().find("Winner: AlwaysDefect") != string::npos, "simulator: fast mode");

    istringstream in("\n\nquit\n");
    ostringstream out2;
    sim.runDetailed(names, in, out2);
    check(out2.str().find("Round 2") != string::npos, "simulator: detailed mode steps");
    check(out2.str().find("Round 3") == string::npos, "simulator: detailed mode quit");

    istringstream in2("\n\n\n\n\n\n\n");
    ostringstream out3;
    sim.runDetailed(names, in2, out3);
    check(out3.str().find("Round 5") != string::npos, "simulator: detailed mode stops at steps");
    check(out3.str().find("Round 6") == string::npos, "simulator: detailed mode limit");
}

void testArguments() {
    char* argv1[] = {(char*)"prog", (char*)"TitForTat", (char*)"Grudger", (char*)"Random"};
    Arguments a1(4, argv1);
    check(a1.getMode() == "detailed", "arguments: default mode for 3");
    check(a1.getNames().size() == 3, "arguments: names");
    check(a1.getSteps() == 100, "arguments: default steps");

    char* argv2[] = {(char*)"prog", (char*)"A", (char*)"B", (char*)"C", (char*)"D",
                     (char*)"--steps=20", (char*)"--configs=cfg", (char*)"--matrix=m.txt"};
    Arguments a2(8, argv2);
    check(a2.getMode() == "tournament", "arguments: default mode for 4");
    check(a2.getSteps() == 20, "arguments: steps");
    check(a2.getConfigDir() == "cfg", "arguments: configs");
    check(a2.getMatrixPath() == "m.txt", "arguments: matrix");

    char* argv3[] = {(char*)"prog", (char*)"A", (char*)"B", (char*)"C", (char*)"--mode=fast"};
    Arguments a3(5, argv3);
    check(a3.getMode() == "fast", "arguments: mode");

    int errors = 0;
    char* bad1[] = {(char*)"prog", (char*)"A", (char*)"B"};
    char* bad2[] = {(char*)"prog", (char*)"A", (char*)"B", (char*)"C", (char*)"--mode=slow"};
    char* bad3[] = {(char*)"prog", (char*)"A", (char*)"B", (char*)"C", (char*)"D", (char*)"--mode=fast"};
    char* bad4[] = {(char*)"prog", (char*)"A", (char*)"B", (char*)"C", (char*)"--steps=0"};
    char* bad5[] = {(char*)"prog", (char*)"A", (char*)"B", (char*)"C", (char*)"--unknown=1"};
    try { Arguments x(3, bad1); } catch (invalid_argument&) { errors++; }
    try { Arguments x(5, bad2); } catch (invalid_argument&) { errors++; }
    try { Arguments x(6, bad3); } catch (invalid_argument&) { errors++; }
    try { Arguments x(5, bad4); } catch (invalid_argument&) { errors++; }
    try { Arguments x(5, bad5); } catch (invalid_argument&) { errors++; }
    check(errors == 5, "arguments: wrong arguments throw");
}

int main() {
    testHistory();
    testTrivialStrategies();
    testNonTrivialStrategies();
    testMatrix();
    testGame();
    testFactory();
    testTournament();
    testSimulator();
    testArguments();

    cout << "Passed: " << passed << ", failed: " << failed << endl;
    if (failed > 0) {
        return 1;
    }
    return 0;
}
