#include <Windows.h>
#include <iostream>
#include "HackProcess.h"

CHackProcess fProcess;

const DWORD Player_Base = 0x102AC9C;

#define F6_KEY 0x75
bool b_ShotNow = false;
const DWORD dw_attack = 0x104EC08;
const DWORD dw_teamOffset = 0xAC;

int i_shoot =  5;
int i_DontShoot = 4;

int NumOfPlayers = 32;
const DWORD dw_PlayerCount = 0x5CF0DC;
const DWORD dw_crosshairOffs = 0x1754;

const DWORD dw_entityBase = 0x1038314;
const DWORD dw_EntityLoopDistance = 0x10;

struct MyPlayer{
	DWORD CLocalPlayer;
	int Team;
	int CrosshairEntityID;
	
	void ReadInformation(){
		ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(fProcess.__dwordClient + Player_Base), &CLocalPlayer, sizeof(DWORD), 0);
		
		ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(CLocalPlayer + dw_teamOffset), &Team, sizeof(int), 0);
		
		ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(CLocalPlayer + dw_crosshairOffs), &CrosshairEntityID, sizeof(int), 0);
		
		ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(fProcess.__dwordEngine + dw_PlayerCount), &NumOfPlayers, sizeof(int), 0);
	}
}MyPlayer;

struct PlayerList{
	DWORD CBaseEntity;
	int Team;
	
	void ReadInformation(int Player){
		ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(fProcess.__dwordClient + dw_entityBase + (Player*dw_EntityLoopDistance)), &CBaseEntity, sizeof(int), 0);
		
		ReadProcessMemory(fProcess.__HandleProcess, (PBYTE*)(CBaseEntity + dw_teamOffset), &Team, sizeof(int), 0);
	}
}PlayerList[32];

void TriggerBot(){
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
	
	while(!GetAsyncKeyState(F6_KEY)){
		MyPlayer.ReadInformation();
		
		for(int c = 0; c < NumOfPlayers; c++){
			PlayerList[c].ReadInformation(c);
		}
		TriggerBot();
	}
}