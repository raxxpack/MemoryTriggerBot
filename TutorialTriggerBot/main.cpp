//
//  main.cpp
//  TutorialTriggerBot
//
//  Created by Rahim Mitha on 2014-04-28.
//  Copyright (c) 2014 Rahim Mitha. All rights reserved.
//  Credit to Fleep.
//

#include <Windows.h>
#include <iostream>
#include "HackProcess.h"

CHackProcess fProcess;

//Valid as of Apr 29 2014
const DWORD Player_Base = 0x102AC9C;

//Toggle the hack
#define VK_LEFT_MOUSE_CLICK 0x01
#define VK_F_KEY 0x46

//Close the hack
#define F6_KEY 0x75

bool isTriggerbotActive = false;
bool b_ShotNow = false;
const DWORD dw_attack = 0x104EC08;
const DWORD dw_teamOffset = 0xAC;

int i_shoot =  5;
int i_DontShoot = 4;

int NumOfPlayers = 32;
const DWORD dw_PlayerCount = 0x5CF0DC;
const DWORD dw_crosshairOffs = 0x1754;

//Valid as of Apr 29 2014
const DWORD dw_entityBase = 0x1038314;
//Valid as of Apr 29 2014
const DWORD dw_EntityLoopDistance = 0x10;

struct MyPlayer {
	DWORD CLocalPlayer;
	int Team;
	int CrosshairEntityID;
	
	void ReadInformation() {
		ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(fProcess.__dwordClient + Player_Base), &CLocalPlayer, sizeof(DWORD), 0);
		
		ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(CLocalPlayer + dw_teamOffset), &Team, sizeof(int), 0);
		
		ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(CLocalPlayer + dw_crosshairOffs), &CrosshairEntityID, sizeof(int), 0);
	}
}MyPlayer;

struct PlayerList {
	DWORD CBaseEntity;
	int Team;
	
	void ReadInformation(int Player) {
		ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(fProcess.__dwordClient + dw_entityBase + (Player*dw_EntityLoopDistance)), &CBaseEntity, sizeof(int), 0);
		
		ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(CBaseEntity + dw_teamOffset), &Team, sizeof(int), 0);
	}
}PlayerList[32];

void LeftClick() {
	INPUT Input = {0};
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	::SendInput(1, &Input, sizeof(INPUT));
	
	::ZeroMemory(&Input, sizeof(Input));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	::SendInput(1, &Input, sizeof(INPUT));
}

void SimulatedMouseTriggerBot() {
	//Check if valid target
	if (MyPlayer.CrosshairEntityID <= 0 || PlayerList[MyPlayer.CrosshairEntityID].Team == MYPlayer.Team || MyPlayer.CrosshairEntityID > 32) {
		return;
	}
	
	LeftClick();
}

void MemoryTriggerBot() {
	if(!b_ShotNow){
		WriteProcessMemory(fProcess.__HandleProcess, (int*)(fProcess.__dwordClient + dw_attack), &i_DontShoot, sizeof(int), NULL);
		b_ShotNow = !b_ShotNow;
	}
	
	if(MyPlayer.CrosshairEntityID == 0){
		return;
	}
	
	if(PlayerList[MyPlayer.CrosshairEntityID].Team == MyPlayer.Team)
		return;
	
	if(MyPlayer.CrosshairEntityID > 32)
		return;
	
	if(b_ShotNow){
		WriteProcessMemory(fProcess.__HandleProcess, (int*)(fProcess.__dwordClient + dw_attack), &i_shoot, sizeof(int), NULL);
		b_ShotNow = !b_ShotNow;
	}
}

int main(){
	fProcess.RunProcess();
	std::cout << "Game Found! Triggerbot Activated" << std::endl;
	
	while(!GetAsyncKeyState(F6_KEY)) {
		
		if (isTriggerbotActive) {
			
			if (GetAsynchKeyState(VK_F_KEY)) {
				isTriggerbotActive = false;
			} else {
				
				
				ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(fProcess.__dwordEngine + dw_PlayerCount), &NumOfPlayers, sizeof(int), 0);
				
				MyPlayer.ReadInformation();
				
				for(int i = 0; i < NumOfPlayers; i++){
					PlayerList[i].ReadInformation(i);
				}
				SimulatedMouseTriggerBot();
			}
			
		}
		else {
			Sleep(15);
			if (GetAsynchKeyState(VK_F_KEY) {
				isTriggerbotActive = true;
			}
				
		}
	}
}
				