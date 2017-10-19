#include "stdafx.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <random>

using namespace std;
void processChoice(int choice);
bool playerTurn = true;
bool hasHealed = false;
random_device random;
mt19937 seed(random());

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

public: int currHealth;
	int maxHealth;
	int energy;
	int maxEnergy;
	int hitChanceModifier;
	int rechargeRate;
	bool recharged;
	bool dodged;
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
	}

	int getHealth() {
		return currHealth;
	}

	void takeDamage(int damage) {
		if (currHealth - damage < 0)
			currHealth = 0;
		else
			currHealth -= damage;
	}

	void heal(int amount) {
		if (currHealth + amount > maxHealth)
			currHealth = maxHealth;
		else
			currHealth += amount;
	}

	void rechargeEnergy() {
		if (energy + rechargeRate > maxEnergy)
			energy = maxEnergy;
		else
			energy += rechargeRate;
	}

	void removeEnergy(int amount) {
		if (energy - amount < 0)
			energy = 0;
		else
			energy -= amount;
	}

	int getEnergy() {
		return energy;
	}

	string getName() {
		return name;
	}

};

Creature playerCreature(CREATURE_MAX_HEALTH, "Player");
Creature enemyCreature(CREATURE_MAX_HEALTH, "Computer");

bool attack(Creature &attacker, Creature &target, int cost, int hitChance, int minDamage, int maxDamage);
void doTurn();

int menu() {
	cout << "0. Attack" << endl;
	cout << "1. Special" << endl;
	cout << "2. Recharge" << endl;
	cout << "3. Dodge" << endl;
	cout << "4. Heal" << endl;
	int choice;
	cin >> choice;
	processChoice(choice);
	return choice;
}


void endTurn() {
	playerCreature.rechargeEnergy();
	enemyCreature.rechargeEnergy();
	if (playerTurn) {
		playerTurn = false;
		hasHealed = false;
		cout << endl;
		cout << ">> Player turn end" << endl;
		cout << endl;
	}
	else {
		cout << endl;
		cout << ">> Enemy turn end" << endl;
		cout << endl;
		hasHealed = false;
		playerTurn = true;
	}
	if (playerCreature.getHealth() <= 0) {
		cout << playerCreature.getName() << " has died" << endl;
	} else if (enemyCreature.getHealth() <= 0) {
		cout << enemyCreature.getName() << " has died" << endl;
	} else {
		doTurn();
	}
}

void makeDecision(Creature &player, Creature &enemy);

void doTurn() {
	if (playerTurn) {
		if (playerCreature.recharged || playerCreature.dodged) {
			playerCreature.rechargeRate = CREATURE_DEFAULT_RECHARGE_RATE;
			playerCreature.hitChanceModifier = CREATURE_DEFAULT_HIT_CHANCE;
			playerCreature.dodged = false;
			playerCreature.recharged = false;
		}
		cout << endl;
		cout << ">> Player turn start" << endl;
		cout << "HP: " << playerCreature.getHealth() << "/" << playerCreature.maxHealth << " E:" << playerCreature.getEnergy() << "/" << playerCreature.maxEnergy << endl;
		cout << endl;
		menu();
	}
	else {
		if (enemyCreature.recharged || enemyCreature.dodged) {
			enemyCreature.rechargeRate = CREATURE_DEFAULT_RECHARGE_RATE;
			enemyCreature.hitChanceModifier = CREATURE_DEFAULT_HIT_CHANCE;
			enemyCreature.recharged = false;
			enemyCreature.dodged = false;
		}
		cout << endl;
		cout << ">> Enemy turn start" << endl;
		cout << "HP: " << enemyCreature.getHealth() << "/" << enemyCreature.maxHealth << " E:" << enemyCreature.getEnergy() << "/" << enemyCreature.maxEnergy << endl;
		cout << endl;
		makeDecision(playerCreature, enemyCreature);
	}
}

void death(Creature &died) {
	//TODO
}

void recharge(Creature &self) {
	self.rechargeRate *= 4;
	self.hitChanceModifier += 10;
	self.recharged = true;
	cout << self.getName() << " recharged" << endl;
	endTurn();
}

void dodge(Creature &self) {
	self.rechargeRate /= 2;
	self.hitChanceModifier -= 30;
	self.dodged = true;
	cout << self.getName() << " dodged" << endl;
	endTurn();
}

bool attack(Creature &attacker, Creature &target, int cost, int hitChance, int minDamage, int maxDamage) {
	if (attacker.getEnergy() >= cost) {
		attacker.removeEnergy(cost);
		uniform_int_distribution<> hitChanceRange(0, 100);
		int hit = hitChanceRange(seed);
		if (hit >= (100 - hitChance + target.hitChanceModifier)) {
			uniform_int_distribution<> damageRange(minDamage, maxDamage);
			int damage = damageRange(seed);
			target.takeDamage(damage);
			cout << ">> Hit for " << damage << " damage, " << target.getName() << "'s health is now " << target.getHealth() << "/" << target.maxHealth << endl;
			if (cost > 0)
				cout << ">> Cost " << cost << " energy " << attacker.getName() << " has " << attacker.getEnergy() << " energy remaining" << endl;
		}
		else {
			cout << ">> Missed!" << endl;
		}
		endTurn();
		return true;
	}
	cout << "Not enough energy to use attack" << endl;
	return false;
}

void heal(Creature &creature) {
	int amountToHeal = creature.getEnergy() / 2;
	if (creature.getHealth() + amountToHeal > creature.maxHealth)
		amountToHeal = creature.maxHealth - creature.getHealth();
	creature.heal(amountToHeal);
	creature.removeEnergy(amountToHeal);
	cout << ">> Healed " << creature.getName() << " for " << amountToHeal << " HP" << endl;
	cout << creature.getName() << "'s HP is now " << creature.getHealth() << endl;
	hasHealed = true;
	doTurn();
}

const int ATTACK = 0, SPECIAL = 1, RECHARGE = 2, DODGE = 3, HEAL = 4;

void processChoice(int choice) {
	switch (choice) {
	case ATTACK: 
		attack(playerCreature, enemyCreature, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
		break;
	case SPECIAL:
		if (!attack(playerCreature, enemyCreature, SPECIAL_COST, SPECIAL_HIT_CHANCE, SPECIAL_HIT_MIN, SPECIAL_HIT_MAX)) {
			menu();
		}
		break;
	case RECHARGE: 
		recharge(playerCreature);
		break;
	case DODGE:
		dodge(playerCreature);
		break;
	case HEAL: 
		if (!hasHealed)
			heal(playerCreature);
		else {
			cout << ">> Already healed this turn" << endl;
			menu();
		}
		break;
	default:
		cout << ">> Invalid choice" << endl;
		menu();
		break;
	}
	cout << endl;
}


void makeDecision(Creature &player, Creature &enemy) {
	if (player.dodged) {
		if (enemy.getHealth() <= 50 && enemy.getHealth() >= 25) {
			if (!hasHealed)
				heal(enemy);
			if (enemy.getHealth() >= 50) {
				attack(enemy, player, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
			}
			else {
				recharge(enemy);
			}
		}
		else if (enemy.getHealth() <= 25) {
			if (!hasHealed)
				heal(enemy);
			dodge(enemy);
		}
	}
	else if (player.recharged) {
		if (enemy.getEnergy() == enemy.maxEnergy) {
			attack(enemy, player, SPECIAL_COST, SPECIAL_HIT_CHANCE, SPECIAL_HIT_MIN, SPECIAL_HIT_MAX);
		}
		else {
			attack(enemy, player, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
		}
	}
	else {
		if (enemy.getEnergy() == enemy.maxEnergy) {
			attack(enemy, player, SPECIAL_COST, SPECIAL_HIT_CHANCE, SPECIAL_HIT_MIN, SPECIAL_HIT_MAX);
		}
		else if (enemy.getHealth() <= 50 && enemy.getHealth() >= 25) {
			if (!hasHealed)
				heal(enemy);
			attack(enemy, player, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
		}
		else if (enemy.getHealth() <= 25) {
			if (!hasHealed)
				heal(enemy);
			dodge(enemy);
		}
		else {
			attack(enemy, player, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
		}
	}
	attack(enemy, player, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
}

int main() {
	cout << ">> Player HP = " << playerCreature.getHealth() << endl;
	cout << ">> Enemy HP = " << enemyCreature.getHealth() << endl;
	doTurn();
	cin.ignore();
	getchar();
    return 0;
}

