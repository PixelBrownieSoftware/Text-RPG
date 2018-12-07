#include "o_character.h"

o_character::o_character(int health, std::string name, int attack, double speed):
	maxhealth(health),health(health), name(name), attack(o_attack(attack)), speed(speed)
{
	charge = 0;
}

void o_character::Update(float DeltaTime) 
{
	charge += DeltaTime * (speed);
	if (charge > 100)
		charge = 100;

	if (health <= 0) 
	{
		health = 0;
		charge = 0;
	}
		
	if (health > maxhealth)
		health = maxhealth;
}

o_character::~o_character()
{
	//std::cout << "DESTROYED" << std::endl;
}

void o_character::SetAttackMode() 
{
	if (is_attacking == true) 
		is_attacking = false;
	else 
		is_attacking = true;
}

bool o_character::GetAttackMode() 
{
	return is_attacking;
}

void o_character::SetPlayable(bool boolean) 
{
	is_playable = boolean;
}

bool o_character::GetPlayable() 
{
	return is_playable;
}

void o_character::Attack() {}
