# C++ Boxing
### Contributors: Steven De Jesus

### Summary of program:

The program uses OpenGL, file input/output handling,
classes, and many other concepts to create a fleshed out
boxing game where you have the option to either
create a new character or load a saved one, and choose between an
arcade mode (multiple fights in a row) and a versus mode (a single fight where you choose your opponent). 
More specifically, you are able to create your own character by
inputting your name, and the desired weightclass you want your boxer to be.
The game is turn based, so while playing, you have the option to either punch or block, then your opponent
attacks, and the game continues until one person loses. You can either play single player and control the inputs
of both the player and opponent, or you can play with somebody else, as you control one player and someone else
controls the opponent. The game has some real boxing logic in it as well, as 
blocking may leave the player "shaken", an effect where they're essentially off balance
and the opponent gets another chance of attack while the player is open. This opens the door for some strategy during the 
gameplay.

### Concepts:
Classes (header and implementation): Here, I utilize classes to define the Boxer class, and implement it in the separate implementation file.


Enum: Here, I use enum to define the state of both the boxer, with its respective weight classes and corresponding values such as health and power, and the state of the screen during gameplay.


Input validation: There is a few instances of input validation here in the Boxer.cpp file, but not too much since GLFW has strong input validation since you can determine what happens based on events that happen, and if those specific events don't happen, the program won't carry on.


File input and output: In this program, I heavily use file input and output to read from various txt files which hold information for opponents in the arcade mode, in the versus mode, and in the users file. I also use it to save the data of a user when a character is created, and it is then accessed when loading characters from that txt file.


Graphics: This project relies heavily on all the graphics technology and skills we learned in module 4, since the main force of this project is its graphical interface.

### Operating system ran on: MacOS Sonoma 14.1

### No extra installations needed

### Make sure that the data folder is in the same directory as the Boxer.cpp file, as it relies on being in the same directory to read and write to those files.

### Bugs: Only known bug is that if the player and opponent press their respective keys at the same time or close enough, the characters will inch off the screen.

