#pragma once
#include <iostream>
#include <string>
#include "o_attack.h"

class o_character
{
public:

	std::string name;
	int health;
	int maxhealth;
	o_attack attack;

	double speed;
	float charge;

	o_character(int health, std::string name, int attack, double speed);
	~o_character();

	void Update(float DeltaTime);
	void Attack();

	void SetAttackMode();
	bool GetAttackMode();

	void SetPlayable(bool boolean);
	bool GetPlayable();

private:
	bool is_playable;
	bool is_attacking = false;
};

