#include "Boxer.h"
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;

/**
 * @file Boxer.cpp
 * @brief Implementation file defining the functionalities of the Boxer class.
 *
 * This file implements the functionalities declared in the Boxer class header file (Boxer.h).
 * The Boxer class represents a character in a game, managing attributes and actions related to boxers.
 *
 * Functionalities included in this implementation:
 * - Reading and writing boxer data from files
 * - Validating and manipulating boxer attributes
 * - Interacting with the game engine for character creation, selection, and combat
 * - Handling opponents and combat mechanics
 * - Conversion functions for enums to strings and vice versa
 *
 * This file is part of a larger program and provides the implementations of the functionalities
 * for the Boxer class.
 */

Boxer::Boxer(): name(), health(), power(), weight() {}


vector<Boxer> Boxer::readBoxerData(const string fileName) {
    vector<Boxer> boxers;
    ifstream file(fileName);
    if (file.is_open()) {

        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string name, health, power, weightClass;
            getline(ss, name, ',');
            getline(ss, health, ',');
            getline(ss, power, ',');
            getline(ss, weightClass, ',');

            Boxer boxer;
            boxer.setName(name);
            boxer.setHealth(stoi(health));
            boxer.setPower(stoi(power));
            boxer.setWeight(boxer.stringToEnum(weightClass));

            boxers.push_back(boxer);
        }
        file.close();
    } else {
        cout << "Unable to open file." << endl;
    }
    return boxers;
}


int Boxer::countLines(const string fileName) {
    ifstream file(fileName);
    int count = 0;
    string line;
    while (getline(file, line)) {
        ++count;
    }
    return count;
}

void Boxer::appendBoxerData(const string fileName, Boxer boxer) {
    const int maxLines = 3;
    int linesCount = countLines(fileName);

    if (linesCount < maxLines) {
        ofstream file(fileName, ios::app); // Open in append mode
        if (file.is_open()) {
            file << boxer.getName() << "," << boxer.getHealth() << "," << boxer.getPower() << "," << boxer.getWeight() << "\n";
            file.close();
        } else {
            cout << "Unable to open file." << endl;
        }
    } else {
        // Replace the last line
        vector<string> lines;
        ifstream readFile(fileName);
        string line;
        while (getline(readFile, line)) {
            lines.push_back(line);
        }
        readFile.close();

        lines.pop_back(); // Remove the last line

        ofstream writeFile(fileName);
        for (const string &l : lines) {
            writeFile << l << "\n";
        }
        writeFile << boxer.getName() << "," << boxer.getHealth() << "," << boxer.getPower() << "," << boxer.getWeight() << "\n";
        writeFile.close();
    }
}

bool Boxer::verifyName(string name) {
    while (true) {
        if (name.find(' ') != string::npos) {
            cout << "Invalid input. Please enter your first name without spaces. ";
        } else if (name.length() == 0) {
            cout << "No input. ";
        } else {
            bool validName = true;
            for (char c: name) {
                if (!isalpha(c)) { // Check if each character is a letter
                    validName = false;
                    break;
                }
            }
            if (validName && name.length() > 1) {
                return true; // Exits the loop if the input meets the conditions
            } else {
                return false;
            }
        }
    }
}


/// creating mini functions to be called by engine

// the way it'll work is by having engine read from the backend, and having all the data in the backend

// welcome menu: no need

// create new char & load char: need to get input for name from engine

// creating new char using boxer object reference to change it and input from engine
bool Boxer::createNewChar(Boxer &player, string engineInput) {
    if (verifyName(engineInput)) {
        player.setName(engineInput);
        return true;
    } else {
        return false;
    }
}

bool Boxer::chooseWeightClass(Boxer &player, string engineInput) {
    switch (stoi(engineInput)) {
        case 1:
            player.setWeight(FEATHERWEIGHT);
            break;
        case 2:
            player.setWeight(MIDDLEWEIGHT);
            break;
        case 3:
            player.setWeight(HEAVYWEIGHT);
            break;
    }
    return true;
}

void Boxer::initializeBoxers(Boxer &player) {
    const int FEATHERWEIGHT_HEALTH = 100;
    const int FEATHERWEIGHT_POWER = 15;

    const int MIDDLEWEIGHT_HEALTH = 125;
    const int MIDDLEWEIGHT_POWER = 25;

    const int HEAVYWEIGHT_HEALTH = 150;
    const int HEAVYWEIGHT_POWER = 35;

    switch (player.weight) {
        case FEATHERWEIGHT:
            player.setHealth(FEATHERWEIGHT_HEALTH);
            player.setPower(FEATHERWEIGHT_POWER);
            break;
        case MIDDLEWEIGHT:
            player.setHealth(MIDDLEWEIGHT_HEALTH);
            player.setPower(MIDDLEWEIGHT_POWER);
            break;
        case HEAVYWEIGHT:
            player.setHealth(HEAVYWEIGHT_HEALTH);
            player.setPower(HEAVYWEIGHT_POWER);
            break;
    }
}

vector<Boxer> Boxer::showLoadedChars() {
    vector<Boxer> savedBoxers = readBoxerData("../data/users.txt");
    return savedBoxers;
}

bool Boxer::loadNewChar(Boxer &player, string engineInput) {
    // Load character from save and allow user to select one
    vector<Boxer> savedBoxers = readBoxerData("../data/users.txt");
    if (stoi(engineInput) >= 1 && stoi(engineInput) <= savedBoxers.size()) {
        player = savedBoxers[stoi(engineInput) - 1];
        return true;
    } else {
        return false;
    }
}

// Opponent selection
bool Boxer::chooseOpp(Boxer &opponent, string engineInput){
    vector<Boxer> opponents = readBoxerData("../data/opponents.txt");
    opponent = opponents[stoi(engineInput) - 1];
    return true;
}

// Opponent selection for arcade
vector<Boxer> Boxer::arcadeBoxers(string engineInput){
    vector<Boxer> opponents;
    if (engineInput == "1"){
        opponents = readBoxerData("../data/arcadeFW.txt");
    } else if (engineInput == "2"){
        opponents = readBoxerData("../data/arcadeMW.txt");
    } else if (engineInput == "3"){
        opponents = readBoxerData("../data/arcadeHW.txt");
    }

    return opponents;
}

void Boxer::playerPunch (Boxer &player, Boxer &opponent){
    int playerDamage = player.attack();
    opponent.decreaseHealth(playerDamage);
}

void Boxer::opponentPunch (Boxer &opponent, Boxer &player){
    int opponentDamage = opponent.attack();
    player.decreaseHealth(opponentDamage);
}

bool Boxer::shaken(){
    int shakeChance = rand() % 100; // Generates a number from 0 to 99
    if (shakeChance < 40) {
        return true;
    }
    else {
        return false;
    }
}

string Boxer::getName() {
    return name;
}

int Boxer::getHealth() {
    return health;
}

int Boxer::getPower() {
    return power;
}

WeightClass Boxer::getWeight() {
    return weight;
}

void Boxer::setName(string paramName) {
    name = paramName;
}

void Boxer::setHealth(int paramHealth) {
    health = paramHealth;
}

void Boxer::setPower(int paramPower) {
    power = paramPower;
}

void Boxer::setWeight(WeightClass paramWeight) {
    weight = paramWeight;
}

void Boxer::decreaseHealth(int damage) {
    health -= damage;
}

bool Boxer::isAlive() {
    return health > 0;
}

int Boxer::attack() {
    return rand() % power + 1;
}

void Boxer::displayPlayerPunch(const string opponentName, int damage) {
    cout << "You ";
    cout << "\033[1;34mPUNCHED\033[0m " << opponentName << " with power \033[1;32m" << damage << "\033[0m!" << endl;
}

void Boxer::displayOpponentPunch(const string opponentName, int damage) {
    cout << opponentName << " \033[1;34mPUNCHED\033[0m you with power \033[1;31m" << damage << "\033[0m!" << endl;
}

string Boxer::boxToString(const WeightClass weight) {
    switch (weight) {
        case FEATHERWEIGHT:
            return "Featherweight";
        case MIDDLEWEIGHT:
            return "Middleweight";
        case HEAVYWEIGHT:
            return "Heavyweight";
        default:
            return "Unknown Weight Class";
    }
}

WeightClass Boxer::stringToEnum(const string weightClassStr) {
    if (weightClassStr == "Featherweight") {
        return FEATHERWEIGHT;
    } else if (weightClassStr == "Middleweight") {
        return MIDDLEWEIGHT;
    } else if (weightClassStr == "Heavyweight") {
        return HEAVYWEIGHT;
    } else {
        // Handle unknown strings or return a default value
        return FEATHERWEIGHT; // Or any other default value you prefer
    }
}

ostream& operator<<(std::ostream& outs, const WeightClass weight) {
    outs << Boxer::boxToString(weight);
    return outs;
}