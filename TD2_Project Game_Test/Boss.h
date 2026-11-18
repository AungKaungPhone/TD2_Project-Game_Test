#pragma once
#include <vector>

// Delete this line if you already have your own Vector2.
// すでにVector2を定義している場合は、この行を削除する。
struct Vector2 { float x, y; };

enum class BossState { Intro, Idle, Move, Warning, Attack, PhaseChange, Dead };
enum class HazardKind { Chair, Paper, Laptop, Pen, Envelope, Beam, Briefcase };

// One struct for everything the boss throws, so player collision is a single loop.
// ボスが投げるオブジェクトを1つの構造体で管理し、当たり判定をまとめて処理する。
struct Hazard {
    HazardKind kind;
    Vector2 pos;        // CENTER of the object / オブジェクトの中心座標
    Vector2 vel;
    float w, h;         // hit box size / 当たり判定の幅と高さ
    float angle;
    float spin;
    bool alive;
    int timer;          // frames since spawn / 出現してからのフレーム数

    // chair only / イス専用の変数
    bool launched;      // false while it floats beside the boss / ボスの横で浮遊している間はfalse
    int throwAt;        // frame (since spawn) when it gets thrown / 投げられるまでのフレーム数
    Vector2 offset;     // where it floats, relative to the boss / ボスからの相対的な浮遊位置

    // briefcase only / ブリーフケース専用
    Vector2 origin;     // where it was thrown from / 投げ始めた位置
    float outDist;      // how far it flies before turning back / 戻り始めるまでの距離
    int phase;          // 0 = flying out, 1 = pause, 2 = returning / 0=飛んでいく 1=停止 2=戻る
    int phaseTimer;     // frames in the current phase / 現在のフェーズのフレーム数
};

class Boss {
public:
    Boss();

    void LoadResources(); // call ONCE at startup / ゲーム開始時に1回だけ呼び出す
    void Update();
    void Draw();

    void SetTarget(Vector2 playerPos) { target_ = playerPos; }
    void TakeDamage(int dmg); // call when a beer can hits him / ビール缶がボスに当たったときに呼び出す

    bool IsDead() const { return state_ == BossState::Dead; }
    int GetHp() const { return hp_; }
    int GetPhase() const { return phase_; } // 1 or 2 / フェーズ（1 または 2）
    Vector2 GetPos() const { return pos_; }
    float GetWidth() const { return w_; }
    float GetHeight() const { return h_; }
    const std::vector<Hazard>& GetHazards() const { return hazards_; }
    std::vector<Hazard>& GetHazards() { return hazards_; }
    // The non-const version lets Collision remove an attack that hit the player.
    // const なしの版は、プレイヤーに当たった攻撃物をCollisionが消すために使う。

private:
    // ---- tuning / animation sizes (change these when your sprites are ready) ----
    // ---- 調整用の数値・アニメーション設定（スプライト完成後に変更する）----

    static const int kAttackCount = 7;
    // total attacks: 0-2 are used in both phases, 3-6 only in phase 2
    // 攻撃の総数。0〜2は両フェーズで使用、3〜6はフェーズ2のみ

    static const int kPhase1AttackCount = 3;
    // attacks available in phase 1 / フェーズ1で使える攻撃の数

    static const int kIdleFrames = 2;
    // number of levitate/idle frames / 待機・浮遊アニメーションのフレーム数

    static const int kIdleFrameSpeed = 20;
    // game frames per idle frame / 待機アニメーションの1コマを表示するフレーム数

    static const int kAttackFrames = 4;
    // number of "raise hand" frames / 手を上げる攻撃アニメーションのフレーム数

    static const int kAttackFrameSpeed = 8;
    // game frames per attack frame / 攻撃アニメーションの1コマを表示するフレーム数

    static const int kWarningFrames = 45;
    // warning delay before every attack / 各攻撃が始まる前の予告時間

    static const int kRestFrames = 60;
    // pause between attacks / 攻撃と攻撃の間の待機時間

    static const int kFrameW = 128;
    static const int kFrameH = 128;
    // size of ONE sprite frame in pixels / スプライト1コマの大きさ（ピクセル）

    static const int kRestFramesPhase2 = 30;
    // pause between attacks in phase 2 (shorter) / フェーズ2での攻撃の間隔（短くなる）

    static const int kWarningFramesPhase2 = 30;
    // warning delay in phase 2 (shorter) / フェーズ2での予告時間（短くなる）

    static const int kPhaseChangeFrames = 150;
    // length of the roar when phase 2 starts (boss cannot be hurt) / フェーズ2開始時の咆哮の長さ（この間ボスは無敵）

    bool IsPhase2() const { return phase_ == 2; }
    int RestFrames() const { return IsPhase2() ? kRestFramesPhase2 : kRestFrames; }
    int WarningFrames() const { return IsPhase2() ? kWarningFramesPhase2 : kWarningFrames; }

    void Bob();
    void UpdateDead();
    void StartNextAttack();
    void UpdateHazards();
    void UpdateAttackChairs();
    void UpdateAttackPaper();
    void UpdateAttackLaptops();
    void UpdateAttackPens();       // Attack 4 (phase 2): pen darts / 攻撃4（フェーズ2）: ペンの連射
    void UpdateAttackEmails();     // Attack 5 (phase 2): email blast / 攻撃5（フェーズ2）: メールの全方位弾
    void UpdateAttackFired();      // Attack 6 (phase 2): YOU'RE FIRED lock-on beam / 攻撃6（フェーズ2）: ロックオンビーム
    void UpdateAttackBriefcase();  // Attack 7 (phase 2): briefcase boomerang / 攻撃7（フェーズ2）: ブリーフケースのブーメラン
    void SpawnChair(float offX, float offY, int throwAt);
    void DrawHandGlow();

    BossState state_ = BossState::Intro;

    Vector2 pos_{ 960.0f, 1500.0f };
    // starts below the screen / 画面の下から出現する

    Vector2 target_{ 960.0f, 900.0f };
    // player position / プレイヤーの座標

    float w_ = 256.0f, h_ = 256.0f;
    // boss size (match your sprite) / ボスのサイズ（スプライトに合わせる）

    int hp_ = 1500, maxHp_ = 1500;

    int timer_ = 0;
    // frames in the current state / 現在の状態が続いているフレーム数

    int frameCount_ = 0;
    // always counting (idle animation / bobbing) / 常にカウントする（待機アニメーション・上下移動用）

    int attack_ = -1;
    // 0 = chairs, 1 = paper shower, 2 = laptop stampede
    // 0 = イス、1 = 紙吹雪、2 = ノートパソコンの突進

    int phase_ = 1;
    // 1 = normal, 2 = berserk (starts at half HP) / 1 = 通常、2 = 暴走（HPが半分で開始）

    float shakeX_ = 0.0f, shakeY_ = 0.0f;
    // sprite shake while he roars / 咆哮中のスプライトの揺れ

    float deadVelY_ = 0.0f;
    // falling speed after he is defeated / 撃破後の落下速度

    Vector2 aimDir_{ 0.0f, 1.0f };
    // locked aim direction of the current pen burst / ペンの連射で固定した狙いの方向

    bool markerOn_ = false;
    // YOU'RE FIRED: show the target marker / ロックオンの照準を表示するか

    bool locked_ = false;
    // YOU'RE FIRED: the marker has locked on / 照準が固定されたか

    float lockX_ = 960.0f;
    // YOU'RE FIRED: X position of the marker / 照準のX座標

    float moveTargetX_ = 960.0f;
    float handGlow_ = 0.0f;

    int spawnCount_ = 0;
    int nextSpawn_ = 0;

    float hoverY_ = 300.0f;
    // how high he levitates / ボスが浮遊する高さ

    float groundY_ = 1100.0f;
    // floor Y (laptops run along it) - set to your floor
    // 床のY座標（ノートパソコンが走る位置）。ゲームの床に合わせて設定する。

    float attackX_[kAttackCount];
    int attackDuration_[kAttackCount];
    float laneX_[3];

    std::vector<Hazard> hazards_;

    // ---- resources (-1 = not loaded yet, a colored box is drawn instead) ----
    // ---- リソース（-1 = 未読み込み。代わりに色付きの四角形を描画する）----

    int texIdle_ = -1;      // sprite sheet, 2 frames / スプライトシート（2コマ）
    int texAttack_ = -1;    // sprite sheet, 4 frames / スプライトシート（4コマ）
    int texChair_ = -1;
    int texPaper_ = -1;
    int texLaptop_ = -1;
    int texPen_ = -1;
    int texEnvelope_ = -1;
    int texBeam_ = -1;
    int texBriefcase_ = -1;
    int texHandGlow_ = -1;   // hand glow sheet, 2 frames / 手の光のシート（2コマ）
};