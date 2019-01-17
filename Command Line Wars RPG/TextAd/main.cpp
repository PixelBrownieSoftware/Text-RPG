#include "o_character.h"
#include "o_attack.h"
#include "Maths.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <time.h>
#include <chrono>
#include <Windows.h>
#include <ctime>
#include <thread>
#include <queue>
#include <random>
#include <stdlib.h>
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
std::vector<o_character*> players;
std::vector<o_character*> enemies;
std::vector<o_character*> allchars;

std::queue<o_character*> player_queue;	//all the players that are ready
std::queue<o_character*> enemy_queue;	//all the enemies that are ready

std::queue<o_character*> allQueue;
std::queue<misc_turn> movequeue;
char key;
HANDLE hcon;

enum GAME_STATES { MAIN_MENU, CHARACTER_CREATION, GAME, END_GAME };
enum BATTLE_STATE_MACHINE { IDLE, PROCESSING, FINISH};
enum KEY_STATE 
{
	KEY_NONE = -1,
	KEY_ENTER = 13,
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
enum VICTORY_STATE { UNDECIDED, PLAYERS, ENEMIES};

/*
Credit to One Lone Coder's console game engine, for doing all the dirty work of figuring out the colours of the chars
https://github.com/OneLoneCoder/videos/blob/master/olcConsoleGameEngine.h
*/
enum COLOUR
{
	FG_BLACK = 0x0000,
	FG_DARK_BLUE = 0x0001,
	FG_DARK_GREEN = 0x0002,
	FG_DARK_CYAN = 0x0003,
	FG_DARK_RED = 0x0004,
	FG_DARK_MAGENTA = 0x0005,
	FG_DARK_YELLOW = 0x0006,
	FG_GREY = 0x0007, // Thanks MS :-/
	FG_DARK_GREY = 0x0008,
	FG_BLUE = 0x0009,
	FG_GREEN = 0x000A,
	FG_CYAN = 0x000B,
	FG_RED = 0x000C,
	FG_MAGENTA = 0x000D,
	FG_YELLOW = 0x000E,
	FG_WHITE = 0x000F,
	BG_BLACK = 0x0000,
	BG_DARK_BLUE = 0x0010,
	BG_DARK_GREEN = 0x0020,
	BG_DARK_CYAN = 0x0030,
	BG_DARK_RED = 0x0040,
	BG_DARK_MAGENTA = 0x0050,
	BG_DARK_YELLOW = 0x0060,
	BG_GREY = 0x0070,
	BG_DARK_GREY = 0x0080,
	BG_BLUE = 0x0090,
	BG_GREEN = 0x00A0,
	BG_CYAN = 0x00B0,
	BG_RED = 0x00C0,
	BG_MAGENTA = 0x00D0,
	BG_YELLOW = 0x00E0,
	BG_WHITE = 0x00F0,
};

VICTORY_STATE VICTORY = UNDECIDED;
GAME_STATES GAME_STATE;
BATTLE_STATE_MACHINE BSM;
KEY_STATE KEYBOARD_STATE = KEY_NONE;

int selected_character = -1;
std::string character_names[] = { 
	"Pete",
	"Milbert", 
	"Bert",
	"Koko",
	"Loki",
	"Sinro",
	"Zack",
	"Slime",
	"Skeleton",
	"Okami",
	"Peast",
	"Stanley",
	"Ratchet",
	"Prownie",
	"Brownie",
	"Pixel"
};
std::string character_victories[] = {
	"You guys are pretty weak to be honest.",
	"YEAH! That'll teach you to mess with me!",
	"I'm hungry.",
	"Ha! Ha! Ha! Ha! Ha! Ha!",
	"I think we won.",
	"We win! We win! Horay!",
	"You're done for.",
	"Bloop.",
	"We succeed.",
	"You're no match for me!",
	"You were already gone from the start.",
	"Victory is ours",
	"Nobody's going to remember you!",
	"That's all from me!",
	"Can we go home now?",
	"What are we going to have for dinner?"
	"How much are we going to get paid for this.",
	"Take that dofus!"
};

KEY_STATE GetKeyDown(char ke) 
{
	KEY_STATE stat = (KEY_STATE)ke;
	return stat;
}

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
		selected_character = -1;
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

	int pi = 0;
	for (o_character* p : players)
	{
		if (p->health >= 0)
		{
			SetConsoleTextAttribute(hcon, FG_CYAN);
			if (p->GetAttackMode())
				SetConsoleTextAttribute(hcon, BG_CYAN);
		}
		if (p->health <= 0)
			SetConsoleTextAttribute(hcon, 0x0014);

		if (choiceStr == 'h')
		{
			if (selected_character == pi)
				SetConsoleTextAttribute(hcon, FG_YELLOW);
		}
			
		std::cout << p->name << " \nHP: " << p->health << "/ " << p->maxhealth << " Charge: " << round(p->charge) << "%" << std::endl;
		pi++;
	}
	SetConsoleTextAttribute(hcon, FG_WHITE);
	
	std::cout << "\n";

	//Counter for matching the selected character
	int i = 0;
	for (o_character* p : enemies)
	{
		if (choiceStr == 'f')
		{
			if (selected_character == i)
				SetConsoleTextAttribute(hcon, FG_YELLOW);
		}

		if (selected_character != i) 
		{
			SetConsoleTextAttribute(hcon, FG_RED);
			if (p->health <= 0)
				SetConsoleTextAttribute(hcon, BG_DARK_RED);
		
			if (p->GetAttackMode())
				SetConsoleTextAttribute(hcon, BG_DARK_GREEN);
		}

		std::cout << p->name << "\nHP: " << p->health << " Charge: " << round(p->charge) << "%" << std::endl;
		i++;
	}
	SetConsoleTextAttribute(hcon, FG_WHITE);

}

void Attack(misc_turn turndat)
{
	if (turndat.character->health <= 0) 
	{
		movequeue.pop();
		return;
	}
		
	turndat.character->health -= turndat.attack->attack;
	movequeue.pop();
}

void PollEnemyEvents() 
{
	std::vector<o_character*>::iterator character = players.begin();
	int select = (rand() % players.size()) + 0;

	std::advance(character, select);
	o_character* charattk = *character;

	//If this character is dead, the enemy will just heal
	if (charattk->health <= 0) 
	{
		enemy_queue.front()->health += 1;
		PopQueue(false);
		return;
	}


	o_attack* attk = (o_attack*)&enemy_queue.front()->attack;

	misc_turn turn(attk, charattk);
	movequeue.push(turn);
	PopQueue(false);
}

void InitializeTime() 
{
	time_t t;
	time(&t);
	srand(t);
}

void Commands() 
{
	char n = ' ';
	static int choice = 0;
	bool isPlayable = true;
	std::vector<o_character*>::iterator it;

	std::vector<o_character*>::iterator character;
	o_character* charattk = players.front();
	o_attack* attk;

	if(player_queue.front()->health == 0)
		BSM = FINISH;

	switch (BSM)
	{

	case IDLE:

		std::cout << player_queue.front()->name << "\n" << "F - fight, H - heal" << std::endl;

		choiceStr = key;
		if (choiceStr == 'f' || choiceStr == 'h')
			BSM = PROCESSING;

		break;

	case PROCESSING:
		std::cout << "" << std::endl;
		if (choiceStr == 'f')
		{
			std::cout << "F to confirm attack" << std::endl;
			auto SelectCharacter = [&]()
			{
				character = enemies.begin();
				std::advance(character, choice);
				charattk = *character;
				selected_character = choice;
			};
			choice = ClampVal(choice, 0, enemies.size() - 1);
			SelectCharacter();
		}
		if (choiceStr == 'h')
		{
			std::cout << "F to confirm heal" << std::endl;
			auto SelectCharacter = [&]()
			{
				character = players.begin();
				std::advance(character, choice);
				charattk = *character;
				selected_character = choice;
			};
			choice = ClampVal(choice, 0, players.size() - 1);
			SelectCharacter();
			//Set this to the character on the top of the command
		}
		switch (KEYBOARD_STATE)
		{
		case KEY_UP:
			choice--;
			break;

		case KEY_DOWN:
			choice++;
			break;

		case KEY_f:

			if (choiceStr == 'h') 
			{
				charattk->health += rand() % 4 + 1;
				BSM = FINISH;
			}
			if (choiceStr == 'f') 
			{
				attk = (o_attack*)&player_queue.front()->attack;
				misc_turn turn(attk, charattk);
				movequeue.push(turn);
				BSM = FINISH;
			}
			break;
		}

		//Reset their charge and pop them from the queue
		//Set their is attacking mode to false

		break;

	case FINISH:

		PopQueue(true);
		BSM = IDLE;
		break;

	}
}

void UpdateCharacters() 
{
	for (o_character* a : allchars)
	{
		a->Update((float)deltaTime);

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

void CheckIfSideDead() 
{
	int enemies_defeated = 0;
	for (o_character* a : enemies)
		if (a->health <= 0)
			enemies_defeated++;

	int players_defeated = 0;
	for (o_character* a : players)
		if (a->health <= 0)
			players_defeated++;

	if (enemies_defeated == enemies.size())
		VICTORY = PLAYERS;
	else if (players_defeated == players.size())
		VICTORY = ENEMIES;
	else
		VICTORY = UNDECIDED;
}

void AddToAllChars() 
{
	for (o_character* a : enemies)
	{
		a->SetPlayable(false);
		allchars.push_back(a);
	}
	for (o_character* a : players)
	{
		a->SetPlayable(true);
		allchars.push_back(a);
	}
}

void UpdateGame() 
{
	clock_t ti;
	ti = clock();
	DrawCharacter();
	UpdateCharacters();

	if (player_queue.size() > 0)
		Commands();

	if (enemy_queue.size() > 0)
		PollEnemyEvents();
		
	if (movequeue.size() > 0)
		Attack(movequeue.front());

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

void Wait(int seconds) 
{
	using namespace std::literals::chrono_literals;
	for(int i = seconds; i > 0; i--)
		std::this_thread::sleep_for(1s);
}

void GenerateCharacter(std::string nam, bool playable) 
{
	o_character* chara = new o_character(rand() % 30 + 10, nam, rand() % 1 + 3, rand() % 10 + 5);

	if (playable)
		players.push_back(chara);
	else
		enemies.push_back(chara);
}

std::string RandomName(std::string names[])
{
	int rannum = 0;

	if (sizeof(*names) > sizeof(names)) 
		rannum = rand() % (sizeof(*names) / sizeof(names));
	else 
		rannum = rand() % (sizeof(names) / sizeof(*names));

	return names[rannum];
}

void DisplayCharacterVictories(std::string name) 
{
	std::string quote = RandomName(character_victories);
	std::cout << name << ": " << quote << std::endl;
}

int main() 
{
	BSM = IDLE;
	
	std::cout << "Command Line Wars RPG" << std::endl;
	Wait(1);
	std::cout << "Created by Pixel Brownie Software (2018)" << std::endl;
	Wait(1);
	std::cout << "Press Enter to start!" << std::endl;
	Wait(1);

	InitializeTime();

	int player_characters_count = rand() % 5 + 1;
	int enemy_characters_count = rand() % 6 + 3;
	hcon = GetStdHandle(STD_OUTPUT_HANDLE);

	while (GAME_STATE != END_GAME)
	{
		key = GetKey();
		KEYBOARD_STATE = GetKeyDown(key);
		//DebugKeyInputs();

		switch (GAME_STATE)
		{
		case MAIN_MENU:
			switch (KEYBOARD_STATE)
			{
			case KEY_ENTER:
				GAME_STATE = CHARACTER_CREATION;
				break;
			}
			break;

		case CHARACTER_CREATION:
			system("cls");

			for (int i = 0; i < player_characters_count; i++)
			{
				std::string nam = "";
				std::cout << "Name character " << i << std::endl;
				std::cin >> nam;
				GenerateCharacter(nam,true);
			}
			for (int i = 0; i < enemy_characters_count; i++) 
			{
				std::string nam = RandomName(character_names);
				GenerateCharacter(nam, false);
			}
			AddToAllChars();
			GAME_STATE = GAME;
				
			break;

		case GAME:

			UpdateGame(); 
			CheckIfSideDead();
			
			if (VICTORY != UNDECIDED)
				GAME_STATE = END_GAME;

			break;

		}
	}

	system("cls");

	switch (VICTORY)
	{
	case PLAYERS:

		std::cout << "You won!" << std::endl;
		
		for (o_character *a : players) 
		{
			DisplayCharacterVictories(a->name);
		}
		
		break;

	case ENEMIES:

		std::cout << "Defeated...\n" << std::endl;
		for (o_character *a : enemies)
		{
			DisplayCharacterVictories(a->name);
		}
		break;
	}
	Wait(9);
	for (o_character* c : players) 
	{
		delete c;
	}
	for (o_character* c : enemies)
	{
		delete c;
	}
	std::cin.get();
}
