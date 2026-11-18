#include"BossPlayerCollision.h"

/* 無敵の時間 */
const int kInvincibleDuration = 50;

/* 矩形の当たり判定チェック用 */
bool CheckRectHit(
	float x1, float y1, float w1, float h1,
	float x2, float y2, float w2, float h2) {

	float left1 = x1 - w1 / 2;
	float rigth1 = x1 + w1 / 2;
	float top1 = y1 - h1 / 2;
	float bottom1 = y1 + h1 / 2;

	float left2 = x2 - w2 / 2;
	float rigth2 = x2 + w2 / 2;
	float top2 = y2 - h2 / 2;
	float bottom2 = y2 + h2 / 2;

	if (rigth1<left2){
		return false;
	}

	if (left1>rigth2){
		return false;
	}

	if (bottom1<top2){
		return false;
	}

	if (top1>bottom2){
		return false;
	}

	return true;
}

void CheckBossPlayerCollision(Boss& boss, Player& player, int& playerHp) {

	float playerX = player.GetX();
	float playerY = player.GetY();
	float playerW = player.GetWidth();
	float playerH = player.GetHeight();

	if (player.IsInvincible()) {
		return;
	}

	const std::vector<Hazard>& hazards = boss.GetHazards();

	for (int i = 0; i < hazards.size(); i++){

		const Hazard& hazard = hazards[i];

		if (!hazard.alive){
			continue;
		}

		if (CheckRectHit(playerX,playerY,playerW,playerH,
			hazard.pos.x,hazard.pos.y,hazard.w,hazard.h)){
			/* 当たった時の行動を書く */

			playerHp--;
			player.StartInvincibleTimer(kInvincibleDuration);

			break;
		}
	}
}