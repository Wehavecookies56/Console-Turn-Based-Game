#include "stdafx.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <random>
#include <conio.h>

using namespace std;
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

void processChoice(char choice, Creature &player, Creature &enemy);
void doTurn(Creature &player, Creature &enemy);

int menu(Creature &player, Creature &enemy) {
	cout << "0. Attack" << endl;
	cout << "1. Special" << endl;
	cout << "2. Recharge" << endl;
	cout << "3. Dodge" << endl;
	cout << "4. Heal" << endl;
	cin.clear();
	char choice = _getch();
	processChoice(choice, player, enemy);
	return choice;
}


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
		doTurn(player, enemy);
	}
}

void makeDecision(Creature &player, Creature &enemy);

void doTurn(Creature &player, Creature &enemy) {
	if (playerTurn) {
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

void death(Creature &died) {
	//TODO
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


bool attack(Creature &attacker, Creature &target, Creature &player, Creature &enemy, int cost, int hitChance, int minDamage, int maxDamage) {
	if (player.getHealth() <= 0 || enemy.getHealth() <= 0) {
		cout << "This shouldn't happen in the first place" << endl;
		return false;
	} else {
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
	hasHealed = true;
	doTurn(player, enemy);
}

enum commands { ATTACK = '0', SPECIAL = '1', RECHARGE = '2', DODGE = '3', HEAL = '4' };

void processChoice(char choice, Creature &player, Creature& enemy) {
	switch (choice) {
	case ATTACK: 
		cout << "attack 1" << endl;
		attack(player, enemy, player, enemy, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
		break;
	case SPECIAL:
		cout << "attack 2" << endl;
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
		if (!hasHealed)
			heal(player, player, enemy);
		else {
			cout << ">> Already healed this turn" << endl;
			menu(player, enemy);
		}
		break;
	default:
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
			if (!hasHealed)
				heal(enemy, player, enemy);
			if (enemy.getHealth() >= 50) {
				cout << "attack 3" << endl;
				attack(enemy, player, player, enemy, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
			} else {
				recharge(enemy, player, enemy);
			}
		} else if (enemy.getHealth() <= 25) {
			if (!hasHealed)
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
			if (!hasHealed)
				heal(enemy, player, enemy);
			cout << "attack 7" << endl;
			attack(enemy, player, player, enemy, ATTACK_COST, ATTACK_HIT_CHANCE, ATTACK_HIT_MIN, ATTACK_HIT_MAX);
		} else if (enemy.getHealth() <= 25) {
			if (!hasHealed)
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
	Creature playerCreature(CREATURE_MAX_HEALTH, "Player");
	Creature enemyCreature(CREATURE_MAX_HEALTH, "Computer");
	cout << ">> Player HP = " << playerCreature.getHealth() << endl;
	cout << ">> Enemy HP = " << enemyCreature.getHealth() << endl;
	doTurn(playerCreature, enemyCreature);
	cin.ignore();
	_getch();
    return 0;
}

