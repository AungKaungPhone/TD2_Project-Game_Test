#include <Novice.h>
#include "Boss.h"
#include "Player.h"
#include "Collision.h"

const char kWindowTitle[] = "TD2 Test Run";

// Entry point of the Windows app (main function)
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// Initialize the library / ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1920, 1200);

	// Boxes that receive key input / キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// ---- Created ONCE, before the loop / ループの前に1回だけ作る ----
	Boss boss;
	boss.LoadResources();

	Player player;
	player.LoadResources();

	// Background image, the same size as the window.
	// ウィンドウと同じ大きさの背景画像
	int bgTexture = Novice::LoadTexture("./Resources/Boss/Office.png");

	// Loop until the window's X button is pressed / ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// Start of the frame / フレームの開始
		Novice::BeginFrame();

		// Get key input / キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから (update starts here)
		///
		player.Update(keys, preKeys);

		// Tell the boss where the player is / ボスにプレイヤーの位置を教える
		boss.SetTarget({ player.GetX(), player.GetY() });
		boss.Update();

		// Check every boss attack against the player.
		// ボスの全ての攻撃とプレイヤーの当たり判定を調べる。
		CheckBossAttackHits(boss, player);

		// Test hit on the boss with N (later: your beer can)
		// Nキーでボスにテストダメージ(後でビール缶に置き換える)
		if (preKeys[DIK_N] == 0 && keys[DIK_N] != 0) {
			boss.TakeDamage(20);
		}
		///
		/// ↑更新処理ここまで (update ends here)
		///

		///
		/// ↓描画処理ここから (draw starts here)
		///
		// Draw the background first, so everything else is drawn on top of it.
		// 背景を最初に描画し、ほかのものをその上に重ねる
		Novice::DrawSprite(0, 0, bgTexture, 1.0f, 1.0f, 0.0f, 0xFFFFFFFF);

		boss.Draw();
		player.Draw();
		///
		/// ↑描画処理ここまで (draw ends here)
		///

		// End of the frame / フレームの終了
		Novice::EndFrame();

		// Leave the loop when ESC is pressed / ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// Finalize the library / ライブラリの終了
	Novice::Finalize();
	return 0;
}