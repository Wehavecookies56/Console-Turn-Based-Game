#include "stdafx.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <random>
#include <conio.h>

using namespace std;
random_device random;
mt19937 numberGen(random());

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

class Creature {

public: 
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
	Creature(int maxHealth, string name) {
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

void processChoice(char choice, Creature &player, Creature &enemy);
void doTurn(Creature &player, Creature &enemy);

/*	
	Displays the actions the player can perform and take input for the choice
	&player: instance of the player
	&enemy: instance of the enemy

*/	
void menu(Creature &player, Creature &enemy) {
	cout << "0. Attack" << endl;
	cout << "1. Special" << endl;
	cout << "2. Recharge" << endl;
	cout << "3. Dodge" << endl;
	cout << "4. Heal" << endl;
	char choice = _getch();
	processChoice(choice, player, enemy);
}

/*
	Function to end the turn of either the player or the enemy
*/
void endTurn(Creature &player, Creature &enemy) {
	if (player.getHealth() <= 0) {
		cout << player.getName() << " has died" << endl;
		return;
	} else if (enemy.getHealth() <= 0) {
		cout << enemy.getName() << " has died" << endl;
		return;
	} else {
		cout << "not dead" << endl;
		player.rechargeEnergy();
		enemy.rechargeEnergy();
		if (player.isTurn) {
			cout << endl;
			cout << ">> Player turn end" << endl;
			cout << endl;
			player.isTurn = false;
			enemy.isTurn = true;
			player.healed = false;
		}
		else {
			cout << endl;
			cout << ">> Enemy turn end" << endl;
			cout << endl; 
			player.isTurn = true;
			enemy.isTurn = false;
			enemy.healed = false;
		}
		doTurn(player, enemy);
	}
}

void makeDecision(Creature &player, Creature &enemy);

void doTurn(Creature &player, Creature &enemy) {
	if (player.isTurn) {
		if (player.recharged || player.dodged) {
			player.rechargeRate = CREATURE_DEFAULT_RECHARGE_RATE;
			player.hitChanceModifier = CREATURE_DEFAULT_HIT_CHANCE;
			player.dodged = false;
			player.recharged = false;
		}
		cout << endl;
		cout << ">> Player turn start" << endl;
		cout << "HP: " << player.getHealth() << "/" << player.maxHealth << " E:" << player.getEnergy() << "/" << player.maxEnergy << endl;
		cout << endl;
		menu(player, enemy);
	} else {
		if (enemy.recharged || enemy.dodged) {
			enemy.rechargeRate = CREATURE_DEFAULT_RECHARGE_RATE;
			enemy.hitChanceModifier = CREATURE_DEFAULT_HIT_CHANCE;
			enemy.recharged = false;
			enemy.dodged = false;
		}
		cout << endl;
		cout << ">> Enemy turn start" << endl;
		cout << "HP: " << enemy.getHealth() << "/" << enemy.maxHealth << " E:" << enemy.getEnergy() << "/" << enemy.maxEnergy << endl;
		cout << endl;
		cout << "make decision" << endl;
		makeDecision(player, enemy);
	}
}

void recharge(Creature &self, Creature &player, Creature &enemy) {
	self.rechargeRate *= 4;
	self.hitChanceModifier += 10;
	self.recharged = true;
	cout << self.getName() << " recharged" << endl;
	endTurn(player, enemy);
}

void dodge(Creature &self, Creature &player, Creature &enemy) {
	self.rechargeRate /= 2;
	self.hitChanceModifier -= 30;
	self.dodged = true;
	cout << self.getName() << " dodged" << endl;
	endTurn(player, enemy);
}


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
bool attack(Creature &attacker, Creature &target, Creature &player, Creature &enemy, int cost, int hitChance, int minDamage, int maxDamage) {
	if (player.getHealth() <= 0 || enemy.getHealth() <= 0) {
		cout << "This shouldn't happen in the first place" << endl;
		return false;
	} else {
		// Check if the attacker has enough energy to use the attack
		if (attacker.getEnergy() >= cost) {
			// Remove the energy used by the attack
			attacker.removeEnergy(cost);
			// Generate random number between 0 and 100 for a percentage hit chance, this makes each number have an equal chance of generating
			uniform_int_distribution<> hitChanceRange(0, 100);
			// Generate the random number in the range using the random number generator
			int hit = hitChanceRange(numberGen);
			// Check whether the attack hit with the hit chance and the target's hit chance modifier allowing for a greater number of numbers within the 0 to 100 range of being a successful hit
			if (hit >= (100 - hitChance + target.hitChanceModifier)) {
				// Generate a random number within the damage range
				uniform_int_distribution<> damageRange(minDamage, maxDamage);
				int damage = damageRange(numberGen);
				// Damage the target
				target.takeDamage(damage);
				cout << ">> Hit for " << damage << " damage, " << target.getName() << "'s health is now " << target.getHealth() << "/" << target.maxHealth << endl;
				// Display the energy cost of the attack if it has a cost
				if (cost > 0)
					cout << ">> Cost " << cost << " energy " << attacker.getName() << " has " << attacker.getEnergy() << " energy remaining" << endl;
			}
			else {
				cout << ">> Missed!" << endl;
			}
			endTurn(player, enemy);
			return true;
		}
		cout << "Not enough energy to use attack" << endl;
		return false;
	}
}

void heal(Creature &creature, Creature &player, Creature &enemy) {
	int amountToHeal = creature.getEnergy() / 2;
	if (creature.getHealth() + amountToHeal > creature.maxHealth)
		amountToHeal = creature.maxHealth - creature.getHealth();
	creature.heal(amountToHeal);
	creature.removeEnergy(amountToHeal);
	cout << ">> Healed " << creature.getName() << " for " << amountToHeal << " HP" << endl;
	cout << creature.getName() << "'s HP is now " << creature.getHealth() << endl;
	creature.healed = true;
	doTurn(player, enemy);
}

enum commands { ATTACK = '0', SPECIAL = '1', RECHARGE = '2', DODGE = '3', HEAL = '4' };

/*
	Responds to the player's choice
	choice: the key pressed by the player
	&player: the player instance
	&enemy: the enemy instance
*/
void processChoice(char choice, Creature &player, Creature &enemy) {
	switch (choice) {
	case ATTACK: 
		cout << "attack 1" << endl;
		attack(player, enemy, player, enemy, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
		break;
	case SPECIAL:
		cout << "attack 2" << endl;
		// Use the special attack, if the player doesn't have enough energy display the menu to ask for input again
		if (!attack(player, enemy, player, enemy, SPECIAL_COST, SPECIAL_HIT_CHANCE, SPECIAL_HIT_MIN, SPECIAL_HIT_MAX)) {
			menu(player, enemy);
		}
		break;
	case RECHARGE: 
		recharge(player, player, enemy);
		break;
	case DODGE:
		dodge(player, player, enemy);
		break;
	case HEAL: 
		// Check if the player has already healed so they can only heal once per turn
		if (!player.healed)
			heal(player, player, enemy);
		else {
			cout << ">> Already healed this turn" << endl;
			menu(player, enemy);
		}
		break;
	default:
		// Display the menu again as the input was invalid
		cout << ">> Invalid choice" << endl;
		menu(player, enemy);
		break;
	}
	cout << endl;
}


void makeDecision(Creature &player, Creature &enemy) {
	cout << "this doesn't show at the end" << endl;
	if (player.dodged) {
		if (enemy.getHealth() <= 50 && enemy.getHealth() >= 25) {
			if (!enemy.healed)
				heal(enemy, player, enemy);
			if (enemy.getHealth() >= 50) {
				cout << "attack 3" << endl;
				attack(enemy, player, player, enemy, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
			} else {
				recharge(enemy, player, enemy);
			}
		} else if (enemy.getHealth() <= 25) {
			if (!enemy.healed)
				heal(enemy, player, enemy);
			dodge(enemy, player, enemy);
		}
	} else if (player.recharged) {
		if (enemy.getEnergy() == enemy.maxEnergy) {
			cout << "attack 4" << endl;
			attack(enemy, player, player, enemy, SPECIAL_COST, SPECIAL_HIT_CHANCE, SPECIAL_HIT_MIN, SPECIAL_HIT_MAX);
		} else {
			cout << "attack 5" << endl;
			attack(enemy, player, player, enemy, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
		}
	} else {
		if (enemy.getEnergy() == enemy.maxEnergy) {
			cout << "attack 6" << endl;
			attack(enemy, player, player, enemy, SPECIAL_COST, SPECIAL_HIT_CHANCE, SPECIAL_HIT_MIN, SPECIAL_HIT_MAX);
		} else if (enemy.getHealth() <= 50 && enemy.getHealth() >= 25) {
			if (!enemy.healed)
				heal(enemy, player, enemy);
			cout << "attack 7" << endl;
			attack(enemy, player, player, enemy, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
		} else if (enemy.getHealth() <= 25) {
			if (!enemy.healed)
				heal(enemy, player, enemy);
			dodge(enemy, player, enemy);
		} else {
			cout << "attack 8" << endl;
			attack(enemy, player, player, enemy, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
		}
	}
	cout << "this does" << endl;
	cout << "attack 9" << endl;
	attack(enemy, player, player, enemy, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
}

int main() {
	// Instantiate the player and enemy objects
	Creature playerCreature(CREATURE_MAX_HEALTH, "Player");
	Creature enemyCreature(CREATURE_MAX_HEALTH, "Computer");
	// Set player turn to true so the player starts first
	playerCreature.isTurn = true;
	// Start the turns
	doTurn(playerCreature, enemyCreature);
	// Take input at the end to prevent the game from instantly closing at the end
	cin.ignore();
	_getch();
    return 0;
}

