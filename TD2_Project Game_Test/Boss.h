#pragma once
#include <vector>

struct Vector2 { float x, y; };   // delete this line if you already have your own Vector2

enum class BossState { Intro, Idle, Move, Warning, Attack, Dead };
enum class HazardKind { Chair, Paper, Laptop };

// One struct for everything the boss throws, so player collision is a single loop.
struct Hazard {
    HazardKind kind;
    Vector2 pos;        // CENTER of the object
    Vector2 vel;
    float w, h;         // hit box size
    float angle;
    float spin;
    bool alive;
    int timer;          // frames since spawn

    // chair only
    bool launched;      // false while it floats beside the boss
    int throwAt;        // frame (since spawn) when it gets thrown
    Vector2 offset;     // where it floats, relative to the boss
};

class Boss {
public:
    Boss();

    void LoadResources();                 // call ONCE at startup
    void Update();
    void Draw();

    void SetTarget(Vector2 playerPos) { target_ = playerPos; }
    void TakeDamage(int dmg);             // call when a beer can hits him

    bool IsDead() const { return state_ == BossState::Dead; }
    int  GetHp() const { return hp_; }
    Vector2 GetPos() const { return pos_; }
    float GetWidth() const { return w_; }
    float GetHeight() const { return h_; }
    const std::vector<Hazard>& GetHazards() const { return hazards_; }

private:
    // ---- tuning / animation sizes (change these when your sprites are ready) ----
    static const int kAttackCount = 3;
    static const int kIdleFrames = 4;        // number of levitate/idle frames
    static const int kIdleFrameSpeed = 10;   // game frames per idle frame
    static const int kAttackFrames = 6;      // number of "raise hand" frames
    static const int kAttackFrameSpeed = 8;  // game frames per attack frame
    static const int kWarningFrames = 45;    // warning delay before every attack
    static const int kRestFrames = 60;       // pause between attacks

    void Bob();
    void StartNextAttack();
    void UpdateHazards();
    void UpdateAttackChairs();
    void UpdateAttackPaper();
    void UpdateAttackLaptops();
    void SpawnChair(float offX, float offY, int throwAt);
    void DrawHandGlow();

    BossState state_ = BossState::Intro;
    Vector2 pos_{ 960.0f, 1500.0f };         // starts below the screen
    Vector2 target_{ 960.0f, 900.0f };       // player position
    float w_ = 300.0f, h_ = 300.0f;          // boss size (match your sprite)
    int hp_ = 1000, maxHp_ = 1000;

    int timer_ = 0;          // frames in the current state
    int frameCount_ = 0;     // always counting (idle animation / bobbing)
    int attack_ = -1;        // 0 = chairs, 1 = paper shower, 2 = laptop stampede
    float moveTargetX_ = 960.0f;
    float handGlow_ = 0.0f;
    int spawnCount_ = 0;
    int nextSpawn_ = 0;

    float hoverY_ = 300.0f;  // how high he levitates
    float groundY_ = 1050.0f;// floor Y (laptops run along it) - set to your floor
    float attackX_[kAttackCount];
    int attackDuration_[kAttackCount];
    float laneX_[3];

    std::vector<Hazard> hazards_;

    // ---- resources (-1 = not loaded yet, a colored box is drawn instead) ----
    int texIdle_[kIdleFrames];
    int texAttack_[kAttackFrames];
    int texChair_ = -1;
    int texPaper_ = -1;
    int texLaptop_ = -1;
};
