#pragma once

class Player {
public:
    Player();

    void LoadResources();                                  // call ONCE at startup
    void Update(const char* keys, const char* preKeys);    // pass your keys / preKeys arrays
    void Draw();

    void SetGroundY(float y) { groundY_ = y; }             // must match Boss groundY_

    float GetX() const { return x_; }                      // CENTER of the player
    float GetY() const { return y_; }
    float GetWidth() const { return w_; }
    float GetHeight() const { return h_; }
    bool  IsOnGround() const { return onGround_; }

private:
    // ---- tuning ----
    float speed_ = 9.0f;          // run speed (pixels per frame)
    float jumpPower_ = 24.0f;     // start velocity of the jump
    float gravity_ = 1.0f;        // pulled down every frame
    float maxFallSpeed_ = 30.0f;
    float cutVelocity_ = 8.0f;    // releasing jump early cuts the rise to this (short hop)
    int   maxJumps_ = 1;          // set to 2 for a double jump

    // ---- state ----
    float x_ = 960.0f;
    float y_ = 990.0f;            // center
    float w_ = 60.0f;
    float h_ = 120.0f;
    float velY_ = 0.0f;
    float groundY_ = 1050.0f;     // floor Y
    bool  onGround_ = true;
    int   jumpsUsed_ = 0;

    // ---- resources (-1 = not loaded yet, a green box is drawn instead) ----
    int texPlayer_ = -1;
};