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
#include <conio.h>


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


char choiceStr = ' ';
float oldtime = 0;
double deltaTime = 0.000;
std::list<o_character*> players;
std::list<o_character*> enemies;
std::list<o_character*> allchars;

std::queue<o_character*> player_queue;	//all the players that are ready
std::queue<o_character*> enemy_queue;	//all the enemies that are ready

std::queue<o_character*> allQueue;
std::queue<misc_turn> movequeue;
char key;

enum GAME_STATE {IDLE, PROCESSING, FINISH};
enum KEY_STATE 
{
	KEY_NONE = -1,
	KEY_UP = 72,
	KEY_DOWN = 80,
	KEY_LEFT = 75,
	KEY_RIGHT = 77,
	KEY_a = 97,
	KEY_b,
	KEY_c,
	KEY_d,
	KEY_e,
	KEY_f,
	KEY_A = 65,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
};

GAME_STATE GAME_STATEMACHINE;
KEY_STATE KEYBOARD_STATE = KEY_NONE;

std::string selected_character;
o_character Player = o_character(15, "Hero", 2, 15);

KEY_STATE GetKeyDown(char ke) 
{
	KEY_STATE stat = (KEY_STATE)ke;
	return stat;
}

//For getting the key values
char GetKey()
{
	if (_kbhit() != 0) 
	{
		char key = _getch();
		return key;
	}
	return' ';
}

void PopQueue(bool isplayable)
{
	if (isplayable) 
	{
		player_queue.front()->SetAttackMode();
		player_queue.front()->charge = 0;
		player_queue.pop();
	}
	else 
	{
		enemy_queue.front()->SetAttackMode();
		enemy_queue.front()->charge = 0;
		enemy_queue.pop();
	}
}

void DrawCharacter() 
{
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
	std::cout << selected_character << std::endl;

}
int ClampVal(int num, int min, int max) 
{
	if (num < min) 	
		num = min;

	if (num > max)
		num = max;

	return num;
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
	o_attack* attk = (o_attack*)&enemy_queue.front()->attack;

	misc_turn turn(attk, charattk);
	movequeue.push(turn);
	PopQueue(false);
}

//This is for playable characters only.
void Commands() 
{
	char n = ' ';
	static int choice = 0;
	bool isPlayable = true;
	std::list<o_character*>::iterator it;

	std::list<o_character*>::iterator character;
	o_character* charattk;
	o_attack* attk;

	switch (GAME_STATEMACHINE)
	{

	case IDLE:

		std::cout << player_queue.front()->name << "\n" << "F - fight, H - heal" << std::endl;

		choiceStr = key;
		if (choiceStr == 'f' || choiceStr == 'h')
		{
			GAME_STATEMACHINE = PROCESSING;
		}


		break;

	case PROCESSING:
		if (choiceStr == 'f')
		{

			auto SelectCharacter = [&]()
			{
				character = enemies.begin();
				std::advance(character, choice);
				charattk = *character;
				selected_character = charattk->name;
			};
			choice = ClampVal(choice, 0, enemies.size() - 1);
			SelectCharacter();
			
			switch (KEYBOARD_STATE)
			{
				case KEY_UP:

					choice++;
					break;

				case KEY_DOWN:

					choice--;
					break;

				case KEY_f:

					attk = (o_attack*)&player_queue.front()->attack;
					misc_turn turn(attk, charattk);
					movequeue.push(turn);
					GAME_STATEMACHINE = FINISH;
					break;
			}

		/*
			while (key != 'f')
				if (choice >= enemies.size())
				{
					return;
				}
			}
		*/


		}
		if (choiceStr == 'h')
		{
			//Set this to the character on the top of the command
			player_queue.front()->health += 1;
			GAME_STATEMACHINE = FINISH;
		}


		//Reset their charge and pop them from the queue
		//Set their is attacking mode to false

		break;

	case FINISH:

		PopQueue(true);
		GAME_STATEMACHINE = IDLE;
		break;

	}
	//isPlayable = allQueue.front()->GetPlayable();
}


void PollEvents()
{

	PollEnemyEvents();
	
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
				if (a->GetPlayable() == true) 
				{
					player_queue.push(a);
				} else
				{
					enemy_queue.push(a);
				}
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

void GenerateRandom() 
{

}

void UpdateGame() 
{
	clock_t ti;
	ti = clock();

	DrawCharacter();

	UpdateCharacters();

	key = GetKey();
	KEYBOARD_STATE = GetKeyDown(key);

	if (player_queue.size() > 0)
		Commands();

	if (enemy_queue.size() > 0)
		PollEnemyEvents();

	if (movequeue.size() > 0)
		attack(movequeue.front());


	clock_t t = clock() - ti;

	deltaTime = t / (double)CLOCKS_PER_SEC;
}

void DebugKeyInputs() 
{
	if (key != ' ') 
	{
		//std::cout << "Hex value for " << key << " : " << (int)key << std::endl;
		std::cout << (char)KEYBOARD_STATE << std::endl;
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

		//DebugKeyInputs();

		UpdateGame();

		if (CheckIfSideDead())
			break;
	}

	std::cout << "You won!" << std::endl;
	using namespace std::literals::chrono_literals;
	std::this_thread::sleep_for(2s);

	std::cin.get();
}