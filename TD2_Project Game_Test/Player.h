#pragma once

// ============================================================
//  Player class: run left/right, jump and HP
//  プレイヤークラス: 左右移動、ジャンプ、HP
// ============================================================
class Player {
public:
	Player();

	void LoadResources();                                  // call ONCE at startup / 起動時に1回だけ呼ぶ
	void Update(const char* keys, const char* preKeys);    // every frame / 毎フレーム
	void Draw();                                           // every frame / 毎フレーム

	// Must match Boss groundY_ / ボスのgroundY_と同じ値にすること
	void SetGroundY(float y) {
		groundY_ = y;
		if (onGround_) y_ = groundY_ - h_ * 0.5f;
	}

	// ---- Getters (for collision) / ゲッター(当たり判定用) ----
	float GetX() const { return x_; }                      // CENTER / 中心座標
	float GetY() const { return y_; }
	float GetWidth() const { return w_; }
	float GetHeight() const { return h_; }
	bool IsOnGround() const { return onGround_; }

	// ---- HP / 体力 ----
	void TakeDamage(int damage);                           // call when a boss attack hits / ボスの攻撃が当たったときに呼ぶ
	bool IsDead() const { return hp_ <= 0; }
	bool IsInvincible() const { return invincibleTimer_ > 0; }
	int GetHp() const { return hp_; }
	int GetMaxHp() const { return maxHp_; }

private:
	// ---- Update steps / 更新の手順 ----
	void HandleMove(const char* keys);
	void HandleJump(const char* keys, const char* preKeys);
	void ApplyGravity();
	void HandleLanding();
	void DrawShadow();
	void DrawHpBar();

	// ---- Tuning / 調整用の数値 ----
	float speed_ = 9.0f;           // run speed (pixels per frame) / 走る速さ
	float jumpPower_ = 24.0f;      // start speed of the jump / ジャンプの初速
	float gravity_ = 1.0f;         // pulls down every frame / 毎フレーム下に引く力
	float maxFallSpeed_ = 30.0f;   // fall speed limit / 落下速度の上限
	float cutVelocity_ = 8.0f;     // letting go early cuts the rise to this (short hop) / 早く離すと上昇がこの速さに(小ジャンプ)
	int maxJumps_ = 1;             // set to 2 for a double jump / 2にすると二段ジャンプ

	static const int kInvincibleFrames = 60;   // invincible for 1 second after a hit / 被弾後1秒間は無敵

	// ---- State / 状態 ----
	float x_ = 960.0f;
	float y_ = 1040.0f;            // center / 中心
	float w_ = 60.0f;
	float h_ = 120.0f;
	float velY_ = 0.0f;            // up is negative / 上方向がマイナス
	float groundY_ = 1100.0f;      // floor Y / 床のY座標
	bool onGround_ = true;
	int jumpsUsed_ = 0;

	// ---- HP / 体力 ----
	int hp_ = 100;
	int maxHp_ = 100;
	int invincibleTimer_ = 0;      // frames of invincibility left / 残りの無敵フレーム数

	// ---- Resources (-1 = not loaded, a green box is drawn instead) ----
	// ---- リソース(-1 = 未読み込み。代わりに緑の箱を描画) ----
	int texPlayer_ = -1;
};