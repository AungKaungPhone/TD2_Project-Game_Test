#include "Player.h"
#include <Novice.h>

namespace {
	const float kScreenW = 1920.0f;

	// Shadow on the floor / 床の影
	const float kShadowW = 40.0f;
	const float kShadowH = 10.0f;
	const float kShadowFadeHeight = 600.0f;   // shadow is smallest at this height / この高さで影が最小
	const float kShadowMinScale = 0.4f;
	const unsigned int kColorShadow = 0x000000A0;

	const unsigned int kColorPlayer = 0x00FF00FF;   // placeholder box / 仮の箱の色

	// HP bar (bottom left) / HPバー(左下)
	const int kHpBarX = 40;
	const int kHpBarY = 1130;
	const int kHpBarW = 500;
	const int kHpBarH = 28;
	const int kHpBarBorder = 3;
	const unsigned int kColorHpFrame = 0xFFFFFFFF;
	const unsigned int kColorHpBack = 0x202020FF;
	const unsigned int kColorHpGreen = 0x00C800FF;   // green HP / 緑のHP
}

Player::Player() {
	// start standing on the floor / 床の上に立った状態で開始
	y_ = groundY_ - h_ * 0.5f;
}

// Replace the path with your own sprite when it is ready.
// スプライトができたら、パスを自分のファイルに変えてコメントを外してください。
void Player::LoadResources() {
	// texPlayer_ = Novice::LoadTexture("./Resources/player.png");
}

// ============================================================
//  Update: every frame / 更新: 毎フレーム
// ============================================================
void Player::Update(const char* keys, const char* preKeys) {
	if (invincibleTimer_ > 0) invincibleTimer_--;

	// A defeated player cannot move, but still falls to the floor.
	// 倒されたプレイヤーは動けないが、床までは落ちる。
	if (!IsDead()) {
		HandleMove(keys);
		HandleJump(keys, preKeys);
	}
	ApplyGravity();
	HandleLanding();
}

// ---- Take damage from a boss attack / ボスの攻撃でダメージを受ける ----
void Player::TakeDamage(int damage) {
	if (IsDead() || IsInvincible()) return;

	hp_ -= damage;
	if (hp_ < 0) hp_ = 0;
	invincibleTimer_ = kInvincibleFrames;   // invincible for a moment / しばらく無敵
}

// ---- Run left / right (A D or arrow keys) / 左右移動(A D または矢印キー) ----
void Player::HandleMove(const char* keys) {
	float move = 0.0f;
	if (keys[DIK_A] != 0 || keys[DIK_LEFT] != 0)  move -= 1.0f;
	if (keys[DIK_D] != 0 || keys[DIK_RIGHT] != 0) move += 1.0f;
	x_ += move * speed_;

	// keep inside the screen / 画面の外に出ない
	float halfW = w_ * 0.5f;
	if (x_ < halfW) x_ = halfW;
	if (x_ > kScreenW - halfW) x_ = kScreenW - halfW;
}

// ---- Jump (Space or W): hold = higher, tap = short hop ----
// ---- ジャンプ(Space または W): 長押しで高く、ちょん押しで低く ----
void Player::HandleJump(const char* keys, const char* preKeys) {
	bool jumpPressed =
		(preKeys[DIK_SPACE] == 0 && keys[DIK_SPACE] != 0) ||
		(preKeys[DIK_W] == 0 && keys[DIK_W] != 0);
	bool jumpHeld = keys[DIK_SPACE] != 0 || keys[DIK_W] != 0;

	// start a jump / ジャンプ開始
	if (jumpPressed && jumpsUsed_ < maxJumps_) {
		velY_ = -jumpPower_;   // up is negative Y / 上はマイナス
		jumpsUsed_++;
		onGround_ = false;
	}

	// let go early = shorter jump / 早くボタンを離すと低いジャンプ
	if (!jumpHeld && velY_ < -cutVelocity_) {
		velY_ = -cutVelocity_;
	}
}

// ---- Gravity / 重力 ----
void Player::ApplyGravity() {
	velY_ += gravity_;
	if (velY_ > maxFallSpeed_) velY_ = maxFallSpeed_;
	y_ += velY_;
}

// ---- Landing on the floor / 床に着地 ----
void Player::HandleLanding() {
	float feetY = y_ + h_ * 0.5f;

	if (feetY >= groundY_) {
		y_ = groundY_ - h_ * 0.5f;
		velY_ = 0.0f;
		onGround_ = true;
		jumpsUsed_ = 0;   // can jump again / またジャンプできる
	}
	else {
		onGround_ = false;
	}
}

// ============================================================
//  Draw / 描画
// ============================================================
void Player::Draw() {
	DrawShadow();

	// Blink while invincible (skip drawing every few frames).
	// 無敵の間は点滅させる(数フレームおきに描画しない)。
	bool hidden = invincibleTimer_ > 0 && (invincibleTimer_ / 4) % 2 == 0;

	// player: sprite if loaded, otherwise a green box / スプライトがあれば描画、なければ緑の箱
	if (!hidden) {
		int left = (int)(x_ - w_ * 0.5f);
		int top = (int)(y_ - h_ * 0.5f);

		if (texPlayer_ >= 0) {
			Novice::DrawSprite(left, top, texPlayer_, 1.0f, 1.0f, 0.0f, 0xFFFFFFFF);
		}
		else {
			Novice::DrawBox(left, top, (int)w_, (int)h_, 0.0f, kColorPlayer, kFillModeSolid);
		}
	}

	DrawHpBar();
}

// Shadow gets smaller the higher you jump, so it is easy to judge the landing.
// 高くジャンプするほど影が小さくなり、着地位置が分かりやすい。
void Player::DrawShadow() {
	float height = groundY_ - (y_ + h_ * 0.5f);   // 0 on the ground / 地面なら0
	float scale = 1.0f - height / kShadowFadeHeight;
	if (scale < kShadowMinScale) scale = kShadowMinScale;

	Novice::DrawEllipse((int)x_, (int)groundY_,
		(int)(kShadowW * scale), (int)(kShadowH * scale),
		0.0f, kColorShadow, kFillModeSolid);
}

// Green HP bar in the bottom left corner. / 左下の緑のHPバー
void Player::DrawHpBar() {
	int fillW = kHpBarW * hp_ / maxHp_;

	// white frame, dark background, then the green bar / 白い枠、暗い背景、緑のゲージの順に描画
	Novice::DrawBox(kHpBarX - kHpBarBorder, kHpBarY - kHpBarBorder,
		kHpBarW + kHpBarBorder * 2, kHpBarH + kHpBarBorder * 2, 0.0f, kColorHpFrame, kFillModeSolid);
	Novice::DrawBox(kHpBarX, kHpBarY, kHpBarW, kHpBarH, 0.0f, kColorHpBack, kFillModeSolid);
	Novice::DrawBox(kHpBarX, kHpBarY, fillW, kHpBarH, 0.0f, kColorHpGreen, kFillModeSolid);
}