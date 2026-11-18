#pragma once
#include "Boss.h"
#include "Player.h"

// ============================================================
//  Collision between the player and the boss attacks
//  プレイヤーとボスの攻撃の当たり判定
// ============================================================

// A box described by its CENTER and its size.
// 中心座標とサイズで表す四角形。
struct Box {
	float x, y;   // center / 中心
	float w, h;   // size / 大きさ
};

// True if the two boxes overlap. / 2つの四角形が重なっていればtrue
bool IsBoxHit(const Box& a, const Box& b);

// Hit boxes / 当たり判定の四角形
Box GetPlayerBox(const Player& player);
Box GetHazardBox(const Hazard& hazard);

// Damage of one attack. Phase 2 attacks hurt more.
// 攻撃1回分のダメージ。フェーズ2の攻撃は痛くなる。
int GetHazardDamage(const Hazard& hazard, int bossPhase);

// Check EVERY boss attack against the player. Call once per frame.
// ボスの全ての攻撃とプレイヤーの当たり判定を調べる。毎フレーム1回呼ぶ。
// Returns the damage dealt this frame (0 = no hit).
// このフレームに与えたダメージを返す（0 = 当たっていない）。
int CheckBossAttackHits(Boss& boss, Player& player);