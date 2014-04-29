//
//  main.cpp
//  TutorialTriggerBot
//
//  Created by Rahim Mitha on 2014-04-28.
//  Copyright (c) 2014 Rahim Mitha. All rights reserved.
//

#include <iostream>
#include "HackProcess.h"
//#include <Windows.h>

class PlayerDataVec {
public:
	float xMouse;
	float yMouse;
	int isValid;
	float xPos;
	float yPos;
	float zPos;
	
	float xVelocity;
	float yVelocity;
	float zVelocity;
	
	int isAlive;
	int clientNum;
	char name[16];
	int pose; //eg. Crouched, standing, scoped
	int team;
	bool isVisible;
	DWORD isInGame;
	int health;
	
};

PlayerDataVec GetPlayerDataVec() {
	PlayerDataVec playerRet;
	ReadProcessMemory(HProcHandle, (PVOID)0x12886A0,&playerRet.health,4,NULL);
	ReadProcessMemory(HProcHandle, (PVOID)0x74E35C,&playerRet.isInGame,4,NULL);
    //Get our client number so we don't include ourselfs in the ESP
	ReadProcessMemory(HProcHandle, (PVOID)0x74E338,&playerRet.clientNum,4,NULL);
	//GET OUR current team so we don't shoot at team mates
	ReadProcessMemory(HProcHandle, (PVOID)0x83928C,&playerRet.team,4,NULL);
	//GET OUR FIELD OF VIEW
	ReadProcessMemory(HProcHandle, (PVOID)0x797610,&fov[0],4,NULL);
	ReadProcessMemory(HProcHandle, (PVOID)0x797614,&fov[1],4,NULL);
	ReadProcessMemory(HProcHandle, (PVOID)0x79B698,&viewAngles.x,4,NULL);
	ReadProcessMemory(HProcHandle, (PVOID)0x79B69C,&viewAngles.y,4,NULL);
	ReadProcessMemory(HProcHandle, (PVOID)0x79B6A0,&viewAngles.z,4,NULL);
	ReadProcessMemory(HProcHandle, (PVOID)0x797618,&playerRet.xPos,4,NULL);
	ReadProcessMemory(HProcHandle, (PVOID)0x79761C,&playerRet.yPos,4,NULL);
	ReadProcessMemory(HProcHandle, (PVOID)0x797620,&playerRet.zPos,4,NULL);
	//std::stringstream ss;
	//ss << /*"Player health:" <<*/ playerRet.health;
	//DrawString(HDC_Desktop, 300, 600, RGB(255, 0, 0), ss.str().c_str());
	
    return playerRet;
}

//Note this function may look like a lot but if you have trouble with it have a good read of the comments and
//you will realize that its not as bad as it seems
//We send in all our variables from the menu to see if we turn ESP specific functions on or not e.g. ESP Distance
int main(int argc, const char  * argv[])
{
	//Get our player's information
	PlayerDataVec playerVec = GetPlayerVec();
	int enemyCount = 0; // used to loop our enemy data
	//Set up our array to hold enemy data, 64 in our example is the max number a cod 4 game can have
	PlayerDataVec enemiesVec[MAXPLAYERS];
	PlayerDataVec viableEnemiesVec[MAXPLAYERS];
	
	//if we are not in game then no point in running hack
	if(playerVec.isInGame > 0)
	{
		//If we haven't added added our resolution to our array then we need to do that before drawing anything
		//the reason why this is here is because getting the resolution at the beginning fails although there are probably better ways to do this
		if(!GotRes)
			UpdateResolution();
		
        for (int i = 0; i < MAXPLAYERS; i++)
        {
			PlayerDataVec enemyVec;
			ReadProcessMemory(HProcHandle, (PVOID)(0x839270 + i * 0x4CC),&enemyVec.isValid,4,NULL);
			
			ReadProcessMemory(HProcHandle, (PVOID)(0x84F2D8 + i * 0x1DC + 0x1C0),&enemyVec.isAlive,4,NULL);
			
			ReadProcessMemory(HProcHandle, (PVOID)(0x84F2D8 + i * 0x1DC + 0xCC),&enemyVec.clientNum,4,NULL);
			
            //read our player's name :), i believe it has up to 16 chars
            for (int x = 0; x < 16; x++)
                ReadProcessMemory(HProcHandle, (PVOID)(0x839270 + i * 0x4CC + (0xC + x)),&enemyVec.name[x], 1,NULL);// only one character at a time
			
            //get our enemy's team
			ReadProcessMemory(HProcHandle, (PVOID)(0x839270 + i * 0x4CC + 0x1C),&enemyVec.team,4,NULL);
			
			//Get our enemy's health
			ReadProcessMemory(HProcHandle, (PVOID)(0x12886A0 + i * 0x274),&enemyVec.health,4,NULL);
			
            //if player is alive, valid and is NOT US then we add him to the list of possible victims and ESP targets
            if (enemyVec.isValid == 1 && enemyVec.clientNum != playerVec.clientNum)
            {
                //Same team as us so color them green
                if (enemyVec.team == playerVec.team)
                    enemyVec.color = Friendly;//Color(0, 255, 0); //change your friendly color here if you like
                //Enemy
                else
					enemyVec.color = Enemy;//change your enemy color here if you like
				
                //grab enemy's pos on the map
				ReadProcessMemory(HProcHandle, (PVOID)(0x839270 + i * 0x4CC + 0x398),&enemyVec.xPos,4,NULL); // 0x4CC size of struct
				ReadProcessMemory(HProcHandle, (PVOID)(0x839270 + i * 0x4CC + 0x39C),&enemyVec.yPos,4,NULL);
				ReadProcessMemory(HProcHandle, (PVOID)(0x839270 + i * 0x4CC + 0x3A0),&enemyVec.zPos,4,NULL);
			    //grab the enemy's pose, we need this incase the enemy is prone that way we move the ESP box down and aimbot lower
				ReadProcessMemory(HProcHandle, (PVOID)(0x839270 + i * 0x4CC + 0x470),&enemyVec.pose,4,NULL);
				
                //Add our new player info to our [array/list]
                //add our enemy information to the list if hes alive otherwise ignore them
                enemiesVec[enemyCount] = enemyVec;
				enemyCount++;
            }
		}
		
		playerVec = GetPlayerVec();
        float dist;
        float drawx;
        float drawy;
		
        //Now we have all our enemies stored and we need to display the ESP on them
        for (int i = 0; i < enemyCount; i++)
        {
			//Find the Distance between Us and the enemy
			dist = SubVectorDist(playerVec, enemiesVec[i]).length() / 48;
			
            //converts our 3d Coordinates to 2d
			if (WorldToScreen(enemiesVec[i].VecCoords(), playerVec.VecCoords()))
            {
                //say that our enemy is ok to aim at
                enemiesVec[i].visible = true;
				
                if ((((enemiesVec[i].pose & 0x08) != 0)) ||
                    ((enemiesVec[i].pose & 0x100) != 0) ||
                    ((enemiesVec[i].pose & 0x200) != 0))
                {
                    drawx = 700.0f / dist;
                    drawy = 400.0f / dist;
                    ScreenY += 4.6f;
                }
                else
                {
                    drawx = 400.0f / dist;
                    drawy = 700.0f / dist;
                }
                //We find exacly where to draw including from our Window's e.g.
                //if the window has been moved since our last loop
                //add an extra bit to our screenX depending on our pos in relation to the en
                //take away any height based on our own position e.g. if were crawling the ESP needs to be slightly adjusted
				
				//DRAW OUR DAMN ESP :) Looks more complicated than it is
				if(Esp_box_Name)
				{
					DrawESPBox(HDC_Desktop,
							   enemiesVec[i].color,
							   (int)(ScreenX)-(drawx/2), // so we don't start our rectangle in the middle of the player but to their left a bit that way the rectangle covers most of the player
							   (int)(ScreenY - (drawy+(drawy/2))), //a bit of a workaround but you can adapt your own pos to your game
							   drawx,
							   drawy,
							   2);
				}
				
				//how our player name with distance
				if(ESPdistance)
				{
					// Create a rectangle to indicate where on the screen TEXT should be drawn
					//THIS IS USED TO POSITION OUR TEXT
					//Left, Top, Right, Bottom that's the order
					RECT nameRect = {(ScreenX)-(drawx/2), //L
						(ScreenY - drawy/2),//T
						(ScreenX)+(drawx*4),//R
						(ScreenY + drawy)};//B
					
					//DRAW OUR PLAYER NAME AND DISTANCE, Get our player's name and distance string to display
					std::stringstream nameNdistance;
					nameNdistance << std::string(enemiesVec[i].name) << " (" << I_O::FloatToString(dist) << "m)";
					
					//Display our Player's name
					//M_font->DrawText(NULL, nameNdistance.str().c_str(), -1, &nameRect, DT_NOCLIP,
					//Text is the Same colour as our ESP rectangle
					DrawString(HDC_Desktop,
							   (ScreenX)-(drawx/2),
							   (ScreenY - drawy/2),
							   (playerVec.team == enemiesVec[i].team ? FriendlyCol : EnemyCol),
							   nameNdistance.str().c_str());
				}
				
				if(ESPhealth)
				{
					//Here we call our function in charge of drawing our ENEMY's health Bar
					DrawEnemyHealth(
									(int)(ScreenX - (drawx/2)),
									(int)(ScreenY - (drawy*1.5)),
									drawx, drawy/4, enemiesVec[i].health,
									(playerVec.team == enemiesVec[i].team ? FriendlyCol : EnemyCol));
				}
				
				if(ESPsnapLine)
				{
					//These are the lines that you see from our player to the enemies
					//Oh snap
					//-----------------------------DRAW SNAP LINES-----------------------
					//Draw From us to the enemies
					DrawLine(HDC_Desktop,
							 resolution[0]/2, //Start X
							 resolution[1],//Start Y
							 (ScreenX - (drawx/2)),//End line X
							 (ScreenY - (drawy/2)),//End line Y
							 //Draw line color based on who we are aiming, Enemies line goes RED, Friendlies GO Green
							 (playerVec.team == enemiesVec[i].team ? FriendlyCol : EnemyCol)
							 );
				}
			}
            else enemiesVec[i].visible = false;
		}
		
		//ESP OVER WE NOW START DEALING WITH AIMBOT :)
		//---------------------------------AIMBOT PART-------------------------------
		//This is a bit buggy but works very well in most cases
		if(ESPaimbot)
		{
			//Get only valid player's for our aimbot to use
			//with this array we filter out any team mates so that we only shoot at enemies
			int validPlayerCounter = 0;
			PlayerDataVec validAimbotEnemies[MAXPLAYERS];
			for (int i = 0; i < enemyCount; i++)
			{
				if (enemiesVec[i].team != playerVec.team)
				{
					validAimbotEnemies[validPlayerCounter] = enemiesVec[i];
					//This is something to do with our Aimbot, here we simply check if we had a previous target, if that target has been killed
					//then we force the game to find a new player to aim at
					if(FocusTarget == validPlayerCounter)
					{
						if(validAimbotEnemies[validPlayerCounter].health < 1)
							FocusTarget = -1;
					}
					validPlayerCounter++;
				}
			}
			
			//check if our hot key is being pressed
			//Mouse 2, if you want to use other hot keys Go here
			//http://msdn.microsoft.com/en-us/library/ms927178.aspx
			//Currently its RIGHT MOUSE KEY 0x02
			if (GetAsyncKeyState(0x02))
			{
				//FocusingOnEnemy = true;
				int target = 0;
				if (/*FocusingOnEnemy && */FocusTarget != -1)
				{
					//If our previous target is still alive we focus on them otherwise go after someone else
					if (validAimbotEnemies[FocusTarget].health > 0)
						target = FocusTarget;
					else target = FindClosestEnemyIndex(validAimbotEnemies, validPlayerCounter, playerVec);
				}
				else//By default aim at the first guy that appears, with this we focus on whos closest to us
					target = FindClosestEnemyIndex(validAimbotEnemies, validPlayerCounter, playerVec);
				
				//if there are more enemies we find the closest one to us then aim
				if (target != -1) //-1 means something went wrong
				{
					dist = SubVectorDist(playerVec, validAimbotEnemies[target]).length() / 48;
					//needs to be called so certain vars are set for our aimsys
					WorldToScreen(validAimbotEnemies[target].VecCoords(), playerVec.VecCoords());
					//(FocusTarget)AS long as our hotkey is pressed we keep looking at the same guy,
					//if this wasn't here and you held down the hotkey button, if another enemy got closer to you than your
					//current target then the aimbot would aim at the other guy.
					//This would be a problem if enemies keep moving around the aimbot is always locking into different people
					FocusTarget = target;
					if ((((validAimbotEnemies[target].pose & 0x08) != 0)) ||
						((validAimbotEnemies[target].pose & 0x100) != 0) ||
						((validAimbotEnemies[target].pose & 0x200) != 0))
					{
						drawx = 700.0f / dist;
						drawy = 400.0f / dist;
						ScreenY += 4.6f;
					}
					else
					{
						drawx = 400.0f / dist;
						drawy = 700.0f / dist;
					}
					
					//We find exacly where to draw including from our Window's e.g.
					//if the window has been moved since our last loop
					int x = (int)(ScreenX);
					int y = (int)(ScreenY - drawy);
					
					//this condition is only here in case all enemies are dead to aim at NO one
					//previously if all were dead it would aim at the last guy killed
					if (validAimbotEnemies[target].health > 0)
					{
						//Incase the window is moved we need to grab window Rect again
						GetWindowRect(Handle, &WindowRect);
						//put our mouse on the enemy, WindowRect is grabbed on our constructor at the top
						//And this is optimized for Windowed Games but will work just fine on Non Windowed
						SetCursorPos((WindowRect.left)  + x, (WindowRect.top) + y);
					}
				}
			}
			//stopped pressing Hotkey aimbot, therefore we look for a new target as soon as its pressed again
			//if player leaves key held we keep aiming at the same target until hes dead or invalid
			else//otherwise we stop staring at them and change targets
				FocusTarget = -1;	
		}
	}
	
	return 0;
}

