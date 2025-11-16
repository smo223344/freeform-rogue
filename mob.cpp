#include "mob.h"
#include "combat.h"

// ============================================================================
// MOB Base Class
// ============================================================================

MOB::MOB(Entity* ent, int max_health, double ap_accumulation)
    : entity(ent), hp(max_health), max_hp(max_health),
      ap_rate(ap_accumulation), action_points(0.0) {
}

void MOB::accumulate_ap() {
    action_points += ap_rate;
}

bool MOB::can_act(double cost) const {
    return action_points >= cost;
}

void MOB::spend_ap(double cost) {
    action_points -= cost;
    if (action_points < 0.0) {
        action_points = 0.0;
    }
}

void MOB::take_damage(int damage) {
    hp -= damage;
    if (hp < 0) {
        hp = 0;
    }
}

bool MOB::is_alive() const {
    return hp > 0;
}

// Combat hooks - default implementations (can be overridden)
void MOB::modify_attack_stats(CombatStats& stats) const {
    // Default: no modifications
    (void)stats;  // Suppress unused parameter warning
}

void MOB::modify_defense_stats(CombatStats& stats) const {
    // Default: no modifications
    (void)stats;
}

void MOB::on_attack(MOB* target, CombatResult& result) {
    // Default: no special effects
    (void)target;
    (void)result;
}

void MOB::on_defend(MOB* attacker, const CombatResult& result) {
    // Default: no special effects
    (void)attacker;
    (void)result;
}

void MOB::on_kill(MOB* target) {
    // Default: no special effects
    (void)target;
}

void MOB::on_death(MOB* killer) {
    // Default: no special effects
    (void)killer;
}

// ============================================================================
// Player Class
// ============================================================================

Player::Player(Entity* ent)
    : MOB(ent, 100, 100.0) {  // 100 HP, 100 AP per turn (one action)
}

void Player::take_turn() {
    // Player turn is handled by input system in main game loop
    // This is called to accumulate action points
    accumulate_ap();
}

// ============================================================================
// Enemy Class
// ============================================================================

Enemy::Enemy(Entity* ent, int max_health, double ap_accumulation)
    : MOB(ent, max_health, ap_accumulation) {
}

void Enemy::take_turn() {
    accumulate_ap();

    // Simple AI: spend action points if we have enough
    // For now, enemies just idle
    // TODO: Implement pathfinding and attacking
    if (can_act(MOVE_COST)) {
        // AI would move here
        spend_ap(MOVE_COST);
    }
}
