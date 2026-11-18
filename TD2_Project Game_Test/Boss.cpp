#include "Boss.h"
#include <Novice.h>
#include <math.h>
#include <stdlib.h>
#include <algorithm>

namespace {
    // ---- Phase 2 sprite sizes (change here if your images differ) ----
    // ---- フェーズ2のスプライトの大きさ（画像のサイズが違う場合はここを変更）----

    // Envelope and pencil: one 128x128 image each.
    // 封筒と鉛筆: それぞれ128x128の画像1枚
    const int kEnvelopeSrcW = 128;
    const int kEnvelopeSrcH = 128;
    const float kEnvelopeDrawSize = 128.0f;   // size on screen / 画面上の大きさ
    const int kPenSrcW = 128;
    const int kPenSrcH = 128;
    const float kPenDrawSize = 128.0f;        // size on screen / 画面上の大きさ
    // Which way the pencil TIP points in your image, in degrees (the code turns the tip toward the player).
    // 画像の中で鉛筆の先端が向いている方向（度）。この値を元に、先端がプレイヤーを向くように回転させる。
    //   0 = right, 90 = down, 180 = left, -90 = up
    //   0 = 右、90 = 下、180 = 左、-90 = 上
    //   45 = down-right, 135 = down-left, -45 = up-right, -135 = up-left
    //   45 = 右下、135 = 左下、-45 = 右上、-135 = 左上
    const float kPenTipAngleDeg = 90.0f;
    const float kDegToRad = 0.0174532925f;    // degrees to radians / 度からラジアンへの変換

    // The boss figure ends at this row of the 128x128 sprite frame (his feet).
    // ボスの絵は、128x128のスプライト1コマのこの行で終わっている（足の位置）。
    const int kBossFeetRow = 122;

    // Fall after he is defeated: a smaller gravity and speed limit = a slower fall.
    // 撃破後の落下: 重力と最大速度を小さくするほど、ゆっくり落ちる。
    const float kDeadGravity = 0.3f;
    const float kDeadMaxFallSpeed = 8.0f;

    // YOU'RE FIRED laser beam: 5 frames, each 256x512 (sheet 1280x512).
    // YOU'RE FIREDのレーザー: 5コマ、1コマ256x512（シート1280x512）
    const int kBeamFrames = 5;
    const int kBeamFrameW = 256;
    const int kBeamFrameH = 512;
    const int kBeamFrameSpeed = 4;            // game frames per beam frame / 1コマを表示するフレーム数

    // Hand glow: 2 frames, sheet 64x32 (each frame 32x32).
    // 手の光: 2コマ、シート64x32（1コマ32x32）
    const int kGlowFrames = 2;
    const int kGlowFrameW = 32;
    const int kGlowFrameH = 32;
    const int kGlowFrameSpeed = 8;            // game frames per glow frame / 1コマを表示するフレーム数
    const float kGlowDrawSize = 128.0f;       // width on screen / 画面上の幅

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

    // Draw ONE frame of a sprite sheet (frames are lined up left to right).
    // スプライトシート（コマが横一列に並んだ画像）から1コマを描画する。
    // (cx, cy) is the center position, w and h are the size on screen.
    // (cx, cy) は中心座標、w と h は画面上での大きさ。
    void DrawSheetFrame(int tex, int frame, int frameW, int frameH,
        float cx, float cy, float w, float h, unsigned int fallbackColor, unsigned int tint = 0xFFFFFFFF) {
        int left = (int)(cx - w * 0.5f);
        int top = (int)(cy - h * 0.5f);
        int right = (int)(cx + w * 0.5f);
        int bottom = (int)(cy + h * 0.5f);
        if (tex >= 0) {
            Novice::DrawQuad(left, top, right, top, left, bottom, right, bottom,
                frame * frameW, 0, frameW, frameH, tex, tint);
        }
        else {
            Novice::DrawBox(left, top, (int)w, (int)h, 0.0f, fallbackColor, kFillModeSolid);
        }
    }

    // Draw a sprite (or one frame of a sheet) stretched to w x h and rotated around its center.
    // スプライト（またはシートの1コマ）を w x h に拡大縮小し、中心を軸に回転させて描画する。
    // If the texture is not loaded, a colored box of the HIT BOX size (hitW x hitH) is drawn instead.
    // 画像が読み込まれていない場合は、代わりに当たり判定サイズ(hitW x hitH)の色付き四角形を描画する。
    void DrawSpriteRotated(int tex, int srcX, int srcY, int srcW, int srcH,
        float cx, float cy, float w, float h, float angle,
        float hitW, float hitH, unsigned int fallbackColor) {
        if (tex < 0) {
            DrawTex(-1, cx, cy, hitW, hitH, angle, fallbackColor);
            return;
        }
        float c = cosf(angle);
        float s = sinf(angle);
        float hw = w * 0.5f;
        float hh = h * 0.5f;
        // Corners relative to the center: left-top, right-top, left-bottom, right-bottom.
        // 中心からの相対位置: 左上、右上、左下、右下
        float px[4] = { -hw,  hw, -hw, hw };
        float py[4] = { -hh, -hh,  hh, hh };
        int x[4];
        int y[4];
        for (int i = 0; i < 4; i++) {
            x[i] = (int)(cx + px[i] * c - py[i] * s);
            y[i] = (int)(cy + px[i] * s + py[i] * c);
        }
        Novice::DrawQuad(x[0], y[0], x[1], y[1], x[2], y[2], x[3], y[3],
            srcX, srcY, srcW, srcH, tex, 0xFFFFFFFF);
    }
}

Boss::Boss() {
    // X positions where the boss moves before each attack.
    // 各攻撃の前にボスが移動するX座標
    attackX_[0] = 960.0f;
    attackX_[1] = 960.0f;
    attackX_[2] = 1500.0f;
    attackX_[3] = 960.0f;   // Pen darts (phase 2) / ペンの連射（フェーズ2）
    attackX_[4] = 960.0f;   // Email blast (phase 2) / メールの全方位弾（フェーズ2）
    attackX_[5] = 960.0f;   // YOU'RE FIRED (phase 2) / ロックオンビーム（フェーズ2）
    attackX_[6] = 960.0f;   // Briefcase boomerang (phase 2) / ブリーフケース（フェーズ2）

    // Duration of each attack in frames (60 FPS).
    // 各攻撃の継続時間（60FPS）
    attackDuration_[0] = 300;   // Chair attack.
    // イス攻撃
    attackDuration_[1] = 300;   // Paper shower attack.
    // 紙吹雪攻撃
    attackDuration_[2] = 340;   // Laptop stampede attack.
    // ノートパソコン突進攻撃
    attackDuration_[3] = 170;   // Pen darts (phase 2).
    // ペンの連射（フェーズ2）
    attackDuration_[4] = 170;   // Email blast (phase 2).
    // メールの全方位弾（フェーズ2）
    attackDuration_[5] = 240;   // YOU'RE FIRED lock-on beam (phase 2).
    // ロックオンビーム（フェーズ2）
    attackDuration_[6] = 240;   // Briefcase boomerang (phase 2).
    // ブリーフケースのブーメラン（フェーズ2）

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
    // Sprite sheet: 2 frames, 256x128 (each frame is 128x128).
    // スプライトシート: 2コマ、256x128（1コマは128x128）
    texIdle_ = Novice::LoadTexture("./Resources/Boss/BOSS Idle.png");

    // Load the attack animation frames.
    // 攻撃アニメーションの画像を読み込む
    // Sprite sheet: 4 frames, 512x128 (each frame is 128x128).
    // スプライトシート: 4コマ、512x128（1コマは128x128）
    texAttack_ = Novice::LoadTexture("./Resources/Boss/BOSS Attack Animation.png");

    // Load the images of objects thrown by the boss.
    // ボスが投げる物の画像を読み込む
    // texChair_  = Novice::LoadTexture("./Resources/chair.png");
    // texPaper_  = Novice::LoadTexture("./Resources/paper.png");
    // texLaptop_ = Novice::LoadTexture("./Resources/laptop.png");
    // ---- Phase 2 attack sprites / フェーズ2の攻撃のスプライト ----
    texPen_ = Novice::LoadTexture("./Resources/Boss/Pencil1.png");                     // 128x128
    texEnvelope_ = Novice::LoadTexture("./Resources/Boss/Envelope.png");              // 128x128
    texBeam_ = Novice::LoadTexture("./Resources/Boss/LaserBean.png");                 // 5 frames, 256x512 each / 5コマ、1コマ256x512
    texHandGlow_ = Novice::LoadTexture("./Resources/Boss/HandGlow Animation.png");    // 2 frames, sheet 64x32 / 2コマ、シート64x32
    // texBriefcase_ = Novice::LoadTexture("./Resources/briefcase.png");
}

void Boss::Bob() {
    // Use the sine function to make the boss gently bob up and down.
    // sin関数を使って、ボスを上下にゆっくり揺らす
    pos_.y = hoverY_ + sinf(frameCount_ * 0.06f) * 12.0f;
}

void Boss::StartNextAttack() {
    if (IsPhase2()) {
        // Phase 2: pick a random attack, but never the same one twice in a row.
        // フェーズ2: ランダムに攻撃を選ぶ（同じ攻撃が2回続くことはない）
        int next = attack_;
        while (next == attack_) next = rand() % kAttackCount;
        attack_ = next;
    }
    else {
        attack_ = (attack_ + 1) % kPhase1AttackCount;   // Switch attacks in order: 1 -> 2 -> 3.
        // 攻撃を1、2、3の順番で切り替える
    }
    moveTargetX_ = attackX_[attack_];
    spawnCount_ = 0;
    nextSpawn_ = 0;
    state_ = BossState::Move;
    timer_ = 0;
}

void Boss::TakeDamage(int dmg) {
    // The boss cannot be hurt while rising in or while changing phase.
    // 登場中とフェーズ移行中はダメージを受けない
    if (state_ == BossState::Intro || state_ == BossState::PhaseChange || state_ == BossState::Dead) return;

    hp_ -= dmg; // Reduce HP by the amount of damage received. / 受けたダメージ分だけHPを減らす
    if (hp_ <= 0) {
        hp_ = 0;
        state_ = BossState::Dead;
        hazards_.clear();
        handGlow_ = 0.0f;
    }
    else if (phase_ == 1 && hp_ <= maxHp_ / 2) {
        // Half HP: start phase 2. Everything on screen disappears so the player gets a fair start.
        // HPが半分になったのでフェーズ2へ。画面上の攻撃物を消して、プレイヤーが公平に始められるようにする
        phase_ = 2;
        hazards_.clear();
        handGlow_ = 0.0f;
        state_ = BossState::PhaseChange;
        timer_ = 0;
    }
}

// ------------------------------------------------------------------
// Defeated: the boss falls to the ground. After landing he stays there
// and plays the normal idle animation.
// 撃破: ボスは地面まで落下する。着地した後はその場で、
// 通常の待機アニメーションを再生する。
// ------------------------------------------------------------------
void Boss::UpdateDead() {
    // Landing position: his FEET touch the floor (groundY_).
    // 着地位置: ボスの足が床（groundY_）につく位置
    float scaleY = h_ / (float)kFrameH;
    float landY = groundY_ - ((float)kBossFeetRow - (float)kFrameH * 0.5f) * scaleY;

    if (pos_.y < landY) {
        deadVelY_ += kDeadGravity;               // Gravity. / 重力
        if (deadVelY_ > kDeadMaxFallSpeed) deadVelY_ = kDeadMaxFallSpeed;
        pos_.y += deadVelY_;
        if (pos_.y >= landY) {                   // Landed. / 着地
            pos_.y = landY;
            deadVelY_ = 0.0f;
        }
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
        if (timer_ >= RestFrames()) StartNextAttack(); // Start the next attack after the rest period. / 待機時間が終わったら次の攻撃へ
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
        handGlow_ = (float)timer_ / (float)WarningFrames();
        if (timer_ >= WarningFrames()) {
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
        case 3: UpdateAttackPens();       break;   // Phase 2 only. / フェーズ2のみ
        case 4: UpdateAttackEmails();     break;   // Phase 2 only. / フェーズ2のみ
        case 5: UpdateAttackFired();      break;   // Phase 2 only. / フェーズ2のみ
        case 6: UpdateAttackBriefcase();  break;   // Phase 2 only. / フェーズ2のみ
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

    case BossState::PhaseChange:
        // Phase 2 starts: the boss roars in the center, shakes, and his hands glow red.
        // フェーズ2の開始。ボスは中央で咆哮し、揺れながら手が赤く光る
        pos_.x += (960.0f - pos_.x) * 0.05f;
        Bob();
        handGlow_ = (float)timer_ / 60.0f;
        if (handGlow_ > 1.0f) handGlow_ = 1.0f;
        shakeX_ = (float)(rand() % 13 - 6);
        shakeY_ = (float)(rand() % 13 - 6);
        if (timer_ >= kPhaseChangeFrames) {
            shakeX_ = 0.0f;
            shakeY_ = 0.0f;
            handGlow_ = 0.0f;
            state_ = BossState::Idle;
            timer_ = 0;
        }
        break;

    case BossState::Dead:
        UpdateDead();
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

    // Phase 2: 3 chairs, shorter float time, faster throws.
    // フェーズ2: イスが3個になり、浮遊時間が短く、投げるのも速くなる
    const int floatFrames = IsPhase2() ? 120 : 180;   // 2 seconds (phase 2) / 3 seconds (phase 1)
    const int throwGap = IsPhase2() ? 30 : 40;        // Delay between each throw. / 投げる間隔

    if (timer_ == spawnAt) {
        SpawnChair(-260.0f, 40.0f, floatFrames);               // Left chair.
        // 左側のイス
        SpawnChair(260.0f, 40.0f, floatFrames + throwGap);     // Right chair is thrown a little later.
        // 右側のイスは少し遅れて投げる
        if (IsPhase2()) {
            SpawnChair(0.0f, -230.0f, floatFrames + throwGap * 2);   // Phase 2 only: a third chair above the boss.
            // フェーズ2のみ: ボスの頭上に3個目のイス
        }
    }
}

// ------------------------------------------------------------------
// Attack 2: after a short warning, paper falls in three lanes.
// 攻撃2：少し予告した後、3つのレーンから紙を降らせる
// ------------------------------------------------------------------
void Boss::UpdateAttackPaper() {
    const int delay = 70;            // Flash the paper lanes during this delay.
    // この時間の間、紙が落ちるレーンを点滅表示する

    // Phase 2: the rain lasts longer, drops more often and falls faster.
    // フェーズ2: 雨が長く続き、紙の数が増え、落下も速くなる
    const int rainFrames = IsPhase2() ? 160 : 150;
    const int interval = IsPhase2() ? 2 : 3;                 // Frames between drops. / 紙を出す間隔
    const float fallSpeed = IsPhase2() ? 20.0f : 16.0f;

    if (timer_ < delay) return;
    if (timer_ < delay + rainFrames && timer_ % interval == 0) {
        for (int lane = 0; lane < 3; lane++) {
            Hazard p{};
            p.kind = HazardKind::Paper;
            p.w = 50.0f; p.h = 70.0f;
            p.pos = { laneX_[lane] + (float)(rand() % 81 - 40), -60.0f };
            p.vel = { 0.0f, fallSpeed + (float)(rand() % 6) };
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

    // Phase 2: more laptops, faster, with shorter gaps between them.
    // フェーズ2: ノートパソコンの数が増え、速くなり、間隔も短くなる
    const int totalLaptops = IsPhase2() ? 16 : 10;
    const float baseSpeed = IsPhase2() ? 26.0f : 20.0f;
    const int gapMin = IsPhase2() ? 8 : 12;
    const int gapRange = IsPhase2() ? 10 : 14;

    if (timer_ < delay) return;
    if (spawnCount_ < totalLaptops) {
        if (--nextSpawn_ <= 0) {
            Hazard l{};
            l.kind = HazardKind::Laptop;
            l.w = 130.0f; l.h = 90.0f;
            l.pos = { -100.0f, groundY_ - l.h * 0.5f };
            l.vel = { baseSpeed + (float)(rand() % 7), 0.0f };
            l.alive = true;
            hazards_.push_back(l);

            spawnCount_++;
            nextSpawn_ = gapMin + rand() % gapRange;     // Use random spawn intervals to create an uneven stream.
            // 出現間隔をランダムにして、ばらつきを作る
        }
    }
}

// ------------------------------------------------------------------
// Attack 4 (phase 2): Pen darts. Fast bursts aimed at the player.
// The aim is locked when each burst starts, so the player can dodge by moving.
// 攻撃4（フェーズ2）：ペンの連射。プレイヤーに向けて高速で撃つ。
// 狙いは連射の開始時に固定されるので、動けば避けられる。
// ------------------------------------------------------------------
void Boss::UpdateAttackPens() {
    const int startAt = 30;          // Start after the hand is raised. / 手を上げた後に開始
    const int burstCount = 2;
    const int pensPerBurst = 6;
    const int gap = 5;               // Frames between pens. / ペンの間隔
    const int burstLen = 50;         // Frames per burst (pens + pause). / 1回の連射の長さ
    const float speed = 28.0f;

    if (timer_ < startAt) return;
    int t = timer_ - startAt;
    int burst = t / burstLen;
    int local = t % burstLen;
    if (burst >= burstCount) return;

    if (local == 0) {   // Lock the aim at the start of each burst. / 連射の開始時に狙いを固定
        float dx = target_.x - pos_.x;
        float dy = target_.y - pos_.y;
        float len = sqrtf(dx * dx + dy * dy);
        if (len < 1.0f) len = 1.0f;
        aimDir_ = { dx / len, dy / len };
    }

    if (local % gap == 0 && local / gap < pensPerBurst) {
        Hazard p{};
        p.kind = HazardKind::Pen;
        p.w = 64.0f; p.h = 64.0f;
        p.pos = pos_;
        p.vel = { aimDir_.x * speed, aimDir_.y * speed };
        p.angle = atan2f(aimDir_.y, aimDir_.x);
        p.alive = true;
        hazards_.push_back(p);
    }
}

// ------------------------------------------------------------------
// Attack 5 (phase 2): Email blast. Rings of envelopes burst out of the boss.
// Every other ring is turned by half a step, so the gaps move.
// 攻撃5（フェーズ2）：メールの全方位弾。ボスから封筒がリング状に広がる。
// 1つおきにリングを半分ずらすので、隙間の位置が変わる。
// ------------------------------------------------------------------
void Boss::UpdateAttackEmails() {
    const int startAt = 30;
    const int ringGap = 35;          // Frames between rings. / リングの間隔
    const int ringCount = 3;
    const int perRing = 16;          // Envelopes in one ring. / 1リングの封筒の数
    const float speed = 9.0f;
    const float kPi = 3.14159265f;

    if (timer_ < startAt) return;
    int t = timer_ - startAt;
    if (t % ringGap != 0) return;
    int ring = t / ringGap;
    if (ring >= ringCount) return;

    float offsetAngle = (ring % 2 == 1) ? kPi / (float)perRing : 0.0f;   // Half a step on odd rings. / 奇数番目は半分ずらす
    for (int i = 0; i < perRing; i++) {
        float a = offsetAngle + (2.0f * kPi * (float)i) / (float)perRing;
        Hazard e{};
        e.kind = HazardKind::Envelope;
        e.w = 84.0f; e.h = 64.0f;
        e.pos = pos_;
        e.vel = { cosf(a) * speed, sinf(a) * speed };
        e.alive = true;
        hazards_.push_back(e);
    }
}

// ------------------------------------------------------------------
// Attack 6 (phase 2): YOU'RE FIRED. A marker follows the player, locks on,
// then a beam strikes that spot. The player has to keep moving.
// 攻撃6（フェーズ2）：YOU'RE FIRED。照準がプレイヤーを追いかけて固定され、
// その場所にビームが落ちる。プレイヤーは動き続ける必要がある。
// ------------------------------------------------------------------
void Boss::UpdateAttackFired() {
    const int startAt = 30;
    const int strikeCount = 2;
    const int strikeLen = 90;        // Frames per strike. / 1回の攻撃の長さ
    const int trackFrames = 50;      // The marker follows the player. / 照準が追いかける時間
    const int lockFrames = 25;       // The marker is locked and flashing. / 照準が固定されて点滅する時間

    markerOn_ = false;
    locked_ = false;
    if (timer_ < startAt) return;

    int t = timer_ - startAt;
    int strike = t / strikeLen;
    int local = t % strikeLen;
    if (strike >= strikeCount) return;

    if (local < trackFrames) {
        lockX_ = target_.x;
        markerOn_ = true;
    }
    else if (local < trackFrames + lockFrames) {
        markerOn_ = true;
        locked_ = true;
    }
    else if (local == trackFrames + lockFrames) {
        // The beam covers the whole height of the screen. / ビームは画面の高さ全体を覆う
        Hazard b{};
        b.kind = HazardKind::Beam;
        b.w = 110.0f; b.h = 1200.0f;
        b.pos = { lockX_, 600.0f };
        b.alive = true;
        hazards_.push_back(b);
    }
}

// ------------------------------------------------------------------
// Attack 7 (phase 2): Briefcase boomerang. It flies out, pauses, then comes
// back along the same path, so dodging once is not enough.
// 攻撃7（フェーズ2）：ブリーフケースのブーメラン。飛んでいって一瞬止まり、
// 同じ道を戻ってくる。1回避けるだけでは足りない。
// ------------------------------------------------------------------
void Boss::UpdateAttackBriefcase() {
    const int throwAt[2] = { 30, 110 };   // Throw frames. / 投げるフレーム
    const float speed = 22.0f;
    const float outDist = 800.0f;

    for (int i = 0; i < 2; i++) {
        if (timer_ != throwAt[i]) continue;

        float dx = target_.x - pos_.x;
        float dy = target_.y - pos_.y;
        float len = sqrtf(dx * dx + dy * dy);
        if (len < 1.0f) len = 1.0f;

        Hazard b{};
        b.kind = HazardKind::Briefcase;
        b.w = 140.0f; b.h = 100.0f;
        b.pos = pos_;
        b.origin = pos_;
        b.vel = { dx / len * speed, dy / len * speed };
        b.outDist = outDist;
        b.spin = 0.25f;
        b.alive = true;
        hazards_.push_back(b);
    }
}

// ------------------------------------------------------------------
void Boss::UpdateHazards() {
    const float kChairSpeed = IsPhase2() ? 30.0f : 24.0f;   // Chairs fly faster in phase 2. / フェーズ2ではイスが速く飛ぶ

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

        case HazardKind::Pen:
        case HazardKind::Envelope:
            // Fly in a straight line until they leave the screen.
            // 画面外に出るまでまっすぐ飛ぶ
            h.pos.x += h.vel.x;
            h.pos.y += h.vel.y;
            if (h.pos.x < -200 || h.pos.x > 2120 || h.pos.y < -200 || h.pos.y > 1400)
                h.alive = false;
            break;

        case HazardKind::Beam:
            // The beam only stays for a short moment.
            // ビームはごく短い間だけ残る
            if (h.timer > kBeamFrames * kBeamFrameSpeed) h.alive = false;
            break;

        case HazardKind::Briefcase:
        {
            h.angle += h.spin;
            float speed = sqrtf(h.vel.x * h.vel.x + h.vel.y * h.vel.y);
            if (h.phase == 0) {
                // Flying out. / 飛んでいく
                h.pos.x += h.vel.x;
                h.pos.y += h.vel.y;
                float dx = h.pos.x - h.origin.x;
                float dy = h.pos.y - h.origin.y;
                if (dx * dx + dy * dy >= h.outDist * h.outDist) {
                    h.phase = 1;
                    h.phaseTimer = 0;
                }
            }
            else if (h.phase == 1) {
                // Pause at the far end and spin faster. / 先端で止まって速く回る
                h.phaseTimer++;
                h.angle += 0.2f;
                if (h.phaseTimer >= 25) h.phase = 2;
            }
            else {
                // Come back along the same path. / 同じ道を戻る
                float dx = h.origin.x - h.pos.x;
                float dy = h.origin.y - h.pos.y;
                float len = sqrtf(dx * dx + dy * dy);
                if (len < 30.0f) {
                    h.alive = false;
                }
                else {
                    h.pos.x += dx / len * speed;
                    h.pos.y += dy / len * speed;
                }
            }
            break;
        }

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

    // Hand positions measured on the 128x128 sprite frame, relative to the frame center.
    // 128x128のスプライト1コマ上で測った手の位置（コマの中心からの相対位置）。
    // Arms down: idle and the first 2 attack frames. Arms raised: the last 2 attack frames.
    // 腕を下げている間: 待機と攻撃の最初の2コマ。腕を上げている間: 攻撃の最後の2コマ。
    bool armsRaised = (state_ == BossState::Attack) && (timer_ / kAttackFrameSpeed >= 2);
    float leftX = armsRaised ? -47.0f : -30.0f;
    float rightX = armsRaised ? 44.0f : 28.0f;
    float handY = armsRaised ? 17.5f : 20.0f;

    // The sprite is drawn scaled to w_ x h_, so scale the hand positions the same way.
    // スプライトは w_ x h_ に拡大して描画するので、手の位置も同じ倍率にする。
    float scaleX = w_ / (float)kFrameW;
    float scaleY = h_ / (float)kFrameH;

    Vector2 hands[2] = {
        { pos_.x + leftX * scaleX,  pos_.y + handY * scaleY },
        { pos_.x + rightX * scaleX, pos_.y + handY * scaleY }
    };
    // Use the sine function to make the glow pulse.
    // sin関数で光の強さを周期的に変化させる
    float pulse = 0.7f + 0.3f * sinf(frameCount_ * 0.4f);

    // Glow sprite: a 2-frame animation drawn on both hands. It grows and fades in as the glow ramps up.
    // 手の光のスプライト: 2コマのアニメーションを両手に描画する。光が強くなるにつれて大きく、濃くなる。
    if (texHandGlow_ >= 0) {
        int glowFrame = (frameCount_ / kGlowFrameSpeed) % kGlowFrames;
        float strength = (handGlow_ > 1.0f) ? 1.0f : handGlow_;
        float glowW = kGlowDrawSize * (0.5f + 0.5f * strength) * pulse;
        float glowH = glowW * (float)kGlowFrameH / (float)kGlowFrameW;
        unsigned int glowColor = (IsPhase2() ? 0xFF886600u : 0xFFFFFF00u) | (unsigned int)(255.0f * strength);   // Reddish in phase 2. / フェーズ2では赤っぽくなる
        for (int i = 0; i < 2; i++) {
            DrawSheetFrame(texHandGlow_, glowFrame, kGlowFrameW, kGlowFrameH,
                hands[i].x, hands[i].y, glowW, glowH, 0x00000000, glowColor);
        }
        return;
    }

    // No sprite yet: draw the placeholder glow. / スプライトがない場合: 仮の光を描画する
    for (int i = 0; i < 2; i++) {
        for (int layer = 0; layer < 3; layer++) {
            float r = (70.0f - layer * 20.0f) * handGlow_ * pulse;
            int a = (int)((60 + layer * 60) * handGlow_);
            unsigned int baseColor = IsPhase2() ? 0xFF553300u : 0x66CCFF00u;   // Red glow in phase 2. / フェーズ2では赤く光る
            unsigned int color = baseColor | (unsigned int)a;
            Novice::DrawEllipse((int)hands[i].x, (int)hands[i].y, (int)r, (int)r, 0.0f,
                color, kFillModeSolid);
        }
    }
}

void Boss::Draw() {
    // Red screen flash while the boss roars (phase change).
    // 咆哮中（フェーズ移行中）は画面を赤く点滅させる
    if (state_ == BossState::PhaseChange && (timer_ / 8) % 2 == 0) {
        Novice::DrawBox(0, 0, 1920, 1200, 0.0f, 0xFF000030, kFillModeSolid);
    }

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

    // YOU'RE FIRED: a marker follows the player, then locks and flashes before the beam.
    // YOU'RE FIRED: 照準がプレイヤーを追いかけ、ビームの前に固定されて点滅する
    if (state_ == BossState::Attack && attack_ == 5 && markerOn_) {
        unsigned int markerColor = 0xFFFF00A0;
        if (locked_) markerColor = ((timer_ / 3) % 2 == 0) ? 0xFF0000FF : 0xFFFFFFFF;
        Novice::DrawBox((int)lockX_ - 4, 0, 8, 1200, 0.0f, markerColor, kFillModeSolid);
        Novice::DrawEllipse((int)lockX_, (int)groundY_, 70, 18, 0.0f, markerColor, kFillModeSolid);
    }

    // ---- Boss ----
    // ---- ボス本体 ----
    int tex = texIdle_;
    int frame = (frameCount_ / kIdleFrameSpeed) % kIdleFrames;   // Idle animation loops.
    // 待機アニメーションはループする
    if (state_ == BossState::Attack) {
        tex = texAttack_;
        frame = timer_ / kAttackFrameSpeed;
        if (frame >= kAttackFrames) frame = kAttackFrames - 1;   // Keep displaying the final animation frame.
        // 最後のアニメーションフレームを表示し続ける
    }
    // Draw one frame of the sprite sheet, or a placeholder box if it is not loaded.
    // スプライトシートから1コマを描画する。読み込まれていない場合は仮の四角形を描画する
    // Phase 2: the boss turns red. He flashes white while roaring.
    // フェーズ2: ボスが赤くなる。咆哮中は白く点滅する
    // After he is defeated, he returns to his normal color.
    // 撃破された後は、元の色に戻る
    unsigned int tint = (IsPhase2() && state_ != BossState::Dead) ? 0xFFA0A0FF : 0xFFFFFFFF;
    if (state_ == BossState::PhaseChange && (timer_ / 4) % 2 == 0) tint = 0xFFFFFFFF;
    DrawSheetFrame(tex, frame, kFrameW, kFrameH, pos_.x + shakeX_, pos_.y + shakeY_, w_, h_, 0xCC3333FF, tint);
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
        case HazardKind::Pen:
            // The pencil is rotated to face the direction it flies.
            // 鉛筆は飛んでいく向きに合わせて回転させる
            DrawSpriteRotated(texPen_, 0, 0, kPenSrcW, kPenSrcH, h.pos.x, h.pos.y,
                kPenDrawSize, kPenDrawSize, h.angle - kPenTipAngleDeg * kDegToRad, h.w, h.h, 0x3366FFFF);
            break;
        case HazardKind::Envelope:
            DrawSpriteRotated(texEnvelope_, 0, 0, kEnvelopeSrcW, kEnvelopeSrcH, h.pos.x, h.pos.y,
                kEnvelopeDrawSize, kEnvelopeDrawSize, h.angle, h.w, h.h, 0xFFFFCCFF);
            break;
        case HazardKind::Beam:
        {
            // Play the 5 beam frames, then hold the last one.
            // 5コマのレーザーアニメーションを再生し、最後のコマで止める
            int beamFrame = h.timer / kBeamFrameSpeed;
            if (beamFrame >= kBeamFrames) beamFrame = kBeamFrames - 1;
            DrawSpriteRotated(texBeam_, beamFrame * kBeamFrameW, 0, kBeamFrameW, kBeamFrameH, h.pos.x, h.pos.y,
                (float)kBeamFrameW, h.h, 0.0f, h.w, h.h, 0xFF3333C0);
            break;
        }
        case HazardKind::Briefcase: DrawTex(texBriefcase_, h.pos.x, h.pos.y, h.w, h.h, h.angle, 0x8B4513FF); break;
        }
    }

    // ---- HP bar ----
    // ---- HPバー ----
    // Draw the HP bar background and the red bar based on current HP.
    // HPの背景と、現在のHPに応じた赤いゲージを描画する
    Novice::DrawBox(460, 40, 1000, 24, 0.0f, 0x333333FF, kFillModeSolid);
    Novice::DrawBox(460, 40, (int)(1000.0f * hp_ / maxHp_), 24, 0.0f, 0xFF0000FF, kFillModeSolid);
    // White marker at 50% HP, where phase 2 starts. / HPが50%の位置（フェーズ2が始まる位置）に白い目印
    Novice::DrawBox(958, 36, 4, 32, 0.0f, 0xFFFFFFFF, kFillModeSolid);
}