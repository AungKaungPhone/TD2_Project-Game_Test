#include <Novice.h>
#include "Boss.h"
#include "Player.h"
#include"BossPlayerCollision.h"

const char kWindowTitle[] = "TD2 Test Run";

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	Novice::Initialize(kWindowTitle, 1920, 1200);

	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// ---- created ONCE, before the loop ----
	Boss boss;
	boss.LoadResources();

	Player player;
	player.LoadResources();

	/*===============
	    確認用
	=================*/

	int playerHp = 10;

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		/// ---- update ----
		player.Update(keys, preKeys);

		boss.SetTarget({ player.GetX(), player.GetY() });
		boss.Update();

		/*   当たり判定     */
		CheckBossPlayerCollision(boss, player, playerHp);


		// test hit on the boss (later: your beer can)
		if (preKeys[DIK_RETURN] == 0 && keys[DIK_RETURN] != 0) {
			boss.TakeDamage(20);
		}

		/// ---- draw ----
		boss.Draw();
		player.Draw();

		/* 確認用  */
		Novice::ScreenPrintf(10, 10, "PlayerHP : %d", playerHp);


		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();
	return 0;
}