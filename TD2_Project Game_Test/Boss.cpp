#include "Boss.h"
#include <Novice.h>
#include <math.h>
#include <stdlib.h>
#include <algorithm>

namespace {
    // Draw the sprite if it is loaded; otherwise, draw a colored placeholder box.
    // テクスチャが読み込まれていれば画像を描画し、なければ仮の色付き四角形を描画する。
    // (cx, cy) is the center position of the object.
    // (cx, cy) はオブジェクトの中心座標。
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

    // X positions where the boss moves before each attack.
    // 各攻撃の前にボスが移動するX座標
    attackX_[0] = 960.0f;
    attackX_[1] = 960.0f;
    attackX_[2] = 1500.0f;

    // Duration of each attack in frames (60 FPS).
    // 各攻撃の継続時間（60FPS）
    attackDuration_[0] = 300;   // Chair attack.
    // イス攻撃
    attackDuration_[1] = 300;   // Paper shower attack.
    // 紙吹雪攻撃
    attackDuration_[2] = 340;   // Laptop stampede attack.
    // ノートパソコン突進攻撃

    // X positions of the three paper-fall lanes.
    // 紙が落ちてくる3つのレーンのX座標
    laneX_[0] = 480.0f;
    laneX_[1] = 960.0f;
    laneX_[2] = 1440.0f;
}

// ------------------------------------------------------------------
// Replace these paths with your own image files when the sprites are ready.
// スプライトが完成したら、使用する画像ファイルのパスに変更する。
// Until then, colored boxes are used as placeholders.
// 画像が読み込まれない場合は、代わりに色付きの四角形を表示する。
// ------------------------------------------------------------------
void Boss::LoadResources() {
    // Load the idle / floating animation frames.
    // 待機・浮遊アニメーションの画像を読み込む
    texIdle_[0] = Novice::LoadTexture("./Resources/Boss/Boss Attack 1.png");
    // texIdle_[1] = Novice::LoadTexture("./Resources/boss/idle_1.png");
    // texIdle_[2] = Novice::LoadTexture("./Resources/boss/idle_2.png");
    // texIdle_[3] = Novice::LoadTexture("./Resources/boss/idle_3.png");

    // Load the attack animation frames.
    // 攻撃アニメーションの画像を読み込む
    // texAttack_[0] = Novice::LoadTexture("./Resources/boss/attack_0.png");
    // texAttack_[1] = Novice::LoadTexture("./Resources/boss/attack_1.png");
    // Add one loading line for each animation frame.
    // フレーム数に合わせて読み込み処理を追加する

    // Load the images of objects thrown by the boss.
    // ボスが投げる物の画像を読み込む
    // texChair_  = Novice::LoadTexture("./Resources/chair.png");
    // texPaper_  = Novice::LoadTexture("./Resources/paper.png");
    // texLaptop_ = Novice::LoadTexture("./Resources/laptop.png");
}

void Boss::Bob() {
    // Use the sine function to make the boss gently bob up and down.
    // sin関数を使って、ボスを上下にゆっくり揺らす
    pos_.y = hoverY_ + sinf(frameCount_ * 0.06f) * 12.0f;
}

void Boss::StartNextAttack() {
    attack_ = (attack_ + 1) % kAttackCount;   // Switch attacks in order: 1 -> 2 -> 3 (use rand() for random order).
    // 攻撃を1、2、3の順番で切り替える（ランダムにする場合はrand()を使う）
    moveTargetX_ = attackX_[attack_];
    spawnCount_ = 0;
    nextSpawn_ = 0;
    state_ = BossState::Move;
    timer_ = 0;
}

void Boss::TakeDamage(int dmg) {
    if (state_ == BossState::Intro || state_ == BossState::Dead) return;
    hp_ -= dmg; // Reduce HP by the amount of damage received. / 受けたダメージ分だけHPを減らす
    if (hp_ <= 0) {
        hp_ = 0;
        state_ = BossState::Dead;
        hazards_.clear();
        handGlow_ = 0.0f;
    }
}

void Boss::Update() {
    frameCount_++; // Advance the animation frame counter. / アニメーション用のフレーム数を進める
    timer_++; // Count frames spent in the current state. / 現在の状態が続いたフレーム数を数える
    UpdateHazards(); // Update the positions of chairs, paper, and laptops. / イス・紙・ノートパソコンの位置を更新する

    switch (state_) {
    case BossState::Intro:
        // The boss rises into the air when the scene starts.
        // シーン開始時にボスが空中へ上昇する
        pos_.y += (hoverY_ - pos_.y) * 0.03f;
        if (fabsf(pos_.y - hoverY_) < 2.0f) {
            pos_.y = hoverY_;
            state_ = BossState::Idle;
            timer_ = 0;
        }
        break;

    case BossState::Idle:
        Bob();
        if (timer_ >= kRestFrames) StartNextAttack(); // Start the next attack after the rest period. / 待機時間が終わったら次の攻撃へ
        break;

    case BossState::Move:
        // The boss moves sideways to the attack position.
        // 攻撃位置まで横に移動する
        pos_.x += (moveTargetX_ - pos_.x) * 0.05f;
        Bob();
        if (fabsf(pos_.x - moveTargetX_) < 3.0f) {
            pos_.x = moveTargetX_;
            state_ = BossState::Warning;
            timer_ = 0;
        }
        break;

    case BossState::Warning:
        // Attack warning: the hands glow and an exclamation mark flashes above the boss.
        // 攻撃前の予告。手が光り、頭上に「!」を表示する
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
        // Fade the hand glow after the attack animation starts.
        // 攻撃開始後、手の光を徐々に弱くする
        if (timer_ > kAttackFrames * kAttackFrameSpeed) handGlow_ *= 0.95f;
        if (timer_ >= attackDuration_[attack_]) {
            state_ = BossState::Idle;
            timer_ = 0;
            handGlow_ = 0.0f;
        }
        break;

    case BossState::Dead:
        pos_.y += 6.0f;   // Placeholder death animation: move the boss downward.
        // 仮の撃破演出として、ボスを下へ移動させる
        break;
    }
}

// ------------------------------------------------------------------
// Attack 1: two chairs float beside the boss, then get thrown.
// 攻撃1：イスが左右に浮かび、一定時間後に投げられる
// ------------------------------------------------------------------
void Boss::SpawnChair(float offX, float offY, int throwAt) {
    Hazard c{};
    c.kind = HazardKind::Chair;
    c.w = 120.0f; c.h = 140.0f;
    c.pos = { pos_.x + offX, pos_.y + offY + 250.0f };   // The chair starts low and rises beside the boss.
    // 最初は低い位置に出現し、ボスの横まで上昇する
    c.offset = { offX, offY };
    c.throwAt = throwAt;
    c.alive = true;
    hazards_.push_back(c);
}

void Boss::UpdateAttackChairs() {
    const int spawnAt = 20;          // Spawn the chairs while the boss is raising his hands.
    // ボスが手を上げている間にイスを出現させる
    const int floatFrames = 180;     // Float for 3 seconds at 60 FPS.
    // 60FPSで3秒間浮遊する

    if (timer_ == spawnAt) {
        SpawnChair(-260.0f, 40.0f, floatFrames);         // Left chair.
        // 左側のイス
        SpawnChair(260.0f, 40.0f, floatFrames + 40);    // Right chair is thrown a little later.
        // 右側のイスは少し遅れて投げる
    }
}

// ------------------------------------------------------------------
// Attack 2: after a short warning, paper falls in three lanes.
// 攻撃2：少し予告した後、3つのレーンから紙を降らせる
// ------------------------------------------------------------------
void Boss::UpdateAttackPaper() {
    const int delay = 70;            // Flash the paper lanes during this delay.
    // この時間の間、紙が落ちるレーンを点滅表示する
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
// Attack 3: laptops appear on the left and charge to the right.
// 攻撃3：画面左からノートパソコンが右方向へ突進する
// ------------------------------------------------------------------
void Boss::UpdateAttackLaptops() {
    const int delay = 50;            // Flash the left edge during this delay.
    // この時間の間、画面左端を点滅表示する
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
            nextSpawn_ = 12 + rand() % 14;     // Use random spawn intervals to create an uneven stream.
            // 出現間隔をランダムにして、ばらつきを作る
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
                // Make the chair float beside the boss.
                // ボスの左右で浮遊させる
                float wantX = pos_.x + h.offset.x;
                float wantY = pos_.y + h.offset.y + sinf(h.timer * 0.15f) * 10.0f;
                h.pos.x += (wantX - h.pos.x) * 0.1f;
                h.pos.y += (wantY - h.pos.y) * 0.1f;
                h.angle = sinf(h.timer * 0.1f) * 0.2f;

                if (h.timer >= h.throwAt) {         // Aim and throw the chair toward the player.
                    // プレイヤーの方向へ投げる
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
            h.pos.x += sinf(h.timer * 0.15f) * 1.5f;       // Fall while swaying from side to side.
            // 左右に揺れながら落下する
            h.pos.y += h.vel.y;
            h.angle = sinf(h.timer * 0.15f) * 0.4f;
            if (h.pos.y > 1300.0f) h.alive = false;
            break;

        case HazardKind::Laptop:
            h.pos.x += h.vel.x;
            // Add a small hopping motion.
            // 跳ねるような動きを加える
            h.pos.y = groundY_ - h.h * 0.5f - fabsf(sinf(h.timer * 0.25f)) * 25.0f;
            if (h.pos.x > 2120.0f) h.alive = false;
            break;
        }
    }

    // Remove hazards that are no longer active, such as objects that left the screen.
    // 画面外などで消えたオブジェクトを配列から削除する
    hazards_.erase(std::remove_if(hazards_.begin(), hazards_.end(),
        [](const Hazard& h) { return !h.alive; }), hazards_.end());
}

// ------------------------------------------------------------------
void Boss::DrawHandGlow() {
    if (handGlow_ <= 0.01f) return;

    Vector2 hands[2] = {
        { pos_.x - 150.0f, pos_.y - 110.0f },    // Adjust these hand positions to match the boss sprite.
        // 使用するスプライトに合わせて手の位置を調整する
        { pos_.x + 150.0f, pos_.y - 110.0f }
    };
    // Use the sine function to make the glow pulse.
    // sin関数で光の強さを周期的に変化させる
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
    // ---- Attack warnings (drawn behind other objects) ----
    // ---- 攻撃の予告表示（ほかのオブジェクトより後ろに描画）----
    if (state_ == BossState::Attack && attack_ == 1 && timer_ < 70) {          // Paper-fall lanes.
        // 紙が落ちるレーン
        if ((timer_ / 6) % 2 == 0) {
            for (int i = 0; i < 3; i++)
                Novice::DrawBox((int)laneX_[i] - 60, 0, 120, 1200, 0.0f, 0xFF000040, kFillModeSolid);
        }
    }
    if (state_ == BossState::Attack && attack_ == 2 && timer_ < 50) {          // Starting position of the laptop stampede.
        // ノートパソコン突進の開始位置
        if ((timer_ / 6) % 2 == 0)
            Novice::DrawBox(0, (int)groundY_ - 140, 120, 140, 0.0f, 0xFF000080, kFillModeSolid);
    }

    // ---- Boss ----
    // ---- ボス本体 ----
    int tex = -1;
    if (state_ == BossState::Attack) {
        int f = timer_ / kAttackFrameSpeed;
        if (f >= kAttackFrames) f = kAttackFrames - 1;     // Keep displaying the final animation frame.
        // 最後のアニメーションフレームを表示し続ける
        tex = texAttack_[f];
    }
    else {
        tex = texIdle_[(frameCount_ / kIdleFrameSpeed) % kIdleFrames];
    }
    // Draw the boss using the selected texture or a placeholder box.
    // 選択した画像、または仮の四角形でボスを描画する
    DrawTex(tex, pos_.x, pos_.y, w_, h_, 0.0f, 0xCC3333FF);
    DrawHandGlow();

    // Flash an exclamation mark above the boss during the warning.
    // 攻撃予告中、ボスの頭上に「!」を点滅表示する
    if (state_ == BossState::Warning && (timer_ / 5) % 2 == 0) {
        Novice::DrawBox((int)pos_.x - 15, (int)(pos_.y - h_ * 0.5f) - 100, 30, 60, 0.0f, 0xFFFF00FF, kFillModeSolid);
        Novice::DrawBox((int)pos_.x - 15, (int)(pos_.y - h_ * 0.5f) - 30, 30, 20, 0.0f, 0xFFFF00FF, kFillModeSolid);
    }

    // ---- Objects thrown by the boss ----
    // ---- ボスが投げる物 ----
    for (const auto& h : hazards_) {
        switch (h.kind) {
        case HazardKind::Chair:  DrawTex(texChair_, h.pos.x, h.pos.y, h.w, h.h, h.angle, 0x8B5A2BFF); break;
        case HazardKind::Paper:  DrawTex(texPaper_, h.pos.x, h.pos.y, h.w, h.h, h.angle, 0xFFFFFFFF); break;
        case HazardKind::Laptop: DrawTex(texLaptop_, h.pos.x, h.pos.y, h.w, h.h, h.angle, 0x888888FF); break;
        }
    }

    // ---- HP bar ----
    // ---- HPバー ----
    // Draw the HP bar background and the red bar based on current HP.
    // HPの背景と、現在のHPに応じた赤いゲージを描画する
    Novice::DrawBox(460, 40, 1000, 24, 0.0f, 0x333333FF, kFillModeSolid);
    Novice::DrawBox(460, 40, (int)(1000.0f * hp_ / maxHp_), 24, 0.0f, 0xFF0000FF, kFillModeSolid);
}