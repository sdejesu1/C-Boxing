#ifndef BOXER_H
#define BOXER_H

#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

/**
 * @file Boxer.h
 * @brief Header file containing the Boxer class declaration and related functions.
 *
 * This file declares the Boxer class, representing a boxer character in a game.
 * The class manages attributes and functionalities for reading/writing boxer data,
 * interactions with the game engine, manipulation of boxer attributes, and simulating combat.
 *
 * The Boxer class includes functionalities for:
 * - Reading and writing boxer data from files
 * - Validating and initializing boxer attributes
 * - Interacting with the game engine for character creation, selection, and combat
 * - Handling opponents and combat mechanics
 * - Converting enums to strings and vice versa
 *
 * Enums:
 * - WeightClass: Represents different weight classes for boxers.
 *
 * This header file is part of a larger program and provides the necessary declarations
 * for using the Boxer class and its functionalities.
 */

enum WeightClass {
    FEATHERWEIGHT,
    MIDDLEWEIGHT,
    HEAVYWEIGHT
};

class Boxer {
private:
    string name;
    int health;
    int power;
    WeightClass weight;

public:
    Boxer();
    // for engine, to simplify
    // in step order for simplicity
    bool verifyName(string name);
    bool createNewChar(Boxer &player, string engineInput);
    bool chooseWeightClass(Boxer &player, string engineInput);
    void initializeBoxers(Boxer &player);
    int countLines(const string fileName);
    void appendBoxerData(const string fileName, Boxer boxer);
    vector<Boxer> readBoxerData(const string fileName);
    vector<Boxer> showLoadedChars();
    bool loadNewChar(Boxer &player, string engineInput);
    bool chooseOpp(Boxer &opponent, string engineInput);
    vector<Boxer> arcadeBoxers(string engineInput);
    void playerPunch (Boxer &player, Boxer &opponent);
    void opponentPunch (Boxer &opponent, Boxer &player);


    bool shaken();
    string getName();
    int getHealth();
    int getPower();
    WeightClass getWeight();
    void setName(string paramName);
    void setHealth(int paramHealth);
    void setPower(int paramPower);
    void setWeight(WeightClass paramWeight);
    void decreaseHealth(int damage);
    bool isAlive();
    int attack();
    void displayPlayerPunch(const string opponentName, int damage);
    void displayOpponentPunch(const string opponentName, int damage);
    static string boxToString(const WeightClass weight);
    WeightClass stringToEnum(const string weightClassStr);
};

ostream& operator<<(std::ostream& outs, const WeightClass weight);

#endif /* BOXER_H */