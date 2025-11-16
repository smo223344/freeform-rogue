#include "combat.h"
#include "mob.h"
#include <cstdlib>
#include <algorithm>

// ============================================================================
// Combat System Implementation
// ============================================================================

CombatResult Combat::resolve_attack(MOB* attacker, MOB* defender) {
    CombatResult result;

    if (!attacker || !defender) {
        return result;  // Invalid combatants
    }

    // Get combat statistics from both MOBs
    CombatStats attack_stats = get_attack_stats(attacker);
    CombatStats defense_stats = get_defense_stats(defender);

    // Phase 1: Roll to hit
    result.hit = roll_to_hit(attack_stats.hit_chance);
    if (!result.hit) {
        return result;  // Miss - no damage
    }

    // Phase 2: Roll for critical hit
    result.critical = roll_critical(attack_stats.crit_chance);

    // Phase 3: Calculate damage
    result.damage_dealt = calculate_damage(attack_stats, defense_stats, result.critical);

    // Phase 4: Apply damage
    apply_damage(defender, result.damage_dealt);
    result.target_killed = !defender->is_alive();

    // Phase 5: Trigger combat hooks for special effects
    attacker->on_attack(defender, result);
    defender->on_defend(attacker, result);

    // Phase 6: Handle death
    if (result.target_killed) {
        attacker->on_kill(defender);
        defender->on_death(attacker);
    }

    return result;
}

CombatStats Combat::get_attack_stats(MOB* attacker) {
    CombatStats stats;

    // Base stats
    stats.base_damage = 10;
    stats.hit_chance = 0.85;   // 85% base hit chance
    stats.crit_chance = 0.10;   // 10% base crit chance
    stats.crit_multiplier = 2.0;

    // Allow MOB to modify stats (items, skills, buffs)
    attacker->modify_attack_stats(stats);

    return stats;
}

CombatStats Combat::get_defense_stats(MOB* defender) {
    CombatStats stats;

    // Base defense stats
    stats.armor = 0;  // No armor by default

    // Allow MOB to modify stats (armor, dodge, buffs)
    defender->modify_defense_stats(stats);

    return stats;
}

bool Combat::roll_to_hit(double hit_chance) {
    // Simple random roll between 0.0 and 1.0
    double roll = static_cast<double>(rand()) / RAND_MAX;
    return roll < hit_chance;
}

bool Combat::roll_critical(double crit_chance) {
    double roll = static_cast<double>(rand()) / RAND_MAX;
    return roll < crit_chance;
}

int Combat::calculate_damage(const CombatStats& attack_stats,
                               const CombatStats& defense_stats,
                               bool is_critical) {
    int damage = attack_stats.base_damage;

    // Apply critical multiplier
    if (is_critical) {
        damage = static_cast<int>(damage * attack_stats.crit_multiplier);
    }

    // Apply armor reduction
    damage -= defense_stats.armor;

    // Minimum damage of 1
    damage = std::max(1, damage);

    // TODO: Add damage variance (e.g., ±20% randomness)
    // TODO: Add elemental damage types
    // TODO: Add damage type resistances

    return damage;
}

void Combat::apply_damage(MOB* defender, int damage) {
    defender->take_damage(damage);
}
