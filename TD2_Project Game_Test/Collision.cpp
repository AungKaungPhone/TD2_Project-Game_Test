#include "Collision.h"
#include <math.h>

// ============================================================
//  Tuning values / 調整用の数値
// ============================================================
namespace {

	// Damage of each attack in phase 1 (the player has 100 HP).
	// フェーズ1での各攻撃のダメージ（プレイヤーのHPは100）。
	const int kDamageChair = 15;      // Attack 1 / 攻撃1: イス
	const int kDamagePaper = 5;       // Attack 2 / 攻撃2: 紙（数が多いので小さめ）
	const int kDamageLaptop = 12;     // Attack 3 / 攻撃3: ノートパソコン
	const int kDamagePen = 8;         // Attack 4 / 攻撃4: ペン
	const int kDamageEnvelope = 8;    // Attack 5 / 攻撃5: 封筒
	const int kDamageBeam = 25;       // Attack 6 / 攻撃6: YOU'RE FIRED ビーム
	const int kDamageBriefcase = 18;  // Attack 7 / 攻撃7: ブリーフケース

	// Phase 2 attacks deal 3/2 = 1.5x damage.
	// フェーズ2の攻撃は 3/2 = 1.5倍のダメージ。
	const int kPhase2DamageMul = 3;
	const int kPhase2DamageDiv = 2;

	// The player's hit box is a bit smaller than the sprite, so near misses feel fair.
	// プレイヤーの当たり判定は絵より少し小さくして、ギリギリの回避を公平にする。
	const float kPlayerHitScale = 0.8f;

	// Attacks that disappear when they hit the player.
	// Laptops and the beam keep going (the player is invincible for a moment after a hit).
	// プレイヤーに当たると消える攻撃。
	// ノートパソコンとビームは消えない（被弾後しばらくプレイヤーは無敵になる）。
	bool DisappearsOnHit(HazardKind kind) {
		switch (kind) {
		case HazardKind::Chair:
		case HazardKind::Paper:
		case HazardKind::Pen:
		case HazardKind::Envelope:
		case HazardKind::Briefcase:
			return true;
		case HazardKind::Laptop:
		case HazardKind::Beam:
			return false;
		}
		return false;
	}

} // namespace

// ============================================================
//  Box vs box / 四角形どうしの判定
// ============================================================
bool IsBoxHit(const Box& a, const Box& b) {
	// Boxes overlap when the distance between the centers is smaller than the sum of the half sizes.
	// 中心どうしの距離が、半分のサイズの合計より小さければ重なっている。
	bool overlapX = fabsf(a.x - b.x) < (a.w + b.w) * 0.5f;
	bool overlapY = fabsf(a.y - b.y) < (a.h + b.h) * 0.5f;
	return overlapX && overlapY;
}

Box GetPlayerBox(const Player& player) {
	return { player.GetX(), player.GetY(),
		player.GetWidth() * kPlayerHitScale, player.GetHeight() * kPlayerHitScale };
}

Box GetHazardBox(const Hazard& hazard) {
	return { hazard.pos.x, hazard.pos.y, hazard.w, hazard.h };
}

// ============================================================
//  Damage / ダメージ
// ============================================================
int GetHazardDamage(const Hazard& hazard, int bossPhase) {
	int damage = 0;

	switch (hazard.kind) {
	case HazardKind::Chair:     damage = kDamageChair;     break;
	case HazardKind::Paper:     damage = kDamagePaper;     break;
	case HazardKind::Laptop:    damage = kDamageLaptop;    break;
	case HazardKind::Pen:       damage = kDamagePen;       break;
	case HazardKind::Envelope:  damage = kDamageEnvelope;  break;
	case HazardKind::Beam:      damage = kDamageBeam;      break;
	case HazardKind::Briefcase: damage = kDamageBriefcase; break;
	}

	// Phase 2: stronger attacks. / フェーズ2: 攻撃が強くなる。
	if (bossPhase == 2) {
		damage = damage * kPhase2DamageMul / kPhase2DamageDiv;
	}
	return damage;
}

// ============================================================
//  Player vs every boss attack / プレイヤー vs ボスの全ての攻撃
// ============================================================
int CheckBossAttackHits(Boss& boss, Player& player) {
	// No damage while the player is dead or invincible.
	// プレイヤーが倒れている間、または無敵の間はダメージなし。
	if (player.IsDead() || player.IsInvincible()) return 0;

	Box playerBox = GetPlayerBox(player);

	for (Hazard& hazard : boss.GetHazards()) {
		if (!hazard.alive) continue;
		if (!IsBoxHit(playerBox, GetHazardBox(hazard))) continue;

		int damage = GetHazardDamage(hazard, boss.GetPhase());
		player.TakeDamage(damage);

		if (DisappearsOnHit(hazard.kind)) {
			hazard.alive = false;   // removed on the next Boss::Update / 次のBoss::Updateで消える
		}

		// One hit per frame is enough: the player is invincible from now on.
		// 1フレームに1回当たれば十分。ここからプレイヤーは無敵になる。
		return damage;
	}
	return 0;
}