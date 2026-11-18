#pragma once
#include<Novice.h>
#include"Player.h"
#include"Boss.h"

bool CheckRectHit();
void CheckBossPlayerCollision(Boss& boss, Player& player, int& playerHp);
