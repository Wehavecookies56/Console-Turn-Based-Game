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
	Function to end the turn of either the player or the enemy and then start the next turn
	&player: instance of the player
	&enemy: instance of the enemy
*/
void endTurn(Creature &player, Creature &enemy) {
	// Check if the player is dead
	if (player.getHealth() <= 0) {
		cout << player.getName() << " has died" << endl;
		return;
	// Check if the enemy is dead
	} else if (enemy.getHealth() <= 0) {
		cout << enemy.getName() << " has died" << endl;
		return;
	// If neither are dead, proceed
	} else {
		// Recharge the energy for the turn
		player.rechargeEnergy();
		enemy.rechargeEnergy();
		// Check if it's the player'sturn
		if (player.isTurn) {
			cout << endl;
			cout << ">> Player turn end" << endl;
			cout << endl;
			// Switch to enemy turn
			player.isTurn = false;
			enemy.isTurn = true;
			player.healed = false;
		// Otherwise it's the enemies turn
		} else {
			cout << endl;
			cout << ">> Enemy turn end" << endl;
			cout << endl; 
			// Switch to player turn
			player.isTurn = true;
			enemy.isTurn = false;
			enemy.healed = false;
		}
		// Start the next turn
		doTurn(player, enemy);
	}
}

void makeDecision(Creature &player, Creature &enemy);

/*
	Start the turn for either the player or enemy
	&player: instance of the player
	&enemy: instance of the enemy
*/
void doTurn(Creature &player, Creature &enemy) {
	// Check if it's the player's turn
	if (player.isTurn) {
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
		cout << "HP: " << player.getHealth() << "/" << player.maxHealth << " E:" << player.getEnergy() << "/" << player.maxEnergy << endl;
		cout << endl;
		// Display the menu
		menu(player, enemy);
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
		cout << "HP: " << enemy.getHealth() << "/" << enemy.maxHealth << " E:" << enemy.getEnergy() << "/" << enemy.maxEnergy << endl;
		cout << endl;
		// Make the enemy's decision
		makeDecision(player, enemy);
	}
}

/*
	Multiplies the user's recharge rate by 4 until their next turn and makes them 10% more likely to be hit
	&self: instance of the user
	&player: instance of the player
	&enemy: instance of the enemy
*/
void recharge(Creature &self, Creature &player, Creature &enemy) {
	self.rechargeRate *= 4;
	self.hitChanceModifier += 10;
	self.recharged = true;
	cout << self.getName() << " recharged" << endl;
	// End the turn
	endTurn(player, enemy);
}

/*
	Halves the user's recharge rate and makes them 30% less likely to be hit until their next turn
	&self: instance of the user
	&player: instance of the player
	&enemy: instance of the enemy
*/
void dodge(Creature &self, Creature &player, Creature &enemy) {
	self.rechargeRate /= 2;
	self.hitChanceModifier -= 30;
	self.dodged = true;
	cout << self.getName() << " dodged" << endl;
	// End the turn
	endTurn(player, enemy);
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
	if (player.getHealth() <= 0 || enemy.getHealth() <= 0) {
		cout << "This shouldn't happen in the first place" << endl;
		return false;
	} else {
		// Check if the attacker has enough energy to use the attack
		if (attacker.getEnergy() >= attack.cost) {
			// Remove the energy used by the attack
			attacker.removeEnergy(attack.cost);
			// Generate random number between 0 and 100 for a percentage hit chance, this makes each number have an equal chance of generating
			uniform_int_distribution<> hitChanceRange(0, 100);
			// Generate the random number in the range using the random number generator
			int hit = hitChanceRange(numberGen);
			// Check whether the attack hit with the hit chance and the target's hit chance modifier allowing for a greater number of numbers within the 0 to 100 range of being a successful hit
			if (hit >= (100 - attack.hitChance + target.hitChanceModifier)) {
				// Generate a random number within the damage range
				uniform_int_distribution<> damageRange(attack.minDamage, attack.maxDamage);
				int damage = damageRange(numberGen);
				// Damage the target
				target.takeDamage(damage);
				cout << ">> Hit for " << damage << " damage, " << target.getName() << "'s health is now " << target.getHealth() << "/" << target.maxHealth << endl;
				// Display the energy cost of the attack if it has a cost
				if (attack.cost > 0)
					cout << ">> Cost " << attack.cost << " energy " << attacker.getName() << " has " << attacker.getEnergy() << " energy remaining" << endl;
			}
			else {
				cout << ">> Missed!" << endl;
			}
			// End the turn
			endTurn(player, enemy);
			return true;
		}
		// Not enough energy so the attack was not successful and the turn is not ended
		cout << "Not enough energy to use attack" << endl;
		return false;
	}
}

/*
	Heals the user using half their energy, does not end the turn, however cannot be used more than once per turn
	&creature: instance of the user
	&player: instance of the player
	&enemy: instance of the enemy
*/
void heal(Creature &creature, Creature &player, Creature &enemy) {
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
	creature.healed = true;
	// Show the menu again so the player can perform another action, only necessary for player as the enemy does not use the menu
	if (player.isTurn)
		menu(player, enemy);
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
		// Use the standard attack
		attack(player, enemy, player, enemy, standard);
		break;
	case SPECIAL:
		// Use the special attack, if the player doesn't have enough energy display the menu to ask for input again
		if (!attack(player, enemy, player, enemy, special)) {
			menu(player, enemy);
		}
		break;
	case RECHARGE: 
		// Recharge energy
		recharge(player, player, enemy);
		break;
	case DODGE:
		// Dodge to reduce hit chance of the opponent
		dodge(player, player, enemy);
		break;
	case HEAL: 
		// Check if the player has already healed so they can only heal once per turn
		if (!player.healed)
			heal(player, player, enemy);
		else {
			cout << ">> Already healed this turn" << endl;
			// Display the menu again 
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
				} else {
					recharge(enemy, player, enemy);
				}
			} else {
				recharge(enemy, player, enemy);
			}
		// If the enemy has less than 25 health heal and dodge
		} else if (enemy.getHealth() < 25) {
			if (!enemy.healed)
				heal(enemy, player, enemy);
			dodge(enemy, player, enemy);
		// Otherwise recharge
		} else {
			recharge(enemy, player, enemy);
		}
	// Check if the player recharged their turn
	} else if (player.recharged) {
		// Check if the enemy has max energy
		if (enemy.getEnergy() == enemy.maxEnergy) {
			// Use the special attack
			attack(enemy, player, player, enemy, special);
		} else {
			//Otherwise use the standard attack
			attack(enemy, player, player, enemy, standard);
		}
	// Otherwise if the player didn't recharge or dodge
	} else {
		// Check if the enemy has max energy
		if (enemy.getEnergy() == enemy.maxEnergy) {
			// Use special attack
			attack(enemy, player, player, enemy, special);
		// If the enemy has between 50 and 25 health
		} else if (enemy.getHealth() <= 50 && enemy.getHealth() >= 25) {
			// Heal if they haven't
			if (!enemy.healed)
				heal(enemy, player, enemy);
			// Use the standard attack
			attack(enemy, player, player, enemy, standard);
		// Otherwise if the enemy has less than 25 health
		} else if (enemy.getHealth() < 25) {
			// Heal if they haven't already, then dodge
			if (!enemy.healed)
				heal(enemy, player, enemy);
			dodge(enemy, player, enemy);
		// Otherwise recharge
		} else {
			recharge(enemy, player, player);
		}
	}
	// If no condition is met somehow, use the standard attack so the enemy does at least something every turn
	attack(enemy, player, player, enemy, standard);
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

