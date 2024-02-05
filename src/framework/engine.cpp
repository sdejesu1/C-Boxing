#include "engine.h"
#include "../../Boxer.cpp"
#include <thread>

using namespace std;

enum state {
    menu, // entry menu
    exit_screen, // if you choose exit instead of play
    character_menu,  // choose char, create or load
    create_c,  // create, make name
    choose_weight, // choose weight class
    load_c, // load character

    // game states for arcade mode
    choose_game_mode, // versus or arcade
    choose_opp, // choose opponent (for versus)
    arcade_choose_weight, // choose which arcade weight class you want to fight in
    fight_1, // first fight
    fight_2, // second fight
    fight_3, // third and final fight

    arcade_win, // special state for end of arcade
    over_win, // if you win, at the end of either versus or fight 3
    over_lose, // if you lose

    game  // the game itself

};
state screen;

// Colors
color brown = {0.396f, 0.263f, 0.129f, 1.0f};
color skyBlue = {0.529f, 0.808f, 0.922f, 1.0f};
color gold = {1.0f, 0.843f, 0.0f, 1.0f};


Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();
}

Engine::~Engine() {}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(width, height, "engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag", nullptr,
                                                  "shape");

    // Configure text shader and renderer
    textShader = shaderManager->loadShader("../res/shaders/text.vert", "../res/shaders/text.frag", nullptr, "text");
    fontRenderer = make_unique<FontRenderer>(shaderManager->getShader("text"), "../res/fonts/MxPlus_IBM_BIOS.ttf", 24);

    // Set uniforms
    textShader.use().setVector2f("vertex", vec4(100, 100, .5, .5));
    shapeShader.use().setMatrix4("projection", this->PROJECTION);
}

// member variables for the boxing ring and characters
unique_ptr<Rect> boxingRing;
unique_ptr<Rect> userCharacter;
unique_ptr<Rect> oppCharacter;
// string variable for name input
string createCharInput;
// creating boxer objects
Boxer functionCaller;
Boxer player;
Boxer opponent;
// vector of boxers for arcade
vector <Boxer> arcadeOpps;
// bool flags that will be used to change the traversal of the program when screen = game
bool fight1;
bool fight2;
bool fight3;


// flags to enforce turns
bool playerTurn = true;
bool opponentTurn = false;
bool playerShaken = false; // Flag to track if opponent's action has executed
bool opponentShaken = false; // Flag to track if opponent's action has executed
bool counter = false;

// counter function for player
void pressCPlayer(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        // Start the punch animation
        functionCaller.playerPunch(player, opponent);
        userCharacter->moveY(80.0f);
    }
    if (key == GLFW_KEY_C && action == GLFW_RELEASE) {
        userCharacter->moveY(-80.0f);
        counter = false;
    }
}

// counter function for opponent
void pressCOpp(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        // Start the punch animation
        functionCaller.opponentPunch(opponent, player);
        oppCharacter->moveY(-80.0f);
    }
    if (key == GLFW_KEY_C && action == GLFW_RELEASE) {
        oppCharacter->moveY(80.0f);
        counter = false;
    }
}

// function to get input on the press of p, punch, or b, block. Has punch and block functionality as well as
// the shaken functionality. S and V for opponent, P and B for player

void pressPorB(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        // Start the punch animation
        functionCaller.playerPunch(player, opponent);
        userCharacter->moveY(80.0f);
    }
    if (key == GLFW_KEY_P && action == GLFW_RELEASE) {
        userCharacter->moveY(-80.0f);
        playerTurn = false;
        opponentTurn = true;
    }

    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        userCharacter->moveY(-40.0f);
    }
    if (key == GLFW_KEY_B && action == GLFW_RELEASE) {
        userCharacter->moveY(40.0f);
        if (functionCaller.shaken()) {
            playerTurn = false;
            playerShaken = true;
            opponentTurn = true;
        } else {
            counter = true;
        }
    }
}

void pressSorV(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        // Start the punch animation
        functionCaller.opponentPunch(opponent, player);
        oppCharacter->moveY(-80.0f);
    }
    if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
        oppCharacter->moveY(80.0f);
        opponentTurn = false;
        playerTurn = true;
    }

    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        oppCharacter->moveY(40.0f);
    }
    if (key == GLFW_KEY_V && action == GLFW_RELEASE) {
        oppCharacter->moveY(-40.0f);
        if (functionCaller.shaken()) {
            opponentTurn = false;
            opponentShaken = true;
            playerTurn = true;
        } else {
            counter = true;
        }
    }
}


void Engine::initShapes() {
    // Initialize the boxing ring (larger rectangle)
    color ringColor = {0.4f, 0.4f, 0.4f, 1.0f}; // Choose a color for the ring
    vec2 ringSize = {width - 150.0f, height - 200.0f}; // Adjust the size of the ring
    vec2 ringPos = {width / 2, height / 2}; // Position the ring at the center of the window
    boxingRing = make_unique<Rect>(shapeShader, ringPos, ringSize, ringColor);

    // Initialize the player character
    color userColor = {0.8f, 0.2f, 0.2f, 1.0f}; // Choose a color for the movable character
    vec2 userSize = {30.0f, 50.0f}; // Adjust the size of the movable character
    vec2 userStartPos = {width / 2, 4 * (height / 10)}; // Start the movable character at the center of the ring
    userCharacter = make_unique<Rect>(shapeShader, userStartPos, userSize, userColor);

    // Initialize the opponent character
    color oppColor = {0.2f, 0.8f, 0.2f, 1.0f}; // Choose a color for the fixed character
    vec2 oppSize = {40.0f, 60.0f}; // Adjust the size of the fixed character
    vec2 oppPos = {width / 2, 6 * (height / 10)}; // Position the fixed character at a specific location
    oppCharacter = make_unique<Rect>(shapeShader, oppPos, oppSize, oppColor);
}

void Engine::processInput() {
    glfwPollEvents();

    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    /// when screen = menu
    if (screen == menu && keys[GLFW_KEY_1]) {
        screen = character_menu;
    } else if (screen == menu && keys[GLFW_KEY_2]) {
        screen = exit_screen;
    }

    /// when screen = character_menu
    if (screen == character_menu && keys[GLFW_KEY_A]) {
        screen = create_c;
    } else if (screen == character_menu && keys[GLFW_KEY_B]) {
        screen = load_c;
    }

    /// when screen = create_c
    if (screen == create_c) {
        static bool keyPressed[GLFW_KEY_LAST] = {false};  // Track key press states
        // Check if any alphanumeric key is pressed and update userInput
        for (int key = GLFW_KEY_A; key <= GLFW_KEY_Z; ++key) {
            if (keys[key] && !keyPressed[key]) {
                // Convert GLFW key to character (A-Z)
                char keyPressedChar = ('A' + (key - GLFW_KEY_A));
                createCharInput += keyPressedChar;
                keyPressed[key] = true;  // Mark key as pressed
            } else if (!keys[key]) {
                keyPressed[key] = false;  // Reset key press state when key is released
            }
        }

        // Check if Backspace is pressed to delete characters from userInput
        if (keys[GLFW_KEY_BACKSPACE] && !createCharInput.empty()) {
            createCharInput.pop_back();
        }

        // Check if Enter is pressed to finish input and proceed
        if (keys[GLFW_KEY_ENTER]) {
            if (functionCaller.createNewChar(player, createCharInput)) {
                screen = choose_weight;
            }
        }

    }
    /// when screen = choose_weight
    if (screen == choose_weight) {
        string chooseWeightInput;
        if (keys[GLFW_KEY_1]) {
            chooseWeightInput = "1"; // passing along Featherweight data
            if (functionCaller.chooseWeightClass(player, chooseWeightInput)) {
                functionCaller.initializeBoxers(player);
                functionCaller.appendBoxerData("../data/users.txt", player);  // users file
                screen = choose_game_mode;
            } else {
                screen = menu;
            }
        } else if (keys[GLFW_KEY_2]) {
            chooseWeightInput = "2"; // passing along Middleweight data
            if (functionCaller.chooseWeightClass(player, chooseWeightInput)) {
                functionCaller.initializeBoxers(player);
                functionCaller.appendBoxerData("../data/users.txt", player);  // users file
                screen = choose_game_mode;
            } else {
                screen = menu;
            }
        } else if (keys[GLFW_KEY_3]) {
            chooseWeightInput = "3"; // passing along Heavyweight data
            if (functionCaller.chooseWeightClass(player, chooseWeightInput)) {
                functionCaller.initializeBoxers(player);
                functionCaller.appendBoxerData("../data/users.txt", player);  // users file
                screen = choose_game_mode;
            } else {
                screen = menu;
            }
        }
    }

    /// when screen = load_c
    if (screen == load_c) {
        string loadCharInput;
        if (keys[GLFW_KEY_1]) {
            loadCharInput = "1"; // passing along loaded character data
            if (functionCaller.loadNewChar(player, loadCharInput)) {
                screen = choose_game_mode;
            }
        } else if (keys[GLFW_KEY_2]) {
            loadCharInput = "2"; // passing along loaded character data
            if (functionCaller.loadNewChar(player, loadCharInput)) {
                screen = choose_game_mode;
            }
        } else if (keys[GLFW_KEY_3]) {
            loadCharInput = "3"; // passing along loaded character data
            if (functionCaller.loadNewChar(player, loadCharInput)) {
                screen = choose_game_mode;
            }
        }
    }


    /**
    game states for arcade mode
    arcade_choose_weight, // choose which arcade weight class you want to fight in
    fight_1, // first fight
    fight_2, // second fight
    fight_3, // third and final fight
     */

    /// when screen = choose_game_mode
    if (screen == choose_game_mode) {
        if (keys[GLFW_KEY_A]) {
            screen = arcade_choose_weight;
        } else if (keys[GLFW_KEY_B]) {
            screen = choose_opp;
        }
    }

    /// when screen = arcade_choose_weight
    if (screen == arcade_choose_weight) {
        string arcadeWeightChoice;
        if (keys[GLFW_KEY_1]) {
            arcadeWeightChoice = "1";
            arcadeOpps = functionCaller.arcadeBoxers(arcadeWeightChoice);
            screen = fight_1;
        } else if (keys[GLFW_KEY_2]) {
            arcadeWeightChoice = "2";
            arcadeOpps = functionCaller.arcadeBoxers(arcadeWeightChoice);
            screen = fight_1;
        } else if (keys[GLFW_KEY_3]) {
            arcadeWeightChoice = "3";
            arcadeOpps = functionCaller.arcadeBoxers(arcadeWeightChoice);
            screen = fight_1;
        }
    }

    if (screen == fight_1){
        // Check if Enter is pressed to finish input and proceed
        if (keys[GLFW_KEY_ENTER]) {
            oppCharacter->setColor(skyBlue);
            opponent = arcadeOpps[0];
            fight1 = true;
            screen = game;
        }
    }

    // resetting any game variables, to restart the match and stats
    if (screen == fight_2){
        // Check if Enter is pressed to finish input and proceed
        if (keys[GLFW_KEY_ENTER]) {
            oppCharacter->setColor(brown);
            opponent = arcadeOpps[1];
            functionCaller.initializeBoxers(player);
            // flags to enforce turns
            playerTurn = true;
            opponentTurn = false;
            playerShaken = false; // Flag to track if opponent's action has executed
            opponentShaken = false; // Flag to track if opponent's action has executed
            counter = false;
            fight1 = false;
            fight2 = true;
            screen = game;
        }
    }

    if (screen == fight_3){
        // Check if Enter is pressed to finish input and proceed
        if (keys[GLFW_KEY_ENTER]) {
            oppCharacter->setColor(gold);
            opponent = arcadeOpps[2];
            functionCaller.initializeBoxers(player);
            // flags to enforce turns
            playerTurn = true;
            opponentTurn = false;
            playerShaken = false; // Flag to track if opponent's action has executed
            opponentShaken = false; // Flag to track if opponent's action has executed
            counter = false;
            fight2 = false;
            fight3 = true;
            screen = game;
        }
    }



    /// when screen = choose_opp
    if (screen == choose_opp) {
        string oppChoice;
        if (keys[GLFW_KEY_1]) {
            oppChoice = "1";
            if (functionCaller.chooseOpp(opponent, oppChoice)) {
                screen = game; // passing along chosen opponent
            }
        } else if (keys[GLFW_KEY_2]) {
            oppChoice = "2";
            if (functionCaller.chooseOpp(opponent, oppChoice)) {
                screen = game; // passing along chosen opponent
            }
        } else if (keys[GLFW_KEY_3]) {
            oppChoice = "";
            if (functionCaller.chooseOpp(opponent, oppChoice)) {
                screen = game; // passing along chosen opponent
            }
        }
    }

    // game logic
    if (screen == game) {
        if (player.isAlive() && opponent.isAlive()) {

            // normal conditions
            if (playerTurn && !playerShaken && !counter) {
                glfwSetKeyCallback(window, pressPorB);
            }
            if (opponentTurn && !opponentShaken && !counter) {
                glfwSetKeyCallback(window, pressSorV);
            }

            // if player is shaken
            if (playerTurn && playerShaken && !counter) {
                // Player's turn logic
                glfwSetKeyCallback(window, pressSorV);
                playerShaken = false;
                playerTurn = false;
                opponentTurn = true;
            }
            // if opponent is shaken
            if (opponentTurn && opponentShaken && !counter) {
                glfwSetKeyCallback(window, pressPorB);
                opponentShaken = false;
                opponentTurn = false;
                playerTurn = true;
            }

            // if opp and player have counter
            if (playerTurn && counter) {
                glfwSetKeyCallback(window, pressCPlayer);
            }
            if (opponentTurn && counter) {
                glfwSetKeyCallback(window, pressCOpp);
            }

        } else {
            // Game over logic
            // if player is alive and is in arcade (fight bools are true), send to next destination
            if (player.isAlive()) {
                if (fight1){
                    screen = fight_2;
                }
                if (fight2){
                    screen = fight_3;
                }
                if (fight3){
                    screen = arcade_win;
                }
                if (!fight1 && !fight2 && !fight3){
                    screen = over_win;
                }
            } else {
                screen = over_lose;
            }
        }
    }
}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void Engine::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color
    glClear(GL_COLOR_BUFFER_BIT);

    // Set shader to use for all shapes
    shapeShader.use();
    // string array for all menus
    vector<string> strArray;
    // Render differently depending on screen
    switch (screen) {
        // render screen and messages for opening menu
        case menu: {
            strArray.clear();
            strArray.emplace_back("Welcome to C++ Boxing!");
            strArray.emplace_back("                      ");
            strArray.emplace_back("       Options:       ");
            strArray.emplace_back("                      ");
            strArray.emplace_back("       1. Play        ");
            strArray.emplace_back("       2. Quit        ");

            int strArrayIndex = 0;
            for (int i = 8; i > 2; i--) {
                this->fontRenderer->renderText(strArray[strArrayIndex],
                                               width / 2 - (12 * strArray[strArrayIndex].length()),
                                               i * (height / 10), 1, vec3{1, 1, 1});
                ++strArrayIndex;
            }
            break;
        }
        // for character menu
        case character_menu: {
            strArray.clear();
            strArray.emplace_back("                       ");
            strArray.emplace_back("A. Create New Character");
            strArray.emplace_back("                       ");
            strArray.emplace_back("B.   Load Character    ");
            strArray.emplace_back("                       ");
            strArray.emplace_back("                       ");
            int strArrayIndex = 0;
            for (int i = 8; i > 2; i--) {
                this->fontRenderer->renderText(strArray[strArrayIndex],
                                               width / 2 - (12 * strArray[strArrayIndex].length()),
                                               i * (height / 10), 1, vec3{1, 1, 1});
                ++strArrayIndex;
            }
            break;
        }
        // for creating character and writing name
        case create_c: {
            strArray.clear();
            strArray.emplace_back("    Write your name:    ");
            strArray.emplace_back("                        ");
            strArray.emplace_back("                        ");
            strArray.emplace_back("                        ");
            strArray.emplace_back("                        ");
            strArray.emplace_back("                        ");
            int strArrayIndex = 0;
            for (int i = 8; i > 2; i--) {
                this->fontRenderer->renderText(strArray[strArrayIndex],
                                               width / 2 - (12 * strArray[strArrayIndex].length()),
                                               i * (height / 10), 1, vec3{1, 1, 1});
                ++strArrayIndex;
            }
            // Render user input on the screen
            this->fontRenderer->renderText(createCharInput, width / 2 - (12 * createCharInput.length()),
                                           (height / 10),
                                           1,
                                           vec3{1, 1, 1});
            break;
        }
        // for choosing weight class when creating character
        case choose_weight: {
            strArray.clear();
            strArray.emplace_back("Choose Your Weight Class");
            strArray.emplace_back("                        ");
            strArray.emplace_back("1.   Featherweight      ");
            strArray.emplace_back("                        ");
            strArray.emplace_back("2.   Middleweight       ");
            strArray.emplace_back("                        ");
            strArray.emplace_back("3.   Heavyweight        ");
            int strArrayIndex = 0;
            for (int i = 9; i > 2; i--) {
                this->fontRenderer->renderText(strArray[strArrayIndex],
                                               width / 2 - (12 * strArray[strArrayIndex].length()),
                                               i * (height / 10), 1, vec3{1, 1, 1});
                ++strArrayIndex;
            }
            break;
        }
        // loading character
        case load_c: {
            vector<Boxer> showLoadedChars = functionCaller.showLoadedChars();
            strArray.clear();
            if (showLoadedChars.empty()) {
                strArray.emplace_back("Choose Your Character");
                strArray.emplace_back("                     ");
                strArray.emplace_back("1.                   ");
                strArray.emplace_back("                     ");
                strArray.emplace_back("2.                   ");
                strArray.emplace_back("                     ");
                strArray.emplace_back("3.                   ");
                int strArrayIndex = 0;
                for (int i = 9; i > 2; i--) {
                    this->fontRenderer->renderText(strArray[strArrayIndex],
                                                   width / 2 - (12 * strArray[strArrayIndex].length()),
                                                   i * (height / 10), 1, vec3{1, 1, 1});
                    ++strArrayIndex;
                }
            } else {
                // Display loaded boxers and allow user to select
                strArray.emplace_back("Choose Your Character");
                for (int i = 0; i < showLoadedChars.size(); ++i) {
                    strArray.push_back(to_string(i + 1) + ". " + showLoadedChars[i].getName() + ": Health = " +
                                       to_string(showLoadedChars[i].getHealth()));
                    strArray.push_back(
                            " Weight Class = " + (showLoadedChars[i].boxToString(showLoadedChars[i].getWeight())));
                    strArray.emplace_back("                     ");
                }
                int strArrayIndex = 0;
                for (int i = 1; i < (showLoadedChars.size() * 3) + 1; i++) {
                    this->fontRenderer->renderText(strArray[strArrayIndex],
                                                   width / 2 - (12 * strArray[strArrayIndex].length()),
                                                   (10 - i) * (height / 10), 1, vec3{1, 1, 1});
                    ++strArrayIndex;
                }
            }
            break;
        }
        // choosing game mode
        case choose_game_mode: {
            strArray.clear();
            strArray.emplace_back("  Choose Game Mode  ");
            strArray.emplace_back("                    ");
            strArray.emplace_back("A.      Arcade      ");
            strArray.emplace_back("                    ");
            strArray.emplace_back("B.      Versus      ");
            strArray.emplace_back("                    ");
            strArray.emplace_back("                    ");
            int strArrayIndex = 0;
            for (int i = 9; i > 2; i--) {
                this->fontRenderer->renderText(strArray[strArrayIndex],
                                               width / 2 - (12 * strArray[strArrayIndex].length()),
                                               i * (height / 10), 1, vec3{1, 1, 1});
                ++strArrayIndex;
            }
            break;
        }
        // choosing the weightclass you're fighting in
        case arcade_choose_weight: {
            strArray.clear();
            strArray.emplace_back("       Arcade:       ");
            strArray.emplace_back("     Weightclass     ");
            strArray.emplace_back("                     ");
            strArray.emplace_back("1.  Featherweight    ");
            strArray.emplace_back("                     ");
            strArray.emplace_back("2.  Middleweight     ");
            strArray.emplace_back("                     ");
            strArray.emplace_back("3.  Heavyweight      ");
            int strArrayIndex = 0;
            for (int i = 9; i > 1; i--) {
                this->fontRenderer->renderText(strArray[strArrayIndex],
                                               width / 2 - (12 * strArray[strArrayIndex].length()),
                                               i * (height / 10), 1, vec3{1, 1, 1});
                ++strArrayIndex;
            }
            break;
        }

        // first fight
        case fight_1: {
            strArray.clear();
            strArray.emplace_back("       Fight 1       ");
            strArray.emplace_back("                     ");
            strArray.push_back(arcadeOpps[0].getName());
            strArray.emplace_back("                     ");
            strArray.emplace_back("    Are You Ready?   ");
            strArray.emplace_back("                     ");
            strArray.emplace_back("     Press Enter     ");
            int strArrayIndex = 0;
            for (int i = 9; i > 1; i--) {
                this->fontRenderer->renderText(strArray[strArrayIndex],
                                               width / 2 - (12 * strArray[strArrayIndex].length()),
                                               i * (height / 10), 1, vec3{1, 1, 1});
                ++strArrayIndex;
            }
            break;
        }


            // second fight
        case fight_2: {
            strArray.clear();
            strArray.emplace_back("      Good work!      ");
            strArray.emplace_back("       Fight 2       ");
            strArray.emplace_back("                     ");
            strArray.push_back(arcadeOpps[1].getName());
            strArray.emplace_back("                     ");
            strArray.emplace_back("    Are You Ready?   ");
            strArray.emplace_back("                     ");
            strArray.emplace_back("     Press Enter     ");
            int strArrayIndex = 0;
            for (int i = 9; i > 1; i--) {
                this->fontRenderer->renderText(strArray[strArrayIndex],
                                               width / 2 - (12 * strArray[strArrayIndex].length()),
                                               i * (height / 10), 1, vec3{1, 1, 1});
                ++strArrayIndex;
            }
            break;
        }


            // final fight
        case fight_3: {
            strArray.clear();
            strArray.emplace_back("     Great work!      ");
            strArray.emplace_back("       Fight 3       ");
            strArray.emplace_back("                     ");
            strArray.push_back(arcadeOpps[2].getName());
            strArray.emplace_back("                     ");
            strArray.emplace_back("    Are You Ready?   ");
            strArray.emplace_back("                     ");
            strArray.emplace_back("     Press Enter     ");
            int strArrayIndex = 0;
            for (int i = 9; i > 1; i--) {
                this->fontRenderer->renderText(strArray[strArrayIndex],
                                               width / 2 - (12 * strArray[strArrayIndex].length()),
                                               i * (height / 10), 1, vec3{1, 1, 1});
                ++strArrayIndex;
            }
            break;
        }

        // choose opponent for versus
        case choose_opp: {
            strArray.clear();
            strArray.emplace_back("Choose Your Opponent");
            strArray.emplace_back("                    ");
            strArray.emplace_back("1. Manny Pacquiao   ");
            strArray.emplace_back("                    ");
            strArray.emplace_back("2. Canelo Alvarez   ");
            strArray.emplace_back("                    ");
            strArray.emplace_back("3.   Mike Tyson     ");
            int strArrayIndex = 0;
            for (int i = 9; i > 2; i--) {
                this->fontRenderer->renderText(strArray[strArrayIndex],
                                               width / 2 - (12 * strArray[strArrayIndex].length()),
                                               i * (height / 10), 1, vec3{1, 1, 1});
                ++strArrayIndex;
            }
            break;
        }

        // render game screen plus all indicators like name and health

        case game: {
            // Render boxing ring
            boxingRing->setUniforms();
            boxingRing->draw();

            // Render fixed character
            oppCharacter->setUniforms();
            oppCharacter->draw();

            // Render movable character
            userCharacter->setUniforms();
            userCharacter->draw();
            // name and health of user, and opponent
            this->fontRenderer->renderText(player.getName() + " - RED",
                                           1.1 * (width / 5) - (12 * (player.getName() + " - RED").length()),
                                           9 * (height / 10), .7, vec3{1, 1, 1});
            this->fontRenderer->renderText(to_string(player.getHealth()),
                                           0.7 * (width / 5) - (player.getName().length()), 8 * (height / 10), .7,
                                           vec3{1, 1, 1});


            this->fontRenderer->renderText(opponent.getName(), 3.4 * (width / 5) - (opponent.getName().length()),
                                           9 * (height / 10), .7, vec3{1, 1, 1});
            this->fontRenderer->renderText(to_string(opponent.getHealth()),
                                           4 * (width / 5) - (opponent.getName().length()), 8 * (height / 10), .7,
                                           vec3{1, 1, 1});

            if (playerTurn && counter) {
                string message = "Press 'C' to counter!";
                this->fontRenderer->renderText(message, width / 3.45 - message.length(), 2 * (height / 10), .5,
                                               vec3{1, 1, 1});
            } else if (opponentTurn && counter) {
                string message = "Press 'C' to counter!";
                this->fontRenderer->renderText(message, width / 3.45 - message.length(), 7 * (height / 10), .5,
                                               vec3{1, 1, 1});
            }

            if (playerTurn && opponentShaken) {
                string message = "Opponent shaken!";
                this->fontRenderer->renderText(message, width / 3.45 - message.length(), 1 * (height / 10), .5,
                                               vec3{1, 1, 1});
            } else if (opponentTurn && playerShaken) {
                string message = "Player shaken!";
                this->fontRenderer->renderText(message, width / 3.45 - message.length(), 8 * (height / 10), .5,
                                               vec3{1, 1, 1});
            }


            if (playerTurn && !counter) {
                string message = "Press 'P' to punch or 'B' to block!";
                this->fontRenderer->renderText(message, width / 3.45 - message.length(), 2 * (height / 10), .5,
                                               vec3{1, 1, 1});
            }


            if (opponentTurn && !counter) {
                string message = "Press 'S' to punch or 'V' to block!";
                this->fontRenderer->renderText(message, width / 3.45 - message.length(), 7 * (height / 10), .5,
                                               vec3{1, 1, 1});
            }
            break;
        }

        // displaying the message you get when winning the arcade
        case arcade_win: {
            string message = "You are the champion!";
            // DONE: Display the message on the screen
            this->fontRenderer->renderText(message, width / 2 - (12 * message.length()), height / 2, 1,
                                           vec3{1, 1, 1});
            break;
        }

        // displaying message you get when winning in versus

        case over_win: {
            string message = "You knocked out your opponent!";
            // DONE: Display the message on the screen
            this->fontRenderer->renderText(message, width / 2 - (12 * message.length()), height / 2, 1,
                                           vec3{1, 1, 1});
            break;
        }

        // displaying message you get when losing any time
        case over_lose: {
            string message = "Your opponent knocked you out!";
            // DONE: Display the message on the screen
            this->fontRenderer->renderText(message, width / 2 - (12 * message.length()), height / 2, 1,
                                           vec3{1, 1, 1});
            break;
        }

        case exit_screen: {
            string message = "Goodbye!";
            // DONE: Display the message on the screen
            this->fontRenderer->renderText(message, width / 2 - (12 * message.length()), height / 2, 1,
                                           vec3{1, 1, 1});
            break;
        }
    }
    glfwSwapBuffers(window);
}


bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}