#include <list>
#include <iostream>
#include "o_character.h"
#include "o_attack.h"
#include <algorithm>
#include <time.h>
#include <chrono>
#include <thread>
#include <queue>
#include <random>

struct misc_turn 
{
	public:
		o_character* character;
		o_attack* attack;

		misc_turn(o_attack* attackq, o_character* chara):
			attack(attackq), character(chara)
		{
		}
};


std::string choiceStr = "";
float oldtime = 0;
double deltaTime = 0.000;
std::list<o_character*> players;
std::list<o_character*> enemies;
std::list<o_character*> allchars;
std::queue<o_character*> allQueue;
std::queue<misc_turn> movequeue;

enum GAME_STATE {IDLE, PROCESSING, FINISH};
GAME_STATE GAME_STATEMACHINE;

o_character Player = o_character(15, "Hero", 2, 15);

void DrawCharacter() 
{
	system("color 1a");
	system("cls");
	
	for (o_character* p : players)
	{
		std::cout << p->name << "  " << p->health << " " << round(p->charge) << "%"<< " " << p->GetAttackMode() << std::endl;
	}

	std::cout << "\n";

	for (o_character* p : enemies)
	{
		std::cout << p->name << "  " << p->health << " " << round(p->charge) << "%" << std::endl;
	}

}

void attack(misc_turn turndat)
{
	if (turndat.character->health <= 0)
		return;

	turndat.character->health -= turndat.attack->attack;
	movequeue.pop();
}

void PollEnemyEvents() 
{
	std::list<o_character*>::iterator character = players.begin();
	int select = (rand() % players.size()) + 0;
	
	std::advance(character, select);
	o_character* charattk = *character;

	o_attack* attk = (o_attack*)&allQueue.front()->attack;

	misc_turn turn(attk, charattk);

	movequeue.push(turn);
}

void PollEvents()
{
	std::string n = "";
	int choice = 0;
	bool isPlayable = true;
	std::list<o_character*>::iterator it;

	switch (GAME_STATEMACHINE)
	{

	case IDLE:

		isPlayable = allQueue.front()->GetPlayable();

		if (isPlayable)
		{
			std::cout << allQueue.front()->name << "\n" << "F - fight, H - heal" << std::endl;
			std::cin >> n;

			choiceStr = n;
			if (choiceStr == "f" || choiceStr == "h")
			{
				GAME_STATEMACHINE = PROCESSING;
			}
		}
		else 
		{
			PollEnemyEvents();
			GAME_STATEMACHINE = FINISH;
		}
		

		break;

	case PROCESSING:
		if (isPlayable)
		{
			if (choiceStr == "f")
			{
				std::list<o_character*>::iterator character;
				o_character* charattk;
				o_attack* attk;

				std::cin >> choice;
				if (choice >= enemies.size())
				{
					return;
				}
				character = enemies.begin();
				std::advance(character, choice);
				charattk = *character;

				attk = (o_attack*)&allQueue.front()->attack;

				misc_turn turn(attk, charattk);

				movequeue.push(turn);

			}
			if (choiceStr == "h")
			{
				//Set this to the character on the top of the command
				allQueue.front()->health += 1;
			}
		}
		

		//Reset their charge and pop them from the queue
		//Set their is attacking mode to false
		GAME_STATEMACHINE = FINISH;

		break;

	case FINISH:

		allQueue.front()->SetAttackMode();
		allQueue.front()->charge = 0;
		allQueue.pop();
		GAME_STATEMACHINE = IDLE;
		break;

	}
}

void UpdateCharacters() 
{
	for (o_character* a : allchars)
	{
		if (a->health > 0) 
		{
			a->Update(deltaTime);
		}
		else {
			a->charge = 0;
		}
		if (a->charge >= 100)
		{

			if (a->GetAttackMode() == false)
			{
				a->SetAttackMode();
				allQueue.push(a);
			}
		}
	}
}

bool CheckIfSideDead() 
{
	int s = 0;
	for (o_character* a : enemies)
	{
		if (a->health <= 0)
		{
			s++;
		}
	}
	if(s == enemies.size())
		return true;

	return false;
}

void AddToAllChars() 
{
	for (o_character* a : enemies)
	{
		a->SetPlayable(false);
		allchars.push_front(a);
	}
	for (o_character* a : players)
	{
		a->SetPlayable(true);
		allchars.push_front(a);
	}
}

int main() 
{
	GAME_STATEMACHINE = IDLE;
	players.push_back(&Player);
	o_character p2(10, "Freeman", 1, 25);
	players.push_back(&p2);
	
	o_character e2(4, "Solider 76", 2,16);
	enemies.push_back(&e2);
	o_character e4(3, "Chie", 1, 8);
	enemies.push_back(&e4);
	o_character e5(5, "Spike",1, 3);
	enemies.push_back(&e5);
	o_character e6(2, "Alyx", 1,6);
	enemies.push_back(&e6);
	o_character e7(10, "Mahan", 2, 69);
	enemies.push_back(&e7);

	AddToAllChars();
	while (1) 
	{
		clock_t ti;
		ti = clock();

		DrawCharacter();
		
		UpdateCharacters();

		if (allQueue.size() > 0) 
		{
			PollEvents();
		}

		if (movequeue.size() > 0) 
		{
			attack(movequeue.front());
		}
		
		if (CheckIfSideDead())
			break;

		clock_t t = clock() - ti;

		deltaTime = t / (double)CLOCKS_PER_SEC;

	}

	std::cout << "You won!" << std::endl;
	using namespace std::literals::chrono_literals;
	std::this_thread::sleep_for(2s);

	std::cin.get();
}