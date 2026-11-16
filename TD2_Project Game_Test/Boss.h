
#pragma once
#include <vector>

// Delete this line if you already have your own Vector2.
// すでにVector2を定義している場合は、この行を削除する。
struct Vector2 { float x, y; };

enum class BossState { Intro, Idle, Move, Warning, Attack, Dead };
enum class HazardKind { Chair, Paper, Laptop };

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
    Vector2 GetPos() const { return pos_; }
    float GetWidth() const { return w_; }
    float GetHeight() const { return h_; }
    const std::vector<Hazard>& GetHazards() const { return hazards_; }

private:
    // ---- tuning / animation sizes (change these when your sprites are ready) ----
    // ---- 調整用の数値・アニメーション設定（スプライト完成後に変更する）----

    static const int kAttackCount = 3;

    static const int kIdleFrames = 4;
    // number of levitate/idle frames / 待機・浮遊アニメーションのフレーム数

    static const int kIdleFrameSpeed = 10;
    // game frames per idle frame / 待機アニメーションの1コマを表示するフレーム数

    static const int kAttackFrames = 6;
    // number of "raise hand" frames / 手を上げる攻撃アニメーションのフレーム数

    static const int kAttackFrameSpeed = 8;
    // game frames per attack frame / 攻撃アニメーションの1コマを表示するフレーム数

    static const int kWarningFrames = 45;
    // warning delay before every attack / 各攻撃が始まる前の予告時間

    static const int kRestFrames = 60;
    // pause between attacks / 攻撃と攻撃の間の待機時間

    void Bob();
    void StartNextAttack();
    void UpdateHazards();
    void UpdateAttackChairs();
    void UpdateAttackPaper();
    void UpdateAttackLaptops();
    void SpawnChair(float offX, float offY, int throwAt);
    void DrawHandGlow();

    BossState state_ = BossState::Intro;

    Vector2 pos_{ 960.0f, 1500.0f };
    // starts below the screen / 画面の下から出現する

    Vector2 target_{ 960.0f, 900.0f };
    // player position / プレイヤーの座標

    float w_ = 300.0f, h_ = 300.0f;
    // boss size (match your sprite) / ボスのサイズ（スプライトに合わせる）

    int hp_ = 1000, maxHp_ = 1000;

    int timer_ = 0;
    // frames in the current state / 現在の状態が続いているフレーム数

    int frameCount_ = 0;
    // always counting (idle animation / bobbing) / 常にカウントする（待機アニメーション・上下移動用）

    int attack_ = -1;
    // 0 = chairs, 1 = paper shower, 2 = laptop stampede
    // 0 = イス、1 = 紙吹雪、2 = ノートパソコンの突進

    float moveTargetX_ = 960.0f;
    float handGlow_ = 0.0f;

    int spawnCount_ = 0;
    int nextSpawn_ = 0;

    float hoverY_ = 300.0f;
    // how high he levitates / ボスが浮遊する高さ

    float groundY_ = 1050.0f;
    // floor Y (laptops run along it) - set to your floor
    // 床のY座標（ノートパソコンが走る位置）。ゲームの床に合わせて設定する。

    float attackX_[kAttackCount];
    int attackDuration_[kAttackCount];
    float laneX_[3];

    std::vector<Hazard> hazards_;

    // ---- resources (-1 = not loaded yet, a colored box is drawn instead) ----
    // ---- リソース（-1 = 未読み込み。代わりに色付きの四角形を描画する）----

    int texIdle_[kIdleFrames];
    int texAttack_[kAttackFrames];
    int texChair_ = -1;
    int texPaper_ = -1;
    int texLaptop_ = -1;
};
