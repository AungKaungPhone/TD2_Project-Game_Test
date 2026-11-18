#include "Player.h"
#include <Novice.h>

namespace {
    const float kScreenW = 1920.0f;
}

Player::Player() {
    // start standing on the floor
    y_ = groundY_ - h_ * 0.5f;
}

// Replace the path with your own sprite when it is ready.
void Player::LoadResources() {
    // texPlayer_ = Novice::LoadTexture("./Resources/player.png");
}

/*============ 無敵時間用 ===========*/
bool Player::IsInvincible() const {
    return invincibleTimer_ > 0;
}

void Player::StartInvincibleTimer(int duration) {
    invincibleTimer_ = duration;
}

void Player::Update(const char* keys, const char* preKeys) {
    // ---------------- run left / right ----------------
    float move = 0.0f;
    if (keys[DIK_A] != 0 || keys[DIK_LEFT] != 0)  move -= 1.0f;
    if (keys[DIK_D] != 0 || keys[DIK_RIGHT] != 0) move += 1.0f;
    x_ += move * speed_;

    // keep inside the screen
    float halfW = w_ * 0.5f;
    if (x_ < halfW) x_ = halfW;
    if (x_ > kScreenW - halfW) x_ = kScreenW - halfW;

    // ---------------- jump ----------------
    bool jumpPressed =
        (preKeys[DIK_SPACE] == 0 && keys[DIK_SPACE] != 0) ||
        (preKeys[DIK_W] == 0 && keys[DIK_W] != 0);
    bool jumpHeld = keys[DIK_SPACE] != 0 || keys[DIK_W] != 0;

    if (jumpPressed && jumpsUsed_ < maxJumps_) {
        velY_ = -jumpPower_;          // up is negative Y
        jumpsUsed_++;
        onGround_ = false;
    }

    // let go early = shorter jump (hold longer = higher jump)
    if (!jumpHeld && velY_ < -cutVelocity_) {
        velY_ = -cutVelocity_;
    }

    // ---------------- gravity ----------------
    velY_ += gravity_;
    if (velY_ > maxFallSpeed_) velY_ = maxFallSpeed_;
    y_ += velY_;

    // ---------------- landing ----------------
    float feetY = y_ + h_ * 0.5f;
    if (feetY >= groundY_) {
        y_ = groundY_ - h_ * 0.5f;
        velY_ = 0.0f;
        onGround_ = true;
        jumpsUsed_ = 0;
    }
    else {
        onGround_ = false;
    }

    /*====================
        無敵時間
    ====================*/
    if (invincibleTimer_ > 0) {
        invincibleTimer_--;
    }
}

void Player::Draw() {
    // shadow on the floor: gets smaller the higher you jump
    float height = groundY_ - (y_ + h_ * 0.5f);      // 0 on the ground
    float scale = 1.0f - height / 600.0f;
    if (scale < 0.4f) scale = 0.4f;
    Novice::DrawEllipse((int)x_, (int)groundY_, (int)(40.0f * scale), (int)(10.0f * scale),
        0.0f, 0x000000A0, kFillModeSolid);

    // player (sprite if loaded, otherwise a green box)
    int left = (int)(x_ - w_ * 0.5f);
    int top = (int)(y_ - h_ * 0.5f);
    if (texPlayer_ >= 0) {
        Novice::DrawSprite(left, top, texPlayer_, 1.0f, 1.0f, 0.0f, 0xFFFFFFFF);
    }
    else {
        Novice::DrawBox(left, top, (int)w_, (int)h_, 0.0f, 0x00FF00FF, kFillModeSolid);
    }
}