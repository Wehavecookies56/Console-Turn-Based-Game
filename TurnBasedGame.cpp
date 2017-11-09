#include "stdafx.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <random>
#include <conio.h>
#include <Windows.h>
#include <algorithm>

using namespace std;

void clearScreen(HANDLE &handle, short startRow);
void clearScreen(HANDLE &handle, short startRow, short endRow);

const int CREATURE_MAX_HEALTH = 100;
const int CREATURE_MAX_ENERGY = 50;
const int CREATURE_DEFAULT_RECHARGE_RATE = 2;
const int CREATURE_DEFAULT_HIT_CHANCE = 0;
const int ATTACK_HIT_CHANCE = 80;
const int SPECIAL_HIT_CHANCE = 50;
const int ATTACK_HIT_MIN = 1;
const int ATTACK_HIT_MAX = 10;
const int SPECIAL_HIT_MIN = 5;
const int SPECIAL_HIT_MAX = 20;
const int ATTACK_COST = 0;
const int SPECIAL_COST = 50;
const int FIRST_MENU_ITEM = 0;
const int LAST_MENU_ITEM = 4;

class Creature {

public: 
	// ID of the creature
	short id;
	// The current health
	int currHealth;
	// The maximum health
	int maxHealth;
	// The current energy
	int energy;
	// The maximum energy
	int maxEnergy;
	// The percentage to subtract from the hit chance
	int hitChanceModifier;
	// The amount of energy to gain per turn
	int rechargeRate;
	// Whether the creature recharged during their turn
	bool recharged;
	// Whether the creature dodged during their turn
	bool dodged;
	// Whether the creature healed during their turn
	bool healed;
	// Whether it is the creature's turn
	bool isTurn;
	// The name to display for the creature
	string name;

public:
	Creature(short id, int maxHealth, string name) {
		this->id = id;
		this->maxHealth = maxHealth;
		this->name = name;
		currHealth = maxHealth;
		maxEnergy = CREATURE_MAX_ENERGY;
		energy = maxEnergy;
		hitChanceModifier = CREATURE_DEFAULT_HIT_CHANCE;
		rechargeRate = CREATURE_DEFAULT_RECHARGE_RATE;
		recharged = false;
		dodged = false;
		healed = false;
		isTurn = false;
	}

	// Returns the current health of the creature
	int getHealth() {
		return currHealth;
	}

	/*
		Subtract the specified amount of health from the creature, prevents health from dropping below 0
		damage: the amount to subtract
	*/
	void takeDamage(int damage) {
		if (currHealth - damage < 0)
			currHealth = 0;
		else
			currHealth -= damage;
	}

	/*
		Adds the specified amount of health to the creature, prevents the health from going above maxHealth
		amount: the amount of health to heal
	*/
	void heal(int amount) {
		if (currHealth + amount > maxHealth)
			currHealth = maxHealth;
		else
			currHealth += amount;
	}
	/*
		Add rechargeRate to the creature's energy, keeps the energy from going above maxEnergy
	*/
	void rechargeEnergy() {
		if (energy + rechargeRate > maxEnergy)
			energy = maxEnergy;
		else
			energy += rechargeRate;
	}

	/* 
		Subtract the specified amount of energy from the creature, prevents energy from dropping below 0
		amount: the amount to subtract
	*/
	void removeEnergy(int amount) {
		if (energy - amount < 0)
			energy = 0;
		else
			energy -= amount;
	}

	// Returns the energy of the creature
	int getEnergy() {
		return energy;
	}

	// Returns the name of the creature
	string getName() {
		return name;
	}

};

/*
	Generate a random number within a given range
	min: minimum value to generate
	max: maximum value to generate
	returns the random number
*/
int randInt(int min, int max) {
	random_device random;
	mt19937 seed(random());
	uniform_int_distribution<> rand(min, max);
	return rand(seed);
}

bool processChoice(char choice, Creature &player, Creature &enemy, int &selected);
void doTurn(Creature &player, Creature &enemy, short turnID);

enum Colours { BLACK = 0, DARK_BLUE = 1, DARK_GREEN = 2, TURQOUSE = 3, DARK_RED = 4, DARK_PURPLE = 5, GOLD = 6, WHITE = 7, GREY = 8, BLUE = 9, GREEN = 10, CYAN = 11, RED = 12, PURPLE = 13, YELLOW = 14};

/*
	Wait for user input and display continue option and player and enemy stats then clears the screen after the user presses any button
	originalPos: the cursor position when this function is called
	&player: the instance of the player
	&enemy: the instance of the enemy
*/
void pause(COORD originalPos, Creature &player, Creature &enemy) {
	// Get handle and screen buffer info
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	// Clear bottom section of the screen
	clearScreen(handle, info.srWindow.Bottom - 2, info.srWindow.Bottom - 1);
	// Set cursor position to row with player and enemy stats
	SetConsoleCursorPosition(handle, { 0, info.srWindow.Bottom - 2 });
	// Set text colour to green
	SetConsoleTextAttribute(handle, GREEN);
	// Display player stats
	cout << " " << player.getName() << ": HP:" << player.getHealth() << "/" << player.maxHealth << " E:" << player.getEnergy() << "/" << player.maxEnergy << endl;
	// Text to display for enemy stats
	string enemyInfo = enemy.getName() + ": HP:" + to_string(enemy.getHealth()) + "/" + to_string(enemy.maxHealth) + " E:" + to_string(enemy.getEnergy()) + "/" + to_string(enemy.maxEnergy);
	// Set cursor position to the right side of the screen minus the length of the enemy stats string on the same row as the player stats
	SetConsoleCursorPosition(handle, { info.srWindow.Right - (short) enemyInfo.length(), info.srWindow.Bottom - 2 });
	// Set text to red
	SetConsoleTextAttribute(handle, RED);
	// Display enemy stats
	cout << enemyInfo << endl;
	// Set cursor position to bottom row
	SetConsoleCursorPosition(handle, { 0, info.srWindow.Bottom - 1 });
	// Display continue with white square brackets and yellow text for continue
	SetConsoleTextAttribute(handle, WHITE);
	cout << "[";
	SetConsoleTextAttribute(handle, YELLOW);
	cout << "Continue";
	SetConsoleTextAttribute(handle, WHITE);
	cout << "]" << endl;
	// Set cursor position back to the original position
	SetConsoleCursorPosition(handle, originalPos);
	// Wait for user input
	_getch();
	// Clear the screen
	clearScreen(handle, 0);
}

// Key codes for arrow keys and enter
enum KEYS { UP = 72, RIGHT = 77, DOWN = 80, LEFT = 75, ENTER = 13, ARROWS = 244};


/*	
	Displays the actions the player can perform and take input for the choice
	&player: instance of the player
	&enemy: instance of the enemy
	selected: the menu item selected
*/	
void menu(Creature &player, Creature &enemy, int selected) {
	int choice = 0;
	do {
		// Get the handle
		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(handle, &info);
		// Create an arrow of strings for menu options with spaces before and after each word
		string menuItems[5] = { " Attack ", " Special ", " Recharge ", " Dodge ", " Heal " };
		// Remove the spaces from the selected item in the array
		menuItems[selected].erase((menuItems[selected]).length() - 1, (menuItems[selected]).length());
		menuItems[selected].erase(0, 1);

		// Store the current cursor position
		COORD cursorPos = info.dwCursorPosition;

		// Clear the bottom section of the screen
		clearScreen(handle, info.srWindow.Bottom - 2, info.srWindow.Bottom - 1);
		// Set cursor position to the row for the player and enemy stats
		SetConsoleCursorPosition(handle, { 0, info.srWindow.Bottom - 2 });
		// Set the text colour to green
		SetConsoleTextAttribute(handle, GREEN);
		// Display the player stats
		cout << " " << player.getName() << ": HP:" << player.getHealth() << "/" << player.maxHealth << " E:" << player.getEnergy() << "/" << player.maxEnergy << endl;
		// Create string for enemy stats
		string enemyInfo = enemy.getName() + ": HP:" + to_string(enemy.getHealth()) + "/" + to_string(enemy.maxHealth) + " E:" + to_string(enemy.getEnergy()) + "/" + to_string(enemy.maxEnergy);
		// Set cursor position to the right side of the screen minus the length of the enemy stats string on the same row as the player stats
		SetConsoleCursorPosition(handle, { info.srWindow.Right - (short)enemyInfo.length(), info.srWindow.Bottom - 2 });
		// Set the text colour to red
		SetConsoleTextAttribute(handle, RED);
		// Display the enemy stats
		cout << enemyInfo << endl;
		// Set the cursor position to the bottom row
		SetConsoleCursorPosition(handle, { 0, info.srWindow.Bottom - 1 });
		// Loop through the array of menu items
		for (int i = 0; i < sizeof(menuItems) / sizeof(menuItems[0]); i++) {
			// If the item is selected
			if (i == selected) {
				// Display selected item with white square brackets and yellow text
				SetConsoleTextAttribute(handle, WHITE);
				cout << "[";
				SetConsoleTextAttribute(handle, YELLOW);
				cout << menuItems[i];
				SetConsoleTextAttribute(handle, WHITE);
				cout << "]";
				// Else the item is not selected
			}
			else {
				// Display non selected item as grey
				SetConsoleTextAttribute(handle, GREY);
				cout << menuItems[i];
				// Set text back to white
				SetConsoleTextAttribute(handle, WHITE);
			}
		}
		// Set cursor position back to original position and display text
		SetConsoleCursorPosition(handle, cursorPos);
		cout << ">> Select a command" << endl;
		// Set it back again
		SetConsoleCursorPosition(handle, cursorPos);
		// Clear the input
		cin.clear();
		// Get the user input
		choice = _getch();
		// If input is an arrow key (224)
		if (choice == ARROWS) {
			// Get the arrow key input
			choice = _getch();
		}
		// Call process choice to respond to the input
	} while(processChoice(choice, player, enemy, selected));
}

/*
	Function to end the turn of either the player or the enemy and then start the next turn
	&player: instance of the player
	&enemy: instance of the enemy
	turnID: ID of the creature whose turn it is
*/
void endTurn(Creature &player, Creature &enemy, short turnID) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	// Check if the player is dead
	if (player.getHealth() <= 0) {
		cout << ">> " << player.getName() << " has died" << endl;
		pause(info.dwCursorPosition, player, enemy);
		return;
	// Check if the enemy is dead
	} else if (enemy.getHealth() <= 0) {
		cout << ">> " << enemy.getName() << " has died" << endl;
		pause(info.dwCursorPosition, player, enemy);
		return;
	// If neither are dead, proceed
	} else {
		// Recharge the energy for the turn
		player.rechargeEnergy();
		enemy.rechargeEnergy();
		// Check if it's the player'sturn
		if (player.id == turnID) {
			cout << endl;
			cout << ">> Player turn end" << endl;
			pause(info.dwCursorPosition, player, enemy);
			cout << endl;
			// Switch to enemy turn
			player.healed = false;
			return;
		// Otherwise it's the enemies turn
		} else {
			cout << endl;
			cout << ">> Enemy turn end" << endl;
			pause(info.dwCursorPosition, player, enemy);
			cout << endl; 
			// Switch to player turn
			enemy.healed = false;
			return;
		}
	}
}

void makeDecision(Creature &player, Creature &enemy);

/*
	Start the turn for either the player or enemy
	&player: instance of the player
	&enemy: instance of the enemy
	turnID: ID of the creature whose turn it is
*/
void doTurn(Creature &player, Creature &enemy, short turnID) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	short currentTurn = turnID;
	// Turn counter
	int turnNumber = 1;
	while (player.getHealth() > 0 && enemy.getHealth() > 0) {
		clearScreen(handle, 0);
		cout << ">> Turn " << turnNumber << endl;
		// Check if it's the player's turn
		if (currentTurn == player.id) {
			// Reset changed stats after recharging or dodging the previous turn
			if (player.recharged || player.dodged) {
				player.rechargeRate = CREATURE_DEFAULT_RECHARGE_RATE;
				player.hitChanceModifier = CREATURE_DEFAULT_HIT_CHANCE;
				player.dodged = false;
				player.recharged = false;
			}
			// Display the player's current stats
			cout << endl;
			cout << ">> Player turn start" << endl;
			pause(info.dwCursorPosition, player, enemy);
			cout << endl;
			// Display the menu
			menu(player, enemy, FIRST_MENU_ITEM);
			currentTurn = enemy.id;
			turnNumber++;
			continue;
		// Enemy's turn
		} else {
			// Reset changed stats after recharging or dodging the previous turn
			if (enemy.recharged || enemy.dodged) {
				enemy.rechargeRate = CREATURE_DEFAULT_RECHARGE_RATE;
				enemy.hitChanceModifier = CREATURE_DEFAULT_HIT_CHANCE;
				enemy.recharged = false;
				enemy.dodged = false;
			}
			// Display the enemy's current stats
			cout << endl;
			cout << ">> Enemy turn start" << endl;
			pause(info.dwCursorPosition, player, enemy);
			cout << endl;
			// Make the enemy's decision
			makeDecision(player, enemy);
			currentTurn = player.id;
			turnNumber++;
			continue;
		}
	}
}

/*
	Multiplies the user's recharge rate by 4 until their next turn and makes them 10% more likely to be hit
	&self: instance of the user
	&player: instance of the player
	&enemy: instance of the enemy
*/
void recharge(Creature &self, Creature &player, Creature &enemy) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	self.rechargeRate *= 4;
	self.hitChanceModifier += 10;
	self.recharged = true;
	cout << ">> " << self.getName() << " recharged" << endl;
	pause(info.dwCursorPosition, player, enemy);
	// End the turn
	endTurn(player, enemy, self.id);
}

/*
	Halves the user's recharge rate and makes them 30% less likely to be hit until their next turn
	&self: instance of the user
	&player: instance of the player
	&enemy: instance of the enemy
*/
void dodge(Creature &self, Creature &player, Creature &enemy) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	self.rechargeRate /= 2;
	self.hitChanceModifier -= 30;
	self.dodged = true;
	cout << ">> " << self.getName() << " dodged" << endl;
	pause(info.dwCursorPosition, player, enemy);
	// End the turn
	endTurn(player, enemy, self.id);
}

struct Attack {
	int cost{0};
	int hitChance{0};
	int minDamage{0};
	int maxDamage{0};
};

const Attack standard = { ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX };
const Attack special = { SPECIAL_COST, SPECIAL_HIT_CHANCE, SPECIAL_HIT_MIN, SPECIAL_HIT_MAX };

/*
	Function to have a creature attack another creature
	&attacker: Instance of the creature that is attacking
	&target: Instance of the creature being attacked
	&player: Instance of the player
	&enemy: Instance of the enemy
	cost: the energy cost of the attack
	hitChance: percentage chance of the attack hitting
	minDamage: the minimum amount of damage the attack can deal
	maxDamage: the maximum amount of damage the attack can deal
	return whether the attack performed and thus should end the turn
*/
bool attack(Creature &attacker, Creature &target, Creature &player, Creature &enemy, Attack attack) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	// Check if the attacker has enough energy to use the attack
	if (attacker.getEnergy() >= attack.cost) {
		// Remove the energy used by the attack
		attacker.removeEnergy(attack.cost);
		// Generate random number between 0 and 100 for a percentage hit chance, this makes each number have an equal chance of generating
		// Generate the random number in the range using the random number generator
		int hit = randInt(0, 100);
		// Check whether the attack hit with the hit chance and the target's hit chance modifier allowing for a greater number of numbers within the 0 to 100 range of being a successful hit
		if (hit >= (100 - attack.hitChance + target.hitChanceModifier)) {
			// Generate a random number within the damage range
			int damage = randInt(attack.minDamage, attack.maxDamage);
			// Damage the target
			target.takeDamage(damage);
			cout << ">> Hit for " << damage << " damage, " << target.getName() << "'s health is now " << target.getHealth() << "/" << target.maxHealth << endl;
			// Display the energy cost of the attack if it has a cost
			if (attack.cost > 0)
				cout << ">> Cost " << attack.cost << " energy " << attacker.getName() << " has " << attacker.getEnergy() << " energy remaining" << endl;
			pause(info.dwCursorPosition, player, enemy);
		}
		else {
			cout << ">> " << attacker.getName() << "'s attack missed!" << endl;
			pause(info.dwCursorPosition, player, enemy);
		}
		// End the turn
		endTurn(player, enemy, attacker.id);
		return true;
	}
	// Not enough energy so the attack was not successful and the turn is not ended
	cout << "Not enough energy to use attack" << endl;
	pause(info.dwCursorPosition, player, enemy);
	return false;
}

/*
	Heals the user using half their energy, does not end the turn, however cannot be used more than once per turn
	&creature: instance of the user
	&player: instance of the player
	&enemy: instance of the enemy
*/
void heal(Creature &creature, Creature &player, Creature &enemy) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	// Get the amount to heal from half the user's energy
	int amountToHeal = creature.getEnergy() / 2;
	// Check if the amount to heal will result in health greater than their max health
	if (creature.getHealth() + amountToHeal > creature.maxHealth)
		// Amount to heal reduced to the difference between their max health and curren health so less energy is used to heal
		amountToHeal = creature.maxHealth - creature.getHealth();
	// Heal the user
	creature.heal(amountToHeal);
	// Remove the energy used
	creature.removeEnergy(amountToHeal);
	cout << ">> Healed " << creature.getName() << " for " << amountToHeal << " HP" << endl;
	cout << creature.getName() << "'s HP is now " << creature.getHealth() << endl;
	pause(info.dwCursorPosition, player, enemy);
	creature.healed = true;
	// Show the menu again so the player can perform another action, only necessary for player as the enemy does not use the menu
	if (creature.id == player.id)
		menu(player, enemy, FIRST_MENU_ITEM);
}

enum commands { ATTACK = 0, SPECIAL = 1, RECHARGE = 2, DODGE = 3, HEAL = 4 };

/*
	Responds to the player's choice
	choice: the key pressed by the player
	&player: the player instance
	&enemy: the enemy instance
	returns true if the menu should be shown again
*/
bool processChoice(char choice, Creature &player, Creature &enemy, int &selected) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	switch (choice) {
	case UP:
	case LEFT:
		// If the selected item on the menu is the first, go to the last
		if (selected == FIRST_MENU_ITEM) {
			selected = LAST_MENU_ITEM;
		// Else decrease selected to move left
		} else {
			selected--;
		}
		// Update menu
		return true;
	case DOWN:
	case RIGHT:
		// If the selected item on the menu is the last, go to the first
		if (selected == LAST_MENU_ITEM) {
			selected = FIRST_MENU_ITEM;
		// Else increase selected to move right
		} else {
			selected++;
		}
		// Update menu
		return true;
	case ENTER:
		switch (selected) {
			case ATTACK:
				// Use the standard attack
				attack(player, enemy, player, enemy, standard);
				return false;
			case SPECIAL:
				// Use the special attack, if the player doesn't have enough energy display the menu to ask for input again
				if (!attack(player, enemy, player, enemy, special)) {
					menu(player, enemy, selected);
				}
				return false;
			case RECHARGE:
				// Recharge energy
				recharge(player, player, enemy);
				return false;
			case DODGE:
				// Dodge to reduce hit chance of the opponent
				dodge(player, player, enemy);
				return false;
			case HEAL:
				// Check if the player has already healed so they can only heal once per turn
				if (!player.healed)
					heal(player, player, enemy);
				else {
					cout << ">> Already healed this turn" << endl;
					pause(info.dwCursorPosition, player, enemy);
					// Display the menu again 
					return true;
				}
				return false;
			}
		break;
	default:
		// Display the menu again as the input was invalid
		return true;
	}
	cout << endl;
	return true;
}

/*
	Performs the enemy's action by checking certain conditions
	&player: instance of the player
	&enemy: instance of the enemy
*/
void makeDecision(Creature &player, Creature &enemy) {
	// Check if the player has dodged
	if (player.dodged) {
		// Check if the enemy has between 25 and 50 health
		if (enemy.getHealth() <= 50 && enemy.getHealth() >= 25) {
			// Check if the enemy has healed
			if (!enemy.healed) {
				// Heal the enemy
				heal(enemy, player, enemy);
				// Check if the enemy has greater than 50 health after healing, attack if so otherwise recharge
				if (enemy.getHealth() > 50) {
					attack(enemy, player, player, enemy, standard);
					return;
				} else {
					recharge(enemy, player, enemy);
					return;
				}
			} else {
				recharge(enemy, player, enemy);
				return;
			}
		// If the enemy has less than 25 health heal and dodge
		} else if (enemy.getHealth() < 25) {
			if (!enemy.healed)
				heal(enemy, player, enemy);
			dodge(enemy, player, enemy);
			return;
		// Otherwise recharge
		} else {
			recharge(enemy, player, enemy);
			return;
		}
	// Check if the player recharged their turn
	} else if (player.recharged) {
		// Check if the enemy has max energy
		if (enemy.getEnergy() == enemy.maxEnergy) {
			// Use the special attack
			attack(enemy, player, player, enemy, special);
			return;
		} else {
			//Otherwise use the standard attack
			attack(enemy, player, player, enemy, standard);
			return;
		}
	// Otherwise if the player didn't recharge or dodge
	} else {
		// Check if the enemy has max energy
		if (enemy.getEnergy() == enemy.maxEnergy) {
			// Use special attack
			attack(enemy, player, player, enemy, special);
			return;
		// If the enemy has between 50 and 25 health
		} else if (enemy.getHealth() <= 50 && enemy.getHealth() >= 25) {
			// Heal if they haven't
			if (!enemy.healed)
				heal(enemy, player, enemy);
			// Use the standard attack
			attack(enemy, player, player, enemy, standard);
			return;
		// Otherwise if the enemy has less than 25 health
		} else if (enemy.getHealth() < 25) {
			// Heal if they haven't already, then dodge
			if (!enemy.healed)
				heal(enemy, player, enemy);
			dodge(enemy, player, enemy);
			return;
		// Otherwise recharge
		} else {
			recharge(enemy, player, enemy);
			return;
		}
	}
	// If no condition is met somehow, use the standard attack so the enemy does at least something every turn
	attack(enemy, player, player, enemy, standard);
	return;
}

/*
	Clear the screen within a specific area of the console with the start and end row
	&handle: the handle
	startRow: the row to start clearing from
	endRow: the row to stop clearing from
*/
void clearScreen(HANDLE &handle, short startRow, short endRow) {
	// Get the screen info
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	DWORD d = 0;
	// Fill the console with nothing from start row to end row
	FillConsoleOutputCharacter(handle, ' ', info.srWindow.Right * endRow, { 0, startRow }, &d);
	// Reset cursor position
	SetConsoleCursorPosition(handle, { 0, 0 });
}

/*
	Clear the screen from a starting row until the bottom of the window
	&handle: the handle
	startRow: the row to start clearing from
*/
void clearScreen(HANDLE &handle, short startRow) {
	// Get the screen info
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	// Clear console with end row as the bottom of the window
	clearScreen(handle, startRow, info.srWindow.Bottom);
}

int main() {
	// Get the handle
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	// Set the window title
	SetConsoleTitle(TEXT("Turn Based Game"));
	// Instantiate the player and enemy objects
	Creature playerCreature(0, CREATURE_MAX_HEALTH, "Player");
	Creature enemyCreature(1, CREATURE_MAX_HEALTH, "Computer");
	// Set player turn to true so the player starts first
	cout << "Welcome to this turn based game" << endl;
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	// Wait for user input
	pause(info.dwCursorPosition, playerCreature, enemyCreature);
	cout << "Use the up and down arrows to navigate the menus and enter to select" << endl;
	// Wait for user input
	pause(info.dwCursorPosition, playerCreature, enemyCreature);
	// Clear the screen
	clearScreen(handle, 0);
	// Start the turns
	doTurn(playerCreature, enemyCreature, playerCreature.id);
	// Take input at the end to prevent the game from instantly closing at the end
	cin.ignore();
	_getch();
    return 0;
}