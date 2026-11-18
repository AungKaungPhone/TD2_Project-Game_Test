#include "Boss.h"
#include <Novice.h>
#include <math.h>
#include <stdlib.h>
#include <algorithm>

namespace {
    // Draws the sprite if it is loaded, otherwise a colored placeholder box.
    // (cx, cy) is the CENTER of the object.
    void DrawTex(int tex, float cx, float cy, float w, float h, float angle, unsigned int fallbackColor) {
        if (tex >= 0) {
            Novice::DrawSprite((int)(cx - w * 0.5f), (int)(cy - h * 0.5f), tex,
                1.0f, 1.0f, angle, 0xFFFFFFFF);
        }
        else {
            Novice::DrawBox((int)(cx - w * 0.5f), (int)(cy - h * 0.5f), (int)w, (int)h,
                angle, fallbackColor, kFillModeSolid);
        }
    }
}

Boss::Boss() {
    for (int i = 0; i < kIdleFrames; i++)   texIdle_[i] = -1;
    for (int i = 0; i < kAttackFrames; i++) texAttack_[i] = -1;

    // where he floats to before each attack: chairs, paper, laptops
    attackX_[0] = 960.0f;
    attackX_[1] = 960.0f;
    attackX_[2] = 1500.0f;

    // total frames of each attack (60 FPS)
    attackDuration_[0] = 300;   // chairs
    attackDuration_[1] = 300;   // paper shower
    attackDuration_[2] = 340;   // laptop stampede

    // the 3 paper lines
    laneX_[0] = 480.0f;
    laneX_[1] = 960.0f;
    laneX_[2] = 1440.0f;
}

// ------------------------------------------------------------------
// Replace the paths with your own files when the sprites are ready.
// Until then everything is drawn as colored boxes.
// ------------------------------------------------------------------
void Boss::LoadResources() {
    //Levitate / idle frames (kIdleFrames of them)
    texIdle_[0] = Novice::LoadTexture("./Resources/Boss/Boss Attack 1.png");
    // texIdle_[1] = Novice::LoadTexture("./Resources/boss/idle_1.png");
    // texIdle_[2] = Novice::LoadTexture("./Resources/boss/idle_2.png");
    // texIdle_[3] = Novice::LoadTexture("./Resources/boss/idle_3.png");

    // Raise-hand attack frames (kAttackFrames of them)
    // texAttack_[0] = Novice::LoadTexture("./Resources/boss/attack_0.png");
    // texAttack_[1] = Novice::LoadTexture("./Resources/boss/attack_1.png");
    // ... one line per frame

    // Things he throws
    // texChair_  = Novice::LoadTexture("./Resources/chair.png");
    // texPaper_  = Novice::LoadTexture("./Resources/paper.png");
    // texLaptop_ = Novice::LoadTexture("./Resources/laptop.png");
}

void Boss::Bob() {
    pos_.y = hoverY_ + sinf(frameCount_ * 0.06f) * 12.0f;
}

void Boss::StartNextAttack() {
    attack_ = (attack_ + 1) % kAttackCount;   // 1 -> 2 -> 3 in order (use rand() for random)
    moveTargetX_ = attackX_[attack_];
    spawnCount_ = 0;
    nextSpawn_ = 0;
    state_ = BossState::Move;
    timer_ = 0;
}

void Boss::TakeDamage(int dmg) {
    if (state_ == BossState::Intro || state_ == BossState::Dead) return;
    hp_ -= dmg;
    if (hp_ <= 0) {
        hp_ = 0;
        state_ = BossState::Dead;
        hazards_.clear();
        handGlow_ = 0.0f;
    }
}

void Boss::Update() {
    frameCount_++;
    timer_++;
    UpdateHazards();

    switch (state_) {
    case BossState::Intro:
        // rises into the air when the scene starts
        pos_.y += (hoverY_ - pos_.y) * 0.03f;
        if (fabsf(pos_.y - hoverY_) < 2.0f) {
            pos_.y = hoverY_;
            state_ = BossState::Idle;
            timer_ = 0;
        }
        break;

    case BossState::Idle:
        Bob();
        if (timer_ >= kRestFrames) StartNextAttack();
        break;

    case BossState::Move:
        // floats sideways, then stops
        pos_.x += (moveTargetX_ - pos_.x) * 0.05f;
        Bob();
        if (fabsf(pos_.x - moveTargetX_) < 3.0f) {
            pos_.x = moveTargetX_;
            state_ = BossState::Warning;
            timer_ = 0;
        }
        break;

    case BossState::Warning:
        // warning delay: hands start to glow, "!" flashes above him
        Bob();
        handGlow_ = (float)timer_ / kWarningFrames;
        if (timer_ >= kWarningFrames) {
            state_ = BossState::Attack;
            timer_ = 0;
        }
        break;

    case BossState::Attack:
        Bob();
        switch (attack_) {
        case 0: UpdateAttackChairs();   break;
        case 1: UpdateAttackPaper();    break;
        case 2: UpdateAttackLaptops();  break;
        }
        // glow fades after the hand is raised
        if (timer_ > kAttackFrames * kAttackFrameSpeed) handGlow_ *= 0.95f;
        if (timer_ >= attackDuration_[attack_]) {
            state_ = BossState::Idle;
            timer_ = 0;
            handGlow_ = 0.0f;
        }
        break;

    case BossState::Dead:
        pos_.y += 6.0f;   // placeholder: sinks down
        break;
    }
}

// ------------------------------------------------------------------
// Attack 1: two chairs levitate beside him for 3 sec, then get thrown
// ------------------------------------------------------------------
void Boss::SpawnChair(float offX, float offY, int throwAt) {
    Hazard c{};
    c.kind = HazardKind::Chair;
    c.w = 120.0f; c.h = 140.0f;
    c.pos = { pos_.x + offX, pos_.y + offY + 250.0f };   // starts low, rises up
    c.offset = { offX, offY };
    c.throwAt = throwAt;
    c.alive = true;
    hazards_.push_back(c);
}

void Boss::UpdateAttackChairs() {
    const int spawnAt = 20;          // while the hand goes up
    const int floatFrames = 180;     // 3 seconds at 60 FPS

    if (timer_ == spawnAt) {
        SpawnChair(-260.0f, 40.0f, floatFrames);         // left chair
        SpawnChair(260.0f, 40.0f, floatFrames + 40);    // right chair, thrown a bit later
    }
}

// ------------------------------------------------------------------
// Attack 2: Paper Shower, short delay, then rain in 3 lines
// ------------------------------------------------------------------
void Boss::UpdateAttackPaper() {
    const int delay = 70;            // lanes flash during this time
    const int rainFrames = 150;

    if (timer_ < delay) return;
    if (timer_ < delay + rainFrames && timer_ % 3 == 0) {
        for (int lane = 0; lane < 3; lane++) {
            Hazard p{};
            p.kind = HazardKind::Paper;
            p.w = 50.0f; p.h = 70.0f;
            p.pos = { laneX_[lane] + (float)(rand() % 81 - 40), -60.0f };
            p.vel = { 0.0f, 16.0f + (float)(rand() % 6) };
            p.alive = true;
            hazards_.push_back(p);
        }
    }
}

// ------------------------------------------------------------------
// Attack 3: Laptop Stampede, laptops appear at the left and charge right
// ------------------------------------------------------------------
void Boss::UpdateAttackLaptops() {
    const int delay = 50;            // left edge flashes during this time
    const int totalLaptops = 10;

    if (timer_ < delay) return;
    if (spawnCount_ < totalLaptops) {
        if (--nextSpawn_ <= 0) {
            Hazard l{};
            l.kind = HazardKind::Laptop;
            l.w = 130.0f; l.h = 90.0f;
            l.pos = { -100.0f, groundY_ - l.h * 0.5f };
            l.vel = { 20.0f + (float)(rand() % 7), 0.0f };
            l.alive = true;
            hazards_.push_back(l);

            spawnCount_++;
            nextSpawn_ = 12 + rand() % 14;     // random gap = uneven herd
        }
    }
}

// ------------------------------------------------------------------
void Boss::UpdateHazards() {
    const float kChairSpeed = 24.0f;

    for (auto& h : hazards_) {
        h.timer++;

        switch (h.kind) {
        case HazardKind::Chair:
            if (!h.launched) {
                // levitate beside the boss
                float wantX = pos_.x + h.offset.x;
                float wantY = pos_.y + h.offset.y + sinf(h.timer * 0.15f) * 10.0f;
                h.pos.x += (wantX - h.pos.x) * 0.1f;
                h.pos.y += (wantY - h.pos.y) * 0.1f;
                h.angle = sinf(h.timer * 0.1f) * 0.2f;

                if (h.timer >= h.throwAt) {         // throw at the player
                    float dx = target_.x - h.pos.x;
                    float dy = target_.y - h.pos.y;
                    float len = sqrtf(dx * dx + dy * dy);
                    if (len < 1.0f) len = 1.0f;
                    h.vel = { dx / len * kChairSpeed, dy / len * kChairSpeed };
                    h.spin = 0.15f;
                    h.launched = true;
                }
            }
            else {
                h.pos.x += h.vel.x;
                h.pos.y += h.vel.y;
                h.angle += h.spin;
                if (h.pos.x < -200 || h.pos.x > 2120 || h.pos.y < -200 || h.pos.y > 1400)
                    h.alive = false;
            }
            break;

        case HazardKind::Paper:
            h.pos.x += sinf(h.timer * 0.15f) * 1.5f;       // flutter
            h.pos.y += h.vel.y;
            h.angle = sinf(h.timer * 0.15f) * 0.4f;
            if (h.pos.y > 1300.0f) h.alive = false;
            break;

        case HazardKind::Laptop:
            h.pos.x += h.vel.x;
            // little gallop hop
            h.pos.y = groundY_ - h.h * 0.5f - fabsf(sinf(h.timer * 0.25f)) * 25.0f;
            if (h.pos.x > 2120.0f) h.alive = false;
            break;
        }
    }

    hazards_.erase(std::remove_if(hazards_.begin(), hazards_.end(),
        [](const Hazard& h) { return !h.alive; }), hazards_.end());
}

// ------------------------------------------------------------------
void Boss::DrawHandGlow() {
    if (handGlow_ <= 0.01f) return;

    Vector2 hands[2] = {
        { pos_.x - 150.0f, pos_.y - 110.0f },    // match these to your sprite
        { pos_.x + 150.0f, pos_.y - 110.0f }
    };
    float pulse = 0.7f + 0.3f * sinf(frameCount_ * 0.4f);

    for (int i = 0; i < 2; i++) {
        for (int layer = 0; layer < 3; layer++) {
            float r = (70.0f - layer * 20.0f) * handGlow_ * pulse;
            int a = (int)((60 + layer * 60) * handGlow_);
            unsigned int color = 0x66CCFF00u | (unsigned int)a;
            Novice::DrawEllipse((int)hands[i].x, (int)hands[i].y, (int)r, (int)r, 0.0f,
                color, kFillModeSolid);
        }
    }
}

void Boss::Draw() {
    // ---- warnings (drawn behind everything) ----
    if (state_ == BossState::Attack && attack_ == 1 && timer_ < 70) {          // paper lanes
        if ((timer_ / 6) % 2 == 0) {
            for (int i = 0; i < 3; i++)
                Novice::DrawBox((int)laneX_[i] - 60, 0, 120, 1200, 0.0f, 0xFF000040, kFillModeSolid);
        }
    }
    if (state_ == BossState::Attack && attack_ == 2 && timer_ < 50) {          // stampede start
        if ((timer_ / 6) % 2 == 0)
            Novice::DrawBox(0, (int)groundY_ - 140, 120, 140, 0.0f, 0xFF000080, kFillModeSolid);
    }

    // ---- boss ----
    int tex = -1;
    if (state_ == BossState::Attack) {
        int f = timer_ / kAttackFrameSpeed;
        if (f >= kAttackFrames) f = kAttackFrames - 1;     // hold the last frame
        tex = texAttack_[f];
    }
    else {
        tex = texIdle_[(frameCount_ / kIdleFrameSpeed) % kIdleFrames];
    }
    DrawTex(tex, pos_.x, pos_.y, w_, h_, 0.0f, 0xCC3333FF);
    DrawHandGlow();

    // "!" above his head during the warning delay
    if (state_ == BossState::Warning && (timer_ / 5) % 2 == 0) {
        Novice::DrawBox((int)pos_.x - 15, (int)(pos_.y - h_ * 0.5f) - 100, 30, 60, 0.0f, 0xFFFF00FF, kFillModeSolid);
        Novice::DrawBox((int)pos_.x - 15, (int)(pos_.y - h_ * 0.5f) - 30, 30, 20, 0.0f, 0xFFFF00FF, kFillModeSolid);
    }

    // ---- things he throws ----
    for (const auto& h : hazards_) {
        switch (h.kind) {
        case HazardKind::Chair:  DrawTex(texChair_, h.pos.x, h.pos.y, h.w, h.h, h.angle, 0x8B5A2BFF); break;
        case HazardKind::Paper:  DrawTex(texPaper_, h.pos.x, h.pos.y, h.w, h.h, h.angle, 0xFFFFFFFF); break;
        case HazardKind::Laptop: DrawTex(texLaptop_, h.pos.x, h.pos.y, h.w, h.h, h.angle, 0x888888FF); break;
        }
    }

    // ---- HP bar ----
    Novice::DrawBox(460, 40, 1000, 24, 0.0f, 0x333333FF, kFillModeSolid);
    Novice::DrawBox(460, 40, (int)(1000.0f * hp_ / maxHp_), 24, 0.0f, 0xFF0000FF, kFillModeSolid);
}